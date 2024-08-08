CC = g++
CFLAG = -I../ -I../glpng/include
LFLAG = -L../glpng/lib -lglpng -lglut -lGLU -lGL -lm
TARGET = radiosity

radiosity : main.cpp Application.cpp Scene.cpp
	$(CC) $(CFLAG) -o $@ $^ $(LFLAG)

clean :
	rm $(TARGET) *~

