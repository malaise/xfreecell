OBJECTS = window.o plate.o string.o button.o label.o textfield.o container.o frame.o util.o scrollbar.o font.o main.o
INCLUDES = $(wildcard *.h)
CC = g++
CFLAGS = -g -pedantic -Wall -W -Wpointer-arith \
         -Wcast-qual -Wcast-align -Wwrite-strings -Wsign-compare \
         -Wmissing-noreturn -Winline -Wfloat-equal -Wundef

LIBS = -L/usr/X11R6/lib -lX11
LIBNAME = libns.a

all: $(LIBNAME)

$(LIBNAME): $(OBJECTS)
	ar r $(LIBNAME) $(OBJECTS)
	ranlib $(LIBNAME)

%.o: %.cpp $(INCLUDES)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o *~ a.out .nfs* dummy $(LIBNAME)

tgz:
	tar cvzf widget.tgz *.cpp *.h makefile

test: test.o $(OBJECTS) $(INCLUDES)
	$(CC) $(CFLAGS) test.o $(OBJECTS) $(LIBS)

