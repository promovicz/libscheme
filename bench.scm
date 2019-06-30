
(defmacro dotimes (ntimes . body)
  `(do ((%current 0 (+ 1 %current)))
       ((> %current ,ntimes))
     ,@body))

(defmacro dolist (list . body)
  `(do () ((list))
     (set! list (cdr list))
     (let (%current (car list))
       ,@body)))

(define x 0)

(define l '())

(dotimes 2000000
         (set! x %current)
         (set! l (cons %current l)))

(dolist l
        (set! x %current))

(exit)
