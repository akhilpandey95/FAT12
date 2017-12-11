# Makefile for main.c
# Author: Akhil Pandey(https://akhilpandey95.com, https://github.com/akhilpandey95)

all: main.c
	gcc main.c -o fat12

clean:
	$(RM) fat12
