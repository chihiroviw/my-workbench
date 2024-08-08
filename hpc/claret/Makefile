CC = gcc
C++ = g++
CFLAGS = -O -fopenmp
AVXFLAGS = -mavx2 -mfma 

CUDA_INSTALL_PATH = /usr/local/cuda
NVCC       = $(CUDA_INSTALL_PATH)/bin/nvcc 
NVCC_FLAGS = --compile -use_fast_math -O3

LIB = -lm -lstdc++
GPULIB = -L$(CUDA_INSTALL_PATH)/lib64 -lcudart
GLLIB = -lglut -lGLU -lGL

TARGET = check cras cras_sock

all: $(TARGET)

check : check.c mr3_cpu.o mr3_avx.o mr3_gpu.o
	$(CC) $(CFLAGS) $(AVXFLAGS) -o $@ $^ $(LIB) $(GPULIB)
mr3_cpu.o : mr3_cpu.c
	$(CC) $(CFLAGS) -o $@ -c $^
mr3_avx.o : mr3_avx.c
	$(C++) $(CFLAGS) $(AVXFLAGS) -o $@ -c $^
mr3_gpu.o : mr3_gpu.cu
	$(NVCC) $(NVCC_FLAGS) $^ -o $@

cras : cras36.c mr3_cpu.o mr3_avx.o mr3_gpu.o
	$(CC) -DVTGRAPE $(CFLAGS) $(AVXFLAGS) -o $@ $^ $(LIB) $(GPULIB) $(GLLIB)
cras_sock : cras36.c mr3_cpu.o mr3_avx.o mr3_gpu.o sockhelp.o
	$(CC) -DSOCK_ON -DVTGRAPE $(CFLAGS) $(AVXFLAGS) -o $@ $^ $(LIB) $(GPULIB) $(GLLIB)
sockhelp.o : sockhelp.c
	$(CC) $(CFLAGS) -o $@ -c $^

exptest : exptest.c
	$(C++) $(CFLAGS) $(AVXFLAGS) -o $@ $^ 

clean:
	rm *.o $(TARGET)
