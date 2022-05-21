#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <SDL.h>

typedef enum { false, true } bool;

struct Config {
    int FPS;
    int SQUARE_SIZE;
    int MAP_WIDTH;
    int MAP_HEIGHT;
    int SCREEN_WIDTH;
    int SCREEN_HEIGHT;
    int WALL_DENSITY;
    int CELLULAR_ITERATIONS;
};

struct Config loadConfig(char *path);

void makeNoiseGrid(int *grid, int density, int gridH, int gridW);

bool isWithinMapBounds(int x, int y, int gridH, int gridW);

void applyCellularAutomation(int *grid,const int *noiseGrid, int count, int gridH, int gridW);

void renderGrid(SDL_Renderer *renderer, SDL_Rect r, int* grid, int MAP_HEIGHT, int MAP_WIDTH);

int main(int argc, char *argv[]) {

    struct Config CONFIG = loadConfig("./config.txt");
    printf("%d", CONFIG.WALL_DENSITY);

    const int gridH = CONFIG.MAP_HEIGHT, gridW = CONFIG.MAP_WIDTH;

    int grid[gridH][gridW];
    int noiseGrid[gridH][gridW];

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Could not initialize sdl2: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window *window = SDL_CreateWindow("Cellular Automata Map",
                                          100, 100,
                                          CONFIG.SCREEN_WIDTH, CONFIG.SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN);

    if (window == NULL) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
                                                SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {
        SDL_DestroyWindow(window);
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_SetRenderDrawColor( renderer, 0, 255, 0, 255 );
    SDL_RenderClear(renderer);

    SDL_Rect r;
    r.x = 0;
    r.y = 0;
    r.w = CONFIG.SQUARE_SIZE;
    r.h = CONFIG.SQUARE_SIZE;

    SDL_Event e;
    bool quit = false;

    makeNoiseGrid(noiseGrid[0],CONFIG.WALL_DENSITY, gridH, gridW);
    renderGrid(renderer, r, noiseGrid[0], CONFIG.MAP_HEIGHT, CONFIG.MAP_WIDTH);
    applyCellularAutomation(grid[0], noiseGrid[0], CONFIG.CELLULAR_ITERATIONS, CONFIG.MAP_HEIGHT, CONFIG.MAP_WIDTH);
    renderGrid(renderer, r, grid[0], CONFIG.MAP_HEIGHT, CONFIG.MAP_WIDTH);

    while (!quit) {

        while(SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                default: {}
            }
        }

        SDL_Delay(1000 / CONFIG.FPS);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}

struct Config loadConfig(char *path) {
    struct Config Config;

    FILE *pConfig = fopen(path, "r");
    int configValues[6];
    int i = 0;
    char buffer[255];

    if (pConfig == NULL) {
        printf("Unable to open the file");
        return Config;
    }

    while(fgets(buffer, 255, pConfig) != NULL) {
        char *token = strtok(buffer, ":");
        token = strtok(NULL, ":");
        token = atoi(token);
        configValues[i] = token;
        i++;
    }

    fclose(pConfig);

    Config.FPS = configValues[0];
    Config.SQUARE_SIZE = configValues[1];
    Config.MAP_WIDTH = configValues[2];
    Config.MAP_HEIGHT = configValues[3];
    Config.WALL_DENSITY = configValues[4];
    Config.CELLULAR_ITERATIONS = configValues[5];
    Config.SCREEN_HEIGHT = Config.MAP_HEIGHT*Config.SQUARE_SIZE;
    Config.SCREEN_WIDTH = Config.MAP_WIDTH*Config.SQUARE_SIZE;

    return Config;
}

void makeNoiseGrid(int *grid, int density, const int gridH, const int gridW) {
    srand(time(NULL));

    for (int i = 0; i < gridH; i++) {
        for (int j = 0; j < gridW; j++) {
            int rInt = rand() % 100 + 1;
            grid[i * gridW + j] = rInt > density ? 0 : 1;
        }
    }
}

bool isWithinMapBounds(int x, int y, const int gridH, const int gridW) {
    return x < 0 || x >= gridW || y < 0 || y >= gridH ? false : true;
}


void applyCellularAutomation(int *grid, const int *noiseGrid, int count, int gridH, int gridW) {
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < gridH; j++) {
            for (int k = 0; k < gridW; k++) {
                int neighborWallCount = 0;
                for (int y = j - 1; y <= j + 1; y++) {
                    for (int x = k - 1; x <= k + 1; x++) {
                        if (isWithinMapBounds(x, y, gridH, gridW)) {
                            if (y != j || x != k) {
                                if (noiseGrid[y * gridW + x] == 1) {
                                    neighborWallCount++;
                                }
                            }
                        } else {
                            neighborWallCount++;
                        }
                    }
                }
                if (neighborWallCount > 4) {
                    grid[j * gridW + k] = 1;
                } else {
                    grid[j * gridW + k] = 0;
                }
            }
        }
    }
}

void renderGrid(SDL_Renderer *renderer, SDL_Rect r, int* grid, int MAP_HEIGHT, int MAP_WIDTH) {
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {

            if (grid[i * MAP_WIDTH + j] == 0) {
                SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
            } else {
                SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
            }

            SDL_RenderFillRect(renderer, &r );
            SDL_RenderPresent(renderer);
            r.x += r.w;
        }
        r.x = 0;
        r.y += r.h;
    }
    r.x = 0;
    r.y = 0;
}


