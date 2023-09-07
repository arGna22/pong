/**
  * Pong written in C, using SDL2 for graphics
  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_audio.h>

/* Constants */
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define PADDLE_WIDTH 10
#define PADDLE_HEIGHT 50
#define PADDLE_SPEED 300 // In pixels per second
#define BALL_SIDE 10
#define BALL_START_SPEED 100 // In pixels per second 
#define BALL_MAX_SPEED 600 // In pixels per second
#define PI 3.14159265
#define MAX_BOUNCE_ANGLE  5 * PI / 12
#define MAX_POINTS 11

/* Macros */
#define get_bounce_angle(intersect_y, paddle) atan(((intersect_y - (paddle.rect.y + PADDLE_HEIGHT / 2.0)) / (PADDLE_HEIGHT / 2.0)))

/* Struct definitions */
typedef struct paddle {
	SDL_Rect rect;
	int w, h;
	int up, down;
} paddle_t;

typedef struct ball {
	SDL_Rect rect;
	double x, y;
	double dx, dy; 
} ball_t;

typedef struct sound {
	SDL_AudioSpec wavSpec;
	Uint32 wavLength;
	Uint8 *wavBuffer;
	SDL_AudioDeviceID deviceID;
} sound_t;

/* Globals */
SDL_Window* win;
SDL_Renderer* rend;
paddle_t player;  
paddle_t ai;
ball_t ball;
int player_points, ai_points;
int ball_speed = BALL_START_SPEED;
TTF_Font* game_font;
TTF_Font* secondary_font;
sound_t blip_1;
sound_t blip_2;
sound_t blip_3;

/* Function prototypes */
void init();
void key_down_event(SDL_Event event);
void draw_filled_rect(SDL_Rect rect);
void move_paddle(paddle_t *paddle, int ai);
void move_ball();
void key_up_event(SDL_Event event);
int check_collisions(SDL_Rect a, SDL_Rect b);
int get_intersect_y(int a_y1, int a_y2, int b_y1, int b_y2);
void reset_ball();
void ball_collides_paddle(int intersect_y, paddle_t paddle);
void reset_paddles();
void draw_text(TTF_Font *font, char *text, int x, int y, int centered_x, int centered_y);
void reset_game();
void quit();
void start_menu();
void game_end(int player_win);
void determine_ai_direction();

/* Function definitions */
void init()
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER)) {
		printf("Unable to initialize SDL2: %s\n", SDL_GetError());
		SDL_Quit();
	}

	if (TTF_Init()) {
		printf("Unable to initialize SDL2 ttf: %s\n", SDL_GetError());
		TTF_Quit();
	}

	// Create window
	win = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
	if (!win) {
		printf("Unable to create window: %s\n", SDL_GetError());
		SDL_DestroyWindow(win);
		TTF_Quit();
		SDL_Quit();
	}
	
	// Create renderer
	Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
	rend = SDL_CreateRenderer(win, -1, render_flags);
	if (!rend)
	{
		printf("Error creating renderer: %s\n", SDL_GetError());
		SDL_DestroyWindow(win);
		SDL_DestroyRenderer(rend);
		TTF_Quit();
		SDL_Quit();
	}

	// Initialize player and ai
	player_points = ai_points = 0;
	player.rect.x = WINDOW_WIDTH - 25;
	player.rect.y = WINDOW_HEIGHT / 2;
	ai.rect.x = 25 - PADDLE_WIDTH;
	ai.rect.y = WINDOW_HEIGHT / 2; 
	ai.rect.w = player.rect.w = PADDLE_WIDTH;
	ai.rect.h = player.rect.h = PADDLE_HEIGHT;
	ai.up = ai.down = player.up = player.down = 0; 

	// initialize ball
	ball.x = WINDOW_WIDTH / 2;
	ball.y = WINDOW_HEIGHT / 2;

	ball.rect.x = ball.x; 
	ball.rect.y = ball.y;

	ball.rect.w = BALL_SIDE;
	ball.rect.h = BALL_SIDE;

	double inital_angle = (rand() % (int)(2 * MAX_BOUNCE_ANGLE)) - MAX_BOUNCE_ANGLE;
	ball.dx = ball_speed*cos(inital_angle);
	ball.dy = ball_speed*sin(inital_angle); 

	// Create fonts
	game_font = TTF_OpenFont("bit5x3.ttf", 65);
	secondary_font = TTF_OpenFont("bit5x3.ttf", 45);

	// Load game sounds
	SDL_LoadWAV("blip_1.wav", &blip_1.wavSpec, &blip_1.wavBuffer, &blip_1.wavLength);
	blip_1.deviceID = SDL_OpenAudioDevice(NULL, 0, &blip_1.wavSpec, NULL, 0);

	SDL_LoadWAV("blip_2.wav", &blip_2.wavSpec, &blip_2.wavBuffer, &blip_2.wavLength);
	blip_2.deviceID = SDL_OpenAudioDevice(NULL, 0, &blip_2.wavSpec, NULL, 0);

	SDL_LoadWAV("point_scored.wav", &blip_3.wavSpec, &blip_3.wavBuffer, &blip_3.wavLength);
	blip_3.deviceID = SDL_OpenAudioDevice(NULL, 0, &blip_3.wavSpec, NULL, 0);
}

