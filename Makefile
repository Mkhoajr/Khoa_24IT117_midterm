CC=cc

CFLAGS=-Wall -Wextra -Wpedantic -std=c11 -D_DEFAULT_SOURCE -Iinclude

TARGET=myls

SRCS=src/main.c src/options.c src/directory.c src/sorting.c src/display.c src/utils.c

OBJS=src/main.o src/options.o src/directory.o src/sorting.o src/display.o src/utils.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

src/main.o: src/main.c include/options.h include/directory.h include/display.h
	$(CC) $(CFLAGS) -c src/main.c -o src/main.o

src/options.o: src/options.c include/options.h
	$(CC) $(CFLAGS) -c src/options.c -o src/options.o

src/directory.o: src/directory.c include/directory.h include/display.h include/utils.h
	$(CC) $(CFLAGS) -c src/directory.c -o src/directory.o

src/sorting.o: src/sorting.c include/sorting.h include/options.h
	$(CC) $(CFLAGS) -c src/sorting.c -o src/sorting.o

src/display.o: src/display.c include/display.h include/utils.h
	$(CC) $(CFLAGS) -c src/display.c -o src/display.o

src/utils.o: src/utils.c include/utils.h
	$(CC) $(CFLAGS) -c src/utils.c -o src/utils.o

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean

