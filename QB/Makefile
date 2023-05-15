C11 = cc -std=c11
#CFLAGS = -Wall -Werror

QB: QB.c pquestions.o questions.o mark.o mode.h questions.h pquestions.h
	cc QB.c -o QB pquestions.o questions.o mark.o

pquestions.o : pquestions.c mode.h
	$(C11) -c pquestions.c

questions.o : questions.c questions.h
	$(C11) -c questions.c

mark.o : mark.c questions.h
	$(C11) -c mark.c