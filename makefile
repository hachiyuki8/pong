all: sdl

sdl: sdl.o
	clang++ -o sdl sdl.o -L/opt/homebrew/Cellar/sdl2/2.0.18/lib -L/opt/homebrew/Cellar/sdl2_image/2.0.5/lib -L/opt/homebrew/Cellar/sdl2_ttf/2.0.15/lib -lSDL2 -lSDL2_image -lSDL2_ttf

sdl.o: sdl.cpp
	clang++ -c -I/opt/homebrew/Cellar/sdl2/2.0.18/include -I/opt/homebrew/Cellar/sdl2_image/2.0.5/include -I/opt/homebrew/Cellar/sdl2_ttf/2.0.15/include sdl.cpp

clean:
	rm sdl *.o
