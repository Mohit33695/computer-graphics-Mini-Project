#pragma once

#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

inline float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

inline float radiansf(float degrees) {
    return degrees * (float)M_PI / 180.0f;
}

inline float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

inline float smoothstepf(float t) {
    t = clampf(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

struct Vec2 {
    float x, y;
    Vec2() : x(0), y(0) {}
    Vec2(float x_, float y_) : x(x_), y(y_) {}
    Vec2 operator+(const Vec2& o) const { return Vec2(x + o.x, y + o.y); }
    Vec2 operator-(const Vec2& o) const { return Vec2(x - o.x, y - o.y); }
    Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
};

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vec3 operator+(const Vec3& o) const { return Vec3(x + o.x, y + o.y, z + o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x - o.x, y - o.y, z - o.z); }
    Vec3 operator-() const { return Vec3(-x, -y, -z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    Vec3 operator*(const Vec3& o) const { return Vec3(x * o.x, y * o.y, z * o.z); }
    Vec3 operator/(float s) const { return Vec3(x / s, y / s, z / s); }
    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    Vec3& operator-=(const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
};

inline float dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return Vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

inline float length(const Vec3& v) {
    return std::sqrt(dot(v, v));
}

inline Vec3 normalize(const Vec3& v) {
    float len = length(v);
    if (len < 1e-8f) return Vec3(0, 0, 0);
    return v / len;
}

inline Vec3 reflectVec(const Vec3& incident, const Vec3& normal) {
    return incident - normal * (2.0f * dot(normal, incident));
}

inline Vec3 clamp01(const Vec3& c) {
    return Vec3(clampf(c.x, 0.0f, 1.0f),
                clampf(c.y, 0.0f, 1.0f),
                clampf(c.z, 0.0f, 1.0f));
}

struct Vec4 {
    float x, y, z, w;
    Vec4() : x(0), y(0), z(0), w(0) {}
    Vec4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
};

// Column-major 4x4 matrix, compatible with glLoadMatrixf.
struct Mat4 {
    float m[16];

    Mat4() {
        identity();
    }

    void identity() {
        for (int i = 0; i < 16; ++i) m[i] = 0.0f;
        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }

    static Mat4 identityMat() {
        return Mat4();
    }

    static Mat4 translation(const Vec3& t) {
        Mat4 r;
        r.m[12] = t.x;
        r.m[13] = t.y;
        r.m[14] = t.z;
        return r;
    }

    static Mat4 scaling(const Vec3& s) {
        Mat4 r;
        r.m[0] = s.x;
        r.m[5] = s.y;
        r.m[10] = s.z;
        return r;
    }

    static Mat4 rotationX(float a) {
        Mat4 r;
        float c = std::cos(a), s = std::sin(a);
        r.m[5] = c;  r.m[9] = -s;
        r.m[6] = s;  r.m[10] = c;
        return r;
    }

    static Mat4 rotationY(float a) {
        Mat4 r;
        float c = std::cos(a), s = std::sin(a);
        r.m[0] = c;  r.m[8] = s;
        r.m[2] = -s; r.m[10] = c;
        return r;
    }

    static Mat4 rotationZ(float a) {
        Mat4 r;
        float c = std::cos(a), s = std::sin(a);
        r.m[0] = c;  r.m[4] = -s;
        r.m[1] = s;  r.m[5] = c;
        return r;
    }

    static Mat4 perspective(float fovYRadians, float aspect, float nearZ, float farZ) {
        Mat4 r;
        for (int i = 0; i < 16; ++i) r.m[i] = 0.0f;
        float f = 1.0f / std::tan(fovYRadians * 0.5f);
        r.m[0] = f / aspect;
        r.m[5] = f;
        r.m[10] = (farZ + nearZ) / (nearZ - farZ);
        r.m[11] = -1.0f;
        r.m[14] = (2.0f * farZ * nearZ) / (nearZ - farZ);
        return r;
    }

    static Mat4 orthographic(float left, float right, float bottom, float top, float nearZ, float farZ) {
        Mat4 r;
        r.m[0] = 2.0f / (right - left);
        r.m[5] = 2.0f / (top - bottom);
        r.m[10] = -2.0f / (farZ - nearZ);
        r.m[12] = -(right + left) / (right - left);
        r.m[13] = -(top + bottom) / (top - bottom);
        r.m[14] = -(farZ + nearZ) / (farZ - nearZ);
        return r;
    }

    static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
        Vec3 f = normalize(center - eye);
        Vec3 s = normalize(cross(f, up));
        Vec3 u = cross(s, f);

        Mat4 r;
        r.m[0] = s.x;   r.m[4] = s.y;   r.m[8]  = s.z;   r.m[12] = -dot(s, eye);
        r.m[1] = u.x;   r.m[5] = u.y;   r.m[9]  = u.z;   r.m[13] = -dot(u, eye);
        r.m[2] = -f.x;  r.m[6] = -f.y;  r.m[10] = -f.z;  r.m[14] =  dot(f, eye);
        r.m[3] = 0.0f;  r.m[7] = 0.0f;  r.m[11] = 0.0f;  r.m[15] =  1.0f;
        return r;
    }

    Mat4 operator*(const Mat4& o) const {
        Mat4 r;
        for (int c = 0; c < 4; ++c) {
            for (int row = 0; row < 4; ++row) {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    sum += m[k * 4 + row] * o.m[c * 4 + k];
                }
                r.m[c * 4 + row] = sum;
            }
        }
        return r;
    }

    Vec4 operator*(const Vec4& v) const {
        return Vec4(
            m[0] * v.x + m[4] * v.y + m[8]  * v.z + m[12] * v.w,
            m[1] * v.x + m[5] * v.y + m[9]  * v.z + m[13] * v.w,
            m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w,
            m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w
        );
    }

    Vec3 transformPoint(const Vec3& p) const {
        Vec4 r = (*this) * Vec4(p.x, p.y, p.z, 1.0f);
        if (std::fabs(r.w) > 1e-8f) {
            return Vec3(r.x / r.w, r.y / r.w, r.z / r.w);
        }
        return Vec3(r.x, r.y, r.z);
    }

    Vec3 transformDirection(const Vec3& d) const {
        Vec4 r = (*this) * Vec4(d.x, d.y, d.z, 0.0f);
        return Vec3(r.x, r.y, r.z);
    }
};

struct Aabb {
    Vec3 mn, mx;
    Aabb()
        : mn( 1e30f,  1e30f,  1e30f),
          mx(-1e30f, -1e30f, -1e30f) {}

    void expand(const Vec3& p) {
        mn.x = std::min(mn.x, p.x);
        mn.y = std::min(mn.y, p.y);
        mn.z = std::min(mn.z, p.z);
        mx.x = std::max(mx.x, p.x);
        mx.y = std::max(mx.y, p.y);
        mx.z = std::max(mx.z, p.z);
    }

    void expand(const Aabb& b) {
        expand(b.mn);
        expand(b.mx);
    }

    Vec3 center() const {
        return (mn + mx) * 0.5f;
    }
};

inline Aabb transformAabb(const Aabb& b, const Mat4& m) {
    Aabb out;
    Vec3 corners[8] = {
        Vec3(b.mn.x, b.mn.y, b.mn.z),
        Vec3(b.mx.x, b.mn.y, b.mn.z),
        Vec3(b.mn.x, b.mx.y, b.mn.z),
        Vec3(b.mx.x, b.mx.y, b.mn.z),
        Vec3(b.mn.x, b.mn.y, b.mx.z),
        Vec3(b.mx.x, b.mn.y, b.mx.z),
        Vec3(b.mn.x, b.mx.y, b.mx.z),
        Vec3(b.mx.x, b.mx.y, b.mx.z)
    };
    for (int i = 0; i < 8; ++i) out.expand(m.transformPoint(corners[i]));
    return out;
}

inline bool rayAabb(const Vec3& origin, const Vec3& dir, const Aabb& b, float& tHit) {
    float tmin = -1e30f;
    float tmax =  1e30f;

    for (int axis = 0; axis < 3; ++axis) {
        float o = (axis == 0) ? origin.x : (axis == 1 ? origin.y : origin.z);
        float d = (axis == 0) ? dir.x    : (axis == 1 ? dir.y    : dir.z);
        float lo = (axis == 0) ? b.mn.x  : (axis == 1 ? b.mn.y  : b.mn.z);
        float hi = (axis == 0) ? b.mx.x  : (axis == 1 ? b.mx.y  : b.mx.z);

        if (std::fabs(d) < 1e-8f) {
            if (o < lo || o > hi) return false;
        } else {
            float t1 = (lo - o) / d;
            float t2 = (hi - o) / d;
            if (t1 > t2) std::swap(t1, t2);
            if (t1 > tmin) tmin = t1;
            if (t2 < tmax) tmax = t2;
            if (tmin > tmax) return false;
        }
    }

    if (tmax < 0.0f) return false;
    tHit = (tmin >= 0.0f) ? tmin : tmax;
    return true;
}