void quit() 
{
	SDL_FreeWAV(blip_1.wavBuffer);
	SDL_FreeWAV(blip_2.wavBuffer);
	SDL_FreeWAV(blip_3.wavBuffer);

	SDL_DestroyWindow(win);
	SDL_DestroyRenderer(rend);
	TTF_CloseFont(game_font);
	TTF_Quit();
	SDL_Quit();

}

void start_menu()
{
	draw_text(game_font, "PONG", WINDOW_WIDTH / 2, WINDOW_HEIGHT * (3 / 8.0) , 1, 1);
	draw_text(secondary_font, "Press any key to start", WINDOW_WIDTH / 2, WINDOW_HEIGHT * (5 / 8.0), 1, 1); 

	SDL_RenderPresent(rend);

	int key_pressed = 0;
	while (!key_pressed) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type)
			{
				case SDL_QUIT:
					quit();
					exit(0);
				case SDL_KEYDOWN:
					key_pressed = 1;
			}
		}
	}
}

void game_end(int player_win)
{
	char *message;
	if (player_win) message = "You win!"; 
	else message = "You lose!";

	SDL_RenderClear(rend);
	draw_text(game_font, message, WINDOW_WIDTH / 2, WINDOW_HEIGHT * (3 / 8.0), 1, 1);
	draw_text(secondary_font, "Press any key to play again", WINDOW_WIDTH / 2, WINDOW_HEIGHT * (5 / 8.0), 1, 1);

	SDL_RenderPresent(rend);

	int key_pressed = 0;
	while (!key_pressed) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type)
			{
				case SDL_QUIT:
					quit();
					exit(0);
				case SDL_KEYDOWN:
					key_pressed = 1;
					reset_game();
			}
		}
	}
}

void reset_game()
{
	reset_paddles();
	reset_ball();
	player_points = 0;
	ai_points = 0;
}

void draw_filled_rect(SDL_Rect rect)
{
	SDL_SetRenderDrawColor(rend, 255, 255, 255, 0);
	SDL_RenderFillRect(rend, &rect);
	SDL_SetRenderDrawColor(rend, 0, 0, 0, 0);
}

void move_paddle(paddle_t *paddle, int ai)
{
	if (ai && abs(paddle->rect.y - ball.rect.y) <= PADDLE_SPEED / 60) {
		paddle->rect.y = ball.rect.y;
	}
	else if (paddle->up && !paddle->down) paddle->rect.y -= PADDLE_SPEED / 60;
	else if (paddle->down && !paddle->up) paddle->rect.y += PADDLE_SPEED / 60;

	if (paddle->rect.y <= 0) paddle->rect.y = 0;
	else if (paddle->rect.y >= WINDOW_HEIGHT - PADDLE_HEIGHT) paddle->rect.y = WINDOW_HEIGHT - PADDLE_HEIGHT;

}

