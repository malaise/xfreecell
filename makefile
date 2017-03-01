CC = gcc
CXX = g++

CXX11OPT := $(shell $(CXX) -dumpversion | awk -F "." ' \
  ( ($$1 > 4) || ( ($$1 == 4) && ($$2 >= 3))) {print "-std=c++0x"}')


OBJECTS = card.o option.o stack.o subwindows.o undo.o util.o gnmanager.o \
          random.o timeval.o
INCLUDES = $(wildcard *.h)
CFLAGS = -g -DSHAPE -DBOGUSRANDOM -pedantic -Wall -W -Wpointer-arith \
         -Wcast-qual -Wcast-align -Wwrite-strings -Wsign-compare \
         -Wmissing-noreturn -Winline -Wfloat-equal -Wundef
CXXOPTS = $(CXX11OPT) $(CFLAGS)


LIBS = -L./widget -lwidget -L/usr/X11R6/lib -lXext -lXpm -lX11 -lm
STATICDIR = xfreecell-static
DOCS = README CHANGES mshuffle.txt xfreecell.6 Xfreecell.html
DESTDIR ?= /usr/local

all: xfreecell Xfreecell.html

xfreecell: widget/libwidget.a $(OBJECTS) freecell.o
	@echo $(CXX) -o $@ freecell.o $(OBJECTS) $(LIBS)
	@$(CXX) -o $@ freecell.o $(OBJECTS) $(LIBS)

%.html : %.txt
	@type asciidoc >/dev/null 2>&1;\
	if [ $$? -eq 0 ] ; then \
	  echo asciidoc --section-numbers -o $@ $< ;\
	  asciidoc --section-numbers -o $@ $< ;\
	fi

%.o: %.cpp $(INCLUDES)
	@echo $(CXX) -c $<
	@$(CXX) -c $(CFLAGS) $(CXXOPTS) $<

%.o: %.c $(INCLUDES)
	@echo $(CC) -c $<
	@$(CC) -c $(CFLAGS) $(CCOPTS) $<

freecell.o: freecell.cpp $(INCLUDES) ms-compatible/MSNumbers.h xfreecell.xpm
	@echo $(CXX) -c $<
	@$(CXX) -c $(CFLAGS) $(CXXOPTS) $<

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
	rm -f *.o xfreecell Xfreecell.html
	rm -rf $(STATICDIR)
	make -C widget clean
	make -C ms-compatible clean

install: all
	install -d $(DESTDIR)/bin $(DESTDIR)/man/man6
	install xfreecell -t $(DESTDIR)/bin
	install xfreecell.6 -t $(DESTDIR)/man/man6
	@if [ -f Xfreecell.html ] ; then \
	  echo install -d $(DESTDIR)/share/xfreecell ;\
	  install -d $(DESTDIR)/share/xfreecell; \
	  echo install Xfreecell.html -t $(DESTDIR)/share/xfreecell ;\
	  install Xfreecell.html -t $(DESTDIR)/share/xfreecell; \
	fi

