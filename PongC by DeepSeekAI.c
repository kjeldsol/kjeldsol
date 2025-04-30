/*
 * PongC by DeepSeekAI - Classic Pong game implementation using SDL2
 * Written by DeepSeek AI in C. Uses SDL2 library for graphics and input handling.
 *
 * The game is distributed under the GPL 2.0 license.
 * Source code can be freely used, modified and distributed under license terms.
 * License: GPL 2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
 *
 * SDL2 Library: https://www.libsdl.org/
 *
 * ---
 * PongC by DeepSeekAI - Классическая игра Pong с использованием SDL2
 * Написана ИИ DeepSeek на языке C. Использует библиотеку SDL2 для графики и ввода.
 *
 * Игра распространяется под лицензией GPL 2.0.
 * Исходный код можно свободно использовать, модифицировать и распространять в соответствии с условиями лицензии.
 * Лицензия: GPL 2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.ru.html)
 *
 * Библиотека SDL2: https://www.libsdl.org/
 */
#include <SDL2/SDL.h>
#include <stdbool.h>

#define WIN_SCORE 5
#define FPS 60
int WIDTH;
int HEIGHT;
bool is_fullscreen = true;

typedef enum { MENU, PLAYING, PAUSED, GAME_OVER } GameState;
typedef enum { AI_MODE, TWO_PLAYERS, AI_FAST_MODE } GameMode;

typedef struct {
    SDL_Rect rect;
    int speed;
} Paddle;

typedef struct {
    SDL_Rect rect;
    int dx;
    int dy;
} Ball;

SDL_Window* window;
SDL_Renderer* renderer;
GameState game_state;
GameMode game_mode;
bool running;
Paddle player1, player2;
Ball ball;
int score1;
int score2;

void init_game() {
    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);
    WIDTH = is_fullscreen ? dm.w : 800;
    HEIGHT = is_fullscreen ? dm.h : 600;

    int paddle_w = WIDTH * 0.018;
    int paddle_h = HEIGHT * 0.15;
    int ball_size = HEIGHT * 0.03;

    player1.rect = (SDL_Rect){WIDTH*0.03, HEIGHT/2 - paddle_h/2, paddle_w, paddle_h};
    player2.rect = (SDL_Rect){WIDTH*0.97 - paddle_w, HEIGHT/2 - paddle_h/2, paddle_w, paddle_h};
    player1.speed = HEIGHT * 0.015;

    if(game_mode == AI_FAST_MODE) player2.speed = HEIGHT * 0.015;
    else if(game_mode == TWO_PLAYERS) player2.speed = player1.speed;
    else player2.speed = HEIGHT * 0.006;

    ball.rect = (SDL_Rect){WIDTH/2 - ball_size/2, HEIGHT/2 - ball_size/2, ball_size, ball_size};
    ball.dx = WIDTH * 0.007;
    ball.dy = HEIGHT * 0.007;

    score1 = 0;
    score2 = 0;
}

void move_ai() {
    int ball_center = ball.rect.y + ball.rect.h/2;
    int paddle_center = player2.rect.y + player2.rect.h/2;
    int threshold = HEIGHT * 0.07;

    if(ball_center < paddle_center - threshold && player2.rect.y > 0)
        player2.rect.y -= player2.speed;
    else if(ball_center > paddle_center + threshold && player2.rect.y < HEIGHT - player2.rect.h)
        player2.rect.y += player2.speed;
}

void update() {
    if(game_state != PLAYING) return;

    ball.rect.x += ball.dx;
    ball.rect.y += ball.dy;

    if(ball.rect.y <= 0 || ball.rect.y >= HEIGHT - ball.rect.h) ball.dy *= -1;

    if(SDL_HasIntersection(&ball.rect, &player1.rect) || SDL_HasIntersection(&ball.rect, &player2.rect))
        ball.dx *= -1;

    if(ball.rect.x <= 0) {
        score2++;
        if(score2 >= WIN_SCORE) game_state = GAME_OVER;
        else {
            ball.rect.x = WIDTH/2 - ball.rect.w/2;
            ball.rect.y = HEIGHT/2 - ball.rect.h/2;
            ball.dx = abs(ball.dx);
        }
    }
    else if(ball.rect.x >= WIDTH - ball.rect.w) {
        score1++;
        if(score1 >= WIN_SCORE) game_state = GAME_OVER;
        else {
            ball.rect.x = WIDTH/2 - ball.rect.w/2;
            ball.rect.y = HEIGHT/2 - ball.rect.h/2;
            ball.dx = -abs(ball.dx);
        }
    }
}

void draw_score() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    int dot_size = HEIGHT/40;
    int gap = dot_size/2;

    for(int i = 0; i < score1; i++) {
        SDL_Rect dot = {WIDTH/4 - (score1*(dot_size+gap))/2 + i*(dot_size+gap), HEIGHT/20, dot_size, dot_size};
        SDL_RenderFillRect(renderer, &dot);
    }

    for(int i = 0; i < score2; i++) {
        SDL_Rect dot = {3*WIDTH/4 - (score2*(dot_size+gap))/2 + i*(dot_size+gap), HEIGHT/20, dot_size, dot_size};
        SDL_RenderFillRect(renderer, &dot);
    }
}

