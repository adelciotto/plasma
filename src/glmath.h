#ifndef GLMATH_H_INCLUDED
#define GLMATH_H_INCLUDED

#include <math.h>

#define VEC3_ZERO_INIT                                                         \
    { 0.0f, 0.0f, 0.0f }

#define MAT4_IDENTITY_INIT                                                     \
    {                                                                          \
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,      \
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f                                       \
    }

#define MAT4_ZERO_INIT                                                         \
    {                                                                          \
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,      \
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f                                       \
    }

typedef float Vec3[3];
typedef float Mat4[16];

static inline void Vec3Sub(Vec3 lhs, Vec3 rhs, Vec3 out) {
    out[0] = lhs[0] - rhs[0];
    out[1] = lhs[1] - rhs[1];
    out[2] = lhs[2] - rhs[2];
}

static inline void Vec3Norm(Vec3 out) {
    float length = sqrtf(out[0] * out[0] + out[1] * out[1] + out[2] * out[2]);

    out[0] /= length;
    out[1] /= length;
    out[2] /= length;
}

static inline void Vec3Cross(Vec3 lhs, Vec3 rhs, Vec3 out) {
    out[0] = lhs[1] * rhs[2] - lhs[2] * rhs[1];
    out[1] = lhs[2] * rhs[0] - lhs[0] * rhs[2];
    out[2] = lhs[0] * rhs[1] - lhs[1] * rhs[0];
}

static inline void Vec3CrossNorm(Vec3 lhs, Vec3 rhs, Vec3 out) {
    Vec3Cross(lhs, rhs, out);
    Vec3Norm(out);
}

static inline float Vec3Dot(Vec3 lhs, Vec3 rhs) {
    return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2];
}

static inline void Mat4Mul(Mat4 lhs, Mat4 rhs, Mat4 out) {
    out[0] =
        lhs[0] * rhs[0] + lhs[1] * rhs[4] + lhs[2] * rhs[8] + lhs[3] * rhs[12];
    out[1] =
        lhs[0] * rhs[1] + lhs[1] * rhs[5] + lhs[2] * rhs[9] + lhs[3] * rhs[13];
    out[2] =
        lhs[0] * rhs[2] + lhs[1] * rhs[6] + lhs[2] * rhs[10] + lhs[3] * rhs[14];
    out[3] =
        lhs[0] * rhs[3] + lhs[1] * rhs[7] + lhs[2] * rhs[11] + lhs[3] * rhs[15];

    out[4] =
        lhs[4] * rhs[0] + lhs[5] * rhs[4] + lhs[6] * rhs[8] + lhs[7] * rhs[12];
    out[5] =
        lhs[4] * rhs[1] + lhs[5] * rhs[5] + lhs[6] * rhs[9] + lhs[7] * rhs[13];
    out[6] =
        lhs[4] * rhs[2] + lhs[5] * rhs[6] + lhs[6] * rhs[10] + lhs[7] * rhs[14];
    out[7] =
        lhs[4] * rhs[3] + lhs[5] * rhs[7] + lhs[6] * rhs[11] + lhs[7] * rhs[15];

    out[8] = lhs[8] * rhs[0] + lhs[9] * rhs[4] + lhs[10] * rhs[8] +
             lhs[11] * rhs[12];
    out[9] = lhs[8] * rhs[1] + lhs[9] * rhs[5] + lhs[10] * rhs[9] +
             lhs[11] * rhs[13];
    out[10] = lhs[8] * rhs[2] + lhs[9] * rhs[6] + lhs[10] * rhs[10] +
              lhs[11] * rhs[14];
    out[11] = lhs[8] * rhs[3] + lhs[9] * rhs[7] + lhs[10] * rhs[11] +
              lhs[11] * rhs[15];

    out[12] = lhs[12] * rhs[0] + lhs[13] * rhs[4] + lhs[14] * rhs[8] +
              lhs[15] * rhs[12];
    out[13] = lhs[12] * rhs[1] + lhs[13] * rhs[5] + lhs[14] * rhs[9] +
              lhs[15] * rhs[13];
    out[14] = lhs[12] * rhs[2] + lhs[13] * rhs[6] + lhs[14] * rhs[10] +
              lhs[15] * rhs[14];
    out[15] = lhs[12] * rhs[3] + lhs[13] * rhs[7] + lhs[14] * rhs[11] +
              lhs[15] * rhs[15];
}

static inline void Mat4RotateZ(Mat4 in, Mat4 out, float angle) {
    Mat4 transform = MAT4_IDENTITY_INIT;

    float c, s;
    c = cosf(angle);
    s = sinf(angle);

    transform[0] = c;
    transform[1] = s;
    transform[4] = -s;
    transform[5] = c;

    Mat4Mul(in, transform, out);
}

static inline void Mat4RotateX(Mat4 in, Mat4 out, float angle) {
    Mat4 transform = MAT4_IDENTITY_INIT;

    float c, s;
    c = cosf(angle);
    s = sinf(angle);

    transform[5] = c;
    transform[6] = s;
    transform[9] = -s;
    transform[10] = c;

    Mat4Mul(in, transform, out);
}

static inline void Mat4RotateY(Mat4 in, Mat4 out, float angle) {
    Mat4 transform = MAT4_IDENTITY_INIT;

    float c, s;
    c = cosf(angle);
    s = sinf(angle);

    transform[0] = c;
    transform[2] = -s;
    transform[8] = s;
    transform[10] = c;

    Mat4Mul(in, transform, out);
}

static inline void Mat4LookAt(Vec3 eye, Vec3 center, Vec3 up, Mat4 out) {
    Vec3 f, u, s;

    Vec3Sub(center, eye, f);
    Vec3Norm(f);

    Vec3CrossNorm(f, up, s);
    Vec3Cross(s, f, u);

    out[0] = s[0];
    out[1] = u[0];
    out[2] = -f[0];
    out[4] = s[1];
    out[5] = u[1];
    out[6] = -f[1];
    out[8] = s[2];
    out[9] = u[2];
    out[10] = -f[2];
    out[12] = -Vec3Dot(s, eye);
    out[13] = -Vec3Dot(u, eye);
    out[14] = Vec3Dot(f, eye);
    out[3] = out[7] = out[11] = 0.0f;
    out[15] = 1.0f;
}

static inline void Mat4Perspective(float fovy, float aspect, float nearVal,
                            float farVal, Mat4 out) {
    float f, fn;

    f = 1.0f / tanf(fovy * 0.5f);
    fn = 1.0f / (nearVal - farVal);

    out[0] = f / aspect;
    out[5] = f;
    out[10] = (nearVal + farVal) * fn;
    out[11] = -1.0f;
    out[14] = 2.0f * nearVal * farVal * fn;
}

#endif
