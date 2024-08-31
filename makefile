CC = gcc
CXX = g++

CXX11OPT := $(shell $(CXX) -dumpversion | awk -F "." ' \
  ( ($$1 > 4) || ( ($$1 == 4) && ($$2 >= 3))) {print "-std=c++0x"}')


OBJECTS = card.o option.o stack.o subwindows.o undo.o util.o gnmanager.o \
          random.o timeval.o
INCLUDES = $(wildcard *.h)
CFLAGS = -DSHAPE -DBOGUSRANDOM -pedantic -Wall -W -Wpointer-arith \
         -Wcast-qual -Wcast-align -Wwrite-strings -Wsign-compare \
         -Wmissing-noreturn -Winline -Wfloat-equal -Wundef
CXXOPTS = $(CXX11OPT) $(CFLAGS)

.SECONDARY : widget/libwidget.a

LIBS = -L./widget -lwidget -L/usr/X11R6/lib -lXext -lXpm -lX11 -lm
STATICDIR = xfreecell-static
DOCS = README CHANGES mshuffle.txt xfreecell.6 Xfreecell.html
DESTDIR ?= /usr/local

all: libwidget xfreecell Xfreecell.html

xfreecell: freecell.o $(OBJECTS) widget/libwidget.a
	@echo $(CXX) -o $@ freecell.o $(OBJECTS) $(LIBS) 
	@$(CXX) -o $@ freecell.o $(OBJECTS) $(LIBS)

debug:
	$(MAKE) DEBUG=-g xfreecell

%.html : %.txt
	@type asciidoc >/dev/null 2>&1;\
	if [ $$? -eq 0 ] ; then \
	  echo asciidoc --section-numbers -o $@ $< ;\
	  asciidoc --section-numbers -o $@ $< 2>&1 | grep -Ev "^<unknown>:1: SyntaxWarning: invalid escape sequence .*" & exit 0;\
	fi

%.o: %.cpp $(INCLUDES)
	@echo $(CXX) -c $(DEBUG) $<
	@$(CXX) -c $(DEBUG) $(CFLAGS) $(CXXOPTS) $<

%.o: %.c $(INCLUDES)
	@echo $(CC) -c $(DEBUG) $<
	@$(CC) -c $(DEBUG) $(CFLAGS) $(CCOPTS) $<

freecell.o: freecell.cpp $(INCLUDES) ms-compatible/MSNumbers.h xfreecell.xpm
	@echo $(CXX) -c $(DEBUG) $<
	@$(CXX) -c $(DEBUG) $(CFLAGS) $(CXXOPTS) $<

libwidget:
	@echo make -C widget
	@$(MAKE) -C widget CFLAGS="$(CFLAGS)" "CXXOPTS=$(CXXOPTS)"

ms-compatible/MSNumbers.h :
	@echo make -C ms-compatible
	@$(MAKE) -C ms-compatible CFLAGS="$(CFLAGS)" "CXXOPTS=$(CXXOPTS)"

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
	$(MAKE) -C widget clean
	$(MAKE) -C ms-compatible clean

install: all
	install -d $(DESTDIR)/bin $(DESTDIR)/man/man6
	install xfreecell -t $(DESTDIR)/bin
	install xfreecell.6 -t $(DESTDIR)/man/man6
	@if [ -f Xfreecell.html ] ; then \
	  install -d $(DESTDIR)/share/xfreecell; \
	  echo install Xfreecell.html -t $(DESTDIR)/share/xfreecell ;\
	  install Xfreecell.html -t $(DESTDIR)/share/xfreecell; \
	fi

