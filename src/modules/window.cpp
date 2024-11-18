#include "builtin.h"
#include "engine.h"
#include <SDL.h>
#include <SDL_image.h>
#include "lua_util.h"
using engine::window;
static constexpr size_t size_length = std::string("Size").length();
static constexpr size_t position_length = std::string("Position").length();
static constexpr size_t title_length = std::string("Title").length();
static constexpr size_t opacity_length = std::string("Opacity").length();
static constexpr size_t borderless_length = std::string("Borderless").length();
static constexpr size_t always_on_top_length = std::string("AlwaysOnTop").length();
static constexpr size_t fullscreen_length = std::string("Fullscreen").length();
static constexpr size_t icon_length = std::string("Icon").length();
using engine::expect;
using engine::window;
using builtin::vector2;
using builtin::vector2;
using unique_event = std::unique_ptr<builtin::event>; 
using builtin::event;
static unique_event shown;
static unique_event hidden;
static unique_event mouse_entered;
static unique_event mouse_left;
static unique_event size_changed;
static unique_event resized;
static unique_event exposed;
static unique_event position_changed;
static unique_event minimized;
static unique_event maximized;
static unique_event restored;
static unique_event focus_gained;
static unique_event focus_lost;
static unique_event closing;
static bool is_always_on_top = false;
static bool is_borderless = false;

static int index(lua_State* L) {
    size_t length;
    const char key = *luaL_checklstring(L, 2, &length);
    switch (key) {
        case 'S': {
            expect(length == size_length);
            int w, h;
            SDL_GetWindowSize(engine::window(), &w, &h);
            create<builtin::vector2>(L) = vec2i{w, h};
            return 1;
        }
        case 'P': {
            expect(length == position_length);
            int x, y;
            SDL_GetWindowPosition(engine::window(), &x, &y);
            create<builtin::vector2>(L) = vec2i{x, y};
            return 1;
        }
        case 'T': {
            expect(length == title_length);
            lua_pushstring(L, SDL_GetWindowTitle(engine::window()));
            return 1;
        }
        case 'O': {
            expect(length == opacity_length);
            float opacity;
            SDL_GetWindowOpacity(engine::window(), &opacity);
            return 1;
        }
        case 'A': {
            expect(length == always_on_top_length);
            lua_pushboolean(L, is_always_on_top);
            return 1;
        }
        case 'B': {
            expect(length == borderless_length);
            lua_pushboolean(L, is_borderless);
            return 1;
        }
    }
    return 0;
}
static int newindex(lua_State* L) {
    size_t length;
    const char key = *luaL_checklstring(L, 2, &length);
    switch (key) {
        case 'S': {
            expect(length == size_length);
            auto& new_size = check<builtin::vector2>(L, 3);
            SDL_SetWindowSize(window(), static_cast<int>(new_size[0]), static_cast<int>(new_size[1]));
            return 0;
        }
        case 'P': {
            expect(length == position_length);
            auto& pos = check<builtin::vector2>(L, 3);
            SDL_SetWindowPosition(window(), static_cast<int>(pos[0]), static_cast<int>(pos[1]));
            return 0;
        }
        case 'T': {
            expect(length == title_length);
            SDL_SetWindowTitle(window(), luaL_checkstring(L, 3));
            return 0;
        }
        case 'O': {
            expect(length == opacity_length);
            const float opacity = luaL_checknumber(L, 3);
            SDL_SetWindowOpacity(window(), opacity);
            return 1;
        }
        case 'A': {
            expect(length == always_on_top_length);
            const bool b = luaL_checkboolean(L, 3);
            is_always_on_top = b;
            SDL_SetWindowAlwaysOnTop(window(), static_cast<SDL_bool>(b));
            return 0;
        }
        case 'B': {
            expect(length == borderless_length);
            const bool b = luaL_checkboolean(L, 3);
            is_borderless = not b;
            SDL_SetWindowBordered(window(), static_cast<SDL_bool>(is_borderless));
            return 0;
        }
    }
    return 0;
}
static int size(lua_State* L) {
    int width, height;
    SDL_GetWindowSize(window(), &width, &height);
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}
static int set_size(lua_State* L) {
    SDL_SetWindowSize(window(), luaL_checkinteger(L, 1), luaL_checkinteger(L, 2));
    return 0;
}
static int maximize(lua_State* L) {
    SDL_MaximizeWindow(window());
    return 0;
}
static int minimize(lua_State* L) {
    SDL_MinimizeWindow(window());
    return 0;
}

