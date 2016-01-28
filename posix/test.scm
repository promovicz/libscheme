(define (directory path)
  (let ((dir (posix-opendir path)))
    (let loop ((file (posix-readdir dir)))
      (if file
	  (cons file (loop (posix-readdir dir)))
	  '()))))

(write (directory "."))
(newline)
