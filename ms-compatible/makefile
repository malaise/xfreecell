CC=gcc

all: MSNumbers.h

MSNumbers.h: convert
	./convert

convert: convert.c
	@echo $(CC) -o $@ $<
	@$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f convert MSNumbers.h

