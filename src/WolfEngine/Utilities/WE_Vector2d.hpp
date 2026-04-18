#pragma once
#include <math.h>

struct Vec2 {
    float x, y;

    Vec2() : x(0), y(0) {}
    Vec2(float x, float y) : x(x), y(y) {}

    Vec2 operator+(const Vec2& b) const { return {x + b.x, y + b.y}; }
    Vec2 operator-(const Vec2& b) const { return {x - b.x, y - b.y}; }
    Vec2 operator*(const Vec2& b) const { return {x * b.x, y * b.y}; }
    Vec2 operator/(const Vec2& b) const { return {x / b.x, y / b.y}; }
    Vec2 operator*(float s)       const { return {x * s,   y * s};   }
    Vec2 operator-()              const { return {-x, -y};            }
    Vec2& operator+=(const Vec2& b) { x += b.x; y += b.y; return *this; }
    Vec2& operator-=(const Vec2& b) { x -= b.x; y -= b.y; return *this; }
    Vec2& operator*=(float s)       { x *= s;   y *= s;   return *this; }
    bool operator==(const Vec2& b) const { return x == b.x && y == b.y; }

    float dot(const Vec2& b)  const { return x * b.x + y * b.y; }
    float lenSq()             const { return x * x + y * y; }
    float len()               const { return sqrtf(lenSq()); }
    float angle()             const { return atan2f(y, x); }

    Vec2 normalized() const {
        float l = len();
        return l == 0.0f ? Vec2{} : *this * (1.0f / l);
    }

    Vec2 clampLen(float max_len) const {
        float l = len();
        return l > max_len ? normalized() * max_len : *this;
    }

    static Vec2 zero()              { return {0,  0};  }
    static Vec2 one()               { return {1,  1};  }
    static Vec2 up()                { return {0, -1};  }
    static Vec2 down()              { return {0,  1};  }
    static Vec2 left()              { return {-1, 0};  }
    static Vec2 right()             { return {1,  0};  }
    static Vec2 fromAngle(float r)  { return {cosf(r), sinf(r)}; }
};

inline Vec2 operator*(float s, const Vec2& v) { return v * s; }
inline Vec2 lerp(Vec2 a, Vec2 b, float t) { return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t}; }
inline float dist(Vec2 a, Vec2 b)   { return (a - b).len(); }
inline float distSq(Vec2 a, Vec2 b) { return (a - b).lenSq(); }

// ------------------------------------------------------------------ //

struct IntVec2 {
    int x, y;

    IntVec2() : x(0), y(0) {}
    IntVec2(int x, int y) : x(x), y(y) {}

    IntVec2 operator+(const IntVec2& b) const { return {x + b.x, y + b.y}; }
    IntVec2 operator-(const IntVec2& b) const { return {x - b.x, y - b.y}; }
    IntVec2 operator*(const IntVec2& b) const { return {x * b.x, y * b.y}; }
    IntVec2 operator*(int s)            const { return {x * s,   y * s};   }
    IntVec2 operator-()                 const { return {-x, -y};           }
    bool operator==(const IntVec2& b)   const { return x == b.x && y == b.y; }

    int   dot(const IntVec2& b)   const { return x * b.x + y * b.y; }
    int   lenSq()                 const { return x * x + y * y; }
    float len()                   const { return sqrtf((float)lenSq()); }

    int manhattan(const IntVec2& b) const {
        int dx = x - b.x; if (dx < 0) dx = -dx;
        int dy = y - b.y; if (dy < 0) dy = -dy;
        return dx + dy;
    }

    bool inBounds(int w, int h)   const { return x >= 0 && y >= 0 && x < w && y < h; }

    IntVec2 clamped(int w, int h) const {
        return {
            x < 0 ? 0 : x >= w ? w - 1 : x,
            y < 0 ? 0 : y >= h ? h - 1 : y
        };
    }

    Vec2    toVec2()  const { return {(float)x, (float)y}; }
    static IntVec2 zero()   { return {0, 0}; }
};

inline IntVec2 lerp(IntVec2 a, IntVec2 b, float t) {
    return {
        (int)roundf((float)a.x + (float)(b.x - a.x) * t),
        (int)roundf((float)a.y + (float)(b.y - a.y) * t)
    };
}

inline float dist(IntVec2 a, IntVec2 b)   { return (a - b).len(); }
inline int   distSq(IntVec2 a, IntVec2 b) { return (a - b).lenSq(); }

// Conversion
inline IntVec2 toPixel(Vec2 a)      { return {(int)roundf(a.x), (int)roundf(a.y)}; }
inline IntVec2 toPixelTrunc(Vec2 a) { return {(int)a.x, (int)a.y}; }