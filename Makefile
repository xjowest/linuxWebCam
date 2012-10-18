CC=gcc 
CFLAGS= -o webcam -lSDL

all:	
	$(CC) webcam.c $(CFLAGS)

clean:
	rm -f webcam