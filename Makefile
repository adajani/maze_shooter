all:
	g++ -o maze_shooter main.cpp `pkg-config --cflags --libs sdl2 SDL2_image SDL2_mixer SDL2_ttf` -lm 
clean:
	rm -f maze_shooter
	rm -f *.o