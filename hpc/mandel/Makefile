CC = gcc
C++ = g++
CFLAGS = -O -fPIC -fopenmp -mavx2 -mfma
CFLAGS += -I.
SOFLAGS = -shared -Wl,-soname,libblas.so.3
TARGET = mandel # blas

all:$(TARGET)

mandel : mandel.c
	$(C++) $(CFLAGS) -o $@ $< -Llib -lglfw -lGL

blas : blas.c libblas.so
	$(CC) $(CFLAGS) -o $@ $< -L. -lblas

libblas.so : libblas.o
	$(CC) $(SOFLAGS) -o $@ $<
	ln -sf libblas.so libblas.so.3

libblas.o : libblas.c
	$(CC) $(CFLAGS) -c $<

mandel.zip :
	zip -r mandel Makefile mandel.c GLFW lib mandel_org mandel_org_avx

clean:
	rm *.o *.so $(TARGET)



