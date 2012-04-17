CC = g++
CXX = $(CC)

CXX11OPT := $(shell $(CXX) -dumpversion | awk -F "." ' \
  ( ($$1 > 4) || ( ($$1 == 4) && ($$2 >= 3))) {print "-std=c++0x"}')


OBJECTS = card.o option.o stack.o subwindows.o undo.o util.o gnmanager.o random.o
INCLUDES = $(wildcard *.h)
CFLAGS = -g -DSHAPE -DBOGUSRANDOM -pedantic -Wall -W -Wpointer-arith \
         -Wcast-qual -Wcast-align -Wwrite-strings -Wsign-compare \
         -Wmissing-noreturn -Winline -Wfloat-equal -Wundef
CXXOPTS = $(CXX11OPT)


LIBS = -L./widget -lwidget -L/usr/X11R6/lib -lXext -lX11 -lm
STATICDIR = xfreecell-static
DOCS = README CHANGES mshuffle.txt xfreecell.6 Xfreecell.html
DESTDIR ?= /usr/local

all: xfreecell

xfreecell: widget/libwidget.a $(OBJECTS) freecell.o
	@echo $(CC) -o $@ freecell.o $(OBJECTS) $(LIBS)
	@$(CC) -o $@ freecell.o $(OBJECTS) $(LIBS)

doc: Xfreecell.html

%.html : %.txt
	@asciidoc --section-numbers -o $@ $<

%.o: %.cpp $(INCLUDES)
	@echo $(CC) -c $<
	@$(CC) -c $(CFLAGS) $(CXXOPTS) $<

freecell.o: freecell.cpp $(INCLUDES) ms-compatible/MSNumbers.h
	@echo $(CC) -c $<
	@$(CC) -c $(CFLAGS) $(CXXOPTS) $<

widget/libwidget.a:
	@echo make -C widget
	@make -C widget CFLAGS="$(CFLAGS)" "CXXOPTS=$(CXXOPTS)"

ms-compatible/MSNumbers.h :
	@echo make -C ms-compatible
	@make -C ms-compatible CFLAGS="$(CFLAGS)" "CXXOPTS=$(CXXOPTS)"

static-release: xfreecell doc
	mkdir $(STATICDIR)
	strip xfreecell
	mv xfreecell $(STATICDIR)
	cp $(DOCS) $(STATICDIR)
	tar czf xfreecell-bin.tgz $(STATICDIR)
	rm -rf $(STATICDIR)

clean:
	rm -f *.o xfreecell
	rm -rf $(STATICDIR)
	make -C widget clean
	make -C ms-compatible clean

install: all
	install -d $(DESTDIR)/bin $(DESTDIR)/man/man6
	install xfreecell -t $(DESTDIR)/bin
	install xfreecell.6 -t $(DESTDIR)/man/man6