void determine_ai_direction() 
{ 
	if (ai.rect.y > ball.rect.y) {
		ai.up = 1;
		ai.down = 0;
	} else {
		ai.down = 1;
		ai.up = 0;
	}
}

void reset_ball()
{
	ball.x = WINDOW_WIDTH / 2;
	ball.y = WINDOW_HEIGHT / 2;
	ball_speed = BALL_START_SPEED;
	ball.dx = ball.dy = ball_speed; 
}

void draw_text(TTF_Font *font, char *text, int x, int y, int centered_x, int centered_y)
{	
	SDL_Color White = {255, 255, 255};
	SDL_Surface* text_surface = TTF_RenderUTF8_Solid(font, text, White);
	SDL_Texture* text_texture = SDL_CreateTextureFromSurface(rend, text_surface);
	SDL_FreeSurface(text_surface);

	SDL_Rect dest;
	SDL_QueryTexture(text_texture, NULL, NULL, &dest.w, &dest.h);

	if (centered_x) x = x - dest.w / 2;
	if (centered_y) y = y - dest.h / 2; 
	dest.x = x;
	dest.y = y;
	SDL_RenderCopy(rend, text_texture, NULL, &dest);

	SDL_DestroyTexture(text_texture);
}

void move_ball()
{
	ball.x += ball.dx / 60.0;
	ball.y += ball.dy / 60.0;

	ball.rect.x = ball.x;
	ball.rect.y = ball.y;

	int success;

	if (ball.rect.x <= 0) {
		success = SDL_QueueAudio(blip_3.deviceID, blip_3.wavBuffer, blip_3.wavLength);
		SDL_PauseAudioDevice(blip_3.deviceID, 0);

		reset_ball(); 
		reset_paddles();
		player_points++;
	} 
	if (ball.rect.x >= WINDOW_WIDTH - BALL_SIDE) {
		success = SDL_QueueAudio(blip_3.deviceID, blip_3.wavBuffer, blip_3.wavLength);
		SDL_PauseAudioDevice(blip_3.deviceID, 0);

		reset_ball();
		reset_paddles();
		ai_points++;
	}
	if (ball.rect.y <= 0 || ball.rect.y >= WINDOW_HEIGHT - BALL_SIDE) {
		success = SDL_QueueAudio(blip_2.deviceID, blip_2.wavBuffer, blip_2.wavLength);
		SDL_PauseAudioDevice(blip_2.deviceID, 0);
		ball.dy = -ball.dy;
	}	

}

void key_down_event(SDL_Event event)
{
	switch (event.key.keysym.scancode) {
		case SDL_SCANCODE_W:
		case SDL_SCANCODE_UP:
			player.up = 1;
			break;

		case SDL_SCANCODE_S:
		case SDL_SCANCODE_DOWN:
			player.down = 1;
	}
}

void key_up_event(SDL_Event event)
{
	switch (event.key.keysym.scancode) {
		case SDL_SCANCODE_W:
		case SDL_SCANCODE_UP:
			player.up = 0;
			break;

		case SDL_SCANCODE_S:
		case SDL_SCANCODE_DOWN:
			player.down = 0;
	}
}

// Function that checks if 2 rects have collided
int check_collisions(SDL_Rect a, SDL_Rect b)
{
	int a_x1, a_x2, a_y1, a_y2;
	int b_x1, b_x2, b_y1, b_y2;

	a_x1 = a.x;
	a_x2 = a.x + a.w;
	a_y1 = a.y;
	a_y2 = a.y + a.h;

	b_x1 = b.x;
	b_x2 = b.x + b.w;
	b_y1 = b.y;
	b_y2 = b.y + b.h;

	if (b_x1 >= a_x1 && b_x1 <= a_x2 || b_x2 >= a_x1 && b_x2 <= a_x2)
		if (b_y1 >= a_y1 && b_y1 <= a_y2 || b_y2 >= a_y1 && b_y2 <= a_y2)
			return get_intersect_y(a_y1, a_y2, b_y1, b_y2);
	return 0;
}

