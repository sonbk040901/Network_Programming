libj.a: file.o
	gcc jval.o jrb.o dllist.o queue.o fields.o user.o -shared -o ../lib/libj.a
file.o:
	gcc -fPIC -c jval.c jrb.c dllist.c fields.c queue.c user.c
example: gcc -shared -o libhello.so -fPIC hello.c