# Makefile to build both the TM and QB

build:
	make -C tm
	make -C qb

clean:
	make clean -C tm
	make clean -C qb