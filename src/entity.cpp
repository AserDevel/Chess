#include <iostream>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <constants.h>

using namespace std;

class Entity {
    public:
        int x, y, w, h;
        string path;
        
        Entity() {};
        Entity(int x, int y, int w, int h, string fileName) {
            this->x = x;
            this->y = y;
            this->w = w;
            this->h = h;
            this->path = (string)SRC_PATH + "assets/textures/" + fileName;
        }

        void render(SDL_Renderer* renderer) {
            SDL_Surface* surface = NULL;
            SDL_Texture* texture = NULL;
            surface = IMG_Load(&path[0]);

            if (surface == NULL) {
                cout << "Cannot find: " << &path[0] << endl;
                SDL_Quit();
                exit(1);
            }

            texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);

            SDL_Rect rect = { x, y, w, h };
            SDL_RenderCopyEx(renderer, texture, NULL, &rect, 0, NULL, SDL_FLIP_NONE);
            SDL_DestroyTexture(texture);
        }
};
