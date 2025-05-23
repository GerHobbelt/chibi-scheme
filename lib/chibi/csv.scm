
;;> \section{CSV Grammars}

;;> CSV is a simple and compact format for tabular data, which has
;;> made it popular for a variety of tasks since the early days of
;;> computing.  Unfortunately, there are many incompatible dialects
;;> requiring a grammar to specify all of the different options.

(define-record-type Csv-Grammar
  (make-csv-grammar separator-chars quote-char quote-doubling-escapes? escape-char record-separator comment-chars quote-non-numeric?)
  csv-grammar?
  (separator-chars csv-grammar-separator-chars csv-grammar-separator-chars-set!)
  (quote-char csv-grammar-quote-char csv-grammar-quote-char-set!)
  (quote-doubling-escapes? csv-grammar-quote-doubling-escapes? csv-grammar-quote-doubling-escapes?-set!)
  (escape-char csv-grammar-escape-char csv-grammar-escape-char-set!)
  (record-separator csv-grammar-record-separator csv-grammar-record-separator-set!)
  (comment-chars csv-grammar-comment-chars csv-grammar-comment-chars-set!)
  (quote-non-numeric? csv-grammar-quote-non-numeric? csv-grammar-quote-non-numeric?-set!))

;; TODO: Other options to consider:
;; - strip-leading/trailing-whitespace?
;; - newlines-in-quotes?