int get_intersect_y(int a_y1, int a_y2, int b_y1, int b_y2) 
{
	int range_beg = a_y1;
	int range_end = a_y2;
	if (b_y1 >= a_y1 && b_y1 <= a_y2) range_beg = b_y1;
	if (b_y2 >= a_y1 && b_y2 <= a_y2) range_end = b_y2;

	return (range_beg + range_end) / 2;
}

void ball_collides_paddle(int intersect_y, paddle_t paddle)
{
	double bounce_angle = get_bounce_angle(intersect_y, paddle);
	int sign = 1;
	if (ball.dx > 0) sign = -1;
	if (ball_speed < BALL_MAX_SPEED) ball_speed += 50;
	ball.dx = ball_speed*sign*cos(bounce_angle);
	ball.dy = ball_speed*sin(bounce_angle); 

	int success = SDL_QueueAudio(blip_1.deviceID, blip_1.wavBuffer, blip_1.wavLength);
	SDL_PauseAudioDevice(blip_1.deviceID, 0);

}

void reset_paddles()
{
	player.rect.x = WINDOW_WIDTH - 25;
	player.rect.y = WINDOW_HEIGHT / 2;
	ai.rect.x = 25 - PADDLE_WIDTH;
	ai.rect.y = WINDOW_HEIGHT / 2; 
	
	ai.up = player.up = ai.down = player.down = 0;
}

int main()
{
	init();
	int close_requested = 0;
	char player_point_text[2];
	char ai_point_text[2];
	
	start_menu();
	while (!close_requested) 
	{ 
		// Process events
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					close_requested = 1;
					break;
				case SDL_KEYDOWN:
					key_down_event(event);				
					break;
				case SDL_KEYUP:
					key_up_event(event);
			}
		}
		// Moving
		if (ball.dx < 0) {
			determine_ai_direction();
			move_paddle(&ai, 1);
		}
		move_paddle(&player, 0);
		move_ball();
		
		// check collisions
		int intersect_y_player, intersect_y_ai;
		if ((intersect_y_player = check_collisions(player.rect, ball.rect))) {
			ball_collides_paddle(intersect_y_player, player);
			ball.x = player.rect.x - 15; // prevents more than one collision
		} 

		if ((intersect_y_ai = check_collisions(ai.rect, ball.rect))) {
			ball_collides_paddle(intersect_y_ai, ai);
			ball.x = ai.rect.x + 15; 
		}

		// Convert points to text
		sprintf(player_point_text, "%d", player_points);
		sprintf(ai_point_text, "%d", ai_points);

		// Clear screen
		SDL_RenderClear(rend);

		//draw seperating line 
		SDL_Rect r;
		r.w = PADDLE_WIDTH;
		r.h = WINDOW_HEIGHT;
		r.x = WINDOW_WIDTH / 2 - PADDLE_WIDTH;
		r.y = 0;

		draw_filled_rect(r);

		// Rendering
		draw_filled_rect(player.rect);
		draw_filled_rect(ai.rect);
		draw_filled_rect(ball.rect);
		draw_text(game_font, ai_point_text, WINDOW_WIDTH / 4, 30, 0, 0);
		draw_text(game_font, player_point_text, WINDOW_WIDTH * (3.0 / 4), 30, 0, 0);
		SDL_RenderPresent(rend);

		if (player_points == MAX_POINTS) game_end(1);
		else if (ai_points == MAX_POINTS) game_end(0);

		// Timing
		SDL_Delay(1000/60);
	}

	quit();
	return 0;
}
