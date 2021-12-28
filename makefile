all: sdl

sdl: sdl.o
	clang++ --std=c++20 -o sdl sdl.o -L/opt/homebrew/Cellar/sdl2/2.0.18/lib -L/opt/homebrew/Cellar/sdl2_ttf/2.0.15/lib -lSDL2 -lSDL2_ttf

sdl.o: sdl.cpp
	clang++ --std=c++20 -c -I/opt/homebrew/Cellar/sdl2/2.0.18/include/SDL2 -I/opt/homebrew/Cellar/sdl2_ttf/2.0.15/include/SDL2 sdl.cpp

clean:
	rm sdl *.o
