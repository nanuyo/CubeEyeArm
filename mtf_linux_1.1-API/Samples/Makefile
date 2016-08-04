all : SimpleRead

CC = g++

CFLAGS = -L ../lib -lMTF_API -lpthread -lm -W -Wall -lrt
STRIP = strip

SimpleRead :
	$(CC) -o SimpleRead SimpleRead.c $(CFLAGS) 
	$(CC) -o SimpleRead-multi SimpleRead-multi.c $(CFLAGS)
	$(CC) -o SimpleRead-rgb SimpleRead-rgb.c $(CFLAGS)
	$(CC) -o SimpleRead-flip SimpleRead-flip.c $(CFLAGS)
	$(CC) -o SimpleViewer SimpleViewer.c $(CFLAGS) -lglut -lGL
	$(CC) -o SimpleViewer-multi SimpleViewer-multi.c $(CFLAGS) -lglut -lGL
	$(CC) -o SimpleRecordViewer SimpleRecordViewer.c $(CFLAGS) -lglut -lGL

clean :
	rm -rf SimpleRead
	rm -rf SimpleRead-multi
	rm -rf SimpleRead-rgb
	rm -rf SimpleRead-flip
	rm -rf SimpleViewer
	rm -rf SimpleViewer-multi
	rm -rf SimpleRecordViewer