static int set_title(lua_State* L) {
    SDL_SetWindowTitle(window(), luaL_checkstring(L, 1));
    return 0;
}
static int title(lua_State* L) {
    lua_pushstring(L, SDL_GetWindowTitle(window()));
    return 1;
}

static int set_always_on_top(lua_State* L) {
    SDL_SetWindowAlwaysOnTop(window(), SDL_bool(luaL_checkboolean(L, 1)));
    return 0;
}
static int set_borderless(lua_State* L) {
    SDL_SetWindowBordered(window(), SDL_bool(not luaL_checkboolean(L, 1)));
    return 0;
}
static int set_position(lua_State* L) {
    SDL_SetWindowPosition(window(), luaL_checkinteger(L, 1), luaL_checkinteger(L, 2));
    return 0;
}
static int position(lua_State* L) {
    int x, y;
    SDL_GetWindowPosition(window(), &x, &y);
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    return 2;
}
static int set_opacity(lua_State* L) {
    SDL_SetWindowOpacity(window(), static_cast<float>(luaL_checknumber(L, 1)));
    return 0;
}

static int opacity(lua_State* L) {
    float opacity;
    SDL_GetWindowOpacity(window(), &opacity);
    lua_pushnumber(L, opacity);
    return 1;
}
static int set_icon(lua_State* L) {
    const char* file = lua_tostring(L, 1);
    SDL_Surface* icon = IMG_Load(file);
    if (not icon) {
        luaL_error(L, "failed to load icon '%s'", file);
        return 0;
    }
    SDL_SetWindowIcon(window(), icon);
    return 0;
}
static int focus(lua_State* L) {
    SDL_RaiseWindow(window());
    return 0;
}

namespace builtin {
void handle_window_event(lua_State* L, SDL_WindowEvent& e) {
    switch (e.event) {
        case SDL_WINDOWEVENT_SHOWN:
            shown->fire(0);
        break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            focus_gained->fire(0);
        break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            focus_lost->fire(0);
        break;
        case SDL_WINDOWEVENT_EXPOSED:
            exposed->fire(0);
        break;
        case SDL_WINDOWEVENT_RESIZED:
            create<vector2>(L) = vec2i{e.data1, e.data2};
            resized->fire(1);
        break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            create<vector2>(L) = vec2i{e.data1, e.data2};
            size_changed->fire(1);
        break;
        case SDL_WINDOWEVENT_ENTER:
            mouse_entered->fire(0);
        break;
        case SDL_WINDOWEVENT_LEAVE:
            mouse_left->fire(0);
        break;
        case SDL_WINDOWEVENT_MOVED:
            create<vector2>(L) = vec2i{e.data1, e.data2};
            position_changed->fire(1);
        break;
        case SDL_WINDOWEVENT_MAXIMIZED:
            maximized->fire(0);
        break;
        case SDL_WINDOWEVENT_MINIMIZED:
            minimized->fire(0);
        case SDL_WINDOWEVENT_RESTORED:
            restored->fire(0);
        break;
        case SDL_WINDOWEVENT_CLOSE:
            closing->fire(0);
        break;
    }
}
int window_module(lua_State *L) {
    const luaL_Reg lib[] = {
        {"Maximize", maximize},
        {"Minimize", minimize},
        {"SetIcon", set_icon},
        {"Focus", focus},
        {nullptr, nullptr}
    };
    lua_newtable(L);
    luaL_register(L, nullptr, lib);
    register_event(L, hidden, "Hidden");
    register_event(L, shown, "Shown");
    register_event(L, resized, "Resized");
    register_event(L, mouse_entered, "MouseEnter");
    register_event(L, mouse_left, "MouseLeave");
    register_event(L, size_changed, "SizeChanged");
    register_event(L, position_changed, "PositionChanged");
    register_event(L, focus_gained, "FocusGained");
    register_event(L, focus_lost, "FocusLost");
    register_event(L, restored, "Restored");
    register_event(L, closing, "Closing");
    register_event(L, exposed, "Exposed");
    register_event(L, maximized, "Maximized");
    register_event(L, minimized, "Minimized");
    if (luaL_newmetatable(L, "builtin_window_module")) {
        const luaL_Reg meta[] = {
            {metamethod::index, index},
            {metamethod::newindex, newindex},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
    }
    lua_setmetatable(L, -2);
    return 1;
}
}
