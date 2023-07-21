#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>

/* Program constants */
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define PADDLE_WIDTH 10 
#define PADDLE_HEIGHT 50 
#define PADDLE_SPEED 600

/* Struct definitions */
typedef struct ball {
	float x, y;	/* (x, y) position on screen */
	int w, h;	/* width and height of ball */
	int dx, dy;	/* Movement vectors */

} ball_t;

typedef struct paddle {
	float x, y;	/* (x, y) position on screen */
	int w, h;	/* width and height of paddle */
	int up, down; 	/* indicates if paddle is moving up or down */ 

} paddle_t;

/* Function prototypes */
void move_p();
void init();
void draw_paddle(paddle_t paddle);

/* Program globals */
static paddle_t paddle_p;
static paddle_t paddle_ai;
static ball_t ball;

SDL_Window *win = NULL;
SDL_Renderer *rend = NULL;

/* Function definitions */

// Initialization function 
void init() 
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		fprintf(stderr, "error initializing SDL: %s", SDL_GetError());
		exit(-1);	
	}


	if (!(win = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0))) {
		fprintf(stderr, "error creating window: %s\n", SDL_GetError());
		SDL_Quit();
		exit(-1);	
	}


	
	if (!(rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC))) {
		fprintf(stderr, "error creating renderer: %s\n", SDL_GetError());
		SDL_DestroyWindow(win);
		SDL_Quit();
		exit(-1);	
	}

	paddle_p.w = paddle_ai.w = PADDLE_WIDTH;
	paddle_p.h = paddle_ai.h = PADDLE_HEIGHT;
	paddle_p.x = WINDOW_WIDTH - PADDLE_WIDTH - 20;
	paddle_p.y = WINDOW_HEIGHT / 2;

}
// Paddle-specific functions
void move_p() // For player
{
	if (paddle_p.up && !paddle_p.down) paddle_p.y += -PADDLE_SPEED/60;
	if (paddle_p.down && !paddle_p.up) paddle_p.y += PADDLE_SPEED/60;
	if (paddle_p.y <= 0) paddle_p.y = 0;
	if (paddle_p.y + PADDLE_HEIGHT >= WINDOW_HEIGHT) paddle_p.y = WINDOW_HEIGHT - PADDLE_HEIGHT;
}

void draw_paddle(paddle_t p)  // For player and ai
{
	SDL_Rect object;
	object.x = (int)p.x;
	object.y = (int)p.y;
	object.w = p.w;
	object.h = p.h;

	SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
	SDL_RenderFillRect(rend, &object);
	SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
}

int main(void) 
{
	int close_requested = 0;
	init();
	while (!close_requested) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					close_requested = 1;
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.scancode)
					{
						case SDL_SCANCODE_W:
						case SDL_SCANCODE_UP:
							paddle_p.up  = 1;
							break;
						case SDL_SCANCODE_S:
						case SDL_SCANCODE_DOWN:
							paddle_p.down = 1;
							break;
					};
					break;
				case SDL_KEYUP:
					switch (event.key.keysym.scancode)
					{
						case SDL_SCANCODE_W:
						case SDL_SCANCODE_UP:
							paddle_p.up  = 0;
							break;
						case SDL_SCANCODE_S:
						case SDL_SCANCODE_DOWN:
							paddle_p.down = 0;
							break;
					};
			}
				
			// Move things
			move_p();
		}

		SDL_RenderClear(rend); 
		draw_paddle(paddle_p);
		SDL_RenderPresent(rend);
		SDL_Delay(1000/60);
	}

	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(win);
	SDL_Quit();
}
