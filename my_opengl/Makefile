#CC = x86_64-w64-mingw32-gcc
CC = gcc
C++ = g++
CFLAGS = -I/usr/local/include -Iglpng/include
#CFLAGS += -g
#LFLAGS = -L/usr/local/lib -lglfw3 \
#	-lglu32 -lgdi32 -lopengl32
LFLAGS = -Lglpng/lib -lglpng -lglfw -lGLU -lGL -lm
#SOFLAGS = -shared -Wl,-soname,libmygl.so
#SOFLAGS = -fPIC -shared -Wl,-soname,libmygl.so
SOFLAGS = -fPIC -shared -ldl
PNGLIB = glpng/lib/libglpng.a
CUDACC = nvcc
NVCCFLAG = -use_fast_math -O --generate-code arch=compute_50,code=sm_50
#TARGET = anim stereo shadow sample4 sample3 sample2 sample1 zbuf triangle framebuf line
TARGET = stereo_anaglyph stereo_parallel shadow sample4 sample3 sample2 sample1 line framebuf triangle zbuf # blendshape stereo_anaglyph stereo_parallel shadow sample4 sample3 sample2 sample1 zbuf triangle framebuf line

ALL: $(TARGET)

blendshape : blendshape.c objread.c objread.h $(PNGLIB)
	$(CC) $(CFLAGS) -o $@ $< objread.c $(LFLAGS)
anim : anim.cu objread.c objread.h $(PNGLIB)
	$(CUDACC) $(NVCCFLAG) $(CFLAGS) -o $@ $< objread.c $(LFLAGS)
shadow : shadow.c objread.c objread.h $(PNGLIB)
	$(CC) $(CFLAGS) -o $@ $< objread.c $(LFLAGS)
stereo_anaglyph : stereo_anaglyph.c objread.c objread.h $(PNGLIB) make_movie.o
	$(CC) $(CFLAGS) -o $@ $< objread.c make_movie.o $(LFLAGS) -lopencv_highgui -lopencv_core -lopencv_imgproc -lstdc++
stereo_parallel : stereo_parallel.c objread.c objread.h $(PNGLIB) make_movie.o
	$(CC) $(CFLAGS) -o $@ $< objread.c make_movie.o $(LFLAGS) -lopencv_highgui -lopencv_core -lopencv_imgproc -lstdc++
make_movie.o : make_movie.c
	$(C++) -o $@ -c $<
sample4 : sample4.c mygl.so objread.c objread.h $(PNGLIB)
	$(CC) $(CFLAGS) -o $@ $< objread.c $(LFLAGS)
sample3 : sample3.c mygl.so objread.c objread.h $(PNGLIB)
	$(CC) $(CFLAGS) -o $@ $< objread.c $(LFLAGS)
$(PNGLIB) :
	cd glpng/src;make;cd - 
sample2 : sample2.c mygl.so $(PNGLIB)
	$(CC) $(CFLAGS) -o $@ $< $(LFLAGS)
yatagawa2 : yatagawa2.c mygl.so
	$(CC) $(CFLAGS) -o $@ $< $(LFLAGS)
sample1 : sample1.c mygl.so $(PNGLIB)
	$(CC) $(CFLAGS) -o $@ $< $(LFLAGS)
	
mygl.so : mygl.c myglfw.c
	$(CC) $(CFLAGS) $(SOFLAGS) -o $@ $<

zbuf_q : zbuf_q.c
	$(CC) $(CFLAGS) -o $@ $< $(LFLAGS) 
zbuf : zbuf.c
	$(CC) $(CFLAGS) -o $@ $< $(LFLAGS) 
triangle : triangle.c
	$(CC) $(CFLAGS) -o $@ $< $(LFLAGS) 
triangle_simple : triangle_simple.c
	$(CC) $(CFLAGS) -o $@ $< $(LFLAGS) 
framebuf : framebuf.c
	$(CC) $(CFLAGS) -o $@ $< $(LFLAGS) 
framebuf_mario : framebuf_mario.c
	$(CC) $(CFLAGS) -o $@ $< $(LFLAGS) 
line : line.c
	$(CC) $(CFLAGS) -o $@ $< $(LFLAGS) 

clean:
	rm $(TARGET) *.exe *.so *~ 
#	rm $(TARGET) *.exe *.so $(PNGLIB) *~;cd glpng/src;make clean;cd - 

dist:
	tar cvfz mygl.tar.gz *.c *.h Makefile Makefile.cygwin \
	obj/*.obj obj/*.mtl obj/*.png \
	glpng/ \
	radiosity/*.cpp radiosity/*.h radiosity/Makefile radiosity/Makefile.cygwin \
	radiosity/*.obj radiosity/*.mtl

