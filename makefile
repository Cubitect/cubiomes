#CC      = gcc
#AR      = ar
ARFLAGS = cr
override LDFLAGS = -lm
override CFLAGS += -Wall -Wextra -fwrapv

ifeq ($(OS),Windows_NT)
	override CFLAGS += -D_WIN32
	CC = gcc
	RM = del
else
	override LDFLAGS += -pthread
	#RM = rm
endif

.PHONY : all debug release native libcubiomes clean

all: release

debug: CFLAGS += -DDEBUG -O0 -ggdb3
debug: libcubiomes
release: CFLAGS += -O3
release: libcubiomes
native: CFLAGS += -O3 -march=native -ffast-math
native: libcubiomes

ifneq ($(OS),Windows_NT)
release: CFLAGS += -fPIC
#debug: CFLAGS += -fsanitize=undefined
endif


libcubiomes: noise.o biomes.o layers.o biomenoise.o generator.o finders.o util.o quadbase.o
	$(AR) $(ARFLAGS) libcubiomes.a $^

finders.o: finders.c finders.h
	$(CC) -c $(CFLAGS) $<

generator.o: generator.c generator.h
	$(CC) -c $(CFLAGS) $<

biomenoise.o: biomenoise.c
	$(CC) -c $(CFLAGS) $<

biometree.o: biometree.c
	$(CC) -c $(CFLAGS) $<

layers.o: layers.c layers.h
	$(CC) -c $(CFLAGS) $<

biomes.o: biomes.c biomes.h
	$(CC) -c $(CFLAGS) $<

noise.o: noise.c noise.h
	$(CC) -c $(CFLAGS) $<

util.o: util.c util.h
	$(CC) -c $(CFLAGS) $<

quadbase.o: quadbase.c quadbase.h
	$(CC) -c $(CFLAGS) $<

clean:
	$(RM) *.o *.a

