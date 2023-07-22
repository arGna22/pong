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
#define PADDLE_SPEED 300
#define BALL_SIDE 10
#define BALL_START_SPEED 100;

/* Struct definitions */
typedef struct ball {
	float x, y;	/* (x, y) position on screen */
	int w, h;	/* width and height of ball */
	int speed;
	float dx, dy;	/* Movement vectors */

} ball_t;

typedef struct paddle {
	float x, y;	/* (x, y) position on screen */
	int w, h;	/* width and height of paddle */
	int up, down; 	/* indicates if paddle is moving up or down */ 

} paddle_t;

/* Function prototypes */
void move_p();
void init();
void draw_paddle(paddle_t p);
void move_ball();
void draw_ball();
void check_collisions();
void _reset_ball();

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

	ball.x = WINDOW_WIDTH / 2;
	ball.y = WINDOW_HEIGHT / 2;
	ball.h = BALL_SIDE;
	ball.w = BALL_SIDE;
	ball.dx = BALL_START_SPEED;
	ball.dy = BALL_START_SPEED;

}

void move_p()
{
	if (paddle_p.up && !paddle_p.down) paddle_p.y += -PADDLE_SPEED/60;
	if (paddle_p.down && !paddle_p.up) paddle_p.y += PADDLE_SPEED/60;
	if (paddle_p.y <= 0) paddle_p.y = 0;
	if (paddle_p.y + PADDLE_HEIGHT >= WINDOW_HEIGHT) paddle_p.y = WINDOW_HEIGHT - PADDLE_HEIGHT;
}

void draw_paddle(paddle_t p) 
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

// ball specific functions
void move_ball() 
{
	ball.x += ball.dx / 60;
	ball.y += ball.dy / 60;
}

void draw_ball() 
{
	SDL_Rect object;

	object.x = ball.x;
	object.y = ball.y;
	object.w = ball.w; 
	object.h = ball.h; 

	SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
	SDL_RenderFillRect(rend, &object);
	SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
}

void check_collisions() 
{
	// Collisions where ball bounces
	if (ball.y <= 0 || ball.y + BALL_SIDE >= WINDOW_HEIGHT)  {
		ball.dy = -ball.dy;
	}

	if (ball.x + BALL_SIDE >= paddle_p.x && ball.x + BALL_SIDE >= paddle_p.x + PADDLE_WIDTH) {
		if (ball.y + BALL_SIDE >= paddle_p.y && ball.y + BALL_SIDE <= paddle_p.y + PADDLE_HEIGHT)
				ball.dx = -ball.dx;
	}

	// Collisiosn where ball resets
	if (ball.x <= 0 || ball.x + BALL_SIDE >= WINDOW_WIDTH) _reset_ball(); 
}

void _reset_ball() 
{
	ball.x = WINDOW_WIDTH / 2;
	ball.y = WINDOW_HEIGHT / 2;

	ball.dx = BALL_START_SPEED;
	ball.dy = BALL_START_SPEED;
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
		}

		SDL_RenderClear(rend); 
		check_collisions();
		move_p();
		draw_paddle(paddle_p);
		move_ball(); 
		draw_ball();
		SDL_RenderPresent(rend);
		SDL_Delay(1000/60);
	}

	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(win);
	SDL_Quit();
}
