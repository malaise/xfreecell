OBJECTS = window.o plate.o string.o button.o label.o textfield.o container.o frame.o util.o scrollbar.o font.o main.o
INCLUDES = $(wildcard *.h)
CXX = g++

LIBS = -L/usr/X11R6/lib -lX11
LIBNAME = libwidget.a

all: $(LIBNAME)

$(LIBNAME): $(OBJECTS)
	ar r $(LIBNAME) $(OBJECTS)
	ranlib $(LIBNAME)

%.o: %.cpp $(INCLUDES)
	@echo $(CXX) -c $(DEBUG) $<
	@$(CXX) $(DEBUG) $(CFLAGS) $(CXXOPTS) -c $<

clean:
	rm -f *.o *~ a.out .nfs* dummy $(LIBNAME)

test: test.o $(OBJECTS) $(INCLUDES)
	$(CXX) $(CFLAGS) test.o $(OBJECTS) $(LIBS)

