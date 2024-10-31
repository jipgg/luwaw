#include "builtin/definitions.h"
#include "builtin/utility.h"
#include "builtin/method_atom.h"
#include "builtin/typedefs.h"
#include <SDL.h>
#include <SDL_image.h>

using method = builtin::method_atom;
namespace bi = builtin;
namespace cm = common;
static SDL_Renderer* renderer() {
    return SDL_GetRenderer(engine::core::window());
}
static int open_font(lua_State* L) {
    auto path = bi::resolve_path_type(L, 1);
    if (not path) {
        return 0;
    }
    TTF_Font* font = TTF_OpenFont(path->c_str(), luaL_checkinteger(L, 2));
    if (not font) return 0;
    bi::create<bi::font_t>(L, font, TTF_CloseFont);
    return 1;
}
static int load_image(lua_State* L) {
    auto path = bi::resolve_path_type(L, 1);
    if (not path) {
        return 0;
    }
    SDL_Surface* surface = IMG_Load(path->c_str());
    if (not surface) {
        cm::printerr("surface is not");
        return 0;
    } else {
        cm::deferred _([&surface] {
            SDL_FreeSurface(surface);
            surface = nullptr;
            common::print("destroyed surface");
        });
        SDL_Texture* text = SDL_CreateTextureFromSurface(SDL_GetRenderer(engine::core::window()), surface);
        if (not text) {
            common::printerr("failed to create texture");
            return 0;
        }
        bi::create<bi::texture_t>(L, text, SDL_DestroyTexture);
        common::print("created texture");
        lua_pushinteger(L, surface->w);
        lua_pushinteger(L, surface->h);
        return 3;
    }
}
static int load_text(lua_State* L) {
    auto& font = bi::check<bi::font_t>(L, 1);
    const char* text = luaL_checkstring(L, 2);
    SDL_Color fg{0, 0, 0, 0xff};
    if (not lua_isnone(L, 3)) {
        auto& r = bi::check<bi::coloru32_t>(L, 3);
        fg.r = r.red();
        fg.a = r.alpha();
        fg.g = r.green();
        fg.b = r.blue();
    }
    SDL_Surface* surface = TTF_RenderText_Blended(font.get(), text, fg);
    if (not surface) {
        return 0;
    }
    cm::deferred _([&surface] {SDL_FreeSurface(surface);});
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer(), surface);
    if (not texture) return 0;
    bi::create<bi::texture_t>(L, texture, SDL_DestroyTexture);
    lua_pushinteger(L, surface->w);
    lua_pushinteger(L, surface->h);
    return 3;
}

