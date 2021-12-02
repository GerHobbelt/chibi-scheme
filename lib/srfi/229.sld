
(define-library (srfi 229)
  (import (scheme base) (scheme case-lambda) (chibi ast))
  (export case-lambda/tag lambda/tag procedure/tag? procedure-tag)
  (begin
    (define procedure-tag-object (list 'procedure-tag))
    (define (procedure->tagged f tag)
      (make-procedure (procedure-flags f)
                      (procedure-arity f)
                      (procedure-code f)
                      (vector-append (or (procedure-vars f) '#())
                                     (vector tag procedure-tag-object))))
    (define-syntax lambda/tag
      (syntax-rules ()
        ((lambda/tag tag-expr formals . body)
         (procedure->tagged (lambda formals . body) tag-expr))))
    (define-syntax case-lambda/tag
      (syntax-rules ()
        ((case-lambda/tag tag-expr . clauses)
         (procedure->tagged (case-lambda . clauses) tag-expr))))
    (define (procedure/tag? f)
      (and (procedure? f)
           (let ((vars (procedure-vars f)))
             (and (vector? vars)
                  (eq? procedure-tag-object
                       (vector-ref vars (- (vector-length vars) 1)))))))
    (define (procedure-tag f)
      (let ((vars (procedure-vars f)))
        (vector-ref vars (- (vector-length vars) 2))))))
