#include "util.h"
#include "engine.h"
namespace util {
std::array<float, 8> get_quad_transform_raw(const vec2f& dim, const mat3f& transform) {
    vec3f top_left = transform * vec3f{0, 0, 1};
    vec3f top_right = transform * vec3f{dim[0], 0, 1};
    vec3f bottom_right = transform * vec3f{dim[0], dim[1], 1};
    vec3f bottom_left = transform * vec3f{0, dim[1], 1};
    return {
        top_left[0], top_left[1],
        top_right[0], top_right[1],
        bottom_right[0], bottom_right[1],
        bottom_left[0], bottom_left[1],
    };
}
std::array<float, 8> get_quad_transform_raw(const SDL_FRect& quad, const mat3f& transform) {
    const vec3f offset{quad.x, quad.y, 0};
    vec3f top_left = transform * (offset + vec3f{0, 0, 1});
    vec3f top_right = transform * (offset + vec3f{quad.w, 0, 1});
    vec3f bottom_right = transform * (offset + vec3f{quad.w, quad.h, 1});
    vec3f bottom_left = transform * (offset + vec3f{0, quad.h, 1});
    return {
        top_left[0], top_left[1],
        top_right[0], top_right[1],
        bottom_right[0], bottom_right[1],
        bottom_left[0], bottom_left[1],
    };
}
SDL_Renderer* renderer() {
    return SDL_GetRenderer(engine::window());
}
}
