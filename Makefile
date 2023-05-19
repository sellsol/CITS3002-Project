# Makefile to build both the TM and QB

build:
	make -C TM
	make -C QB

clean:
	make clean -C TM
	make clean -C QB