void render_menu() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    int section = WIDTH / 3;

    SDL_RenderFillRect(renderer, &(SDL_Rect){section/2 - 15, HEIGHT/2 - 50, 30, 100});
    SDL_RenderFillRect(renderer, &(SDL_Rect){section + section/2 - 35, HEIGHT/2 - 50, 30, 100});
    SDL_RenderFillRect(renderer, &(SDL_Rect){section + section/2 + 5, HEIGHT/2 - 50, 30, 100});
    SDL_RenderFillRect(renderer, &(SDL_Rect){2*section + section/2 - 55, HEIGHT/2 - 50, 30, 100});
    SDL_RenderFillRect(renderer, &(SDL_Rect){2*section + section/2 - 15, HEIGHT/2 - 50, 30, 100});
    SDL_RenderFillRect(renderer, &(SDL_Rect){2*section + section/2 + 25, HEIGHT/2 - 50, 30, 100});

    SDL_RenderPresent(renderer);
}

void handle_menu_input() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_QUIT) running = false;
        if(event.type == SDL_KEYDOWN) {
            switch(event.key.keysym.scancode) {
                case SDL_SCANCODE_1:
                    game_mode = AI_MODE;
                    game_state = PLAYING;
                    init_game();
                    break;
                case SDL_SCANCODE_2:
                    game_mode = TWO_PLAYERS;
                    game_state = PLAYING;
                    init_game();
                    break;
                case SDL_SCANCODE_3:
                    game_mode = AI_FAST_MODE;
                    game_state = PLAYING;
                    init_game();
                    break;
                case SDL_SCANCODE_F:
                    is_fullscreen = !is_fullscreen;
                    SDL_SetWindowFullscreen(window, is_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                    if(!is_fullscreen) SDL_SetWindowSize(window, 800, 600);
                    init_game();
                break;
                case SDL_SCANCODE_ESCAPE:
                    running = false;
                    break;
                default: break;
            }
        }
    }
}

void handle_game_input() {
    SDL_Event event;
    const Uint8* keystates = SDL_GetKeyboardState(NULL);

    while(SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_WINDOWEVENT:
                if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    WIDTH = event.window.data1;
                    HEIGHT = event.window.data2;
                    init_game();
                }
                break;
            case SDL_KEYDOWN:
                switch(event.key.keysym.scancode) {
                    case SDL_SCANCODE_ESCAPE:
                        running = false;
                        break;
                    case SDL_SCANCODE_SPACE:
                        if(game_state == PLAYING) game_state = PAUSED;
                        else if(game_state == PAUSED) game_state = PLAYING;
                        break;
                    case SDL_SCANCODE_R:
                        init_game();
                        game_state = PLAYING;
                        break;
                    case SDL_SCANCODE_M:
                        game_state = MENU;
                        init_game();
                        break;
                    case SDL_SCANCODE_F:
                        is_fullscreen = !is_fullscreen;
                        SDL_SetWindowFullscreen(window, is_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                        if(!is_fullscreen) SDL_SetWindowSize(window, 800, 600);
                        init_game();
                    break;
                    default: break;
                }
                break;
        }
    }

    if(game_state == PLAYING) {
        if(keystates[SDL_SCANCODE_W])
            player1.rect.y = (player1.rect.y - player1.speed < 0) ? 0 : player1.rect.y - player1.speed;
        if(keystates[SDL_SCANCODE_S])
            player1.rect.y = (player1.rect.y + player1.speed > HEIGHT - player1.rect.h) ? HEIGHT - player1.rect.h : player1.rect.y + player1.speed;

        if(game_mode == TWO_PLAYERS) {
            if(keystates[SDL_SCANCODE_UP])
                player2.rect.y = (player2.rect.y - player2.speed < 0) ? 0 : player2.rect.y - player2.speed;
            if(keystates[SDL_SCANCODE_DOWN])
                player2.rect.y = (player2.rect.y + player2.speed > HEIGHT - player2.rect.h) ? HEIGHT - player2.rect.h : player2.rect.y + player2.speed;
        }
    }
}

void render_game() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &player1.rect);
    SDL_RenderFillRect(renderer, &player2.rect);
    SDL_RenderFillRect(renderer, &ball.rect);

    draw_score();

    if(game_state == PAUSED) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect pause = {WIDTH/2 - 30, HEIGHT/2 - 40, 60, 80};
        SDL_RenderFillRect(renderer, &pause);
    }
    else if(game_state == GAME_OVER) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect box = {WIDTH/2 - 100, HEIGHT/2 - 50, 200, 100};
        SDL_RenderFillRect(renderer, &box);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        if(score1 >= WIN_SCORE) {
            SDL_Rect line = {WIDTH/2 - 20, HEIGHT/2 - 20, 40, 40};
            SDL_RenderFillRect(renderer, &line);
        } else {
            SDL_Rect line1 = {WIDTH/2 - 20, HEIGHT/2 - 30, 40, 20};
            SDL_Rect line2 = {WIDTH/2 - 20, HEIGHT/2 + 10, 40, 20};
            SDL_RenderFillRect(renderer, &line1);
            SDL_RenderFillRect(renderer, &line2);
        }
    }

    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("PongC by DeepSeekAI & Kjeldsol", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_RESIZABLE | (is_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    game_state = MENU;
    running = true;
    init_game();

    while(running) {
        Uint32 frame_start = SDL_GetTicks();

        switch(game_state) {
            case MENU:
                handle_menu_input();
                render_menu();
                break;

            case PLAYING:
            case PAUSED:
            case GAME_OVER:
                handle_game_input();
                if(game_state == PLAYING) {
                    if(game_mode != TWO_PLAYERS) move_ai();
                    update();
                }
                render_game();
                break;
        }

        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if(frame_time < 1000/FPS) SDL_Delay(1000/FPS - frame_time);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