;;> Creates a new CSV grammar from the given spec, an alist of symbols
;;> to values.  The following options are supported:
;;>
;;> \itemlist[
;;> \item{\scheme{'separator-chars} - A non-empty list of characters used to delimit fields, by default \scheme{'(#\\,)} (comma-separated).}
;;> \item{\scheme{'quote-char} - A single character used to quote fields containing special characters, or \scheme{#f} to disable quoting, by default \scheme{#\\"} (a double-quote).}
;;> \item{\scheme{'quote-doubling-escapes?} - If true, two successive \scheme{quote-char}s within quotes are treated as a single escaped \scheme{quote-char} (default true).}
;;> \item{\scheme{'escape-char} - A single character used to escape characters within quoted fields, or \scheme{#f} to disable escapes, by default \scheme{#f} (no explicit escape, use quote doubling).}
;;> \item{\scheme{'record-separator} - A single character used to delimit the record (row), or one of the symbols \scheme{'cr}, \scheme{'crlf}, \scheme{'lf} or \scheme{'lax}.  These correspond to sequences of carriage return and line feed, or in the case of \scheme{'lax} any of the other three sequences.  Defaults to \scheme{'lax}.}
;;> \item{\scheme{'comment-chars} - A list of characters which if found at the start of a record indicate it is a comment, discarding all characters through to the next record-separator. Defaults to the empty list (no comments).}
;;> ]
;;>
;;> Example Gecos grammar:
;;>
;;> \example{
;;> (csv-grammar
;;>   '((separator-chars #\\:)
;;>     (quote-char . #f)))
;;> }
(define (csv-grammar spec)
  (let ((grammar (make-csv-grammar '(#\,) #\" #t #f 'lax '() #f)))
    (for-each
     (lambda (x)
       (case (car x)
         ((separator-chars delimiter)
          (csv-grammar-separator-chars-set! grammar (cdr x)))
         ((quote-char)
          (csv-grammar-quote-char-set! grammar (cdr x)))
         ((quote-doubling-escapes?)
          (csv-grammar-quote-doubling-escapes?-set! grammar (cdr x)))
         ((escape-char)
          (csv-grammar-escape-char-set! grammar (cdr x)))
         ((record-separator newline-type)
          (let ((rec-sep
                 (case (cdr x)
                   ((crlf lax) (cdr x))
                   ((cr) #\return)
                   ((lf) #\newline)
                   (else
                    (if (char? (cdr x))
                        (cdr x)
                        (error "invalid record-separator, expected a char or one of 'lax or 'crlf" (cdr x)))))))
            (csv-grammar-escape-char-set! grammar (cdr x))))
         ((comment-chars)
          (csv-grammar-comment-chars-set! grammar (cdr x)))
         ((quote-non-numeric?)
          (csv-grammar-quote-non-numeric?-set! grammar (cdr x)))
         (else
          (error "unknown csv-grammar spec" x))))
     spec)
    grammar))

;;> The default CSV grammar for convenience, with all of the defaults
;;> from \scheme{csv-grammar}, i.e. comma-delimited with \scheme{#\"}
;;> for quoting, doubled to escape.
(define default-csv-grammar
  (csv-grammar '()))

;;> The default TSV grammar for convenience, splitting fields only on
;;> tabs, with no quoting or escaping.
(define default-tsv-grammar
  (csv-grammar '((separator-chars #\tab) (quote-char . #f))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;> \section{CSV Parsers}

;;> Parsers are low-level utilities to perform operations on records a
;;> field at a time.  You generally want to work with readers, which
;;> build on this to build records into familiar data structures.

;;> Parsers follow the rules of a grammar to parse a single CSV
;;> record, possible comprised of multiple fields.  A parser is a
;;> procedure of three arguments which performs a fold operation over
;;> the fields of the record. The parser signature is:
;;> \scheme{(parser kons knil in)}, where \scheme{kons} itself is
;;> a procedure of three arguments: \scheme{(proc acc index field)}.
;;> \scheme{proc} is called on each field of the record, in order,
;;> along with its zero-based \scheme{index} and the accumulated
;;> result of the last call, starting with \scheme{knil}.

;;> Returns a new CSV parser for the given \var{grammar}.  The parser
;;> by itself can be used to parse a record at a time.
;;>
;;> \example{
;;> (let ((parse (csv-parser)))
;;>   (parse (lambda (vec i field) (vector-set! vec i (string->number field)) vec)
;;>          (make-vector 3)
;;>          (open-input-string "1,2,3")))
;;> }
(define csv-parser
  (opt-lambda ((grammar default-csv-grammar))
    (lambda (kons knil in)
      (when (pair? (csv-grammar-comment-chars grammar))
        (let lp ()
          (when (memv (peek-char in) (csv-grammar-comment-chars grammar))
            (csv-skip-line in grammar)
            (lp))))
      (let lp ((acc knil)
               (index 0)
               (quoted? #f)
               (out (open-output-string)))
        (define (get-field)
          (let ((field (get-output-string out)))
            (cond
             ((and (zero? index) (equal? field "")) field)
             ((and (csv-grammar-quote-non-numeric? grammar) (not quoted?))
              (or (string->number field)
                  (error "unquoted field is not numeric" field)))
             (else field))))
        (define (finish-row)
          (let ((field (get-field)))
            (if (and (zero? index) (equal? field ""))
                ;; empty row, read again
                (lp acc index #f out)
                (kons acc index field))))
        (let ((ch (read-char in)))
          (cond
           ((eof-object? ch)
            (let ((field (get-field)))
              (if (and (zero? index) (equal? field ""))
                  ;; no data
                  ch
                  (kons acc index field))))
           ((memv ch (csv-grammar-separator-chars grammar))
            (lp (kons acc index (get-field))
                (+ index 1)
                #f
                (open-output-string)))
           ((eqv? ch (csv-grammar-quote-char grammar))
            ;; TODO: Consider a strict mode to enforce no text
            ;; before/after the quoted text.
            (csv-read-quoted in out grammar)
            (lp acc index #t out))
           ((eqv? ch (csv-grammar-record-separator grammar))
            (finish-row))
           ((and (eqv? ch #\return)
                 (memq (csv-grammar-record-separator grammar) '(crlf lax)))
            (cond
             ((eqv? (peek-char in) #\newline)
              (read-char in)
              (finish-row))
             ((eq? (csv-grammar-record-separator grammar) 'lax)
              (finish-row))
             (else
              (write-char ch out)
              (lp acc (+ index 1) quoted? out))))
           ((and (eqv? ch #\newline)
                 (eq? (csv-grammar-record-separator grammar) 'lax))
            (finish-row))
           (else
            (write-char ch out)
            (lp acc index quoted? out))))))))

(define (csv-skip-line in grammar)
  (let lp ()
    (let ((ch (read-char in)))
      (cond
       ((eof-object? ch))
       ((eqv? ch (csv-grammar-record-separator grammar)))
       ((and (eqv? ch #\newline)
             (eq? (csv-grammar-record-separator grammar) 'lax)))
       ((and (eqv? ch #\return)
             (memq (csv-grammar-record-separator grammar) '(crlf lax)))
        (cond
         ((eqv? (peek-char in) #\newline) (read-char in))
         ((eq? (csv-grammar-record-separator grammar) 'lax))
         (else (lp))))
       (else (lp))))))

(define (csv-read-quoted in out grammar)
  (let lp ()
    (let ((ch (read-char in)))
      (cond
       ((eof-object? ch)
        (error "unterminated csv quote" (get-output-string out)))
       ((eqv? ch (csv-grammar-quote-char grammar))
        (when (and (csv-grammar-quote-doubling-escapes? grammar)
                   (eqv? ch (peek-char in)))
          (write-char (read-char in) out)
          (lp)))
       ((eqv? ch (csv-grammar-escape-char grammar))
        (write-char (read-char in) out)
        (lp))
       (else
        ;; TODO: Consider an option to disable newlines in quotes.
        (write-char ch out)
        (lp))))))

(define (csv-skip-quoted in grammar)
  (let lp ()
    (let ((ch (read-char in)))
      (cond
       ((eof-object? ch)
        (error "unterminated csv quote"))
       ((eqv? ch (csv-grammar-quote-char grammar))
        (when (and (csv-grammar-quote-doubling-escapes? grammar)
                   (eqv? ch (peek-char in)))
          (read-char in)
          (lp)))
       ((eqv? ch (csv-grammar-escape-char grammar))
        (read-char in)
        (lp))
       (else
        (lp))))))

;;> Returns the number of rows in the input.
(define csv-num-rows
  (opt-lambda ((grammar default-csv-grammar)
               (in (current-input-port)))
    (let lp ((num-rows 0) (start? #t))
      (let ((ch (read-char in)))
        (cond
         ((eof-object? ch) (if start? num-rows (+ num-rows 1)))
         ((eqv? ch (csv-grammar-quote-char grammar))
          (csv-skip-quoted in grammar)
          (lp num-rows #f))
         ((eqv? ch (csv-grammar-record-separator grammar))
          (lp (+ num-rows 1) #f))
         ((and (eqv? ch #\return)
               (memq (csv-grammar-record-separator grammar) '(crlf lax)))
          (cond
           ((eqv? (peek-char in) #\newline)
            (read-char in)
            (lp (+ num-rows 1) #t))
           ((eq? (csv-grammar-record-separator grammar) 'lax)
            (lp (+ num-rows 1) #t))
           (else
            (lp num-rows #f))))
         ((and (eqv? ch #\newline)
               (eq? (csv-grammar-record-separator grammar) 'lax))
          (lp (+ num-rows 1) #t))
         (else
          (lp num-rows #f)))))))

;;> \section{CSV Readers}

;;> A CSV reader reads a single record, returning some representation
;;> of it.  You can either loop manually with these or pass them to
;;> one of the high-level utilities to operate on a whole CSV file at
;;> a time.

;;> The simplest reader, simply returns the field string values in
;;> order as a list.
;;>
;;> \example{
;;> ((csv-read->list) (open-input-string "foo,bar,baz"))
;;> }
(define csv-read->list
  (opt-lambda ((parser (csv-parser)))
    (opt-lambda ((in (current-input-port)))
      (let ((res (parser (lambda (ls i field) (cons field ls)) '() in)))
        (if (pair? res)
            (reverse res)
            res)))))

;;> The equivalent of \scheme{csv-read->list} but returns a vector.
;;>
;;> \example{
;;> ((csv-read->vector) (open-input-string "foo,bar,baz"))
;;> }
(define csv-read->vector
  (opt-lambda ((parser (csv-parser)))
    (let ((reader (csv-read->list parser)))
      (opt-lambda ((in (current-input-port)))
        (let ((res (reader in)))
          (if (pair? res)
              (list->vector res)
              res))))))

;;> The same as \scheme{csv-read->vector} but requires the vector to
;;> be of a fixed size, and may be more efficient.
;;>
;;> \example{
;;> ((csv-read->fixed-vector 3) (open-input-string "foo,bar,baz"))
;;> }
(define csv-read->fixed-vector
  (opt-lambda (size (parser (csv-parser)))
    (opt-lambda ((in (current-input-port)))
      (let ((res (make-vector size)))
        (let ((len (parser (lambda (prev-i i field) (vector-set! res i field) i)
                           0
                           in)))
          (if (zero? len)
              (eof-object)
              res))))))

;;> Returns an SXML representation of the record, as a row with
;;> multiple named columns.
;;>
;;> \example{
;;> ((csv-read->sxml 'city '(name latitude longitude))
;;>  (open-input-string "Tokyo,35°41′23″N,139°41′32″E"))
;;> }
(define csv-read->sxml
  (opt-lambda ((row-name 'row)
               (column-names
                (lambda (i)
                  (string->symbol (string-append "col-" (number->string i)))))
               (parser (csv-parser)))
    (define (get-column-name i)
      (if (procedure? column-names)
          (column-names i)
          (list-ref column-names i)))
    (opt-lambda ((in (current-input-port)))
      (let ((res (parser (lambda (ls i field)
                           `((,(get-column-name i) ,field) ,@ls))
                         (list row-name)
                         in)))
        (if (pair? res)
            (reverse res)
            res)))))

;;> \section{CSV Utilities}

;;> A folding operation on records.  \var{proc} is called successively
;;> on each row and the accumulated result.
;;>
;;> \example{
;;> (csv-fold
;;>  (lambda (row acc) (cons (cadr (assq 'name (cdr row))) acc))
;;>  '()
;;>  (csv-read->sxml 'city '(name latitude longitude))
;;>  (open-input-string
;;>   "Tokyo,35°41′23″N,139°41′32″E
;;> Paris,48°51′24″N,2°21′03″E"))
;;> }
(define csv-fold
  (opt-lambda (proc
               knil
               (reader (csv-read->list))
               (in (current-input-port)))
    (let lp ((acc knil))
      (let ((row (reader in)))
        (cond
         ((eof-object? row) acc)
         (else (lp (proc row acc))))))))

;;> An iterator which simply calls \var{proc} on each record in the
;;> input in order.
;;>
;;> \example{
;;> (let ((count 0))
;;>   (csv-for-each
;;>    (lambda (row) (if (string->number (car row)) (set! count (+ 1 count))))
;;>    (csv-read->list)
;;>    (open-input-string
;;>      "1,foo\\n2,bar\\nthree,baz\\n4,qux"))
;;>    count)
;;> }
(define csv-for-each
  (opt-lambda (proc
               (reader (csv-read->list))
               (in (current-input-port)))
    (csv-fold (lambda (row acc) (proc row)) #f reader in)))

;;> Returns a list containing the result of calling \var{proc} on each
;;> element in the input.
;;>
;;> \example{
;;> (csv-map
;;>  (lambda (row) (string->symbol (cadr row)))
;;>  (csv-read->list)
;;>  (open-input-string
;;>    "1,foo\\n2,bar\\nthree,baz\\n4,qux"))
;;> }
(define csv-map
  (opt-lambda (proc
               (reader (csv-read->list))
               (in (current-input-port)))
    (reverse (csv-fold (lambda (row acc) (cons (proc row) acc)) '() reader in))))

;;> Returns a list of all of the read records in the input.
;;>
;;> \example{
;;> (csv->list
;;>  (csv-read->list)
;;>  (open-input-string
;;>    "1,foo\\n2,bar\\nthree,baz\\n4,qux"))
;;> }
(define csv->list
  (opt-lambda ((reader (csv-read->list))
               (in (current-input-port)))
    (csv-map (lambda (row) row) reader in)))

;;> Returns an SXML representation of the CSV.
;;>
;;> \example{
;;> ((csv->sxml 'city '(name latitude longitude))
;;>  (open-input-string
;;>   "Tokyo,35°41′23″N,139°41′32″E
;;> Paris,48°51′24″N,2°21′03″E"))
;;> }
(define csv->sxml
  (opt-lambda ((row-name 'row)
               (column-names
                (lambda (i)
                  (string->symbol (string-append "col-" (number->string i)))))
               (parser (csv-parser)))
    (opt-lambda ((in (current-input-port)))
      (cons '*TOP*
            (csv->list (csv-read->sxml row-name column-names parser) in)))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;> \section{CSV Writers}

(define (write->string obj)
  (let ((out (open-output-string)))
    (write obj out)
    (get-output-string out)))

(define (csv-grammar-char-needs-quoting? grammar ch)
  (or (eqv? ch (csv-grammar-quote-char grammar))
      (eqv? ch (csv-grammar-escape-char grammar))
      (memv ch (csv-grammar-separator-chars grammar))
      (eqv? ch (csv-grammar-record-separator grammar))
      (memv ch '(#\newline #\return))))

(define (csv-write-quoted obj out grammar)
  (let ((in (open-input-string (if (string? obj) obj (write->string obj)))))
    (write-char (csv-grammar-quote-char grammar) out)
    (let lp ()
      (let ((ch (read-char in)))
        (cond
         ((eof-object? ch))
         ((or (eqv? ch (csv-grammar-quote-char grammar))
              (eqv? ch (csv-grammar-escape-char grammar)))
          (cond
           ((and (csv-grammar-quote-doubling-escapes? grammar)
                 (eqv? ch (csv-grammar-quote-char grammar)))
            (write-char ch out))
           ((csv-grammar-escape-char grammar)
            => (lambda (esc) (write-char esc out)))
           (else (error "no quote defined for" ch grammar)))
          (write-char ch out)
          (lp))
         (else
          (write-char ch out)
          (lp)))))
    (write-char (csv-grammar-quote-char grammar) out)))

(define csv-writer
  (opt-lambda ((grammar default-csv-grammar))
    (opt-lambda (row (out (current-output-port)))
      (let lp ((ls row) (first? #t))
        (when (pair? ls)
          (unless first?
            (write-char (car (csv-grammar-separator-chars grammar)) out))
          (if (or (and (csv-grammar-quote-non-numeric? grammar)
                       (not (number? (car ls))))
                  (and (string? (car ls))
                       (string-any
                        (lambda (ch) (csv-grammar-char-needs-quoting? grammar ch))
                        (car ls)))
                  (and (not (string? (car ls)))
                       (not (number? (car ls)))
                       (not (symbol? (car ls)))))
              (csv-write-quoted (car ls) out grammar)
              (display (car ls) out))
          (lp (cdr ls) #f)))
      (write-string
       (case (csv-grammar-record-separator grammar)
         ((crlf) "\r\n")
         ((lf lax) "\n")
         ((cr) "\r")
         (else (string (csv-grammar-record-separator grammar))))
       out))))

(define csv-write
  (opt-lambda ((writer (csv-writer)))
    (opt-lambda (rows (out (current-output-port)))
      (for-each
       (lambda (row) (writer row out))
       rows))))
