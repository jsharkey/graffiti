CC=g++ -Wall -ansi
CFLAGS=-I. -I/usr/include/freetype2 -I./glpng/include/ -L./glpng/lib/
DEPS=
OBJ=laser.o 
LIBS=-lm -lstdc++ -lgd -lpng -lz -ljpeg -lfreetype -lGL -lGLU -lglpng -lfreetype `sdl-config --cflags --libs`

%.o: %.C $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

plot: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f *.o *~ core