static int set_draw_color(lua_State* L) {
    auto& color = bi::check<bi::coloru32_t>(L, 1);
    SDL_SetRenderDrawColor(renderer(), color.red(), color.green(), color.blue(), color.alpha());
    return 0;
}
static int draw_rect(lua_State* L) {
    if (bi::is_type<bi::recti_t>(L, 1)) {
        auto& rect = bi::check<bi::recti_t>(L, 1);
        SDL_RenderDrawRect(renderer(), &rect);
        return 0;
    } else if (bi::is_type<bi::rect_t>(L, 1)) {
        auto& rect = bi::check<bi::rect_t>(L, 1);
        SDL_Rect dummy{
            static_cast<int>(rect.x),
            static_cast<int>(rect.y),
            static_cast<int>(rect.w),
            static_cast<int>(rect.h)
        };
        SDL_RenderDrawRect(renderer(), &dummy);
        return 0;
    }
    return 0;
}
static std::vector<SDL_Point> point_buffer;
static std::vector<SDL_Rect> rect_buffer;
static int draw_rects(lua_State* L) {
    const int top = lua_gettop(L);
    rect_buffer.resize(0);
    rect_buffer.reserve(top);
    for (int i{1}; i <= top; ++i) {
        rect_buffer.emplace_back(bi::check<SDL_Rect>(L, i));
    }
    SDL_RenderDrawRects(renderer(), rect_buffer.data(), rect_buffer.size());
    return 0;
}
static int draw_points(lua_State* L) {
    const int top = lua_gettop(L);
    point_buffer.resize(0);
    point_buffer.reserve(top);
    for (int i{1}; i <= top; ++i) {
        auto& p = bi::check<bi::vec2i_t>(L, i);
        point_buffer.emplace_back(SDL_Point{p.at(0), p.at(1)});
    }
    SDL_RenderDrawPoints(renderer(), point_buffer.data(), point_buffer.size());
    return 0;
}
static int fill_rect(lua_State* L) {
    if (bi::is_type<bi::recti_t>(L, 1)) {
        auto& rect = bi::check<bi::recti_t>(L, 1);
        SDL_RenderFillRect(renderer(), &rect);
        return 0;
    } else if (bi::is_type<bi::rect_t>(L, 1)) {
        auto& rect = bi::check<bi::rect_t>(L, 1);
        SDL_Rect dummy{
            static_cast<int>(rect.x),
            static_cast<int>(rect.y),
            static_cast<int>(rect.w),
            static_cast<int>(rect.h)
        };
        SDL_RenderFillRect(renderer(), &dummy);
        return 0;
    }
    return 0;
}
static int fill_rects(lua_State* L) {
    const int top = lua_gettop(L);
    rect_buffer.resize(0);
    rect_buffer.reserve(top);
    for (int i{1}; i <= top; ++i) {
        rect_buffer.emplace_back(bi::check<SDL_Rect>(L, i));
    }
    SDL_RenderFillRects(renderer(), rect_buffer.data(), rect_buffer.size());
    return 0;
}
static int draw_line(lua_State* L) {
    auto& t0 = bi::check<bi::vec2i_t>(L, 1);
    auto& t1 = bi::check<bi::vec2i_t>(L, 2);
    SDL_RenderDrawLine(renderer(), t0.at(0), t0.at(1), t1.at(0), t1.at(1));
    return 0;
}
static int draw_point(lua_State* L) {
    auto& p = bi::check<bi::vec2i_t>(L, 1);
    SDL_RenderDrawPoint(renderer(), p.at(0), p.at(1));
    return 0;
}
static int draw_lines(lua_State* L) {
    const int top = lua_gettop(L);
    point_buffer.resize(0);
    point_buffer.reserve(top);
    for (int i{1}; i <= top; ++i) {
        auto& p = bi::check<bi::vec2i_t>(L, i);
        point_buffer.emplace_back(SDL_Point{p.at(0), p.at(1)});
    }
    SDL_RenderDrawLines(renderer(), point_buffer.data(), point_buffer.size());
    return 0;
}
static int render_copy(lua_State* L) {
    constexpr int texture_arg = 1;
    constexpr int dest_arg = 2;
    constexpr int src_arg = 3;
    const auto& texture = bi::check<bi::texture_t>(L, texture_arg);
    const bool has_dest = not lua_isnoneornil(L, dest_arg);
    const bool has_src = not lua_isnoneornil(L, src_arg);
    SDL_RenderCopy(
        renderer(),
        texture.get(),
        has_src? &bi::check<bi::recti_t>(L, src_arg):nullptr,
        has_dest ? &bi::check<bi::recti_t>(L, dest_arg):nullptr);
    return 0;
}
int bi::sdl_import_lib(lua_State *L) {
    const luaL_Reg lib[] = {
        {"openFont", open_font},
        {"loadImage", load_image},
        {"loadText", load_text},
        {nullptr, nullptr}
    };
    lua_newtable(L);
    luaL_register(L, nullptr, lib);
    lua_newtable(L);
    const luaL_Reg render_lib[] = {
        {"setDrawColor", set_draw_color},
        {"renderCopy", render_copy},
        {"drawRect", draw_rect},
        {"drawRects", draw_rects},
        {"fillRect", fill_rect},
        {"fillRects", fill_rects},
        {"drawLine", draw_line},
        {"drawLines", draw_lines},
        {"drawPoint", draw_point},
        {"drawPoints", draw_points},
        {nullptr, nullptr}
    };
    luaL_register(L, nullptr, render_lib);
    lua_setfield(L, -2, "Render");
    return 1;
}

