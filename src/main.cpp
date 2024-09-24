#include <SDL.h>
#include <SDL_ttf.h>
#include <cassert>
#include <iostream>
#include "ecs.h"
#include "common.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
void attach_console();
#endif
struct Position {blaze::StaticVector<float, 2> pos;};

SDL_Window* window{nullptr};
SDL_Renderer* renderer{nullptr};
bool quit{false};

int main(int, char**) {
    #ifdef _WIN32
    attach_console();
    #endif
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    Position p{};
    p.pos[2] = 1;
    constexpr uint32_t wnd_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    window = SDL_CreateWindow("Legion", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, wnd_flags);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    Entity m;
    m.create<Position>();
    if (auto c = get_components<Position>(m)) {
        auto& [posref] = *c;
        posref.get().pos[1] = 99999;
    }
    //m.get<Position>().pos[1] = 1000;
    std::cout << m.get<Position>().pos.at(0) << m.get<Position>().pos.at(1) << '\n';
    std::cout << std::boolalpha << (m.get_if<Position>() != nullptr) << '\n';
    m.remove<Position>();
    std::cout << (m.get_if<Position>() != nullptr) << '\n';

    SDL_Event e;
    while (not quit) {
        {// event handling
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }
            }
        } {//update
        } {//rendering
            SDL_RenderClear(renderer);
            SDL_RenderPresent(renderer);
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
