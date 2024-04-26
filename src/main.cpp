#include <SDL2/SDL.h>

#include <stdio.h>

int main(int argc, char* args[]) {
	printf("Hello Game Engine 2024!\n");

	if (SDL_Init(SDL_INIT_VIDEO)) {
		printf("SDL_Init failed with error: %s\n", SDL_GetError());
	}

	SDL_Window* window = SDL_CreateWindow(
		"Game Engine 2024",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		680,
		480,
		0
	);

	SDL_Delay(5000);

	SDL_Quit();
	return 0;
}
