# Makefile for the QB
C11 = cc

QB: QB.c pquestions.o questions.o mark.o mode.h questions.h pquestions.h
	$(C11) QB.c -o QB pquestions.o questions.o mark.o

pquestions.o : pquestions.c pquestions.h mode.h
	$(C11) -c pquestions.c

questions.o : questions.c questions.h
	$(C11) -c questions.c

mark.o : mark.c mark.h questions.h
	$(C11) -c mark.c

clean:
	rm -f QB pquestions.o questions.o mark.o