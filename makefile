all: sdl

sdl: sdl.o
	clang++ -o sdl sdl.o -L/opt/homebrew/Cellar/sdl2/2.0.18/lib -L/opt/homebrew/Cellar/sdl2_ttf/2.0.15/lib -lSDL2 -lSDL2_ttf

sdl.o: sdl.cpp
	clang++ -c -I/opt/homebrew/Cellar/sdl2/2.0.18/include/SDL2 -I/opt/homebrew/Cellar/sdl2_ttf/2.0.15/include/SDL2 sdl.cpp

clean:
	rm sdl *.o
