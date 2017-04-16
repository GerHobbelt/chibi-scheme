
(import (scheme base)
        (chibi test)
        (rename (srfi 1 test) (run-tests run-srfi-1-tests))
        (rename (srfi 2 test) (run-tests run-srfi-2-tests))
        (rename (srfi 16 test) (run-tests run-srfi-16-tests))
        (rename (srfi 18 test) (run-tests run-srfi-18-tests))
        (rename (srfi 26 test) (run-tests run-srfi-26-tests))
        (rename (srfi 27 test) (run-tests run-srfi-27-tests))
        (rename (srfi 38 test) (run-tests run-srfi-38-tests))
        (rename (srfi 69 test) (run-tests run-srfi-69-tests))
        (rename (srfi 95 test) (run-tests run-srfi-95-tests))
        (rename (srfi 99 test) (run-tests run-srfi-99-tests))
        (rename (srfi 117 test) (run-tests run-srfi-117-tests))
        (rename (srfi 121 test) (run-tests run-srfi-121-tests))
        (rename (srfi 128 test) (run-tests run-srfi-128-tests))
        (rename (srfi 129 test) (run-tests run-srfi-129-tests))
        (rename (srfi 130 test) (run-tests run-srfi-130-tests))
        (rename (srfi 132 test) (run-tests run-srfi-132-tests))
        (rename (srfi 133 test) (run-tests run-srfi-133-tests))
        (rename (srfi 142 test) (run-tests run-srfi-142-tests))
        (rename (chibi base64-test) (run-tests run-base64-tests))
        (rename (chibi crypto md5-test) (run-tests run-md5-tests))
        (rename (chibi crypto rsa-test) (run-tests run-rsa-tests))
        (rename (chibi crypto sha2-test) (run-tests run-sha2-tests))
        (rename (chibi doc-test) (run-tests run-doc-tests))
        ;;(rename (chibi filesystem-test) (run-tests run-filesystem-tests))
        (rename (chibi generic-test) (run-tests run-generic-tests))
        (rename (chibi io-test) (run-tests run-io-tests))
        (rename (chibi iset-test) (run-tests run-iset-tests))
        (rename (chibi loop-test) (run-tests run-loop-tests))
        (rename (chibi match-test) (run-tests run-match-tests))
        (rename (chibi math prime-test) (run-tests run-prime-tests))
        ;;(rename (chibi memoize-test) (run-tests run-memoize-tests))
        (rename (chibi mime-test) (run-tests run-mime-tests))
        (rename (chibi numeric-test) (run-tests run-numeric-tests))
        (rename (chibi parse-test) (run-tests run-parse-tests))
        (rename (chibi pathname-test) (run-tests run-pathname-tests))
        (rename (chibi process-test) (run-tests run-process-tests))
        (rename (chibi regexp-test) (run-tests run-regexp-tests))
        (rename (chibi scribble-test) (run-tests run-scribble-tests))
        (rename (chibi show-test) (run-tests run-show-tests))
        (rename (chibi string-test) (run-tests run-string-tests))
        (rename (chibi system-test) (run-tests run-system-tests))
        (rename (chibi tar-test) (run-tests run-tar-tests))
        ;;(rename (chibi term ansi-test) (run-tests run-term-ansi-tests))
        (rename (chibi uri-test) (run-tests run-uri-tests))
        ;;(rename (chibi weak-test) (run-tests run-weak-tests))
        )

(test-begin "libraries")

(run-srfi-1-tests)
(run-srfi-2-tests)
(run-srfi-16-tests)
(run-srfi-18-tests)
(run-srfi-26-tests)
(run-srfi-27-tests)
(run-srfi-38-tests)
(run-srfi-69-tests)
(run-srfi-95-tests)
(run-srfi-99-tests)
(run-srfi-117-tests)
(run-srfi-121-tests)
(run-srfi-128-tests)
(run-srfi-129-tests)
(run-srfi-130-tests)
(run-srfi-132-tests)
(run-srfi-133-tests)
(run-srfi-142-tests)
(run-base64-tests)
(run-doc-tests)
(run-generic-tests)
(run-io-tests)
(run-iset-tests)
(run-loop-tests)
(run-match-tests)
(run-md5-tests)
(run-mime-tests)
(run-numeric-tests)
(run-parse-tests)
(run-pathname-tests)
(run-prime-tests)
(run-process-tests)
(run-regexp-tests)
(run-rsa-tests)
(run-scribble-tests)
(run-string-tests)
(run-sha2-tests)
(run-show-tests)
(run-system-tests)
(run-tar-tests)
(run-uri-tests)

(test-end)
