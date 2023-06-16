#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

struct tile
{
  int nearby;
  bool discovered;
  bool mine;
  bool flagged;
};

void discoverAroundTile(int win_w, int win_h, struct tile map[win_w][win_h], int x, int y)
  {
    for(int i = -1;i <= 1;i++)
      {
        for(int j = -1;j <= 1;j++)
          {
            if((!map[x + j][y + i].discovered && !map[x + j][y + i].mine) && (x + j >= 0 && x + j < win_w && y + i >= 0 && y + i < win_h))
              {
                if(map[x + j][y + i].nearby == 0)
                  {
                    map[x + j][y + i].discovered = true;
                    discoverAroundTile(win_w, win_h, map, x + j, y + i);
                  }
                else
                    map[x + j][y + i].discovered = true;
              }
          }
        }
  }

void clearAroundTile(int win_w, int win_h, struct tile map[win_w][win_h], int x, int y)
  {
    for(int i = -1;i <= 1;i++)
      {
        for(int j = -1;j <= 1;j++)
          {
            if((!map[x + j][y + i].discovered && !map[x + j][y + i].flagged) && (x + j >= 0 && x + j < win_w && y + i >= 0 && y + i < win_h))
              {
                map[x + j][y + i].discovered = true;
                if(map[x + j][y + i].nearby == 0 && !map[x + j][y + i].mine)
                  discoverAroundTile(win_w, win_h, map, x + j, y + i);
              }
          }
      }
  }

void createMap(int win_w, int win_h, struct tile map[win_w][win_h], SDL_Rect dest[win_w][win_h], int mines_n, int mines[mines_n][2])
  {
    int i = 0;
    bool randomizing = true, unique;
    while(randomizing)
      {
        unique = true;

        mines[i][0] = rand() % 10;
        mines[i][1] = rand() % 10;

        for(int j = 0;j < i;j++)
          {
            if(mines[j][0] == mines[i][0] && mines[j][1] == mines[i][1])
              unique = false;
          }

        if(unique)
          i++;

        if(i == mines_n)
          randomizing = false;
      }
    for(int i = 0;i < win_w;i++)
      {
        for(int j = 0;j < win_h;j++)
          {
            map[j][i].nearby = 0;
            map[j][i].discovered = false;
            map[j][i].mine = false;
            map[j][i].flagged = false;
          }
      }
    for(int i = 0;i < win_w;i++)
      {
        for(int j = 0;j < win_h;j++)
          {
            dest[j][i].x = j*64;
            dest[j][i].y = i*64;
            dest[j][i].w = 64;
            dest[j][i].h = 64;
            for(int n = 0;n < mines_n;n++)
              {
                if(j == mines[n][0] && i == mines[n][1])
                  {
                    map[j][i].mine = true;
                    for(int y = -1;y <= 1;y++)
                      {
                        for(int x = -1;x <= 1;x++)
                          {
                            if((x != 0 || y != 0) && (j + x > -1 && j + x < win_w && i + y > -1 && i + y < win_h) && (!map[j + x][i + y].mine))
                              {
                                map[j + x][i + y].nearby++;
                              }
                          }
                      }
                  }
              }
          }
      }
  }

