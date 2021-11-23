
(define main (dl-open #f RTLD_NOW))

(define %atoi (dl-symbol main "atoi"))

(define info (dl-address %atoi))

