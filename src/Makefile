all: fds132
	
fds132:
	gcc -O2 -Wall -o FDS132_matrix_display FDS132_matrix_display.c ; strip FDS132_matrix_display

install:
	cp FDS132_matrix_display /usr/local/bin/

clean:
	-rm -f FDS132_matrix_display *.core

