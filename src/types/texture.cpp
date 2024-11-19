#include "builtin.h"
#include "lua_util.h"
#include "engine.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
static constexpr const char* type = "Texture";
using builtin::texture;
using builtin::color;
using engine::expect;
using builtin::font;
using builtin::color;

static constexpr size_t color_length{std::string("Color").length()};
static constexpr size_t blend_mode_length{std::string("BlendMode").length()};
static int texture_index(lua_State* L) {
    auto& r = check<texture>(L, 1);
    const std::string_view key = luaL_checkstring(L, 2);
    switch (key.at(0)) {
        case 'W':
            lua_pushinteger(L, r.w);
            return 1;
        case 'H':
            lua_pushinteger(L, r.h);
            return 1;
        case 'C':
            color col;
            SDL_GetTextureColorMod(r.ptr.get(), &col.r, &col.g, &col.b);
            SDL_GetTextureAlphaMod(r.ptr.get(), &col.a);
            create<color>(L, std::move(col));
            return 1;
        case 'B':
            SDL_BlendMode bm;
            SDL_GetTextureBlendMode(r.ptr.get(), &bm);
            lua_pushstring(L, blendmode_to_string(bm));
            return 1;
        default: return lua_err::invalid_member(L, type);
    }
}
static int texture_newindex(lua_State* L) {
    auto& r = check<texture>(L, 1);
    const std::string_view key = luaL_checkstring(L, 2);
    switch (key.at(0)) {
        case 'W': {
            const int v = luaL_checkinteger(L, 3);
            r.w = v;
            return 0;
        }
        case 'H': {
            const int v = luaL_checkinteger(L, 3);
            r.h = v;
            return 0;
        }
        case 'C': {
            const auto& new_color = check<color>(L, 3);
            SDL_SetTextureColorMod(r.ptr.get(), new_color.r, new_color.g, new_color.b);
            SDL_SetTextureAlphaMod(r.ptr.get(), new_color.a);
            return 0;
        }
        case 'B': {
            SDL_BlendMode bm = string_to_blendmode(luaL_checkstring(L, 3));
            SDL_SetTextureBlendMode(r.ptr.get(), bm);
            return 0;
        }
        default:
            return lua_err::invalid_member(L, type);
    }
}
static int texture_ctor_call(lua_State* L) {
    auto path = resolve_path_type(L, 2);
    if (not path) {
        return lua_err::invalid_argument(L, 2, "Path | string");
    }
    SDL_Surface* surface = IMG_Load(path->c_str());
    if (not surface) {
        luaL_error(L, SDL_GetError());
        return 0;
    }
    scope_guard d([&surface] {SDL_FreeSurface(surface);});
    SDL_Texture* txt = SDL_CreateTextureFromSurface(engine::renderer(), surface);
    if (not txt) {
        luaL_error(L, SDL_GetError());
        return 0;
    }
    create<texture>(L, texture{
        builtin::texture_ptr(txt, SDL_DestroyTexture),
        surface->w,
        surface->h
    });
    return 1;
}
static int texture_ctor_from_string(lua_State* L) {
    const char* string = luaL_checkstring(L, 1);
    font& ft = is_type<font>(L, 2) ?
        check<font>(L, 2) : engine::default_font();
    const color& col = is_type<color>(L, 3) ?
        check<color>(L, 3) : color{0xff, 0xff, 0xff, 0xff};
    SDL_Surface* surface = TTF_RenderText_Blended(
        ft.ptr.get(), string, col);
    if (not surface) {
        luaL_error(L, SDL_GetError());
        return 0;
    }
    scope_guard d([&surface]{SDL_FreeSurface(surface);});
    SDL_Texture* txt = SDL_CreateTextureFromSurface(engine::renderer(), surface);
    if (not txt) {
        luaL_error(L, SDL_GetError());
        return 0;
    }
    create<texture>(L, texture{
        builtin::texture_ptr(txt, SDL_DestroyTexture),
        surface->w,
        surface->h
    });
    return 1;
}
namespace builtin {
void register_texture_type(lua_State* L) {
    if (luaL_newmetatable(L, metatable_name<texture>())) {
        const luaL_Reg meta[] = {
            {metamethod::index, texture_index},
            {metamethod::newindex, texture_newindex},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
        lua_pushstring(L, tname::texture);
        lua_setfield(L, -2, metamethod::type);
    }
    lua_pop(L, 1);
    constexpr auto ctor_metatable_name = "texture_ctor_metatable";
    if (luaL_newmetatable(L, ctor_metatable_name)) {
        const luaL_Reg meta[] = {
            {metamethod::call, texture_ctor_call},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
    }
    lua_pop(L, 1);
    const luaL_Reg ctors[] = {
        {"FromString", texture_ctor_from_string},
        {nullptr, nullptr}
    };
    lua_newtable(L);
    luaL_register(L, nullptr, ctors);
    luaL_getmetatable(L, ctor_metatable_name);
    lua_setmetatable(L, -2);
    lua_setglobal(L, type);
}
}