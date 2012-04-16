OBJECTS = card.o freecell.o option.o stack.o subwindows.o undo.o util.o gnmanager.o random.o
INCLUDES = $(wildcard *.h)
CC = g++
CFLAGS = -g -DSHAPE -DBOGUSRANDOM -pedantic -Wall -W -Wpointer-arith \
         -Wcast-qual -Wcast-align -Wwrite-strings -Wsign-compare \
         -Wmissing-noreturn -Winline -Wfloat-equal -Wundef -std=c++0x
LIBS = -L./widget -lns -L/usr/X11R6/lib -lXext -lX11 -lm
STATICDIR = xfreecell-static
DOCS = README CHANGES mshuffle.txt xfreecell.6
DESTDIR ?= /usr/local
all: xfreecell Xfreecell.html

xfreecell: $(OBJECTS) widget/libns.a
	$(CC) -o xfreecell $(CFLAGS) $(OBJECTS) $(LIBS)

%.html : %.txt
	@asciidoc --section-numbers -o $@ $<

%.o: %.cpp $(INCLUDES) ms-compatible/MSNumbers.h
	$(CC) -c $(CFLAGS) $<

widget/libns.a:
	make -C widget

ms-compatible/MSNumbers.h :
	make -C ms-compatible

static: MSNumbers $(OBJECTS) lib
	$(CC) -o xfreecell -static $(CFLAGS) $(OBJECTS) $(LIBS)

static-release: static
	mkdir $(STATICDIR)
	strip xfreecell
	mv xfreecell $(STATICDIR)
	cp $(DOCS) $(STATICDIR)
	tar czf xfreecell-bin.tgz $(STATICDIR)
	rm -rf $(STATICDIR)

clean:
	rm -f *~ *.o a.out xfreecell
	rm -rf $(STATICDIR)
	make -C widget clean
	make -C ms-compatible clean

install: all
	install -d $(DESTDIR)/bin $(DESTDIR)/man/man6
	install xfreecell -t $(DESTDIR)/bin
	install xfreecell.6 -t $(DESTDIR)/man/man6

