CC      = gcc
AR      = ar
ARFLAGS = cr
OCL_FLAGS = -I. -L. -lOpenCL
override LDFLAGS = -lm
override CFLAGS += -Wall -fwrapv

ifeq ($(OS),Windows_NT)
	override CFLAGS += -D_WIN32
	RM = del
else
	override LDFLAGS += -lX11 -pthread
	#RM = rm
endif

.PHONY : all release debug libcubiomes clean

all: release

debug: CFLAGS += -DDEBUG -O0 -ggdb3
debug: libcubiomes find_quadhuts find_compactbiomes
release: CFLAGS += -O3 -march=native
release: libcubiomes find_quadhuts find_compactbiomes

libcubiomes: CFLAGS += -fPIC
libcubiomes: layers.o generator.o finders.o util.o
	$(AR) $(ARFLAGS) libcubiomes.a $^

opencl: ocl_generator.o ocl_tests.o libcubiomes.a
	$(CC) $(CFLAGS) $(OCL_FLAGS) -o ocl_tests $^

find_compactbiomes: find_compactbiomes.o layers.o generator.o finders.o
	$(CC) -o $@ $^ $(LDFLAGS)

find_compactbiomes.o: find_compactbiomes.c
	$(CC) -c $(CFLAGS) $<

find_quadhuts: find_quadhuts.o layers.o generator.o finders.o 
	$(CC) -o $@ $^ $(LDFLAGS)

find_quadhuts.o: find_quadhuts.c
	$(CC) -c $(CFLAGS) $<

finders.o: finders.c finders.h
	$(CC) -c $(CFLAGS) $<

generator.o: generator.c generator.h
	$(CC) -c $(CFLAGS) $<

layers.o: layers.c layers.h
	$(CC) -c $(CFLAGS) $<

util.o: util.c util.h
	$(CC) -c $(CFLAGS) $<

ocl_generator.o: ocl_generator.c ocl_generator.h
	$(CC) -c $(FLAGS) $< $(OCL_FLAGS)

ocl_tests.o: ocl_tests.c ocl_generator.h
	$(CC) -c $(FLAGS) $< $(OCL_FLAGS)

clean:
	$(RM) *.o libcubiomes.a find_quadhuts find_compactbiomes

