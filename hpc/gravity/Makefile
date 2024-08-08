CC = gcc 
CFLAGS = -O -ffast-math -funroll-loops -fopenmp
#CFLAGS += -g
AVXFLAGS = -mavx2 -mfma
LIB = -lm
GPULIB = -L$(CUDA_INSTALL_PATH)/lib64 -lcudart

CUDA_INSTALL_PATH = /usr/local/cuda
NVCC       = $(CUDA_INSTALL_PATH)/bin/nvcc 
NVCC_FLAGS = --compile -use_fast_math -O3

TARGET = check_cpu check_avx check_gpu

all: $(TARGET)

check_cpu : check.c gravity_cpu.c
	$(CC) $(CFLAGS) $^ -o $@ $(LIB) 

check_avx : check.c gravity_avx.c
	$(CC) $(CFLAGS) $(AVXFLAGS) $^ -o $@ $(LIB)

check_gpu : check.c gravity_gpu.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIB) $(GPULIB)

gravity_gpu.o : gravity_gpu.cu
	$(NVCC) $(NVCC_FLAGS) $^ -o $@

clean:
	-rm *.o $(TARGET) *~