bool gameLoop(int win_w, int win_h, SDL_Renderer *renderer, SDL_Event event, SDL_Rect dest[win_w][win_h], SDL_Rect ScoreRect, SDL_Texture *Texture[17], int mines_n, struct tile map[win_w][win_h])
  {
    int framerate = 30, ticksPerFrame = 1000/framerate, discovered;
    bool lost = false, won = false;
    while(!lost || !won)
      {
        int startFrame = SDL_GetTicks();
        SDL_RenderClear(renderer);
        while(SDL_PollEvent(&event))
          {
            switch(event.type)
              {
              case SDL_MOUSEBUTTONDOWN:
                int x, y;
                SDL_GetMouseState(&x, &y);
                switch(event.button.button)
                  {
                  case SDL_BUTTON_LEFT:
                    for(int i = 0;i < win_h;i++)
                      {
                        for(int j = 0;j < win_w;j++)
                          {
                            if(dest[j][i].x < x && dest[j][i].x + 64 > x && dest[j][i].y < y && dest[j][i].y + 64 > y && !map[j][i].flagged && !lost && !won)
                              {
                                map[j][i].discovered = true;
                                if(map[j][i].mine)
                                  lost = true;
                                else
                                  {
                                    if(map[j][i].nearby == 0)
                                      discoverAroundTile(win_w, win_h, map, j, i);
                                  }
                              }
                          }
                      }
                    break;
                  case SDL_BUTTON_RIGHT:
                    for(int i = 0;i < win_h;i++)
                      {
                        for(int j = 0;j < win_w;j++)
                          {
                            if(dest[j][i].x < x && dest[j][i].x + 64 > x && dest[j][i].y < y && dest[j][i].y + 64 > y && !map[j][i].discovered && !lost && !won)
                              {
                                if(map[j][i].flagged)
                                  map[j][i].flagged = false;
                                else
                                  map[j][i].flagged = true;
                              }
                          }
                      }
                    break;
                  }
                break;
              case SDL_KEYDOWN:
                switch(event.key.keysym.sym)
                  {
                  case SDLK_SPACE:
                    for(int i = 0;i < win_h;i++)
                      {
                        for(int j = 0;j < win_w;j++)
                          {
                            if(dest[j][i].x < x && dest[j][i].x + 64 > x && dest[j][i].y < y && dest[j][i].y + 64 > y && !lost && !won)
                              clearAroundTile(win_w, win_h, map, j, i);
                          }
                      }
                    break;
                  }
                break;
              case SDL_QUIT:
                return true;
                break;
              }
          }

        discovered = 0;
        for(int i = 0;i < win_h;i++)
          {
            for(int j = 0;j < win_w;j++)
              {
                if(map[j][i].discovered && !map[j][i].mine)
                  discovered++;
                if(map[j][i].discovered && map[j][i].mine)
                  lost = true;
                int x, y;
                SDL_GetMouseState(&x, &y);
                if(map[j][i].mine && map[j][i].discovered && dest[j][i].x < x && dest[j][i].x + 64 > x && dest[j][i].y < y && dest[j][i].y + 64 > y)
                  SDL_RenderCopy(renderer, Texture[12], NULL, &dest[j][i]);
                else if(map[j][i].mine && map[j][i].discovered)
                  SDL_RenderCopy(renderer, Texture[11], NULL, &dest[j][i]);
                else if(map[j][i].flagged)
                  SDL_RenderCopy(renderer, Texture[16], NULL, &dest[j][i]);
                else if(map[j][i].discovered)
                  {
                    //SDL_RenderCopy(renderer, Texture[14], NULL, &dest[j][i]);
                    switch(map[j][i].nearby)
                      {
                      case 0:
                        SDL_RenderCopy(renderer, Texture[0], NULL, &dest[j][i]);
                        break;
                      case 1:
                        SDL_RenderCopy(renderer, Texture[1], NULL, &dest[j][i]);
                        break;
                      case 2:
                        SDL_RenderCopy(renderer, Texture[2], NULL, &dest[j][i]);
                        break;
                      case 3:
                        SDL_RenderCopy(renderer, Texture[3], NULL, &dest[j][i]);
                        break;
                      case 4:
                        SDL_RenderCopy(renderer, Texture[4], NULL, &dest[j][i]);
                        break;
                      case 5:
                        SDL_RenderCopy(renderer, Texture[5], NULL, &dest[j][i]);
                        break;
                      case 6:
                        SDL_RenderCopy(renderer, Texture[6], NULL, &dest[j][i]);
                        break;
                      case 7:
                        SDL_RenderCopy(renderer, Texture[7], NULL, &dest[j][i]);
                        break;
                      case 8:
                        SDL_RenderCopy(renderer, Texture[8], NULL, &dest[j][i]);
                        break;
                      }
                  }
                else if(dest[j][i].x < x && dest[j][i].x + 64 > x && dest[j][i].y < y && dest[j][i].y + 64 > y)
                  SDL_RenderCopy(renderer, Texture[15], NULL, &dest[j][i]);
                else
                  SDL_RenderCopy(renderer, Texture[13], NULL, &dest[j][i]);
                SDL_RenderDrawRect(renderer, &dest[j][i]);
              }
          }
        if(discovered == (win_w * win_h) - mines_n)
          won = true;
        if(lost)
            SDL_RenderCopy(renderer, Texture[9], NULL, &ScoreRect);
        if(won)
            SDL_RenderCopy(renderer, Texture[10], NULL, &ScoreRect);
        SDL_RenderPresent(renderer);
        int ticks = startFrame - SDL_GetTicks();
        if(ticks < ticksPerFrame)
          SDL_Delay(ticksPerFrame - ticks);
        if(won || lost)
          {
            SDL_Delay(ticksPerFrame * 30 * 3);
            return false;
          }
      }
  }

