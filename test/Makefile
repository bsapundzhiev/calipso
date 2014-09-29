#####################################################
#~calipso makefile
#####################################################

SRC    = gh.c 
TARGET = gh

all: $(SRC)
	gcc -g -O2 -Wall -o$(TARGET) $(SRC)

clean:
	rm -f *.o *.a *~ $(TARGET)

install:
	cp ./gh /usr/bin

format:
	astyle --style=stroustrup -s4 $(SRC)