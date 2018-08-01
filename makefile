CC      = gcc
override LDFLAGS = -lm
override CFLAGS += -Wall -fwrapv -march=native

ifeq ($(OS),Windows_NT)
	override CFLAGS += -D_WIN32
else
	override LDFLAGS += -lX11 -pthread
endif

.PHONY : all debug clean

all: CFLAGS += -O3
all: find_quadhuts find_compactbiomes clean

debug: CFLAGS += -DDEBUG -O0 -g
debug: find_quadhuts find_compactbiomes clean

find_compactbiomes: find_compactbiomes.o layers.o generator.o finders.o
	$(CC) -o $@ $^ $(LDFLAGS)

find_compactbiomes.o: find_compactbiomes.c
	$(CC) -c $(CFLAGS) $<

find_quadhuts: find_quadhuts.o layers.o generator.o finders.o 
	$(CC) -o $@ $^ $(LDFLAGS)

find_quadhuts.o: find_quadhuts.c
	$(CC) -c $(CFLAGS) $<


xmapview.o: xmapview.c xmapview.h
	$(CC) -c $(CFLAGS) $<

finders.o: finders.c finders.h
	$(CC) -c $(CFLAGS) $<

generator.o: generator.c generator.h
	$(CC) -c $(CFLAGS) $<

layers.o: layers.c layers.h
	$(CC) -c $(CFLAGS) $<


clean:
	rm *.o