int main()
{
  time_t t;
  int mines_n = 15, win_w = 10, win_h = 10;
  bool quit = false;
  struct tile map[win_w][win_h];
  SDL_Rect dest[win_w][win_h];
  int mines[mines_n][2];
  srand((unsigned) time(&t));

  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;

  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();

  window = SDL_CreateWindow("Mine Sweeper", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 64 * win_w, 64 * win_h, SDL_WINDOW_OPENGL);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  // Creating text textures
  TTF_Font *font = TTF_OpenFont("OpenSans-Bold.ttf", 24);
  SDL_Color whiteFont = {255, 255, 255, 0};
  SDL_Color redFont = {255, 0, 0, 0};
  SDL_Color greenFont = {0, 255, 0, 0};
  SDL_Texture *Texture[17];
  SDL_Surface *Surface = TTF_RenderText_Solid(font, "0", whiteFont);
  Texture[0] = SDL_CreateTextureFromSurface(renderer, Surface);
  Surface = TTF_RenderText_Solid(font, "1", whiteFont);
  Texture[1] = SDL_CreateTextureFromSurface(renderer, Surface);
  Surface = TTF_RenderText_Solid(font, "2", whiteFont);
  Texture[2] = SDL_CreateTextureFromSurface(renderer, Surface);
  Surface = TTF_RenderText_Solid(font, "3", whiteFont);
  Texture[3] = SDL_CreateTextureFromSurface(renderer, Surface);
  Surface = TTF_RenderText_Solid(font, "4", whiteFont);
  Texture[4] = SDL_CreateTextureFromSurface(renderer, Surface);
  Surface = TTF_RenderText_Solid(font, "5", whiteFont);
  Texture[5] = SDL_CreateTextureFromSurface(renderer, Surface);
  Surface = TTF_RenderText_Solid(font, "6", whiteFont);
  Texture[6] = SDL_CreateTextureFromSurface(renderer, Surface);
  Surface = TTF_RenderText_Solid(font, "7", whiteFont);
  Texture[7] = SDL_CreateTextureFromSurface(renderer, Surface);
  Surface = TTF_RenderText_Solid(font, "8", whiteFont);
  Texture[8] = SDL_CreateTextureFromSurface(renderer, Surface);
  Surface = TTF_RenderText_Solid(font, "YOU LOSE!!!", redFont);
  Texture[9] = SDL_CreateTextureFromSurface(renderer, Surface);
  Surface = TTF_RenderText_Solid(font, "YOU WIN!!!", greenFont);
  Texture[10] = SDL_CreateTextureFromSurface(renderer, Surface);
  SDL_Rect ScoreRect = {(win_w * 64 / 2) - 200, (win_h * 64 / 2) - 100, 400, 200};

  // Creating tile texutres
  Surface = SDL_LoadBMP("./bomb.bmp");
  Texture[11] = SDL_CreateTextureFromSurface(renderer, Surface);
  Texture[12] = SDL_CreateTextureFromSurface(renderer, Surface);
  SDL_SetTextureColorMod(Texture[11], 200, 200, 200);
  SDL_SetTextureColorMod(Texture[12], 200, 255, 200);
  SDL_FreeSurface(Surface);
  Surface = SDL_LoadBMP("./tile.bmp");
  Texture[13] = SDL_CreateTextureFromSurface(renderer, Surface);
  Texture[14] = SDL_CreateTextureFromSurface(renderer, Surface);
  Texture[15] = SDL_CreateTextureFromSurface(renderer, Surface);
  SDL_SetTextureColorMod(Texture[14], 200, 200, 200);
  SDL_SetTextureColorMod(Texture[15], 200, 255, 200);
  SDL_FreeSurface(Surface);
  Surface = SDL_LoadBMP("./flag.bmp");
  Texture[16] = SDL_CreateTextureFromSurface(renderer, Surface);
  SDL_FreeSurface(Surface);

  while(!quit)
    {
      createMap(win_w, win_h, map, dest, mines_n, mines);
      quit = gameLoop(win_w, win_h, renderer, event, dest, ScoreRect, Texture, mines_n, map);
    }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
