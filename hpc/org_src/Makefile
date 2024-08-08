CC = gcc
CFLAGS = -O -fPIC -fopenmp -mavx2 -mfma 
SOFLAGS = -shared -Wl,-soname,libblas.so.3
TARGET = blas

all:$(TARGET)

blas : blas.c libblas.so
	$(CC) $(CFLAGS) -o $@ $< -L. -lblas

libblas.so : libblas.o
	$(CC) $(SOFLAGS) -o $@ $<
	ln -sf libblas.so libblas.so.3

libblas.o : libblas.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm *.o *.so $(TARGET)



