#pragma once
#include <string>
#include "common.h"
#include "components.h"
struct SDL_Window;
struct lua_State;
namespace legion {
namespace engine {
using Update_fn = void(*)(double delta_time);//maybe use std::function for these
using Render_fn = void(*)();
using Start_fn = void(*)();
using Shutdown_fn = void(*)();
struct Start_options {
    std::string window_name{"Legion"};
    Vec2i window_size{1240, 720};
    bool window_resizable{false};
    bool hardware_accelerated{true};
    bool vsync_enabled{true};
    Update_fn update_function{nullptr};
    Render_fn render_function{nullptr};
    Start_fn start_function{nullptr};
    Shutdown_fn shutdown_function{nullptr};
};
struct Game_interface {
    virtual ~Game_interface() = default;
    virtual Start_options config() = 0;
    virtual void start() = 0;
    virtual void update() = 0;
    virtual void teardown() = 0;
};
int bootstrap(Start_options opts = {});
void quit();
namespace renderer {
void clear_frame();
void draw(const Recti64& rect);
void fill(const Recti64& rect);
void set_color(Coloru32 color);
}//renderer end
namespace core {
void start(Start_options opts = {});
void run();
void shutdown();
SDL_Window* get_window();// not preferrable, but need it for textures
lua_State* get_lua_state();
}//core end
}
}
