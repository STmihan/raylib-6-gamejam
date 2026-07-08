#ifndef DATA_VEC_H
#define DATA_VEC_H

namespace data {

struct Vec2 {
    float x;
    float y;
};

inline Vec2 Add(Vec2 a, Vec2 b) { return { a.x + b.x, a.y + b.y }; }
inline Vec2 Scale(Vec2 a, float s) { return { a.x * s, a.y * s }; }
inline Vec2 Lerp(Vec2 a, Vec2 b, float t) { return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t }; }

}

#endif
