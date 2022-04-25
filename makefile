all: stree

stree: stree.c
		gcc stree.c -o stree


clean: 
		rm -f stree