all : SimpleRead

#CC = arm-none-linux-gnueabi-g++
LIBDIR = lib-none-linux-gnueabi/

#CC = arm-linux-gnueabihf-g++-4.7
#LIBDIR = lib-linux-gnueabihf/

#CC = aarch64-linux-gnu-gcc
#LIBDIR = linaro-aarch64-linux-gnu/

CFLAGS = -L ../$(LIBDIR) -lMTF_API -lpthread -lm -W -Wall -lrt

SimpleRead :
	$(CC) -o SimpleRead SimpleRead.c $(CFLAGS)
	$(CC) -o SimpleRead-multi SimpleRead-multi.c $(CFLAGS)
	$(CC) -o SimpleRead-rgb SimpleRead-rgb.c $(CFLAGS)
	$(CC) -o SimpleRead-flip SimpleRead-flip.c $(CFLAGS)

clean :
	rm -rf SimpleRead
	rm -rf SimpleRead-multi
	rm -rf SimpleRead-rgb
	rm -rf SimpleRead-flip
