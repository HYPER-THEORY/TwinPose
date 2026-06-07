/**
 * Copyright (C) 2021-2023 HYPERTHEORY
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <cmath>
#include <functional>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

/* -------------------------------------------------------------------------- */
/* ---- ink/core/Error.h ---------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class Error {
public:
    using ErrorCallback = std::function<void(const std::string&)>;
    
    /**
     * Sets the specified error description string.
     *
     * \param m error message
     */
    static void set(const std::string& m);
    
    /**
     * Sets the specified error description string.
     *
     * \param l error location
     * \param m error message
     */
    static void set(const std::string& l, const std::string& m);
    
    /**
     * Sets the callback that will be triggered when a new error occurs.
     *
     * \param f error callback function
     */
    static void set_callback(const ErrorCallback& f);
    
private:
    static ErrorCallback callback;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/core/File.h ----------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class File {
public:
    /**
     * Reads the contents from the specified file into a string.
     *
     * \param p the path to the file
     */
    static std::string read(const std::string& p);
    
    /**
     * Writes the contents of the string into the specified file.
     *
     * \param p the path to the file
     * \param c the contents
     */
    static void write(const std::string& p, const std::string& c);
    
    /**
     * Writes the contents of the string into the specified file.
     *
     * \param p the path to the file
     * \param c the contents
     */
    static void write(const std::string& p, const char* c);
    
    /**
     * Appends the contents of the string into the specified file.
     *
     * \param p the path to the file
     * \param c the contents
     */
    static void append(const std::string& p, const std::string& c);
    
    /**
     * Appends the contents of the string into the specified file.
     *
     * \param p the path to the file
     * \param c the contents
     */
    static void append(const std::string& p, const char* c);
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/math/Vector2.h -------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class FVec2 {
public:
    float x = 0;    /**< the X component of the vector */
    float y = 0;    /**< the Y component of the vector */
    
    constexpr FVec2() = default;
    
    constexpr FVec2(float x, float y);
    
    constexpr explicit FVec2(float x);
    
    constexpr FVec2 operator+() const;
    
    constexpr FVec2 operator-() const;
    
    constexpr bool operator==(const FVec2& v) const;
    
    constexpr bool operator!=(const FVec2& v) const;
    
    constexpr FVec2& operator+=(float v);
    
    constexpr FVec2& operator+=(const FVec2& v);
    
    constexpr FVec2& operator-=(float v);
    
    constexpr FVec2& operator-=(const FVec2& v);
    
    constexpr FVec2& operator*=(float v);
    
    constexpr FVec2& operator*=(const FVec2& v);
    
    constexpr FVec2& operator/=(float v);
    
    constexpr FVec2& operator/=(const FVec2& v);
    
    constexpr float dot(const FVec2& v) const;
    
    constexpr float cross(const FVec2& v) const;
    
    inline float magnitude() const;
    
    inline float distance(const FVec2& v) const;
    
    inline FVec2 normalize() const;
    
    inline FVec2 rotate(float a) const;
    
    inline std::string to_string(int p = 2) const;
};

constexpr FVec2 operator+(const FVec2& v1, float v2);

constexpr FVec2 operator+(float v1, const FVec2& v2);

constexpr FVec2 operator+(const FVec2& v1, const FVec2& v2);

constexpr FVec2 operator-(const FVec2& v1, float v2);

constexpr FVec2 operator-(float v1, const FVec2& v2);

constexpr FVec2 operator-(const FVec2& v1, const FVec2& v2);

constexpr FVec2 operator*(const FVec2& v1, float v2);

constexpr FVec2 operator*(float v1, const FVec2& v2);

constexpr FVec2 operator*(const FVec2& v1, const FVec2& v2);

constexpr FVec2 operator/(const FVec2& v1, float v2);

constexpr FVec2 operator/(float v1, const FVec2& v2);

constexpr FVec2 operator/(const FVec2& v1, const FVec2& v2);

class DVec2 {
public:
    double x = 0;    /**< the X component of the vector */
    double y = 0;    /**< the Y component of the vector */
    
    constexpr DVec2() = default;
    
    constexpr DVec2(double x, double y);
    
    constexpr explicit DVec2(double x);
    
    constexpr DVec2 operator+() const;
    
    constexpr DVec2 operator-() const;
    
    constexpr bool operator==(const DVec2& v) const;
    
    constexpr bool operator!=(const DVec2& v) const;
    
    constexpr DVec2& operator+=(double v);
    
    constexpr DVec2& operator+=(const DVec2& v);
    
    constexpr DVec2& operator-=(double v);
    
    constexpr DVec2& operator-=(const DVec2& v);
    
    constexpr DVec2& operator*=(double v);
    
    constexpr DVec2& operator*=(const DVec2& v);
    
    constexpr DVec2& operator/=(double v);
    
    constexpr DVec2& operator/=(const DVec2& v);
    
    constexpr double dot(const DVec2& v) const;
    
    constexpr double cross(const DVec2& v) const;
    
    inline double magnitude() const;
    
    inline double distance(const DVec2& v) const;
    
    inline DVec2 normalize() const;
    
    inline DVec2 rotate(double a) const;
    
    inline std::string to_string(int p = 2) const;
};

constexpr DVec2 operator+(const DVec2& v1, double v2);

constexpr DVec2 operator+(double v1, const DVec2& v2);

constexpr DVec2 operator+(const DVec2& v1, const DVec2& v2);

constexpr DVec2 operator-(const DVec2& v1, double v2);

constexpr DVec2 operator-(double v1, const DVec2& v2);

constexpr DVec2 operator-(const DVec2& v1, const DVec2& v2);

constexpr DVec2 operator*(const DVec2& v1, double v2);

constexpr DVec2 operator*(double v1, const DVec2& v2);

constexpr DVec2 operator*(const DVec2& v1, const DVec2& v2);

constexpr DVec2 operator/(const DVec2& v1, double v2);

constexpr DVec2 operator/(double v1, const DVec2& v2);

constexpr DVec2 operator/(const DVec2& v1, const DVec2& v2);

using Vec2 = FVec2;

constexpr FVec2::FVec2(float x, float y) : x(x), y(y) {}

constexpr FVec2::FVec2(float x) : x(x), y(x) {}

constexpr FVec2 FVec2::operator+() const {
    return *this;
}

constexpr FVec2 FVec2::operator-() const {
    return {-x, -y};
}

constexpr bool FVec2::operator==(const FVec2& v) const {
    return x == v.x && y == v.y;
}

constexpr bool FVec2::operator!=(const FVec2& v) const {
    return x != v.x || y != v.y;
}

constexpr FVec2& FVec2::operator+=(float v) {
    x += v;
    y += v;
    return *this;
}

constexpr FVec2& FVec2::operator+=(const FVec2& v) {
    x += v.x;
    y += v.y;
    return *this;
}

constexpr FVec2& FVec2::operator-=(float v) {
    x -= v;
    y -= v;
    return *this;
}

constexpr FVec2& FVec2::operator-=(const FVec2& v) {
    x -= v.x;
    y -= v.y;
    return *this;
}

constexpr FVec2& FVec2::operator*=(float v) {
    x *= v;
    y *= v;
    return *this;
}

constexpr FVec2& FVec2::operator*=(const FVec2& v) {
    x *= v.x;
    y *= v.y;
    return *this;
}

constexpr FVec2& FVec2::operator/=(float v) {
    x /= v;
    y /= v;
    return *this;
}

constexpr FVec2& FVec2::operator/=(const FVec2& v) {
    x /= v.x;
    y /= v.y;
    return *this;
}

constexpr float FVec2::dot(const FVec2& v) const {
    return x * v.x + y * v.y;
}

constexpr float FVec2::cross(const FVec2& v) const {
    return x * v.y - y * v.x;
}

inline float FVec2::magnitude() const {
    return sqrtf(x * x + y * y);
}

inline float FVec2::distance(const FVec2& v) const {
    return sqrtf((x - v.x) * (x - v.x) + (y - v.y) * (y - v.y));
}

inline FVec2 FVec2::normalize() const {
    float l = sqrtf(x * x + y * y);
    return {x / l, y / l};
}

inline FVec2 FVec2::rotate(float a) const {
    return {x * cosf(a) - y * sinf(a), x * sinf(a) + y * cosf(a)};
}

inline std::string FVec2::to_string(int p) const {
    std::stringstream stream;
    stream.setf(std::ios::fixed, std::ios::floatfield);
    stream.precision(p);
    stream << "(" << x << ", " << y << ")";
    return stream.str();
}

constexpr FVec2 operator+(const FVec2& v1, float v2) {
    return {v1.x + v2, v1.y + v2};
}

constexpr FVec2 operator+(float v1, const FVec2& v2) {
    return {v1 + v2.x, v1 + v2.y};
}

constexpr FVec2 operator+(const FVec2& v1, const FVec2& v2) {
    return {v1.x + v2.x, v1.y + v2.y};
}

constexpr FVec2 operator-(const FVec2& v1, float v2) {
    return {v1.x - v2, v1.y - v2};
}

constexpr FVec2 operator-(float v1, const FVec2& v2) {
    return {v1 - v2.x, v1 - v2.y};
}

constexpr FVec2 operator-(const FVec2& v1, const FVec2& v2) {
    return {v1.x - v2.x, v1.y - v2.y};
}

constexpr FVec2 operator*(const FVec2& v1, float v2) {
    return {v1.x * v2, v1.y * v2};
}

constexpr FVec2 operator*(float v1, const FVec2& v2) {
    return {v1 * v2.x, v1 * v2.y};
}

constexpr FVec2 operator*(const FVec2& v1, const FVec2& v2) {
    return {v1.x * v2.x, v1.y * v2.y};
}

constexpr FVec2 operator/(const FVec2& v1, float v2) {
    return {v1.x / v2, v1.y / v2};
}

constexpr FVec2 operator/(float v1, const FVec2& v2) {
    return {v1 / v2.x, v1 / v2.y};
}

constexpr FVec2 operator/(const FVec2& v1, const FVec2& v2) {
    return {v1.x / v2.x, v1.y / v2.y};
}

constexpr DVec2::DVec2(double x, double y) : x(x), y(y) {}

constexpr DVec2::DVec2(double x) : x(x), y(x) {}

constexpr DVec2 DVec2::operator+() const {
    return *this;
}

constexpr DVec2 DVec2::operator-() const {
    return {-x, -y};
}

constexpr bool DVec2::operator==(const DVec2& v) const {
    return x == v.x && y == v.y;
}

constexpr bool DVec2::operator!=(const DVec2& v) const {
    return x != v.x || y != v.y;
}

constexpr DVec2& DVec2::operator+=(double v) {
    x += v;
    y += v;
    return *this;
}

constexpr DVec2& DVec2::operator+=(const DVec2& v) {
    x += v.x;
    y += v.y;
    return *this;
}

constexpr DVec2& DVec2::operator-=(double v) {
    x -= v;
    y -= v;
    return *this;
}

constexpr DVec2& DVec2::operator-=(const DVec2& v) {
    x -= v.x;
    y -= v.y;
    return *this;
}

constexpr DVec2& DVec2::operator*=(double v) {
    x *= v;
    y *= v;
    return *this;
}

constexpr DVec2& DVec2::operator*=(const DVec2& v) {
    x *= v.x;
    y *= v.y;
    return *this;
}

constexpr DVec2& DVec2::operator/=(double v) {
    x /= v;
    y /= v;
    return *this;
}

constexpr DVec2& DVec2::operator/=(const DVec2& v) {
    x /= v.x;
    y /= v.y;
    return *this;
}

constexpr double DVec2::dot(const DVec2& v) const {
    return x * v.x + y * v.y;
}

constexpr double DVec2::cross(const DVec2& v) const {
    return x * v.y - y * v.x;
}

inline double DVec2::magnitude() const {
    return sqrt(x * x + y * y);
}

inline double DVec2::distance(const DVec2& v) const {
    return sqrt((x - v.x) * (x - v.x) + (y - v.y) * (y - v.y));
}

inline DVec2 DVec2::normalize() const {
    double l = sqrt(x * x + y * y);
    return {x / l, y / l};
}

inline DVec2 DVec2::rotate(double a) const {
    return {x * cos(a) - y * sin(a), x * sin(a) + y * cos(a)};
}

inline std::string DVec2::to_string(int p) const {
    std::stringstream stream;
    stream.setf(std::ios::fixed, std::ios::floatfield);
    stream.precision(p);
    stream << "(" << x << ", " << y << ")";
    return stream.str();
}

constexpr DVec2 operator+(const DVec2& v1, double v2) {
    return {v1.x + v2, v1.y + v2};
}

constexpr DVec2 operator+(double v1, const DVec2& v2) {
    return {v1 + v2.x, v1 + v2.y};
}

constexpr DVec2 operator+(const DVec2& v1, const DVec2& v2) {
    return {v1.x + v2.x, v1.y + v2.y};
}

constexpr DVec2 operator-(const DVec2& v1, double v2) {
    return {v1.x - v2, v1.y - v2};
}

constexpr DVec2 operator-(double v1, const DVec2& v2) {
    return {v1 - v2.x, v1 - v2.y};
}

constexpr DVec2 operator-(const DVec2& v1, const DVec2& v2) {
    return {v1.x - v2.x, v1.y - v2.y};
}

constexpr DVec2 operator*(const DVec2& v1, double v2) {
    return {v1.x * v2, v1.y * v2};
}

constexpr DVec2 operator*(double v1, const DVec2& v2) {
    return {v1 * v2.x, v1 * v2.y};
}

constexpr DVec2 operator*(const DVec2& v1, const DVec2& v2) {
    return {v1.x * v2.x, v1.y * v2.y};
}

constexpr DVec2 operator/(const DVec2& v1, double v2) {
    return {v1.x / v2, v1.y / v2};
}

constexpr DVec2 operator/(double v1, const DVec2& v2) {
    return {v1 / v2.x, v1 / v2.y};
}

constexpr DVec2 operator/(const DVec2& v1, const DVec2& v2) {
    return {v1.x / v2.x, v1.y / v2.y};
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/math/Vector3.h -------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class FVec3 {
public:
    float x = 0;    /**< the X component of the vector */
    float y = 0;    /**< the Y component of the vector */
    float z = 0;    /**< the Z component of the vector */
    
    constexpr FVec3() = default;
    
    constexpr FVec3(float x, float y, float z);
    
    constexpr FVec3(const FVec2& v, float z);
    
    constexpr FVec3(float x, const FVec2& v);
    
    constexpr explicit FVec3(float x);
    
    constexpr FVec3 operator+() const;
    
    constexpr FVec3 operator-() const;
    
    constexpr bool operator==(const FVec3& v) const;
    
    constexpr bool operator!=(const FVec3& v) const;
    
    constexpr FVec3& operator+=(float v);
    
    constexpr FVec3& operator+=(const FVec3& v);
    
    constexpr FVec3& operator-=(float v);
    
    constexpr FVec3& operator-=(const FVec3& v);
    
    constexpr FVec3& operator*=(float v);
    
    constexpr FVec3& operator*=(const FVec3& v);
    
    constexpr FVec3& operator/=(float v);
    
    constexpr FVec3& operator/=(const FVec3& v);
    
    constexpr float dot(const FVec3& v) const;
    
    constexpr FVec3 cross(const FVec3& v) const;
    
    inline float magnitude() const;
    
    inline float distance(const FVec3& v) const;
    
    inline FVec3 normalize() const;
    
    inline FVec3 rotate(const FVec3& v, float a) const;
    
    inline std::string to_string(int p = 2) const;
};

constexpr FVec3 operator+(const FVec3& v1, float v2);

constexpr FVec3 operator+(float v1, const FVec3& v2);

constexpr FVec3 operator+(const FVec3& v1, const FVec3& v2);

constexpr FVec3 operator-(const FVec3& v1, float v2);

constexpr FVec3 operator-(float v1, const FVec3& v2);

constexpr FVec3 operator-(const FVec3& v1, const FVec3& v2);

constexpr FVec3 operator*(const FVec3& v1, float v2);

constexpr FVec3 operator*(float v1, const FVec3& v2);

constexpr FVec3 operator*(const FVec3& v1, const FVec3& v2);

constexpr FVec3 operator/(const FVec3& v1, float v2);

constexpr FVec3 operator/(float v1, const FVec3& v2);

constexpr FVec3 operator/(const FVec3& v1, const FVec3& v2);

class DVec3 {
public:
    double x = 0;    /**< the X component of the vector */
    double y = 0;    /**< the Y component of the vector */
    double z = 0;    /**< the Z component of the vector */
    
    constexpr DVec3() = default;
    
    constexpr DVec3(double x, double y, double z);
    
    constexpr DVec3(const DVec2& v, double z);
    
    constexpr DVec3(double x, const DVec2& v);
    
    constexpr explicit DVec3(double x);
    
    constexpr DVec3 operator+() const;
    
    constexpr DVec3 operator-() const;
    
    constexpr bool operator==(const DVec3& v) const;
    
    constexpr bool operator!=(const DVec3& v) const;
    
    constexpr DVec3& operator+=(double v);
    
    constexpr DVec3& operator+=(const DVec3& v);
    
    constexpr DVec3& operator-=(double v);
    
    constexpr DVec3& operator-=(const DVec3& v);
    
    constexpr DVec3& operator*=(double v);
    
    constexpr DVec3& operator*=(const DVec3& v);
    
    constexpr DVec3& operator/=(double v);
    
    constexpr DVec3& operator/=(const DVec3& v);
    
    constexpr double dot(const DVec3& v) const;
    
    constexpr DVec3 cross(const DVec3& v) const;
    
    inline double magnitude() const;
    
    inline double distance(const DVec3& v) const;
    
    inline DVec3 normalize() const;
    
    inline DVec3 rotate(const DVec3& v, double a) const;
    
    inline std::string to_string(int p = 2) const;
};

constexpr DVec3 operator+(const DVec3& v1, double v2);

constexpr DVec3 operator+(double v1, const DVec3& v2);

constexpr DVec3 operator+(const DVec3& v1, const DVec3& v2);

constexpr DVec3 operator-(const DVec3& v1, double v2);

constexpr DVec3 operator-(double v1, const DVec3& v2);

constexpr DVec3 operator-(const DVec3& v1, const DVec3& v2);

constexpr DVec3 operator*(const DVec3& v1, double v2);

constexpr DVec3 operator*(double v1, const DVec3& v2);

constexpr DVec3 operator*(const DVec3& v1, const DVec3& v2);

constexpr DVec3 operator/(const DVec3& v1, double v2);

constexpr DVec3 operator/(double v1, const DVec3& v2);

constexpr DVec3 operator/(const DVec3& v1, const DVec3& v2);

using Vec3 = FVec3;

constexpr FVec3::FVec3(float x, float y, float z) : x(x), y(y), z(z) {}

constexpr FVec3::FVec3(const FVec2& v, float z) : x(v.x), y(v.y), z(z) {}

constexpr FVec3::FVec3(float x, const FVec2& v) : x(x), y(v.x), z(v.y) {}

constexpr FVec3::FVec3(float x) : x(x), y(x), z(x) {}

constexpr FVec3 FVec3::operator+() const {
    return *this;
}

constexpr FVec3 FVec3::operator-() const {
    return {-x, -y, -z};
}

constexpr bool FVec3::operator==(const FVec3& v) const {
    return x == v.x && y == v.y && z == v.z;
}

constexpr bool FVec3::operator!=(const FVec3& v) const {
    return x != v.x || y != v.y || z != v.z;
}

constexpr FVec3& FVec3::operator+=(float v) {
    x += v;
    y += v;
    z += v;
    return *this;
}

constexpr FVec3& FVec3::operator+=(const FVec3& v) {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

constexpr FVec3& FVec3::operator-=(float v) {
    x -= v;
    y -= v;
    z -= v;
    return *this;
}

constexpr FVec3& FVec3::operator-=(const FVec3& v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

constexpr FVec3& FVec3::operator*=(float v) {
    x *= v;
    y *= v;
    z *= v;
    return *this;
}

constexpr FVec3& FVec3::operator*=(const FVec3& v) {
    x *= v.x;
    y *= v.y;
    z *= v.z;
    return *this;
}

constexpr FVec3& FVec3::operator/=(float v) {
    x /= v;
    y /= v;
    z /= v;
    return *this;
}

constexpr FVec3& FVec3::operator/=(const FVec3& v) {
    x /= v.x;
    y /= v.y;
    z /= v.z;
    return *this;
}

constexpr float FVec3::dot(const FVec3& v) const {
    return x * v.x + y * v.y + z * v.z;
}

constexpr FVec3 FVec3::cross(const FVec3& v) const {
    return {y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x};
}

inline float FVec3::magnitude() const {
    return sqrtf(x * x + y * y + z * z);
}

inline float FVec3::distance(const FVec3& v) const {
    return sqrtf((x - v.x) * (x - v.x) + (y - v.y) * (y - v.y) + (z - v.z) * (z - v.z));
}

inline FVec3 FVec3::normalize() const {
    float l = sqrtf(x * x + y * y + z * z);
    return {x / l, y / l, z / l};
}

inline FVec3 FVec3::rotate(const FVec3& v, float a) const {
    return {(cosf(a) + (1 - cosf(a)) * v.x * v.x) * x +
        ((1 - cosf(a)) * v.x * v.y - sinf(a) * v.z) * y +
        ((1 - cosf(a)) * v.x * v.z + sinf(a) * v.y) * z,
        ((1 - cosf(a)) * v.x * v.y + sinf(a) * v.z) * x +
        (cosf(a) + (1 - cosf(a)) * v.y * v.y) * y +
        ((1 - cosf(a)) * v.y * v.z - sinf(a) * v.x) * z,
        ((1 - cosf(a)) * v.x * v.z - sinf(a) * v.y) * x +
        ((1 - cosf(a)) * v.y * v.z + sinf(a) * v.x) * y +
        (cosf(a) + (1 - cosf(a)) * v.z * v.z) * z};
}

inline std::string FVec3::to_string(int p) const {
    std::stringstream stream;
    stream.setf(std::ios::fixed, std::ios::floatfield);
    stream.precision(p);
    stream << "(" << x << ", " << y << ", " << z << ")";
    return stream.str();
}

constexpr FVec3 operator+(const FVec3& v1, float v2) {
    return {v1.x + v2, v1.y + v2, v1.z + v2};
}

constexpr FVec3 operator+(float v1, const FVec3& v2) {
    return {v1 + v2.x, v1 + v2.y, v1 + v2.z};
}

constexpr FVec3 operator+(const FVec3& v1, const FVec3& v2) {
    return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

constexpr FVec3 operator-(const FVec3& v1, float v2) {
    return {v1.x - v2, v1.y - v2, v1.z - v2};
}

constexpr FVec3 operator-(float v1, const FVec3& v2) {
    return {v1 - v2.x, v1 - v2.y, v1 - v2.z};
}

constexpr FVec3 operator-(const FVec3& v1, const FVec3& v2) {
    return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

constexpr FVec3 operator*(const FVec3& v1, float v2) {
    return {v1.x * v2, v1.y * v2, v1.z * v2};
}

constexpr FVec3 operator*(float v1, const FVec3& v2) {
    return {v1 * v2.x, v1 * v2.y, v1 * v2.z};
}

constexpr FVec3 operator*(const FVec3& v1, const FVec3& v2) {
    return {v1.x * v2.x, v1.y * v2.y, v1.z * v2.z};
}

constexpr FVec3 operator/(const FVec3& v1, float v2) {
    return {v1.x / v2, v1.y / v2, v1.z / v2};
}

constexpr FVec3 operator/(float v1, const FVec3& v2) {
    return {v1 / v2.x, v1 / v2.y, v1 / v2.z};
}

constexpr FVec3 operator/(const FVec3& v1, const FVec3& v2) {
    return {v1.x / v2.x, v1.y / v2.y, v1.z / v2.z};
}

constexpr DVec3::DVec3(double x, double y, double z) : x(x), y(y), z(z) {}

constexpr DVec3::DVec3(const DVec2& v, double z) : x(v.x), y(v.y), z(z) {}

constexpr DVec3::DVec3(double x, const DVec2& v) : x(x), y(v.x), z(v.y) {}

constexpr DVec3::DVec3(double x) : x(x), y(x), z(x) {}

constexpr DVec3 DVec3::operator+() const {
    return *this;
}

constexpr DVec3 DVec3::operator-() const {
    return {-x, -y, -z};
}

constexpr bool DVec3::operator==(const DVec3& v) const {
    return x == v.x && y == v.y && z == v.z;
}

constexpr bool DVec3::operator!=(const DVec3& v) const {
    return x != v.x || y != v.y || z != v.z;
}

constexpr DVec3& DVec3::operator+=(double v) {
    x += v;
    y += v;
    z += v;
    return *this;
}

constexpr DVec3& DVec3::operator+=(const DVec3& v) {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

constexpr DVec3& DVec3::operator-=(double v) {
    x -= v;
    y -= v;
    z -= v;
    return *this;
}

constexpr DVec3& DVec3::operator-=(const DVec3& v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

constexpr DVec3& DVec3::operator*=(double v) {
    x *= v;
    y *= v;
    z *= v;
    return *this;
}

constexpr DVec3& DVec3::operator*=(const DVec3& v) {
    x *= v.x;
    y *= v.y;
    z *= v.z;
    return *this;
}

constexpr DVec3& DVec3::operator/=(double v) {
    x /= v;
    y /= v;
    z /= v;
    return *this;
}

constexpr DVec3& DVec3::operator/=(const DVec3& v) {
    x /= v.x;
    y /= v.y;
    z /= v.z;
    return *this;
}

constexpr double DVec3::dot(const DVec3& v) const {
    return x * v.x + y * v.y + z * v.z;
}

constexpr DVec3 DVec3::cross(const DVec3& v) const {
    return {y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x};
}

inline double DVec3::magnitude() const {
    return sqrt(x * x + y * y + z * z);
}

inline double DVec3::distance(const DVec3& v) const {
    return sqrt((x - v.x) * (x - v.x) + (y - v.y) * (y - v.y) + (z - v.z) * (z - v.z));
}

inline DVec3 DVec3::normalize() const {
    double l = sqrt(x * x + y * y + z * z);
    return {x / l, y / l, z / l};
}

inline DVec3 DVec3::rotate(const DVec3& v, double a) const {
    return {(cos(a) + (1 - cos(a)) * v.x * v.x) * x +
        ((1 - cos(a)) * v.x * v.y - sin(a) * v.z) * y +
        ((1 - cos(a)) * v.x * v.z + sin(a) * v.y) * z,
        ((1 - cos(a)) * v.x * v.y + sin(a) * v.z) * x +
        (cos(a) + (1 - cos(a)) * v.y * v.y) * y +
        ((1 - cos(a)) * v.y * v.z - sin(a) * v.x) * z,
        ((1 - cos(a)) * v.x * v.z - sin(a) * v.y) * x +
        ((1 - cos(a)) * v.y * v.z + sin(a) * v.x) * y +
        (cos(a) + (1 - cos(a)) * v.z * v.z) * z};
}

inline std::string DVec3::to_string(int p) const {
    std::stringstream stream;
    stream.setf(std::ios::fixed, std::ios::floatfield);
    stream.precision(p);
    stream << "(" << x << ", " << y << ", " << z << ")";
    return stream.str();
}

constexpr DVec3 operator+(const DVec3& v1, double v2) {
    return {v1.x + v2, v1.y + v2, v1.z + v2};
}

constexpr DVec3 operator+(double v1, const DVec3& v2) {
    return {v1 + v2.x, v1 + v2.y, v1 + v2.z};
}

constexpr DVec3 operator+(const DVec3& v1, const DVec3& v2) {
    return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

constexpr DVec3 operator-(const DVec3& v1, double v2) {
    return {v1.x - v2, v1.y - v2, v1.z - v2};
}

constexpr DVec3 operator-(double v1, const DVec3& v2) {
    return {v1 - v2.x, v1 - v2.y, v1 - v2.z};
}

constexpr DVec3 operator-(const DVec3& v1, const DVec3& v2) {
    return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

constexpr DVec3 operator*(const DVec3& v1, double v2) {
    return {v1.x * v2, v1.y * v2, v1.z * v2};
}

constexpr DVec3 operator*(double v1, const DVec3& v2) {
    return {v1 * v2.x, v1 * v2.y, v1 * v2.z};
}

constexpr DVec3 operator*(const DVec3& v1, const DVec3& v2) {
    return {v1.x * v2.x, v1.y * v2.y, v1.z * v2.z};
}

constexpr DVec3 operator/(const DVec3& v1, double v2) {
    return {v1.x / v2, v1.y / v2, v1.z / v2};
}

constexpr DVec3 operator/(double v1, const DVec3& v2) {
    return {v1 / v2.x, v1 / v2.y, v1 / v2.z};
}

constexpr DVec3 operator/(const DVec3& v1, const DVec3& v2) {
    return {v1.x / v2.x, v1.y / v2.y, v1.z / v2.z};
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/math/Vector4.h -------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class FVec4 {
public:
    float x = 0;    /**< the X component of the vector */
    float y = 0;    /**< the Y component of the vector */
    float z = 0;    /**< the Z component of the vector */
    float w = 0;    /**< the W component of the vector */
    
    constexpr FVec4() = default;
    
    constexpr FVec4(float x, float y, float z, float w);
    
    constexpr FVec4(const FVec3& v, float w);
    
    constexpr FVec4(float x, const FVec3& v);
    
    constexpr FVec4(const FVec2& v, float z, float w);
    
    constexpr FVec4(float x, const FVec2& v, float w);
    
    constexpr FVec4(float x, float y, const FVec2& v);
    
    constexpr FVec4(const FVec2& v1, const FVec2& v2);
    
    constexpr explicit FVec4(float x);
    
    constexpr FVec4 operator+() const;
    
    constexpr FVec4 operator-() const;
    
    constexpr bool operator==(const FVec4& v) const;
    
    constexpr bool operator!=(const FVec4& v) const;
    
    constexpr FVec4& operator+=(float v);
    
    constexpr FVec4& operator+=(const FVec4& v);
    
    constexpr FVec4& operator-=(float v);
    
    constexpr FVec4& operator-=(const FVec4& v);
    
    constexpr FVec4& operator*=(float v);
    
    constexpr FVec4& operator*=(const FVec4& v);
    
    constexpr FVec4& operator/=(float v);
    
    constexpr FVec4& operator/=(const FVec4& v);
    
    constexpr float dot(const FVec4& v) const;
    
    inline float magnitude() const;
    
    inline float distance(const FVec4& v) const;
    
    inline FVec4 normalize() const;
    
    inline std::string to_string(int p = 2) const;
};

constexpr FVec4 operator+(const FVec4& v1, float v2);

constexpr FVec4 operator+(float v1, const FVec4& v2);

constexpr FVec4 operator+(const FVec4& v1, const FVec4& v2);

constexpr FVec4 operator-(const FVec4& v1, float v2);

constexpr FVec4 operator-(float v1, const FVec4& v2);

constexpr FVec4 operator-(const FVec4& v1, const FVec4& v2);

constexpr FVec4 operator*(const FVec4& v1, float v2);

constexpr FVec4 operator*(float v1, const FVec4& v2);

constexpr FVec4 operator*(const FVec4& v1, const FVec4& v2);

constexpr FVec4 operator/(const FVec4& v1, float v2);

constexpr FVec4 operator/(float v1, const FVec4& v2);

constexpr FVec4 operator/(const FVec4& v1, const FVec4& v2);

class DVec4 {
public:
    double x = 0;    /**< the X component of the vector */
    double y = 0;    /**< the Y component of the vector */
    double z = 0;    /**< the Z component of the vector */
    double w = 0;    /**< the W component of the vector */
    
    constexpr DVec4() = default;
    
    constexpr DVec4(double x, double y, double z, double w);
    
    constexpr DVec4(const DVec3& v, double w);
    
    constexpr DVec4(double x, const DVec3& v);
    
    constexpr DVec4(const DVec2& v, double z, double w);
    
    constexpr DVec4(double x, const DVec2& v, double w);
    
    constexpr DVec4(double x, double y, const DVec2& v);
    
    constexpr DVec4(const DVec2& v1, const DVec2& v2);
    
    constexpr explicit DVec4(double x);
    
    constexpr DVec4 operator+() const;
    
    constexpr DVec4 operator-() const;
    
    constexpr bool operator==(const DVec4& v) const;
    
    constexpr bool operator!=(const DVec4& v) const;
    
    constexpr DVec4& operator+=(double v);
    
    constexpr DVec4& operator+=(const DVec4& v);
    
    constexpr DVec4& operator-=(double v);
    
    constexpr DVec4& operator-=(const DVec4& v);
    
    constexpr DVec4& operator*=(double v);
    
    constexpr DVec4& operator*=(const DVec4& v);
    
    constexpr DVec4& operator/=(double v);
    
    constexpr DVec4& operator/=(const DVec4& v);
    
    constexpr double dot(const DVec4& v) const;
    
    inline double magnitude() const;
    
    inline double distance(const DVec4& v) const;
    
    inline DVec4 normalize() const;
    
    inline std::string to_string(int p = 2) const;
};

constexpr DVec4 operator+(const DVec4& v1, double v2);

constexpr DVec4 operator+(double v1, const DVec4& v2);

constexpr DVec4 operator+(const DVec4& v1, const DVec4& v2);

constexpr DVec4 operator-(const DVec4& v1, double v2);

constexpr DVec4 operator-(double v1, const DVec4& v2);

constexpr DVec4 operator-(const DVec4& v1, const DVec4& v2);

constexpr DVec4 operator*(const DVec4& v1, double v2);

constexpr DVec4 operator*(double v1, const DVec4& v2);

constexpr DVec4 operator*(const DVec4& v1, const DVec4& v2);

constexpr DVec4 operator/(const DVec4& v1, double v2);

constexpr DVec4 operator/(double v1, const DVec4& v2);

constexpr DVec4 operator/(const DVec4& v1, const DVec4& v2);

using Vec4 = FVec4;

constexpr FVec4::FVec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

constexpr FVec4::FVec4(const FVec3& v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}

constexpr FVec4::FVec4(float x, const FVec3& v) : x(x), y(v.x), z(v.y), w(v.z) {}

constexpr FVec4::FVec4(const FVec2& v, float z, float w) : x(v.x), y(v.y), z(z), w(w) {}

constexpr FVec4::FVec4(float x, const FVec2& v, float w) : x(x), y(v.x), z(v.y), w(w) {}

constexpr FVec4::FVec4(float x, float y, const FVec2& v) : x(x), y(y), z(v.x), w(v.y) {}

constexpr FVec4::FVec4(const FVec2& v1, const FVec2& v2) : x(v1.x), y(v1.y), z(v2.x), w(v2.y) {}

constexpr FVec4::FVec4(float x) : x(x), y(x), z(x), w(x) {}

constexpr FVec4 FVec4::operator+() const {
    return *this;
}

constexpr FVec4 FVec4::operator-() const {
    return {-x, -y, -z, -w};
}

constexpr bool FVec4::operator==(const FVec4& v) const {
    return x == v.x && y == v.y && z == v.z && w == v.w;
}

constexpr bool FVec4::operator!=(const FVec4& v) const {
    return x != v.x || y != v.y || z != v.z || w != v.w;
}

constexpr FVec4& FVec4::operator+=(float v) {
    x += v;
    y += v;
    z += v;
    w += v;
    return *this;
}

constexpr FVec4& FVec4::operator+=(const FVec4& v) {
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
    return *this;
}

constexpr FVec4& FVec4::operator-=(float v) {
    x -= v;
    y -= v;
    z -= v;
    w -= v;
    return *this;
}

constexpr FVec4& FVec4::operator-=(const FVec4& v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;
    return *this;
}

constexpr FVec4& FVec4::operator*=(float v) {
    x *= v;
    y *= v;
    z *= v;
    w *= v;
    return *this;
}

constexpr FVec4& FVec4::operator*=(const FVec4& v) {
    x *= v.x;
    y *= v.y;
    z *= v.z;
    w *= v.w;
    return *this;
}

constexpr FVec4& FVec4::operator/=(float v) {
    x /= v;
    y /= v;
    z /= v;
    w /= v;
    return *this;
}

constexpr FVec4& FVec4::operator/=(const FVec4& v) {
    x /= v.x;
    y /= v.y;
    z /= v.z;
    w /= v.w;
    return *this;
}

constexpr float FVec4::dot(const FVec4& v) const {
    return x * v.x + y * v.y + z * v.z + w * v.w;
}

inline float FVec4::magnitude() const {
    return sqrtf(x * x + y * y + z * z + w * w);
}

inline float FVec4::distance(const FVec4& v) const {
    return sqrtf((x - v.x) * (x - v.x) + (y - v.y) * (y - v.y) +
                 (z - v.z) * (z - v.z) + (w - v.w) * (w - v.w));
}

inline FVec4 FVec4::normalize() const {
    float l = sqrtf(x * x + y * y + z * z + w * w);
    return {x / l, y / l, z / l, w / l};
}

inline std::string FVec4::to_string(int p) const {
    std::stringstream stream;
    stream.setf(std::ios::fixed, std::ios::floatfield);
    stream.precision(p);
    stream << "(" << x << ", " << y << ", " << z << ", " << w << ")";
    return stream.str();
}

constexpr FVec4 operator+(const FVec4& v1, float v2) {
    return {v1.x + v2, v1.y + v2, v1.z + v2, v1.w + v2};
}

constexpr FVec4 operator+(float v1, const FVec4& v2) {
    return {v1 + v2.x, v1 + v2.y, v1 + v2.z, v1 + v2.w};
}

constexpr FVec4 operator+(const FVec4& v1, const FVec4& v2) {
    return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w};
}

constexpr FVec4 operator-(const FVec4& v1, float v2) {
    return {v1.x - v2, v1.y - v2, v1.z - v2, v1.w - v2};
}

constexpr FVec4 operator-(float v1, const FVec4& v2) {
    return {v1 - v2.x, v1 - v2.y, v1 - v2.z, v1 - v2.w};
}

constexpr FVec4 operator-(const FVec4& v1, const FVec4& v2) {
    return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w};
}

constexpr FVec4 operator*(const FVec4& v1, float v2) {
    return {v1.x * v2, v1.y * v2, v1.z * v2, v1.w * v2};
}

constexpr FVec4 operator*(float v1, const FVec4& v2) {
    return {v1 * v2.x, v1 * v2.y, v1 * v2.z, v1 * v2.w};
}

constexpr FVec4 operator*(const FVec4& v1, const FVec4& v2) {
    return {v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w};
}

constexpr FVec4 operator/(const FVec4& v1, float v2) {
    return {v1.x / v2, v1.y / v2, v1.z / v2, v1.w / v2};
}

constexpr FVec4 operator/(float v1, const FVec4& v2) {
    return {v1 / v2.x, v1 / v2.y, v1 / v2.z, v1 / v2.w};
}

constexpr FVec4 operator/(const FVec4& v1, const FVec4& v2) {
    return {v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w};
}

constexpr DVec4::DVec4(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}

constexpr DVec4::DVec4(const DVec3& v, double w) : x(v.x), y(v.y), z(v.z), w(w) {}

constexpr DVec4::DVec4(double x, const DVec3& v) : x(x), y(v.x), z(v.y), w(v.z) {}

constexpr DVec4::DVec4(const DVec2& v, double z, double w) : x(v.x), y(v.y), z(z), w(w) {}

constexpr DVec4::DVec4(double x, const DVec2& v, double w) : x(x), y(v.x), z(v.y), w(w) {}

constexpr DVec4::DVec4(double x, double y, const DVec2& v) : x(x), y(y), z(v.x), w(v.y) {}

constexpr DVec4::DVec4(const DVec2& v1, const DVec2& v2) : x(v1.x), y(v1.y), z(v2.x), w(v2.y) {}

constexpr DVec4::DVec4(double x) : x(x), y(x), z(x), w(x) {}

constexpr DVec4 DVec4::operator+() const {
    return *this;
}

constexpr DVec4 DVec4::operator-() const {
    return {-x, -y, -z, -w};
}

constexpr bool DVec4::operator==(const DVec4& v) const {
    return x == v.x && y == v.y && z == v.z && w == v.w;
}

constexpr bool DVec4::operator!=(const DVec4& v) const {
    return x != v.x || y != v.y || z != v.z || w != v.w;
}

constexpr DVec4& DVec4::operator+=(double v) {
    x += v;
    y += v;
    z += v;
    w += v;
    return *this;
}

constexpr DVec4& DVec4::operator+=(const DVec4& v) {
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
    return *this;
}

constexpr DVec4& DVec4::operator-=(double v) {
    x -= v;
    y -= v;
    z -= v;
    w -= v;
    return *this;
}

constexpr DVec4& DVec4::operator-=(const DVec4& v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;
    return *this;
}

constexpr DVec4& DVec4::operator*=(double v) {
    x *= v;
    y *= v;
    z *= v;
    w *= v;
    return *this;
}

constexpr DVec4& DVec4::operator*=(const DVec4& v) {
    x *= v.x;
    y *= v.y;
    z *= v.z;
    w *= v.w;
    return *this;
}

constexpr DVec4& DVec4::operator/=(double v) {
    x /= v;
    y /= v;
    z /= v;
    w /= v;
    return *this;
}

constexpr DVec4& DVec4::operator/=(const DVec4& v) {
    x /= v.x;
    y /= v.y;
    z /= v.z;
    w /= v.w;
    return *this;
}

constexpr double DVec4::dot(const DVec4& v) const {
    return x * v.x + y * v.y + z * v.z + w * v.w;
}

inline double DVec4::magnitude() const {
    return sqrt(x * x + y * y + z * z + w * w);
}

inline double DVec4::distance(const DVec4& v) const {
    return sqrt((x - v.x) * (x - v.x) + (y - v.y) * (y - v.y) +
                 (z - v.z) * (z - v.z) + (w - v.w) * (w - v.w));
}

inline DVec4 DVec4::normalize() const {
    double l = sqrt(x * x + y * y + z * z + w * w);
    return {x / l, y / l, z / l, w / l};
}

inline std::string DVec4::to_string(int p) const {
    std::stringstream stream;
    stream.setf(std::ios::fixed, std::ios::floatfield);
    stream.precision(p);
    stream << "(" << x << ", " << y << ", " << z << ", " << w << ")";
    return stream.str();
}

constexpr DVec4 operator+(const DVec4& v1, double v2) {
    return {v1.x + v2, v1.y + v2, v1.z + v2, v1.w + v2};
}

constexpr DVec4 operator+(double v1, const DVec4& v2) {
    return {v1 + v2.x, v1 + v2.y, v1 + v2.z, v1 + v2.w};
}

constexpr DVec4 operator+(const DVec4& v1, const DVec4& v2) {
    return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w};
}

constexpr DVec4 operator-(const DVec4& v1, double v2) {
    return {v1.x - v2, v1.y - v2, v1.z - v2, v1.w - v2};
}

constexpr DVec4 operator-(double v1, const DVec4& v2) {
    return {v1 - v2.x, v1 - v2.y, v1 - v2.z, v1 - v2.w};
}

constexpr DVec4 operator-(const DVec4& v1, const DVec4& v2) {
    return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w};
}

constexpr DVec4 operator*(const DVec4& v1, double v2) {
    return {v1.x * v2, v1.y * v2, v1.z * v2, v1.w * v2};
}

constexpr DVec4 operator*(double v1, const DVec4& v2) {
    return {v1 * v2.x, v1 * v2.y, v1 * v2.z, v1 * v2.w};
}

constexpr DVec4 operator*(const DVec4& v1, const DVec4& v2) {
    return {v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w};
}

constexpr DVec4 operator/(const DVec4& v1, double v2) {
    return {v1.x / v2, v1.y / v2, v1.z / v2, v1.w / v2};
}

constexpr DVec4 operator/(double v1, const DVec4& v2) {
    return {v1 / v2.x, v1 / v2.y, v1 / v2.z, v1 / v2.w};
}

constexpr DVec4 operator/(const DVec4& v1, const DVec4& v2) {
    return {v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w};
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/math/Color.h ---------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class Color {
public:
    /**
     * Converts the specified color from the hexadecimal color code to the RGB
     * values.
     *
     * \param c color
     */
    static Vec3 hex_to_rgb(unsigned int c);
    
    /**
     * Converts the specified color from the RGB values to the hexadecimal color
     * code.
     *
     * \param c color
     */
    static unsigned int rgb_to_hex(const Vec3& c);
    
    /**
     * Converts the specified color from the RGB color space to the sRGB color
     * space.
     *
     * \param c color
     */
    static Vec3 rgb_to_srgb(const Vec3& c);
    
    /**
     * Converts the specified color from the sRGB color space to the RGB color
     * space.
     *
     * \param c color
     */
    static Vec3 srgb_to_rgb(const Vec3& c);
    
    /**
     * Converts the specified color from the RGB color space to the XYZ color
     * space.
     *
     * \param c color
     */
    static Vec3 rgb_to_xyz(const Vec3& c);
    
    /**
     * Converts the specified color from the XYZ color space to the RGB color
     * space.
     *
     * \param c color
     */
    static Vec3 xyz_to_rgb(const Vec3& c);
    
    /**
     * Converts the specified color from the RGB color space to the HSV color
     * space.
     *
     * \param c color
     */
    static Vec3 rgb_to_hsv(const Vec3& c);
    
    /**
     * Converts the specified color from the HSV color space to the RGB color
     * space.
     *
     * \param c color
     */
    static Vec3 hsv_to_rgb(const Vec3& c);
    
    /**
     * Converts the specified color from the RGB color space to the HSL color
     * space.
     *
     * \param c color
     */
    static Vec3 rgb_to_hsl(const Vec3& c);
    
    /**
     * Converts the specified color from the HSL color space to the RGB color
     * space.
     *
     * \param c color
     */
    static Vec3 hsl_to_rgb(const Vec3& c);
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/math/Constants.h ------------------------------------------------ */
/* -------------------------------------------------------------------------- */

namespace ink {

/* common math constants */
constexpr double E        = 2.71828182845904523;
constexpr double LN10     = 2.30258509299404568;
constexpr double LN2      = 0.69314718055994530;
constexpr double LOG10E   = 0.43429448190325182;
constexpr double LOG2E    = 1.44269504088896340;
constexpr double PI       = 3.14159265358979323;
constexpr double PI_2     = 1.57079632679489661;
constexpr double SQRT_1_2 = 0.70710678118654752;
constexpr double SQRT_2   = 1.41421356237309504;

/* angle conversion constants */
constexpr double RAD_TO_DEG = 57.29577951308232087;
constexpr double DEG_TO_RAD = 0.017453292519943295;

}

/* -------------------------------------------------------------------------- */
/* ---- ink/math/Matrix.h --------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

template <int r, int c>
class FMat {
public:
    constexpr FMat();
    
    constexpr FMat(const FVec2& v);
    
    constexpr FMat(const FVec3& v);
    
    constexpr FMat(const FVec4& v);
    
    constexpr FMat(const std::initializer_list<float>& v);
    
    constexpr operator FVec2() const;
    
    constexpr operator FVec3() const;
    
    constexpr operator FVec4() const;
    
    constexpr float* operator[](size_t k);
    
    constexpr const float* operator[](size_t k) const;
    
    constexpr FMat<r, c> operator+() const;
    
    constexpr FMat<r, c> operator-() const;
    
    constexpr bool operator==(const FMat<r, c>& v) const;
    
    constexpr bool operator!=(const FMat<r, c>& v) const;
    
    constexpr FMat<r, c>& operator+=(float v);
    
    constexpr FMat<r, c>& operator+=(const FMat<r, c>& v);
    
    constexpr FMat<r, c>& operator-=(float v);
    
    constexpr FMat<r, c>& operator-=(const FMat<r, c>& v);
    
    constexpr FMat<r, c>& operator*=(float v);
    
    constexpr FMat<r, c>& operator/=(float v);
    
    constexpr FMat<c, r> transpose() const;
    
    std::string to_string(int p = 2) const;
    
    static constexpr FMat<r, c> identity();
    
private:
    float m[r * c];
};

template <int r, int c>
constexpr FMat<r, c> operator+(const FMat<r, c>& v1, float v2);

template <int r, int c>
constexpr FMat<r, c> operator+(float v1, const FMat<r, c>& v2);

template <int r, int c>
constexpr FMat<r, c> operator+(const FMat<r, c>& v1, const FMat<r, c>& v2);

template <int r, int c>
constexpr FMat<r, c> operator-(const FMat<r, c>& v1, float v2);

template <int r, int c>
constexpr FMat<r, c> operator-(float v1, const FMat<r, c>& v2);

template <int r, int c>
constexpr FMat<r, c> operator-(const FMat<r, c>& v1, const FMat<r, c>& v2);

template <int r, int c>
constexpr FMat<r, c> operator*(const FMat<r, c>& v1, float v2);

template <int r, int c>
constexpr FMat<r, c> operator*(float v1, const FMat<r, c>& v2);

template <int l1, int l2, int l3>
constexpr FMat<l1, l3> operator*(const FMat<l1, l2>& v1, const FMat<l2, l3>& v2);

template <int r>
constexpr FMat<r, 1> operator*(const FMat<r, 2>& v1, const FVec2& v2);

template <int r>
constexpr FMat<r, 1> operator*(const FMat<r, 3>& v1, const FVec3& v2);

template <int r>
constexpr FMat<r, 1> operator*(const FMat<r, 4>& v1, const FVec4& v2);

template <int r, int c>
constexpr FMat<r, c> operator/(const FMat<r, c>& v1, float v2);

template <int r, int c>
constexpr FMat<r, c> operator/(float v1, const FMat<r, c>& v2);

template <int r, int c>
class DMat {
public:
    constexpr DMat();
    
    constexpr DMat(const DVec2& v);
    
    constexpr DMat(const DVec3& v);
    
    constexpr DMat(const DVec4& v);
    
    constexpr DMat(const std::initializer_list<double>& v);
    
    constexpr operator DVec2() const;
    
    constexpr operator DVec3() const;
    
    constexpr operator DVec4() const;
    
    constexpr double* operator[](size_t k);
    
    constexpr const double* operator[](size_t k) const;
    
    constexpr DMat<r, c> operator+() const;
    
    constexpr DMat<r, c> operator-() const;
    
    constexpr bool operator==(const DMat<r, c>& v) const;
    
    constexpr bool operator!=(const DMat<r, c>& v) const;
    
    constexpr DMat<r, c>& operator+=(double v);
    
    constexpr DMat<r, c>& operator+=(const DMat<r, c>& v);
    
    constexpr DMat<r, c>& operator-=(double v);
    
    constexpr DMat<r, c>& operator-=(const DMat<r, c>& v);
    
    constexpr DMat<r, c>& operator*=(double v);
    
    constexpr DMat<r, c>& operator/=(double v);
    
    constexpr DMat<c, r> transpose() const;
    
    std::string to_string(int p = 2) const;
    
    static constexpr DMat<r, c> identity();
    
private:
    double m[r * c];
};

template <int r, int c>
constexpr DMat<r, c> operator+(const DMat<r, c>& v1, double v2);

template <int r, int c>
constexpr DMat<r, c> operator+(double v1, const DMat<r, c>& v2);

template <int r, int c>
constexpr DMat<r, c> operator+(const DMat<r, c>& v1, const DMat<r, c>& v2);

template <int r, int c>
constexpr DMat<r, c> operator-(const DMat<r, c>& v1, double v2);

template <int r, int c>
constexpr DMat<r, c> operator-(double v1, const DMat<r, c>& v2);

template <int r, int c>
constexpr DMat<r, c> operator-(const DMat<r, c>& v1, const DMat<r, c>& v2);

template <int r, int c>
constexpr DMat<r, c> operator*(const DMat<r, c>& v1, double v2);

template <int r, int c>
constexpr DMat<r, c> operator*(double v1, const DMat<r, c>& v2);

template <int l1, int l2, int l3>
constexpr DMat<l1, l3> operator*(const DMat<l1, l2>& v1, const DMat<l2, l3>& v2);

template <int r>
constexpr DMat<r, 1> operator*(const DMat<r, 2>& v1, const DVec2& v2);

template <int r>
constexpr DMat<r, 1> operator*(const DMat<r, 3>& v1, const DVec3& v2);

template <int r>
constexpr DMat<r, 1> operator*(const DMat<r, 4>& v1, const DVec4& v2);

template <int r, int c>
constexpr DMat<r, c> operator/(const DMat<r, c>& v1, double v2);

template <int r, int c>
constexpr DMat<r, c> operator/(double v1, const DMat<r, c>& v2);

template<int r, int c>
using Mat = FMat<r, c>;

using Mat2 = FMat<2, 2>;
using Mat3 = FMat<3, 3>;
using Mat4 = FMat<4, 4>;

using Mat2x2 = FMat<2, 2>;
using Mat2x3 = FMat<2, 3>;
using Mat2x4 = FMat<2, 4>;
using Mat3x2 = FMat<3, 2>;
using Mat3x3 = FMat<3, 3>;
using Mat3x4 = FMat<3, 4>;
using Mat4x2 = FMat<4, 2>;
using Mat4x3 = FMat<4, 3>;
using Mat4x4 = FMat<4, 4>;

constexpr float determinant_2x2(const Mat2& m);

constexpr float determinant_3x3(const Mat3& m);

constexpr float determinant_4x4(const Mat4& m);

constexpr Mat2 inverse_2x2(const Mat2& m);

constexpr Mat3 inverse_3x3(const Mat3& m);

constexpr Mat4 inverse_4x4(const Mat4& m);

using DMat2 = DMat<2, 2>;
using DMat3 = DMat<3, 3>;
using DMat4 = DMat<4, 4>;

using DMat2x2 = DMat<2, 2>;
using DMat2x3 = DMat<2, 3>;
using DMat2x4 = DMat<2, 4>;
using DMat3x2 = DMat<3, 2>;
using DMat3x3 = DMat<3, 3>;
using DMat3x4 = DMat<3, 4>;
using DMat4x2 = DMat<4, 2>;
using DMat4x3 = DMat<4, 3>;
using DMat4x4 = DMat<4, 4>;

constexpr double determinant_2x2(const DMat2& m);

constexpr double determinant_3x3(const DMat3& m);

constexpr double determinant_4x4(const DMat4& m);

constexpr DMat2 inverse_2x2(const DMat2& m);

constexpr DMat3 inverse_3x3(const DMat3& m);

constexpr DMat4 inverse_4x4(const DMat4& m);

template <int r, int c>
constexpr FMat<r, c>::FMat() {
    std::fill_n(m, r * c, 0.0f);
}

template <int r, int c>
constexpr FMat<r, c>::FMat(const FVec2& v) {
    std::fill_n(m, r * c, 0.0f);
    m[0] = v.x;
    m[1] = v.y;
}

template <int r, int c>
constexpr FMat<r, c>::FMat(const FVec3& v) {
    std::fill_n(m, r * c, 0.0f);
    m[0] = v.x;
    m[1] = v.y;
    m[2] = v.z;
}

template <int r, int c>
constexpr FMat<r, c>::FMat(const FVec4& v) {
    std::fill_n(m, r * c, 0.0f);
    m[0] = v.x;
    m[1] = v.y;
    m[2] = v.z;
    m[3] = v.w;
}

template <int r, int c>
constexpr FMat<r, c>::FMat(const std::initializer_list<float>& v) {
    std::fill_n(m, r * c, 0.0f);
    std::copy(v.begin(), v.end(), m);
}

template <int r, int c>
constexpr FMat<r, c>::operator FVec2() const {
    return {m[0], m[1]};
}

template <int r, int c>
constexpr FMat<r, c>::operator FVec3() const {
    return {m[0], m[1], m[2]};
}

template <int r, int c>
constexpr FMat<r, c>::operator FVec4() const {
    return {m[0], m[1], m[2], m[3]};
}

template <int r, int c>
constexpr float* FMat<r, c>::operator[](size_t k) {
    return m + k * c;
}

template <int r, int c>
constexpr const float* FMat<r, c>::operator[](size_t k) const {
    return m + k * c;
}

template <int r, int c>
constexpr FMat<r, c> FMat<r, c>::operator+() const {
    return *this;
}

template <int r, int c>
constexpr FMat<r, c> FMat<r, c>::operator-() const {
    FMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = -m[i];
    return matrix;
}

template <int r, int c>
constexpr bool FMat<r, c>::operator==(const FMat<r, c>& v) const {
    int i = r * c;
    while (i --> 0) {
        if (m[i] != v[0][i]) return false;
    }
    return true;
}

template <int r, int c>
constexpr bool FMat<r, c>::operator!=(const FMat<r, c>& v) const {
    int i = r * c;
    while (i --> 0) {
        if (m[i] != v[0][i]) return true;
    }
    return false;
}

template <int r, int c>
constexpr FMat<r, c>& FMat<r, c>::operator+=(float v) {
    int i = r * c;
    while (i --> 0) m[i] += v;
    return *this;
}

template <int r, int c>
constexpr FMat<r, c>& FMat<r, c>::operator+=(const FMat<r, c>& v) {
    int i = r * c;
    while (i --> 0) m[i] += v[0][i];
    return *this;
}

template <int r, int c>
constexpr FMat<r, c>& FMat<r, c>::operator-=(float v) {
    int i = r * c;
    while (i --> 0) m[i] -= v;
    return *this;
}

template <int r, int c>
constexpr FMat<r, c>& FMat<r, c>::operator-=(const FMat<r, c>& v) {
    int i = r * c;
    while (i --> 0) m[i] -= v[0][i];
    return *this;
}

template <int r, int c>
constexpr FMat<r, c>& FMat<r, c>::operator*=(float v) {
    int i = r * c;
    while (i --> 0) m[i] *= v;
    return *this;
}

template <int r, int c>
constexpr FMat<r, c>& FMat<r, c>::operator/=(float v) {
    int i = r * c;
    while (i --> 0) m[i] /= v;
    return *this;
}

template <int r, int c>
constexpr FMat<c, r> FMat<r, c>::transpose() const {
    FMat<c, r> matrix;
    for (int i = 0; i < r; ++i) {
        for (int j = 0; j < c; ++j) {
            matrix[j][i] = m[i * c + j];
        }
    }
    return matrix;
}

template <int r, int c>
std::string FMat<r, c>::to_string(int p) const {
    std::stringstream stream;
    stream.setf(std::ios::fixed, std::ios::floatfield);
    stream.precision(p);
    for (int i = 0; i < r; ++i) {
        stream << "[ ";
        for (int j = 0; j < c - 1; ++j) {
            stream << m[i * c + j] << ", ";
        }
        stream << m[i * c + c - 1] << " ]\n";
    }
    return stream.str();
}

template <int r, int c>
constexpr FMat<r, c> FMat<r, c>::identity() {
    FMat<r, c> matrix;
    int i = std::min(r, c);
    while (i --> 0) matrix[i][i] = 1;
    return matrix;
}

template <int r, int c>
constexpr FMat<r, c> operator+(const FMat<r, c>& v1, float v2) {
    FMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = v1[0][i] + v2;
    return matrix;
}

template <int r, int c>
constexpr FMat<r, c> operator+(float v1, const FMat<r, c>& v2) {
    FMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = v1 + v2[0][i];
    return matrix;
}

template <int r, int c>
constexpr FMat<r, c> operator+(const FMat<r, c>& v1, const FMat<r, c>& v2) {
    FMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = v1[0][i] + v2[0][i];
    return matrix;
}

template <int r, int c>
constexpr FMat<r, c> operator-(const FMat<r, c>& v1, float v2) {
    FMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = v1[0][i] - v2;
    return matrix;
}

template <int r, int c>
constexpr FMat<r, c> operator-(float v1, const FMat<r, c>& v2) {
    FMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = v1 - v2[0][i];
    return matrix;
}

template <int r, int c>
constexpr FMat<r, c> operator-(const FMat<r, c>& v1, const FMat<r, c>& v2) {
    FMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = v1[0][i] - v2[0][i];
    return matrix;
}

template <int r, int c>
constexpr FMat<r, c> operator*(const FMat<r, c>& v1, float v2) {
    FMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = v1[0][i] * v2;
    return matrix;
}

template <int r, int c>
constexpr FMat<r, c> operator*(float v1, const FMat<r, c>& v2) {
    FMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = v1 * v2[0][i];
    return matrix;
}

template <int l1, int l2, int l3>
constexpr FMat<l1, l3> operator*(const FMat<l1, l2>& v1, const FMat<l2, l3>& v2) {
    FMat<l1, l3> matrix;
    for (int i = 0; i < l1; ++i) {
        for (int j = 0; j < l2; ++j) {
            for (int k = 0; k < l3; ++k) {
                matrix[i][k] += v1[i][j] * v2[j][k];
            }
        }
    }
    return matrix;
}

template <int r>
constexpr FMat<r, 1> operator*(const FMat<r, 2>& v1, const FVec2& v2) {
    FMat<r, 1> matrix;
    for (int i = 0; i < r; ++i) {
        matrix[i][0] = v1[i][0] * v2.x + v1[i][1] * v2.y;
    }
    return matrix;
}

template <int r>
constexpr FMat<r, 1> operator*(const FMat<r, 3>& v1, const FVec3& v2) {
    FMat<r, 1> matrix;
    for (int i = 0; i < r; ++i) {
        matrix[i][0] = v1[i][0] * v2.x + v1[i][1] * v2.y + v1[i][2] * v2.z;
    }
    return matrix;
}

template <int r>
constexpr FMat<r, 1> operator*(const FMat<r, 4>& v1, const FVec4& v2) {
    FMat<r, 1> matrix;
    for (int i = 0; i < r; ++i) {
        matrix[i][0] = v1[i][0] * v2.x + v1[i][1] * v2.y + v1[i][2] * v2.z + v1[i][3] * v2.w;
    }
    return matrix;
}

template <int r, int c>
constexpr FMat<r, c> operator/(const FMat<r, c>& v1, float v2) {
    FMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = v1[0][i] / v2;
    return matrix;
}

template <int r, int c>
constexpr FMat<r, c> operator/(float v1, const FMat<r, c>& v2) {
    FMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = v1 / v2[0][i];
    return matrix;
}

template <int r, int c>
constexpr DMat<r, c>::DMat() {
    std::fill_n(m, r * c, 0.0);
}

template <int r, int c>
constexpr DMat<r, c>::DMat(const DVec2& v) {
    std::fill_n(m, r * c, 0.0);
    m[0] = v.x;
    m[1] = v.y;
}

template <int r, int c>
constexpr DMat<r, c>::DMat(const DVec3& v) {
    std::fill_n(m, r * c, 0.0);
    m[0] = v.x;
    m[1] = v.y;
    m[2] = v.z;
}

template <int r, int c>
constexpr DMat<r, c>::DMat(const DVec4& v) {
    std::fill_n(m, r * c, 0.0);
    m[0] = v.x;
    m[1] = v.y;
    m[2] = v.z;
    m[3] = v.w;
}

template <int r, int c>
constexpr DMat<r, c>::DMat(const std::initializer_list<double>& v) {
    std::fill_n(m, r * c, 0.0);
    std::copy(v.begin(), v.end(), m);
}

template <int r, int c>
constexpr DMat<r, c>::operator DVec2() const {
    return {m[0], m[1]};
}

template <int r, int c>
constexpr DMat<r, c>::operator DVec3() const {
    return {m[0], m[1], m[2]};
}

template <int r, int c>
constexpr DMat<r, c>::operator DVec4() const {
    return {m[0], m[1], m[2], m[3]};
}

template <int r, int c>
constexpr double* DMat<r, c>::operator[](size_t k) {
    return m + k * c;
}

template <int r, int c>
constexpr const double* DMat<r, c>::operator[](size_t k) const {
    return m + k * c;
}

template <int r, int c>
constexpr DMat<r, c> DMat<r, c>::operator+() const {
    return *this;
}

template <int r, int c>
constexpr DMat<r, c> DMat<r, c>::operator-() const {
    DMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = -m[i];
    return matrix;
}

template <int r, int c>
constexpr bool DMat<r, c>::operator==(const DMat<r, c>& v) const {
    int i = r * c;
    while (i --> 0) {
        if (m[i] != v[0][i]) return false;
    }
    return true;
}

template <int r, int c>
constexpr bool DMat<r, c>::operator!=(const DMat<r, c>& v) const {
    int i = r * c;
    while (i --> 0) {
        if (m[i] != v[0][i]) return true;
    }
    return false;
}

template <int r, int c>
constexpr DMat<r, c>& DMat<r, c>::operator+=(double v) {
    int i = r * c;
    while (i --> 0) m[i] += v;
    return *this;
}

template <int r, int c>
constexpr DMat<r, c>& DMat<r, c>::operator+=(const DMat<r, c>& v) {
    int i = r * c;
    while (i --> 0) m[i] += v[0][i];
    return *this;
}

template <int r, int c>
constexpr DMat<r, c>& DMat<r, c>::operator-=(double v) {
    int i = r * c;
    while (i --> 0) m[i] -= v;
    return *this;
}

template <int r, int c>
constexpr DMat<r, c>& DMat<r, c>::operator-=(const DMat<r, c>& v) {
    int i = r * c;
    while (i --> 0) m[i] -= v[0][i];
    return *this;
}

template <int r, int c>
constexpr DMat<r, c>& DMat<r, c>::operator*=(double v) {
    int i = r * c;
    while (i --> 0) m[i] *= v;
    return *this;
}

template <int r, int c>
constexpr DMat<r, c>& DMat<r, c>::operator/=(double v) {
    int i = r * c;
    while (i --> 0) m[i] /= v;
    return *this;
}

template <int r, int c>
constexpr DMat<c, r> DMat<r, c>::transpose() const {
    DMat<c, r> matrix;
    for (int i = 0; i < r; ++i) {
        for (int j = 0; j < c; ++j) {
            matrix[j][i] = m[i * c + j];
        }
    }
    return matrix;
}

template <int r, int c>
std::string DMat<r, c>::to_string(int p) const {
    std::stringstream stream;
    stream.setf(std::ios::fixed, std::ios::floatfield);
    stream.precision(p);
    for (int i = 0; i < r; ++i) {
        stream << "[ ";
        for (int j = 0; j < c - 1; ++j) {
            stream << m[i * c + j] << ", ";
        }
        stream << m[i * c + c - 1] << " ]\n";
    }
    return stream.str();
}

template <int r, int c>
constexpr DMat<r, c> DMat<r, c>::identity() {
    DMat<r, c> matrix;
    int i = std::min(r, c);
    while (i --> 0) matrix[i][i] = 1;
    return matrix;
}

template <int r, int c>
constexpr DMat<r, c> operator+(const DMat<r, c>& v1, double v2) {
    DMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = v1[0][i] + v2;
    return matrix;
}

template <int r, int c>
constexpr DMat<r, c> operator+(double v1, const DMat<r, c>& v2) {
    DMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = v1 + v2[0][i];
    return matrix;
}

template <int r, int c>
constexpr DMat<r, c> operator+(const DMat<r, c>& v1, const DMat<r, c>& v2) {
    DMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = v1[0][i] + v2[0][i];
    return matrix;
}

template <int r, int c>
constexpr DMat<r, c> operator-(const DMat<r, c>& v1, double v2) {
    DMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = v1[0][i] - v2;
    return matrix;
}

template <int r, int c>
constexpr DMat<r, c> operator-(double v1, const DMat<r, c>& v2) {
    DMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = v1 - v2[0][i];
    return matrix;
}

template <int r, int c>
constexpr DMat<r, c> operator-(const DMat<r, c>& v1, const DMat<r, c>& v2) {
    DMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = v1[0][i] - v2[0][i];
    return matrix;
}

template <int r, int c>
constexpr DMat<r, c> operator*(const DMat<r, c>& v1, double v2) {
    DMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = v1[0][i] * v2;
    return matrix;
}

template <int r, int c>
constexpr DMat<r, c> operator*(double v1, const DMat<r, c>& v2) {
    DMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = v1 * v2[0][i];
    return matrix;
}

template <int l1, int l2, int l3>
constexpr DMat<l1, l3> operator*(const DMat<l1, l2>& v1, const DMat<l2, l3>& v2) {
    DMat<l1, l3> matrix;
    for (int i = 0; i < l1; ++i) {
        for (int j = 0; j < l2; ++j) {
            for (int k = 0; k < l3; ++k) {
                matrix[i][k] += v1[i][j] * v2[j][k];
            }
        }
    }
    return matrix;
}

template <int r>
constexpr DMat<r, 1> operator*(const DMat<r, 2>& v1, const DVec2& v2) {
    DMat<r, 1> matrix;
    for (int i = 0; i < r; ++i) {
        matrix[i][0] = v1[i][0] * v2.x + v1[i][1] * v2.y;
    }
    return matrix;
}

template <int r>
constexpr DMat<r, 1> operator*(const DMat<r, 3>& v1, const DVec3& v2) {
    DMat<r, 1> matrix;
    for (int i = 0; i < r; ++i) {
        matrix[i][0] = v1[i][0] * v2.x + v1[i][1] * v2.y + v1[i][2] * v2.z;
    }
    return matrix;
}

template <int r>
constexpr DMat<r, 1> operator*(const DMat<r, 4>& v1, const DVec4& v2) {
    DMat<r, 1> matrix;
    for (int i = 0; i < r; ++i) {
        matrix[i][0] = v1[i][0] * v2.x + v1[i][1] * v2.y + v1[i][2] * v2.z + v1[i][3] * v2.w;
    }
    return matrix;
}

template <int r, int c>
constexpr DMat<r, c> operator/(const DMat<r, c>& v1, double v2) {
    DMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = v1[0][i] / v2;
    return matrix;
}

template <int r, int c>
constexpr DMat<r, c> operator/(double v1, const DMat<r, c>& v2) {
    DMat<r, c> matrix;
    int i = r * c;
    while (i --> 0) matrix[0][i] = v1 / v2[0][i];
    return matrix;
}

constexpr float determinant_2x2(const Mat2& m) {
    /*
     * d = M00 * M11 - M01 * M10
     */
    return m[0][0] * m[1][1] - m[0][1] * m[1][0];
}

constexpr float determinant_3x3(const Mat3& m) {
    /*
     * s0 = M11 * M22 - M21 * M12
     * s1 = M12 * M20 - M10 * M22
     * s2 = M10 * M21 - M20 * M11
     * d = M00 * s0 + M01 * s1 + M02 * s2
     */
    float sub0 = m[1][1] * m[2][2] - m[2][1] * m[1][2];
    float sub1 = m[1][2] * m[2][0] - m[1][0] * m[2][2];
    float sub2 = m[1][0] * m[2][1] - m[2][0] * m[1][1];
    return m[0][0] * sub0 + m[0][1] * sub1 + m[0][2] * sub2;
}

constexpr float determinant_4x4(const Mat4& m) {
    /*
     * s0 = M22 * M33 - M32 * M23
     * s1 = M21 * M33 - M31 * M23
     * s2 = M21 * M32 - M31 * M22
     * s3 = M20 * M33 - M30 * M23
     * s4 = M20 * M32 - M30 * M22
     * s5 = M20 * M31 - M30 * M21
     * d = M00 * (M11 * s0 - M12 * s1 + M13 * s2) -
     *     M01 * (M10 * s0 - M12 * s3 + M13 * s4) +
     *     M02 * (M10 * s1 - M11 * s3 + M13 * s5) -
     *     M03 * (M10 * s2 - M11 * s4 + M12 * s5)
     */
    float sub0 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
    float sub1 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
    float sub2 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
    float sub3 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
    float sub4 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
    float sub5 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
    return (m[0][0] * (m[1][1] * sub0 - m[1][2] * sub1 + m[1][3] * sub2) -
            m[0][1] * (m[1][0] * sub0 - m[1][2] * sub3 + m[1][3] * sub4) +
            m[0][2] * (m[1][0] * sub1 - m[1][1] * sub3 + m[1][3] * sub5) -
            m[0][3] * (m[1][0] * sub2 - m[1][1] * sub4 + m[1][2] * sub5));
}

constexpr Mat2 inverse_2x2(const Mat2& m) {
    /*
     * d = M00 * M11 - M01 * M10
     * M = [
     *      M11 / d, -M01 / d,
     *     -M10 / d,  M00 / d,
     * ]
     */
    float inv_det = 1 / (m[0][0] * m[1][1] - m[0][1] * m[1][0]);
    return {
         inv_det * m[1][1],
        -inv_det * m[0][1],
        -inv_det * m[1][0],
         inv_det * m[0][0],
    };
}

constexpr Mat3 inverse_3x3(const Mat3& m) {
    /*
     * i0 = M11 * M22 - M21 * M12
     * i1 = M12 * M20 - M10 * M22
     * i2 = M10 * M21 - M20 * M11
     * d = M00 * i0 + M01 * i1 + M02 * i2
     * M = [
     *     M11 * M22 - M21 * M12, M02 * M21 - M01 * M22, M01 * M12 - M02 * M11,
     *     M12 * M20 - M10 * M22, M00 * M22 - M02 * M20, M10 * M02 - M00 * M12,
     *     M10 * M21 - M20 * M11, M20 * M01 - M00 * M21, M00 * M11 - M10 * M01,
     * ]
     * M = M / d
     */
    float inv0 = m[1][1] * m[2][2] - m[2][1] * m[1][2];
    float inv1 = m[1][2] * m[2][0] - m[1][0] * m[2][2];
    float inv2 = m[1][0] * m[2][1] - m[2][0] * m[1][1];
    float inv_det = 1 / (m[0][0] * inv0 + m[0][1] * inv1 + m[0][2] * inv2);
    return {
        inv_det * inv0,
        inv_det * (m[0][2] * m[2][1] - m[0][1] * m[2][2]),
        inv_det * (m[0][1] * m[1][2] - m[0][2] * m[1][1]),
        inv_det * inv1,
        inv_det * (m[0][0] * m[2][2] - m[0][2] * m[2][0]),
        inv_det * (m[1][0] * m[0][2] - m[0][0] * m[1][2]),
        inv_det * inv2,
        inv_det * (m[2][0] * m[0][1] - m[0][0] * m[2][1]),
        inv_det * (m[0][0] * m[1][1] - m[1][0] * m[0][1]),
    };
}

constexpr Mat4 inverse_4x4(const Mat4& m) {
    /*
     * s00 = M22 * M33 - M23 * M32
     * s01 = M21 * M33 - M23 * M31
     * s02 = M21 * M32 - M22 * M31
     * s03 = M20 * M33 - M23 * M30
     * s04 = M20 * M32 - M22 * M30
     * s05 = M20 * M31 - M21 * M30
     * s06 = M12 * M33 - M13 * M32
     * s07 = M11 * M33 - M13 * M31
     * s08 = M11 * M32 - M12 * M31
     * s09 = M12 * M23 - M13 * M22
     * s10 = M11 * M23 - M13 * M21
     * s11 = M11 * M22 - M12 * M21
     * s12 = M10 * M33 - M13 * M30
     * s13 = M10 * M32 - M12 * M30
     * s14 = M10 * M23 - M13 * M20
     * s15 = M10 * M22 - M12 * M20
     * s16 = M10 * M31 - M11 * M30
     * s17 = M10 * M21 - M11 * M20
     * i00 = M11 * s00 - M12 * s01 + M13 * s02
     * i01 = M10 * s00 - M12 * s03 + M13 * s04
     * i02 = M10 * s01 - M11 * s03 + M13 * s05
     * i03 = M10 * s02 - M11 * s04 + M12 * s05
     * d = M00 * i00 - M01 * i01 + M02 * i02 - M03 * i03
     * M = [
     *      (M11 * s00 - M12 * s01 + M13 * s02),
     *     -(M01 * s00 - M02 * s01 + M03 * s02),
     *      (M01 * s06 - M02 * s07 + M03 * s08),
     *     -(M01 * s09 - M02 * s10 + M03 * s11),
     *     -(M10 * s00 - M12 * s03 + M13 * s04),
     *      (M00 * s00 - M02 * s03 + M03 * s04),
     *     -(M00 * s06 - M02 * s12 + M03 * s13),
     *      (M00 * s09 - M02 * s14 + M03 * s15),
     *      (M10 * s01 - M11 * s03 + M13 * s05),
     *     -(M00 * s01 - M01 * s03 + M03 * s05),
     *      (M00 * s07 - M01 * s12 + M03 * s16),
     *     -(M00 * s10 - M01 * s14 + M03 * s17),
     *     -(M10 * s02 - M11 * s04 + M12 * s05),
     *      (M00 * s02 - M01 * s04 + M02 * s05),
     *     -(M00 * s08 - M01 * s13 + M02 * s16),
     *      (M00 * s11 - M01 * s15 + M02 * s17),
     * ]
     * M = M / d
     */
    float sub00 = m[2][2] * m[3][3] - m[2][3] * m[3][2];
    float sub01 = m[2][1] * m[3][3] - m[2][3] * m[3][1];
    float sub02 = m[2][1] * m[3][2] - m[2][2] * m[3][1];
    float sub03 = m[2][0] * m[3][3] - m[2][3] * m[3][0];
    float sub04 = m[2][0] * m[3][2] - m[2][2] * m[3][0];
    float sub05 = m[2][0] * m[3][1] - m[2][1] * m[3][0];
    float sub06 = m[1][2] * m[3][3] - m[1][3] * m[3][2];
    float sub07 = m[1][1] * m[3][3] - m[1][3] * m[3][1];
    float sub08 = m[1][1] * m[3][2] - m[1][2] * m[3][1];
    float sub09 = m[1][2] * m[2][3] - m[1][3] * m[2][2];
    float sub10 = m[1][1] * m[2][3] - m[1][3] * m[2][1];
    float sub11 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
    float sub12 = m[1][0] * m[3][3] - m[1][3] * m[3][0];
    float sub13 = m[1][0] * m[3][2] - m[1][2] * m[3][0];
    float sub14 = m[1][0] * m[2][3] - m[1][3] * m[2][0];
    float sub15 = m[1][0] * m[2][2] - m[1][2] * m[2][0];
    float sub16 = m[1][0] * m[3][1] - m[1][1] * m[3][0];
    float sub17 = m[1][0] * m[2][1] - m[1][1] * m[2][0];
    float inv00 = m[1][1] * sub00 - m[1][2] * sub01 + m[1][3] * sub02;
    float inv01 = m[1][0] * sub00 - m[1][2] * sub03 + m[1][3] * sub04;
    float inv02 = m[1][0] * sub01 - m[1][1] * sub03 + m[1][3] * sub05;
    float inv03 = m[1][0] * sub02 - m[1][1] * sub04 + m[1][2] * sub05;
    float inv_det = 1 / (m[0][0] * inv00 - m[0][1] * inv01 + m[0][2] * inv02 - m[0][3] * inv03);
    return {
         inv_det * inv00,
        -inv_det * (m[0][1] * sub00 - m[0][2] * sub01 + m[0][3] * sub02),
         inv_det * (m[0][1] * sub06 - m[0][2] * sub07 + m[0][3] * sub08),
        -inv_det * (m[0][1] * sub09 - m[0][2] * sub10 + m[0][3] * sub11),
        -inv_det * inv01,
         inv_det * (m[0][0] * sub00 - m[0][2] * sub03 + m[0][3] * sub04),
        -inv_det * (m[0][0] * sub06 - m[0][2] * sub12 + m[0][3] * sub13),
         inv_det * (m[0][0] * sub09 - m[0][2] * sub14 + m[0][3] * sub15),
         inv_det * inv02,
        -inv_det * (m[0][0] * sub01 - m[0][1] * sub03 + m[0][3] * sub05),
         inv_det * (m[0][0] * sub07 - m[0][1] * sub12 + m[0][3] * sub16),
        -inv_det * (m[0][0] * sub10 - m[0][1] * sub14 + m[0][3] * sub17),
        -inv_det * inv03,
         inv_det * (m[0][0] * sub02 - m[0][1] * sub04 + m[0][2] * sub05),
        -inv_det * (m[0][0] * sub08 - m[0][1] * sub13 + m[0][2] * sub16),
         inv_det * (m[0][0] * sub11 - m[0][1] * sub15 + m[0][2] * sub17),
    };
}

constexpr double determinant_2x2(const DMat2& m) {
    /*
     * d = M00 * M11 - M01 * M10
     */
    return m[0][0] * m[1][1] - m[0][1] * m[1][0];
}

constexpr double determinant_3x3(const DMat3& m) {
    /*
     * s0 = M11 * M22 - M21 * M12
     * s1 = M12 * M20 - M10 * M22
     * s2 = M10 * M21 - M20 * M11
     * d = M00 * s0 + M01 * s1 + M02 * s2
     */
    double sub0 = m[1][1] * m[2][2] - m[2][1] * m[1][2];
    double sub1 = m[1][2] * m[2][0] - m[1][0] * m[2][2];
    double sub2 = m[1][0] * m[2][1] - m[2][0] * m[1][1];
    return m[0][0] * sub0 + m[0][1] * sub1 + m[0][2] * sub2;
}

constexpr double determinant_4x4(const DMat4& m) {
    /*
     * s0 = M22 * M33 - M32 * M23
     * s1 = M21 * M33 - M31 * M23
     * s2 = M21 * M32 - M31 * M22
     * s3 = M20 * M33 - M30 * M23
     * s4 = M20 * M32 - M30 * M22
     * s5 = M20 * M31 - M30 * M21
     * d = M00 * (M11 * s0 - M12 * s1 + M13 * s2) -
     *     M01 * (M10 * s0 - M12 * s3 + M13 * s4) +
     *     M02 * (M10 * s1 - M11 * s3 + M13 * s5) -
     *     M03 * (M10 * s2 - M11 * s4 + M12 * s5)
     */
    double sub0 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
    double sub1 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
    double sub2 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
    double sub3 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
    double sub4 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
    double sub5 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
    return m[0][0] * (m[1][1] * sub0 - m[1][2] * sub1 + m[1][3] * sub2) -
           m[0][1] * (m[1][0] * sub0 - m[1][2] * sub3 + m[1][3] * sub4) +
           m[0][2] * (m[1][0] * sub1 - m[1][1] * sub3 + m[1][3] * sub5) -
           m[0][3] * (m[1][0] * sub2 - m[1][1] * sub4 + m[1][2] * sub5);
}

constexpr DMat2 inverse_2x2(const DMat2& m) {
    /*
     * d = M00 * M11 - M01 * M10
     * M = [
     *      M11 / d, -M01 / d,
     *     -M10 / d,  M00 / d,
     * ]
     */
    double inv_det = 1 / (m[0][0] * m[1][1] - m[0][1] * m[1][0]);
    return {
         inv_det * m[1][1],
        -inv_det * m[0][1],
        -inv_det * m[1][0],
         inv_det * m[0][0],
    };
}

constexpr DMat3 inverse_3x3(const DMat3& m) {
    /*
     * i0 = M11 * M22 - M21 * M12
     * i1 = M12 * M20 - M10 * M22
     * i2 = M10 * M21 - M20 * M11
     * d = M00 * i0 + M01 * i1 + M02 * i2
     * M = [
     *     M11 * M22 - M21 * M12, M02 * M21 - M01 * M22, M01 * M12 - M02 * M11,
     *     M12 * M20 - M10 * M22, M00 * M22 - M02 * M20, M10 * M02 - M00 * M12,
     *     M10 * M21 - M20 * M11, M20 * M01 - M00 * M21, M00 * M11 - M10 * M01,
     * ]
     * M = M / d
     */
    double inv0 = m[1][1] * m[2][2] - m[2][1] * m[1][2];
    double inv1 = m[1][2] * m[2][0] - m[1][0] * m[2][2];
    double inv2 = m[1][0] * m[2][1] - m[2][0] * m[1][1];
    double inv_det = 1 / (m[0][0] * inv0 + m[0][1] * inv1 + m[0][2] * inv2);
    return {
        inv_det * inv0,
        inv_det * (m[0][2] * m[2][1] - m[0][1] * m[2][2]),
        inv_det * (m[0][1] * m[1][2] - m[0][2] * m[1][1]),
        inv_det * inv1,
        inv_det * (m[0][0] * m[2][2] - m[0][2] * m[2][0]),
        inv_det * (m[1][0] * m[0][2] - m[0][0] * m[1][2]),
        inv_det * inv2,
        inv_det * (m[2][0] * m[0][1] - m[0][0] * m[2][1]),
        inv_det * (m[0][0] * m[1][1] - m[1][0] * m[0][1]),
    };
}

constexpr DMat4 inverse_4x4(const DMat4& m) {
    /*
     * s00 = M22 * M33 - M23 * M32
     * s01 = M21 * M33 - M23 * M31
     * s02 = M21 * M32 - M22 * M31
     * s03 = M20 * M33 - M23 * M30
     * s04 = M20 * M32 - M22 * M30
     * s05 = M20 * M31 - M21 * M30
     * s06 = M12 * M33 - M13 * M32
     * s07 = M11 * M33 - M13 * M31
     * s08 = M11 * M32 - M12 * M31
     * s09 = M12 * M23 - M13 * M22
     * s10 = M11 * M23 - M13 * M21
     * s11 = M11 * M22 - M12 * M21
     * s12 = M10 * M33 - M13 * M30
     * s13 = M10 * M32 - M12 * M30
     * s14 = M10 * M23 - M13 * M20
     * s15 = M10 * M22 - M12 * M20
     * s16 = M10 * M31 - M11 * M30
     * s17 = M10 * M21 - M11 * M20
     * i00 = M11 * s00 - M12 * s01 + M13 * s02
     * i01 = M10 * s00 - M12 * s03 + M13 * s04
     * i02 = M10 * s01 - M11 * s03 + M13 * s05
     * i03 = M10 * s02 - M11 * s04 + M12 * s05
     * d = M00 * i00 - M01 * i01 + M02 * i02 - M03 * i03
     * M = [
     *      (M11 * s00 - M12 * s01 + M13 * s02),
     *     -(M01 * s00 - M02 * s01 + M03 * s02),
     *      (M01 * s06 - M02 * s07 + M03 * s08),
     *     -(M01 * s09 - M02 * s10 + M03 * s11),
     *     -(M10 * s00 - M12 * s03 + M13 * s04),
     *      (M00 * s00 - M02 * s03 + M03 * s04),
     *     -(M00 * s06 - M02 * s12 + M03 * s13),
     *      (M00 * s09 - M02 * s14 + M03 * s15),
     *      (M10 * s01 - M11 * s03 + M13 * s05),
     *     -(M00 * s01 - M01 * s03 + M03 * s05),
     *      (M00 * s07 - M01 * s12 + M03 * s16),
     *     -(M00 * s10 - M01 * s14 + M03 * s17),
     *     -(M10 * s02 - M11 * s04 + M12 * s05),
     *      (M00 * s02 - M01 * s04 + M02 * s05),
     *     -(M00 * s08 - M01 * s13 + M02 * s16),
     *      (M00 * s11 - M01 * s15 + M02 * s17),
     * ]
     * M = M / d
     */
    double sub00 = m[2][2] * m[3][3] - m[2][3] * m[3][2];
    double sub01 = m[2][1] * m[3][3] - m[2][3] * m[3][1];
    double sub02 = m[2][1] * m[3][2] - m[2][2] * m[3][1];
    double sub03 = m[2][0] * m[3][3] - m[2][3] * m[3][0];
    double sub04 = m[2][0] * m[3][2] - m[2][2] * m[3][0];
    double sub05 = m[2][0] * m[3][1] - m[2][1] * m[3][0];
    double sub06 = m[1][2] * m[3][3] - m[1][3] * m[3][2];
    double sub07 = m[1][1] * m[3][3] - m[1][3] * m[3][1];
    double sub08 = m[1][1] * m[3][2] - m[1][2] * m[3][1];
    double sub09 = m[1][2] * m[2][3] - m[1][3] * m[2][2];
    double sub10 = m[1][1] * m[2][3] - m[1][3] * m[2][1];
    double sub11 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
    double sub12 = m[1][0] * m[3][3] - m[1][3] * m[3][0];
    double sub13 = m[1][0] * m[3][2] - m[1][2] * m[3][0];
    double sub14 = m[1][0] * m[2][3] - m[1][3] * m[2][0];
    double sub15 = m[1][0] * m[2][2] - m[1][2] * m[2][0];
    double sub16 = m[1][0] * m[3][1] - m[1][1] * m[3][0];
    double sub17 = m[1][0] * m[2][1] - m[1][1] * m[2][0];
    double inv00 = m[1][1] * sub00 - m[1][2] * sub01 + m[1][3] * sub02;
    double inv01 = m[1][0] * sub00 - m[1][2] * sub03 + m[1][3] * sub04;
    double inv02 = m[1][0] * sub01 - m[1][1] * sub03 + m[1][3] * sub05;
    double inv03 = m[1][0] * sub02 - m[1][1] * sub04 + m[1][2] * sub05;
    double inv_det = 1 / (m[0][0] * inv00 - m[0][1] * inv01 + m[0][2] * inv02 - m[0][3] * inv03);
    return {
         inv_det * inv00,
        -inv_det * (m[0][1] * sub00 - m[0][2] * sub01 + m[0][3] * sub02),
         inv_det * (m[0][1] * sub06 - m[0][2] * sub07 + m[0][3] * sub08),
        -inv_det * (m[0][1] * sub09 - m[0][2] * sub10 + m[0][3] * sub11),
        -inv_det * inv01,
         inv_det * (m[0][0] * sub00 - m[0][2] * sub03 + m[0][3] * sub04),
        -inv_det * (m[0][0] * sub06 - m[0][2] * sub12 + m[0][3] * sub13),
         inv_det * (m[0][0] * sub09 - m[0][2] * sub14 + m[0][3] * sub15),
         inv_det * inv02,
        -inv_det * (m[0][0] * sub01 - m[0][1] * sub03 + m[0][3] * sub05),
         inv_det * (m[0][0] * sub07 - m[0][1] * sub12 + m[0][3] * sub16),
        -inv_det * (m[0][0] * sub10 - m[0][1] * sub14 + m[0][3] * sub17),
        -inv_det * inv03,
         inv_det * (m[0][0] * sub02 - m[0][1] * sub04 + m[0][2] * sub05),
        -inv_det * (m[0][0] * sub08 - m[0][1] * sub13 + m[0][2] * sub16),
         inv_det * (m[0][0] * sub11 - m[0][1] * sub15 + m[0][2] * sub17),
    };
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/math/Euler.h ---------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

enum EulerOrder {
    EULER_XYZ,
    EULER_XZY,
    EULER_YXZ,
    EULER_YZX,
    EULER_ZXY,
    EULER_ZYX,
};

class Euler {
public:
    float x = 0;                     /**< the rotation angle of the X axis */
    float y = 0;                     /**< the rotation angle of the Y axis */
    float z = 0;                     /**< the rotation angle of the Z axis */
    EulerOrder order = EULER_XYZ;    /**< the order of rotations */
    
    /**
     * Creates a new Euler object.
     */
    Euler() = default;
    
    /**
     * Creates a new Euler object and initializes it with rotations and order.
     *
     * \param x the rotation angle of the X axis
     * \param y the rotation angle of the Y axis
     * \param z the rotation angle of the Z axis
     * \param o the order of rotations
     */
    Euler(float x, float y, float z, EulerOrder o = EULER_XYZ);
    
    /**
     * Creates a new Euler object and initializes it with rotations and order.
     *
     * \param r the rotation vector
     * \param o the order of rotations
     */
    Euler(const Vec3& r, EulerOrder o = EULER_XYZ);
    
    /**
     * Transforms the Euler angles to rotation matrix.
     */
    Mat3 to_rotation_matrix() const;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/math/Random.h --------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class Random {
public:
    /**
     * Generates a random uniformly distributed number in range [0, 1).
     */
    static double random();
    
    /**
     * Generates a random uniformly distributed number in range [0, 1).
     */
    static float random_f();
    
    /**
     * Sets the seed of the random number generator. The default is zero.
     *
     * \param s seed
     */
    static void set_seed(unsigned int s);
    
private:
    static std::mt19937 generator;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/math/Ray.h ------------------------------------------------------ */
/* -------------------------------------------------------------------------- */

namespace ink {

class Ray {
public:
    Vec3 origin;       /**< the origin of the ray */
    Vec3 direction;    /**< the direction of the ray, must be normalized */
    
    /**
     * Creates a new Ray object.
     */
    Ray() = default;
    
    /**
     * Creates a new Ray object and initializes it with origin and direction.
     *
     * \param o the origin of the ray
     * \param d the direction of the ray, must be normalized
     */
    Ray(const Vec3& o, const Vec3& d);
    
    /**
     * Intersects the ray with the AABB box. Returns the distance to the
     * intersection point if there is an intersection, returns -1 otherwise.
     *
     * \param l the lower boundary of the box
     * \param u the upper boundary of the box
     */
    float intersect_box(const Vec3& l, const Vec3& u) const;
    
    /**
     * Intersects the ray with the plane. Returns the distance to the
     * intersection point if there is an intersection, returns -1 otherwise.
     *
     * \param n the normal of the plane, must be normalized
     * \param d the distance from the origin to the plane
     */
    float intersect_plane(const Vec3& n, float d) const;
    
    /**
     * Intersects the ray with the sphere. Returns the distance to the
     * intersection point if there is an intersection, returns -1 otherwise.
     *
     * \param c the center of the sphere
     * \param r the radius of the sphere
     */
    float intersect_sphere(const Vec3& c, float r) const;
    
    /**
     * Intersects the ray with the triangle. Returns the distance to the
     * intersection point if there is an intersection, returns -1 otherwise.
     *
     * \param a the vertex A of the triangle
     * \param b the vertex B of the triangle
     * \param c the vertex C of the triangle
     */
    float intersect_triangle(const Vec3& a, const Vec3& b, const Vec3& c) const;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/objects/Defines.h ----------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class Defines {
public:
    /**
     * Creates a new Defines object.
     */
    Defines() = default;
    
    /**
     * Returns a string containing all the preprocessor defines.
     */
    std::string get() const;
    
    /**
     * Sets another Defines object to this object.
     *
     * \param d defines
     */
    void set(const Defines& d);
    
    /**
     * Sets the preprocessor defines for the shader. These defines are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n macro name
     */
    void set(const std::string& n);
    
    /**
     * Sets the preprocessor defines for the shader. These defines are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n macro name
     * \param v value
     */
    void set(const std::string& n, const std::string& v);
    
    /**
     * Sets the preprocessor defines for the shader. These defines are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n macro name
     * \param v value
     */
    void set_i(const std::string& n, int v);
    
    /**
     * Sets the preprocessor defines for the shader. These defines are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n macro name
     * \param v value
     */
    void set_l(const std::string& n, long v);
    
    /**
     * Sets the preprocessor defines for the shader. These defines are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n macro name
     * \param v value
     */
    void set_ll(const std::string& n, long long v);
    
    /**
     * If the flag is true, Sets the preprocessor defines for the shader. These
     * defines are then available in the vertex, geometry, and fragment shaders.
     *
     * \param n macro name
     * \param f flag
     */
    void set_if(const std::string& n, bool f);
    
    /**
     * Removes all the preprocessor defines from this Defines object.
     */
    void clear();
    
private:
    std::string defines;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/objects/Enums.h ------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

enum ColorConversion {
    COLOR_RGB_TO_BGR,
    COLOR_BGR_TO_RGB,
    COLOR_SRGB_TO_RGB,
    COLOR_RGB_TO_SRGB,
    COLOR_RGB_TO_XYZ,
    COLOR_XYZ_TO_RGB,
    COLOR_RGB_TO_HSV,
    COLOR_HSV_TO_RGB,
    COLOR_RGB_TO_HSL,
    COLOR_HSL_TO_RGB,
};

enum RenderSide {
    FRONT_SIDE,
    BACK_SIDE,
    DOUBLE_SIDE,
};

enum ImageType {
    IMAGE_UBYTE,
    IMAGE_BYTE,
    IMAGE_USHORT,
    IMAGE_SHORT,
    IMAGE_UINT,
    IMAGE_INT,
    IMAGE_HALF_FLOAT,
    IMAGE_FLOAT,
    IMAGE_UINT_24_8,
};

enum ImageFormat {
    IMAGE_COLOR,
    IMAGE_COLOR_INTEGER,
    IMAGE_DEPTH,
    IMAGE_STENCIL,
    IMAGE_DEPTH_STENCIL,
};

enum ComparisonFunc {
    FUNC_NEVER,
    FUNC_LESS,
    FUNC_EQUAL,
    FUNC_LEQUAL,
    FUNC_GREATER,
    FUNC_NOTEQUAL,
    FUNC_GEQUAL,
    FUNC_ALWAYS,
};

enum StencilOperation {
    STENCIL_ZERO,
    STENCIL_KEEP,
    STENCIL_REPLACE,
    STENCIL_INCR,
    STENCIL_DECR,
    STENCIL_INCR_WRAP,
    STENCIL_DECR_WRAP,
    STENCIL_INVERT,
};

enum BlendOperation {
    BLEND_ADD,
    BLEND_SUBTRACT,
    BLEND_REVERSE_SUBTRACT,
    BLEND_MIN,
    BLEND_MAX,
};

enum BlendFactor {
    FACTOR_ZERO,
    FACTOR_ONE,
    FACTOR_SRC_COLOR,
    FACTOR_ONE_MINUS_SRC_COLOR,
    FACTOR_DST_COLOR,
    FACTOR_ONE_MINUS_DST_COLOR,
    FACTOR_SRC_ALPHA,
    FACTOR_ONE_MINUS_SRC_ALPHA,
    FACTOR_DST_ALPHA,
    FACTOR_ONE_MINUS_DST_ALPHA,
};

enum TextureType {
    TEXTURE_1D,
    TEXTURE_2D,
    TEXTURE_3D,
    TEXTURE_CUBE,
    TEXTURE_1D_ARRAY,
    TEXTURE_2D_ARRAY,
    TEXTURE_CUBE_ARRAY,
};

enum TextureFormat {
    TEXTURE_R8_UNORM,
    TEXTURE_R8_SNORM,
    TEXTURE_R16_UNORM,
    TEXTURE_R16_SNORM,
    TEXTURE_R8G8_UNORM,
    TEXTURE_R8G8_SNORM,
    TEXTURE_R16G16_UNORM,
    TEXTURE_R16G16_SNORM,
    TEXTURE_R3G3B2_UNORM,
    TEXTURE_R4G4B4_UNORM,
    TEXTURE_R5G5B5_UNORM,
    TEXTURE_R8G8B8_UNORM,
    TEXTURE_R8G8B8_SNORM,
    TEXTURE_R10G10B10_UNORM,
    TEXTURE_R12G12B12_UNORM,
    TEXTURE_R16G16B16_UNORM,
    TEXTURE_R2G2B2A2_UNORM,
    TEXTURE_R4G4B4A4_UNORM,
    TEXTURE_R5G5B5A1_UNORM,
    TEXTURE_R8G8B8A8_UNORM,
    TEXTURE_R8G8B8A8_SNORM,
    TEXTURE_R10G10B10A2_UNORM,
    TEXTURE_R10G10B10A2_UINT,
    TEXTURE_R12G12B12A12_UNORM,
    TEXTURE_R16G16B16A16_UNORM,
    TEXTURE_R8G8B8_SRGB,
    TEXTURE_R8G8B8A8_SRGB,
    TEXTURE_R16_SFLOAT,
    TEXTURE_R16G16_SFLOAT,
    TEXTURE_R16G16B16_SFLOAT,
    TEXTURE_R16G16B16A16_SFLOAT,
    TEXTURE_R32_SFLOAT,
    TEXTURE_R32G32_SFLOAT,
    TEXTURE_R32G32B32_SFLOAT,
    TEXTURE_R32G32B32A32_SFLOAT,
    TEXTURE_R11G11B10_SFLOAT,
    TEXTURE_R9G9B9E5_UNORM,
    TEXTURE_R8_SINT,
    TEXTURE_R8_UINT,
    TEXTURE_R16_SINT,
    TEXTURE_R16_UINT,
    TEXTURE_R32_SINT,
    TEXTURE_R32_UINT,
    TEXTURE_R8G8_SINT,
    TEXTURE_R8G8_UINT,
    TEXTURE_R16G16_SINT,
    TEXTURE_R16G16_UINT,
    TEXTURE_R32G32_SINT,
    TEXTURE_R32G32_UINT,
    TEXTURE_R8G8B8_SINT,
    TEXTURE_R8G8B8_UINT,
    TEXTURE_R16G16B16_SINT,
    TEXTURE_R16G16B16_UINT,
    TEXTURE_R32G32B32_SINT,
    TEXTURE_R32G32B32_UINT,
    TEXTURE_R8G8B8A8_SINT,
    TEXTURE_R8G8B8A8_UINT,
    TEXTURE_R16G16B16A16_SINT,
    TEXTURE_R16G16B16A16_UINT,
    TEXTURE_R32G32B32A32_SINT,
    TEXTURE_R32G32B32A32_UINT,
    TEXTURE_D16_UNORM,
    TEXTURE_D24_UNORM,
    TEXTURE_D32_SFLOAT,
    TEXTURE_D24_UNORM_S8_UINT,
    TEXTURE_D32_SFLOAT_S8_UINT,
};

enum TextureWrappingMode {
    TEXTURE_REPEAT,
    TEXTURE_MIRRORED_REPEAT,
    TEXTURE_CLAMP_TO_EDGE,
    TEXTURE_CLAMP_TO_BORDER,
};

enum TextureFilter {
    TEXTURE_NEAREST,
    TEXTURE_LINEAR,
    TEXTURE_NEAREST_MIPMAP_NEAREST,
    TEXTURE_LINEAR_MIPMAP_NEAREST,
    TEXTURE_NEAREST_MIPMAP_LINEAR,
    TEXTURE_LINEAR_MIPMAP_LINEAR,
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/objects/Image.h ------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class Image {
public:
    int width = 0;                /**< the width in pixels */
    int height = 0;               /**< the height in pixels */
    int channel = 0;              /**< the channel per pixel */
    int bytes = 1;                /**< the bytes per channel */
    
    std::vector<uint8_t> data;    /**< the data source */
    
    /**
     * Creates a new Image object.
     */
    Image() = default;
    
    /**
     * Creates a new Image object and initializes it with size, channel, and
     * bytes.
     *
     * \param w the width in pixels
     * \param h the height in pixels
     * \param c the channel per pixel
     * \param b the bytes per channel
     */
    Image(int w, int h, int c, int b = 1);
    
    /**
     * Returns a sub-image sliced from the current image. The new image region
     * spans from (x1, y1) to (x2, y2).
     *
     * \param x1 the X-coordinate of the upper left corner
     * \param y1 the Y-coordinate of the upper left corner
     * \param x2 the X-coordinate of the lower right corner
     * \param y2 the Y-coordinate of the lower right corner
     */
    Image subimage(int x1, int y1, int x2, int y2) const;
    
    /**
     * Flips the image vertically.
     */
    void flip_vertical();
    
    /**
     * Flips the image horizontally.
     */
    void flip_horizontal();
    
    /**
     * Returns an image list split by channel.
     */
    std::vector<Image> split() const;
    
    /**
     * Converts this image from one color space to another color space.
     *
     * \param c color conversion
     */
    void convert(ColorConversion c);
    
    /**
     * Converts this image from the RGB color space to the BGR color space.
     */
    template <typename Type>
    void convert_rgb_to_bgr();
    
    /**
     * Converts this image from the BGR color space to the RGB color space.
     */
    template <typename Type>
    void convert_bgr_to_rgb();
    
    /**
     * Converts this image from the RGB color space to the sRGB color space.
     */
    template <typename Type>
    void convert_rgb_to_srgb();
    
    /**
     * Converts this image from the sRGB color space to the RGB color space.
     */
    template <typename Type>
    void convert_srgb_to_rgb();
    
    /**
     * Converts this image from the RGB color space to the XYZ color space.
     */
    template <typename Type>
    void convert_rgb_to_xyz();
    
    /**
     * Converts this image from the XYZ color space to the RGB color space.
     */
    template <typename Type>
    void convert_xyz_to_rgb();
    
    /**
     * Converts this image from the RGB color space to the HSV color space.
     */
    template <typename Type>
    void convert_rgb_to_hsv();
    
    /**
     * Converts this image from the HSV color space to the RGB color space.
     */
    template <typename Type>
    void convert_hsv_to_rgb();
    
    /**
     * Converts this image from the RGB color space to the HSL color space.
     */
    template <typename Type>
    void convert_rgb_to_hsl();
    
    /**
     * Converts this image from the HSL color space to the RGB color space.
     */
    template <typename Type>
    void convert_hsl_to_rgb();
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/objects/Mesh.h -------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

struct MeshGroup {
    std::string name;
    int position = 0;
    int length = 0;
};

class Mesh {
public:
    std::string name;                 /**< mesh name */
    
    std::vector<MeshGroup> groups;    /**< mesh groups */
    
    std::vector<Vec3> vertex;         /**< the position for each vertex */
    std::vector<Vec3> normal;         /**< the normal for each vertex */
    std::vector<Vec2> uv;             /**< the UV for each vertex */
    std::vector<Vec4> tangent;        /**< the tangent for each vertex */
    std::vector<Vec3> color;          /**< the color for each vertex */
    
    /**
     * Creates a new Mesh object and initializes it with name.
     *
     * \param n mesh name
     */
    Mesh(const std::string& n = "");
    
    /**
     * Translates the mesh. This operation will modify the mesh data.
     *
     * \param x X coordinate
     * \param y Y coordinate
     * \param z Z coordinate
     */
    void translate(float x, float y, float z);
    
    /**
     * Translates the mesh. This operation will modify the mesh data.
     *
     * \param t translate vector
     */
    void translate(const Vec3& t);
    
    /**
     * Rotates the mesh about the X axis. This operation will modify the mesh
     * data.
     *
     * \param a angle
     */
    void rotate_x(float a);
    
    /**
     * Rotates the mesh about the Y axis. This operation will modify the mesh
     * data.
     *
     * \param a angle
     */
    void rotate_y(float a);
    
    /**
     * Rotates the mesh about the Z axis. This operation will modify the mesh
     * data.
     *
     * \param a angle
     */
    void rotate_z(float a);
    
    /**
     * Rotates the mesh with Euler angles. This operation will modify the mesh
     * data.
     *
     * \param e Euler angles
     */
    void rotate(const Euler& e);
    
    /**
     * Scales the mesh. This operation will modify the mesh data.
     *
     * \param x X coordinate
     * \param y Y coordinate
     * \param z Z coordinate
     */
    void scale(float x, float y, float z);
    
    /**
     * Scales the mesh. This operation will modify the mesh data.
     *
     * \param s scale vector
     */
    void scale(const Vec3& s);
    
    /**
     * Normalizes all the normal vectors of the mesh.
     */
    void normalize();
    
    /**
     * Calculates normals from the vertices, adds these normals to the mesh.
     */
    void create_normals();
    
    /**
     * Calculates tangents from the vertices, normals, and UVs information, adds
     * these tangents to the mesh.
     */
    void create_tangents();
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/objects/Instance.h ---------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class Instance {
public:
    std::string name;              /**< instance name */
    
    bool visible = true;           /**< whether the instance will be rendered */
    
    bool cast_shadow = true;       /**< whether the instance will cast shadows */
    
    int priority = 0;              /**< the sorting priority in rendering */
    
    Vec3 position = {0, 0, 0};     /**< the position vector of the instance */
    
    Vec3 scale = {1, 1, 1};        /**< the scaling vector of the instance */
    
    Euler rotation;                /**< the rotation angles of the instance */
    
    Mat4 matrix_local;             /**< the transform matrix in the local space */
    
    Mat4 matrix_global;            /**< the transform matrix in the global space */
    
    Mesh* mesh = nullptr;          /**< the linked mesh of the instance */
    
    /**
     * Creates a new Instance object, which is the minimum unit of rendering.
     *
     * \param n instance name
     */
    Instance(const std::string& n = "");
    
    /**
     * Adds the specified instance as the child of this instance. The index
     * starts from zero.
     *
     * \param i instance
     */
    void add(Instance* i);
    
    /**
     * Adds the specified instances as the children of this instance. The index
     * starts from zero.
     *
     * \param l instance list
     */
    void add(const std::initializer_list<Instance*>& l);
    
    /**
     * Removes the specified instance as the child of this instance.
     *
     * \param i instance
     */
    void remove(Instance* i);
    
    /**
     * Removes the specified instances as the children of this instance.
     *
     * \param l instance list
     */
    void remove(const std::initializer_list<Instance*>& l);
    
    /**
     * Removes all the children from this instance.
     */
    void clear();
    
    /**
     * Returns the child at the specified index of this instance.
     *
     * \param i the index of the child
     */
    Instance* get_child(int i) const;
    
    /**
     * Returns the first child matching the specified name of this instance.
     *
     * \param n the name of the child
     */
    Instance* get_child(const std::string& n) const;
    
    /**
     * Returns the number of children.
     */
    size_t get_child_count() const;
    
    /**
     * Returns the parent of this instance.
     */
    Instance* get_parent() const;
    
    /**
     * Sets the transform (position, rotation and scale) to this instance.
     *
     * \param p position vector
     * \param r rotation angles
     * \param s scale vector
     */
    void set_transform(const Vec3& p, const Euler& r, const Vec3& s);
    
    /**
     * Updates the local transform matrix.
     * This function is equivalent to "matrix_local = transform();".
     */
    void update_matrix_local();
    
    /**
     * Updates the global transform matrix.
     * This function is equivalent to "matrix_global = transform_global();".
     */
    void update_matrix_global();
    
    /**
     * Converts the vector from the global space to the local space.
     * This function only works when matrix_global is prepared.
     *
     * \param v vector
     */
    Vec3 global_to_local(const Vec3& v) const;
    
    /**
     * Converts the vector from the local space to the global space.
     * This function only works when matrix_global is prepared.
     *
     * \param v vector
     */
    Vec3 local_to_global(const Vec3& v) const;
    
    /**
     * Returns a transform matrix in the local space, calculating from the
     * position, rotation, and scale.
     */
    Mat4 transform() const;
    
    /**
     * Returns a transform matrix in the global space, by multiplying the
     * transform matrices of its ancestors.
     */
    Mat4 transform_global() const;
    
    /**
     * Returns a transform matrix calculating from the specified position,
     * rotation and scale.
     *
     * \param p position vector
     * \param r rotation angles
     * \param s scale vector
     */
    static Mat4 transform(const Vec3& p, const Euler& r, const Vec3& s);
    
protected:
    Instance* parent = nullptr;
    
    std::vector<Instance*> children;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/objects/Uniforms.h ---------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class Uniforms {
public:
    /**
     * Creates a new Uniforms object.
     */
    Uniforms() = default;
    
    /**
     * Returns the number of uniform variables.
     */
    size_t get_count() const;
    
    /**
     * Returns the uniform variable name at the specified index.
     *
     * \param i the index of the uniform
     */
    std::string get_name(int i) const;
    
    /**
     * Returns the uniform variable type at the specified index.
     *
     * \param i the index of the uniform
     */
    int get_type(int i) const;
    
    /**
     * Returns the uniform variable location at the specified index.
     *
     * \param i the index of the uniform
     */
    int get_location(int i) const;
    
    /**
     * Returns the data of the uniform data block.
     */
    float* get_data();
    
    /**
     * Returns the data of the uniform data block.
     */
    const float* get_data() const;
    
    /**
     * Sets the value of the uniform variable. These uniform variables are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n variable name
     * \param v value
     */
    void set_i(const std::string& n, int v);
    
    /**
     * Sets the value of the uniform variable. These uniform variables are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n variable name
     * \param v value
     */
    void set_u(const std::string& n, unsigned int v);
    
    /**
     * Sets the value of the uniform variable. These uniform variables are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n variable name
     * \param v value
     */
    void set_f(const std::string& n, float v);
    
    /**
     * Sets the value of the uniform variable. These uniform variables are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n variable name
     * \param v value
     */
    void set_v2(const std::string& n, const Vec2& v);
    
    /**
     * Sets the value of the uniform variable. These uniform variables are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n variable name
     * \param v value
     */
    void set_v3(const std::string& n, const Vec3& v);
    
    /**
     * Sets the value of the uniform variable. These uniform variables are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n variable name
     * \param v value
     */
    void set_v4(const std::string& n, const Vec4& v);
    
    /**
     * Sets the value of the uniform variable. These uniform variables are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n variable name
     * \param v value
     */
    void set_m2(const std::string& n, const Mat2& v);
    
    /**
     * Sets the value of the uniform variable. These uniform variables are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n variable name
     * \param v value
     */
    void set_m3(const std::string& n, const Mat3& v);
    
    /**
     * Sets the value of the uniform variable. These uniform variables are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n variable name
     * \param v value
     */
    void set_m4(const std::string& n, const Mat4& v);
    
    /**
     * Removes all the uniform variables from this Uniforms object.
     */
    void clear();
    
private:
    std::vector<float> data;
    
    std::vector<std::tuple<std::string, int, int>> uniforms;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/objects/Material.h ---------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class Material {
public:
    std::string name;                     /**< material name */
    
    RenderSide side;                      /**< which side of faces will be rendered */
    
    RenderSide shadow_side;               /**< which side of faces will cast shadows */
    
    bool visible = true;                  /**< whether the material will be rendered */
    
    bool wireframe = false;               /**< whether to render mesh as wireframe */
    
    bool depth_test = true;               /**< whether to enable depth test in rendering */
    
    ComparisonFunc depth_func;            /**< which depth comparison function to use */
    
    bool stencil_test = false;            /**< whether to enable stencil test in rendering */
    
    int stencil_writemask = 0xFF;         /**< the mask when writing to stencil buffer */
    
    int stencil_ref = 0;                  /**< the reference value to be used in stencil comparison */
    
    int stencil_mask = 0xFF;              /**< the mask to be used in stencil comparison */
    
    ComparisonFunc stencil_func;          /**< which stencil comparison function to use */
    
    StencilOperation stencil_fail;        /**< the operation when stencil test fails */
    
    StencilOperation stencil_zfail;       /**< the operation when stencil test passes but depth test fails */
    
    StencilOperation stencil_zpass;       /**< the operation when both stencil test and depth test pass */
    
    bool blending = false;                /**< whether to enable blending in rendering */
    
    BlendOperation blend_op_rgb;          /**< which RGB blend operation to use in blending */
    
    BlendOperation blend_op_alpha;        /**< which alpha blend operation to use in blending */
    
    BlendFactor blend_src_rgb;            /**< the RGB source blend factor in blending */
    
    BlendFactor blend_src_alpha;          /**< the alpha source blend factor in blending */
    
    BlendFactor blend_dst_rgb;            /**< the RGB destination blend factor in blending */
    
    BlendFactor blend_dst_alpha;          /**< the alpha destination blend factor in blending */
    
    float alpha_test = 0;                 /**< threshold of alpha test, pixels with lower alpha will be discarded */
    
    bool use_map_with_alpha = true;       /**< whether to use alpha channel from color map */
    
    bool use_vertex_color = false;        /**< whether to use vertex colors from mesh */
    
    bool use_tangent_space = true;        /**< whether the normal map is defined in tangent space */
    
    float normal_scale = 1;               /**< how much the normal map affects the material */
    
    float displacement_scale = 1;         /**< how much the displacement map affects the mesh */
    
    Vec3 color = {1, 1, 1};               /**< the base color of the material, default is white */
    
    float alpha = 1;                      /**< the opacity of the material, range is 0 to 1 */
    
    float specular = 0.5;                 /**< how specular the material appears, range is 0 to 1 */
    
    float metalness = 0;                  /**< how metallic the material appears, range is 0 to 1 */
    
    float roughness = 1;                  /**< how rough the material appears, range is 0 to 1 */
    
    Vec3 emissive = {0, 0, 0};            /**< the emissive color of the material, default is black */
    
    float emissive_intensity = 1;         /**< the emissive intensity of the material, range is 0 to 1 */
    
    float ao_intensity = 1;               /**< the occlusion intensity of the material, range is 0 to 1 */
    
    Image* normal_map = nullptr;          /**< the map determines the normals of the mesh */
    
    Image* displacement_map = nullptr;    /**< the map determines the offsets of the vertices */
    
    Image* color_map = nullptr;           /**< the map affects the base color of the material */
    
    Image* alpha_map = nullptr;           /**< the map affects the alpha of the material */
    
    Image* roughness_map = nullptr;       /**< the map affects how rough the material appears */
    
    Image* metalness_map = nullptr;       /**< the map affects how metallic the material appears */
    
    Image* specular_map = nullptr;        /**< the map affects how specular the material appears */
    
    Image* emissive_map = nullptr;        /**< the map affects the emissive color of the material */
    
    Image* ao_map = nullptr;              /**< the map affects the ambient occlusion of the material */
    
    Image* custom_maps[16];               /**< the custom maps of the material */
    
    void* shader = nullptr;               /**< custom shader determines how lights affect material */
    
    void* reflection_probe = nullptr;     /**< reflection probe affects the environment light of the material */
    
    Uniforms* uniforms = nullptr;         /**< the custom uniforms of the material */
    
    /**
     * Creates a new Material object and initializes it with name.
     *
     * \param n material name
     */
    Material(const std::string& n = "");
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/loader/Loader.h ------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

struct LoadObject {
    std::vector<Mesh> mesh;
    std::vector<Material> material;
};

struct LoadObjOptions {
    bool vertex_color = false;
    std::string group = "g";
};

class Loader {
public:
    /**
     * Loads the image data from the specified file into an image.
     *
     * \param p the path to the file
     */
    static Image load_image(const std::string& p);
    
    /**
     * Loads the image data from the specified file into a HDR image.
     *
     * \param p the path to the file
     */
    static Image load_image_hdr(const std::string& p);
    
    /**
     * Loads the material data from the specified MTL file into a material list.
     *
     * \param p the path to the file
     */
    static LoadObject load_mtl(const std::string& p);
    
    /**
     * Loads the mesh data from the specified OBJ file into a mesh list. Meshes
     * are divided by the custom grouping keyword.
     *
     * \param p the path to the file
     * \param o options for loading OBJ file
     */
    static LoadObject load_obj(const std::string& p, const LoadObjOptions& o = {});
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/camera/Camera.h ------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class Camera {
public:
    float near = 0;                 /**< the nearer clipping plane */
    float far = 0;                  /**< the farther clipping plane */
    
    Vec3 position = {0, 0, 0};      /**< the position of the camera */
    Vec3 direction = {0, 0, -1};    /**< the viewing direction of the camera */
    Vec3 up = {0, 1, 0};            /**< the view-up vector of the camera */
    
    Mat4 viewing;                   /**< the matrix of viewing transform */
    Mat4 projection;                /**< the matrix of projection transform */
    
    /**
     * Creates a new Camera object.
     */
    Camera() = default;
    
    /**
     * Returns true if this camera is perspective.
     */
    bool is_perspective() const;
    
    /**
     * Sets the position, viewing direction, view-up vector of the camera.
     * Updates the viewing matrix and the projection matrix.
     *
     * \param p position of the camera
     * \param d viewing direction (from target to the camera)
     * \param u view-up vector
     */
    void lookat(const Vec3& p, const Vec3& d, const Vec3& u);
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/camera/OrthoCamera.h -------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class OrthoCamera : public Camera {
public:
    float left = 0;     /**< the left clipping plane */
    float right = 0;    /**< the right clipping plane */
    float lower = 0;    /**< the lower clipping plane */
    float upper = 0;    /**< the upper clipping plane */
    
    /**
     * Creates a new OrthoCamera object.
     */
    OrthoCamera() = default;
    
    /**
     * Creates a new OrthoCamera and initializes it with left, right, lower,
     * upper, near, far clipping planes.
     *
     * \param le the left clipping plane
     * \param ri the right clipping plane
     * \param lo the lower clipping plane
     * \param up the upper clipping plane
     * \param n the nearer clipping plane
     * \param f the farther clipping plane
     */
    OrthoCamera(float le, float ri, float lo, float up, float n, float f);
    
    /**
     * Sets the left, right, lower, upper, near, far clipping planes. Updates
     * the projection matrix of the camera.
     *
     * \param le the left clipping plane
     * \param ri the right clipping plane
     * \param lo the lower clipping plane
     * \param up the upper clipping plane
     * \param n the nearer clipping plane
     * \param f the farther clipping plane
     */
    void set(float le, float ri, float lo, float up, float n, float f);
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/camera/PerspCamera.h -------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class PerspCamera : public Camera {
public:
    float fov_y = 0;     /**< the field of view on the Y-axis */
    float aspect = 0;    /**< the aspect ratio of FOV */
    
    /**
     * Creates a new PerspCamera object.
     */
    PerspCamera() = default;
    
    /**
     * Creates a new PerspCamera and initializes it with FOV, aspect, near and
     * far.
     *
     * \param fov the field of view on the Y-axis
     * \param a the aspect ratio of FOV
     * \param n the nearer clipping plane
     * \param f the farther clipping plane
     */
    PerspCamera(float fov, float a, float n, float f);
    
    /**
     * Sets the FOV, aspect, near and far. Updates the projection matrix of the
     * camera.
     *
     * \param fov the field of view on the Y-axis
     * \param a the aspect ratio of FOV
     * \param n the nearer clipping plane
     * \param f the farther clipping plane
     */
    void set(float fov, float a, float n, float f);
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/meshes/BoxMesh.h ------------------------------------------------ */
/* -------------------------------------------------------------------------- */

namespace ink {

class BoxMesh {
public:
    /**
     * Returns a box shaped mesh of width 1, height 1, and depth 1.
     * The box is centred on the origin.
     */
    static Mesh create();
    
private:
    static std::vector<Vec3> vertex;
    static std::vector<Vec2> uv;
    static std::vector<Vec3> normal;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/meshes/CylinderMesh.h ------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class CylinderMesh {
public:
    /**
     * Returns a cylinder shaped mesh of height 1 and radius 0.5.
     * The cylinder is centred on the origin.
     */
    static Mesh create();
    
private:
    static std::vector<Vec3> vertex;
    static std::vector<Vec2> uv;
    static std::vector<Vec3> normal;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/meshes/PlaneMesh.h ---------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class PlaneMesh {
public:
    /**
     * Returns a plane shaped mesh of width 1 and height 1.
     * The plane is centred on the origin.
     */
    static Mesh create();
    
private:
    static std::vector<Vec3> vertex;
    static std::vector<Vec2> uv;
    static std::vector<Vec3> normal;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/meshes/SphereMesh.h --------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class SphereMesh {
public:
    /**
     * Returns a sphere shaped mesh of radius 0.5.
     * The sphere is centred on the origin.
     */
    static Mesh create();
    
private:
    static std::vector<Vec3> vertex;
    static std::vector<Vec2> uv;
    static std::vector<Vec3> normal;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/graphics/Gpu.h -------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink::gpu {

class Rect {
public:
    int x = 0;         /**< the X-coordinate of the lower left corner */
    int y = 0;         /**< the Y-coordinate of the lower left corner */
    int width = 0;     /**< the width of the rectangle */
    int height = 0;    /**< the height of the rectangle */
    
    /**
     * Creates a new Rect object.
     */
    Rect() = default;
    
    /**
     * Creates a new Rect object and initializes it with size.
     *
     * \param w the width of the rectangle
     * \param h the height of the rectangle
     */
    Rect(int w, int h);
    
    /**
     * Creates a new Rect object and initializes it with size and position.
     *
     * \param x the X-coordinate of the lower left corner
     * \param y the Y-coordinate of the lower left corner
     * \param w the width of the rectangle
     * \param h the height of the rectangle
     */
    Rect(int x, int y, int w, int h);
};

class State {
public:
    /**
     * Returns a string describing the current device.
     */
    static std::string get_device_info();
    
    /**
     * Blocks until the execution of all GPU commands is complete.
     */
    static void finish();
    
    /**
     * Forces the execution of all GPU commands in finite time.
     */
    static void flush();
    
    /**
     * Returns the error information from OpenGL.
     */
    static std::string get_error();
    
    /**
     * Clears the color, depth or stencil buffers. Fills the color buffer with
     * the current color clear value.
     *
     * \param c whether to clear the color buffer
     * \param d whether to clear the depth buffer
     * \param s whether to clear the stencil buffer
     */
    static void clear(bool c = true, bool d = true, bool s = true);
    
    /**
     * Returns the color (.xyz) and alpha (.w) for clearing.
     */
    static Vec4 get_clear_color();
    
    /**
     * Sets the specified color and alpha for clearing. The default is (1, 1, 1)
     * and 1.
     *
     * \param c color
     * \param a alpha
     */
    static void set_clear_color(const Vec3& c, float a);
    
    /**
     * Sets the specified color (.xyz) and alpha (.w) for clearing. The default
     * is (1, 1, 1, 1).
     *
     * \param c color with alpha
     */
    static void set_clear_color(const Vec4& c);
    
    /**
     * Returns a vec4 value that represents whether the red, green, blue, and
     * alpha components are enabled for writing.
     */
    static Vec4 get_color_writemask();
    
    /**
     * Determines whether the red, green, blue, and alpha components are enabled
     * for writing. The default is true, true, true, true.
     *
     * \param r red writemask
     * \param g green writemask
     * \param b blue writemask
     * \param a alpha writemask
     */
    static void set_color_writemask(bool r, bool g, bool b, bool a);
    
    /**
     * Enables depth test. If enabled, does depth comparisons and updates the
     * depth buffer.
     */
    static void enable_depth_test();
    
    /**
     * Disables depth test.
     */
    static void disable_depth_test();
    
    /**
     * Returns the depth clear value.
     */
    static double get_clear_depth();
    
    /**
     * Sets the specified depth clear value. The default is 1.
     *
     * \param d clear depth value
     */
    static void set_clear_depth(double d);
    
    /**
     * Returns true if the depth buffer is enabled for writing.
     */
    static bool get_depth_writemask();
    
    /**
     * Determines whether the depth buffer is enabled for writing. The default
     * is true.
     *
     * \param m depth writemask
     */
    static void set_depth_writemask(bool m);
    
    /**
     * Returns the comparison function in depth test.
     */
    static ComparisonFunc get_depth_func();
    
    /**
     * Sets the specified comparison function in depth test. The default is
     * FUNC_LESS.
     *
     * \param f comparison function
     */
    static void set_depth_func(ComparisonFunc f);
    
    /**
     * Enables stencil test. If enabled, does stencil testing and updates the
     * stencil buffer.
     */
    static void enable_stencil_test();
    
    /**
     * Disables stencil test.
     */
    static void disable_stencil_test();
    
    /**
     * Returns the stencil clear value.
     */
    static int get_clear_stencil();
    
    /**
     * Sets the specified stencil clear value. The default is 0.
     *
     * \param s stencil clear value
     */
    static void set_clear_stencil(int s);
    
    /**
     * Returns the stencil bit mask to enable writing of individual bits in the
     * stencil planes.
     */
    static unsigned int get_stencil_writemask();
    
    /**
     * Sets the specified stencil bit mask to enable writing of individual bits
     * in the stencil planes. The default is all 1's.
     *
     * \param m stencil writemask
     */
    static void set_stencil_writemask(unsigned int m);
    
    /**
     * Returns the comparison function in stencil test.
     */
    static ComparisonFunc get_stencil_func();
    
    /**
     * Returns the reference value in stencil test.
     */
    static int get_stencil_ref();
    
    /**
     * Returns the mask in stencil test.
     */
    static int get_stencil_mask();
    
    /**
     * Sets the specified comparison function, reference value, and mask in
     * stencil test. The default is FUNC_ALWAYS, 0 and 1.
     *
     * \param f comparison function
     * \param r reference value
     * \param m mask
     */
    static void set_stencil_func(ComparisonFunc f, int r, int m);
    
    /**
     * Returns the operation when stencil test fails.
     */
    static StencilOperation get_stencil_fail();
    
    /**
     * Returns the operation when the stencil test passes but depth test fails.
     */
    static StencilOperation get_stencil_zfail();
    
    /**
     * Returns the operation when both stencil test and depth test passes.
     */
    static StencilOperation get_stencil_zpass();
    
    /**
     * Sets the specified operations for different cases in stencil test and
     * depth test. The default is STENCIL_KEEP, STENCIL_KEEP, STENCIL_KEEP.
     *
     * \param f the operation when stencil test fails
     * \param zf the operation when stencil test passes but depth test fails
     * \param zp the operation when both stencil test and depth test passes
     */
    static void set_stencil_op(StencilOperation f,
                               StencilOperation zf,
                               StencilOperation zp);
    
    /**
     * Enables blending. If enabled, blends the computed fragment color values
     * with the values in the color buffers.
     */
    static void enable_blending();
    
    /**
     * Disables blending.
     */
    static void disable_blending();
    
    /**
     * Returns the blend operation of the RGB color components in blending.
     */
    static BlendOperation get_blend_op_rgb();
    
    /**
     * Returns the blend operation of the alpha color component in blending.
     */
    static BlendOperation get_blend_op_alpha();
    
    /**
     * Sets the specified blend operation in blending. The default is BLEND_ADD.
     *
     * \param o blend operation
     */
    static void set_blend_op(BlendOperation o);
    
    /**
     * Sets the specified blend operation in blending. The default is BLEND_ADD,
     * BLEND_ADD.
     *
     * \param rgb RGB blend operation
     * \param a alpha blend operation
     */
    static void set_blend_op(BlendOperation rgb, BlendOperation a);
    
    /**
     * Returns the source blend factor of the RGB color components in blending.
     */
    static BlendFactor get_blend_src_rgb();
    
    /**
     * Returns the source blend factor of the alpha color component in blending.
     */
    static BlendFactor get_blend_src_alpha();
    
    /**
     * Returns the destination blend factor of the RGB color components in
     * blending.
     */
    static BlendFactor get_blend_dst_rgb();
    
    /**
     * Returns the destination blend factor of the alpha color component in
     * blending.
     */
    static BlendFactor get_blend_dst_alpha();
    
    /**
     * Sets the specified source and destination blend factors in blending. The
     * default is FACTOR_ONE and FACTOR_ZERO.
     *
     * \param s source blend factor
     * \param d destination blend factor
     */
    static void set_blend_factor(BlendFactor s, BlendFactor d);
    
    /**
     * Sets the specified source and destination blend factors in blending. The
     * default is FACTOR_ONE, FACTOR_ZERO, FACTOR_ONE, FACTOR_ZERO.
     *
     * \param sr RGB source blend factor
     * \param dr RGB destination blend factor
     * \param sa alpha source blend factor
     * \param da alpha destination blend factor
     */
    static void set_blend_factor(BlendFactor sr, BlendFactor dr,
                                 BlendFactor sa, BlendFactor da);
    
    /**
     * Returns the viewport region.
     */
    static Rect get_viewport();
    
    /**
     * Sets the viewport region to render from (x, y) to (x + width, y +
     * height).
     *
     * \param v viewport region
     */
    static void set_viewport(const Rect& v);
    
    /**
     * Enables scissor test. If enabled, discards fragments that are outside the
     * scissor rectangle.
     */
    static void enable_scissor_test();
    
    /**
     * Disables scissor test.
     */
    static void disable_scissor_test();
    
    /**
     * Returns the scissor region in scissor test.
     */
    static Rect get_scissor();
    
    /**
     * Sets the scissor region from (x, y) to (x + width, y + height) in scissor
     * test.
     *
     * \param s scissor region
     */
    static void set_scissor(const Rect& s);
    
    /**
     * Enables wireframe. If enabled, boundary edges of the polygon are drawn as
     * line segments.
     */
    static void enable_wireframe();
    
    /**
     * Disables wireframe.
     */
    static void disable_wireframe();
    
    /**
     * Enables face culling. If enabled, culls polygons based on their winding
     * in window coordinates.
     */
    static void enable_culling();
    
    /**
     * Disables face culling.
     */
    static void disable_culling();
    
    /**
     * Returns the side of faces that will be culled.
     */
    static RenderSide get_cull_side();
    
    /**
     * Determines which side of faces will be culled. The default is BACK_SIDE.
     *
     * \param s culling side
     */
    static void set_cull_side(RenderSide s);
    
    /**
     * Enables polygon offset. If enabled, and if the wireframe is disabled, an
     * offset is added to depth values of a polygon's fragments before the depth
     * comparison is performed.
     */
    static void enable_polygon_offset();
    
    /**
     * Disables polygon offset.
     */
    static void disable_polygon_offset();
    
    /**
     * Returns the polygon offset factor.
     */
    static float get_polygon_offset_factor();
    
    /**
     * Returns the polygon offset units.
     */
    static float get_polygon_offset_units();
    
    /**
     * Sets the specified polygon offset factor and units. The default is 0 and
     * 0.
     *
     * \param f polygon offset factor
     * \param u polygon offset units
     */
    static void set_polygon_offset(float f, float u);
    
    /**
     * Enables dithering. If enabled, dithers color components or indices before
     * they are written to the color buffer.
     */
    static void enable_dithering();
    
    /**
     * Disables dithering.
     */
    static void disable_dithering();
    
    /**
     * Enables multisample anti-aliasing. If enabled, uses multiple fragment
     * samples in computing the final color of a pixel.
     */
    static void enable_multisample();
    
    /**
     * Disables multisample anti-aliasing.
     */
    static void disable_multisample();
    
    /**
     * Enables alpha to coverage. If enabled, computes a temporary coverage
     * value where each bit is determined by the alpha value at the
     * corresponding sample location.
     */
    static void enable_alpha_to_coverage();
    
    /**
     * Disables alpha to coverage.
     */
    static void disable_alpha_to_coverage();
    
    /**
     * Enables seamless cube texture accesses. If enabled, cubemap textures are
     * sampled such that when linearly sampling from the border between two
     * adjacent faces, texels from both faces are used to generate the final
     * sample value.
     */
    static void enable_texture_cube_seamless();
    
    /**
     * Disables seamless cube texture accesses.
     */
    static void disable_texture_cube_seamless();
};

class MaterialState {
public:
    /**
     * Applies the depth dependent settings for the specified material.
     *
     * \param m material
     */
    static void set_depth(const Material& m);
    
    /**
     * Applies the stencil dependent settings for the specified material.
     *
     * \param m material
     */
    static void set_stencil(const Material& m);
    
    /**
     * Applies the blending dependent settings for the specified material.
     *
     * \param m material
     */
    static void set_blending(const Material& m);
    
    /**
     * Applies the wireframe dependent settings for the specified material.
     *
     * \param m material
     */
    static void set_wireframe(const Material& m);
    
    /**
     * Applies the side dependent settings for the specified material.
     *
     * \param m material
     */
    static void set_side(const Material& m);
    
    /**
     * Applies the shadow side dependent settings for the specified material.
     *
     * \param m material
     */
    static void set_shadow_side(const Material& m);
};

class Shader {
public:
    /**
     * Creates a new Shader object.
     */
    Shader();
    
    /**
     * Deletes this Shader object.
     */
    ~Shader();
    
    /**
     * Shader is non-copyable. The copy constructor is deleted.
     */
    Shader(const Shader&) = delete;
    
    /**
     * Shader is non-copyable. The copy assignment operator is deleted.
     */
    Shader& operator=(const Shader&) = delete;
    
    /**
     * Loads the contents of the specified vertex shader to this shader.
     *
     * \param s vertex shader
     */
    void load_vert(const char* s);
    
    /**
     * Loads the contents of the specified vertex shader to this shader.
     *
     * \param s vertex shader
     */
    void load_vert(const std::string& s);
    
    /**
     * Loads the contents of the specified geometry shader to this shader.
     *
     * \param s geometry shader
     */
    void load_geom(const char* s);
    
    /**
     * Loads the contents of the specified geometry shader to this shader.
     *
     * \param s geometry shader
     */
    void load_geom(const std::string& s);
    
    /**
     * Loads the contents of the specified fragment shader to this shader.
     *
     * \param s fragment shader
     */
    void load_frag(const char* s);
    
    /**
     * Loads the contents of the specified fragment shader to this shader.
     *
     * \param s fragment shader
     */
    void load_frag(const std::string& s);
    
    /**
     * Loads the vertex shader from the specified GLSL file to the shader.
     *
     * \param p the path to vertex shader file
     */
    void load_vert_file(const std::string& p);
    
    /**
     * Loads the geometry shader from the specified GLSL file to the shader.
     *
     * \param p the path to geometry shader file
     */
    void load_geom_file(const std::string& p);
    
    /**
     * Loads the fragment shader from the specified GLSL file to the shader.
     *
     * \param p the path to fragment shader file
     */
    void load_frag_file(const std::string& p);
    
    /**
     * Compiles the shader program if the shader has been changed.
     */
    void compile() const;
    
    /**
     * Uses the program of the compiled shader.
     */
    void use_program() const;
    
    /**
     * Sets the preprocessor defines for the shader. These defines are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param d defines
     */
    void set_defines(const Defines& d);
    
    /**
     * Sets the value of the uniform variable. These uniform variables are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n variable name
     * \param v value
     */
    void set_uniform_i(const std::string& n, int v) const;
    
    /**
     * Sets the value of the uniform variable. These uniform variables are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n variable name
     * \param v value
     */
    void set_uniform_u(const std::string& n, unsigned int v) const;
    
    /**
     * Sets the value of the uniform variable. These uniform variables are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n variable name
     * \param v value
     */
    void set_uniform_f(const std::string& n, float v) const;
    
    /**
     * Sets the value of the uniform variable. These uniform variables are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n variable name
     * \param v value
     */
    void set_uniform_v2(const std::string& n, const Vec2& v) const;
    
    /**
     * Sets the value of the uniform variable. These uniform variables are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n variable name
     * \param v value
     */
    void set_uniform_v3(const std::string& n, const Vec3& v) const;
    
    /**
     * Sets the value of the uniform variable. These uniform variables are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n variable name
     * \param v value
     */
    void set_uniform_v4(const std::string& n, const Vec4& v) const;
    
    /**
     * Sets the value of the uniform variable. These uniform variables are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n variable name
     * \param v value
     */
    void set_uniform_m2(const std::string& n, const Mat2& v) const;
    
    /**
     * Sets the value of the uniform variable. These uniform variables are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n variable name
     * \param v value
     */
    void set_uniform_m3(const std::string& n, const Mat3& v) const;
    
    /**
     * Sets the value of the uniform variable. These uniform variables are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param n variable name
     * \param v value
     */
    void set_uniform_m4(const std::string& n, const Mat4& v) const;
    
    /**
     * Sets the value of the uniform variable. These uniform variables are then
     * available in the vertex, geometry, and fragment shaders.
     *
     * \param u uniforms object
     */
    void set_uniforms(const Uniforms& u) const;
    
    /**
     * Returns the GLSL version of the shading language.
     */
    static std::string get_glsl_version();
    
    /**
     * Sets the GLSL version of the shading language. The default is "410".
     *
     * \param v GLSL version
     */
    static void set_glsl_version(const std::string& v);

private:
    uint32_t program = 0;
    
    std::string defines;
    std::string vert_shader;
    std::string geom_shader;
    std::string frag_shader;
    
    static std::string glsl_version;
    
    uint32_t compile_shader(const std::string& s, int32_t t) const;
    
    void compile_shaders() const;
    
    std::string get_link_info() const;
    
    void resolve_defines(std::string& s) const;
    
    static void resolve_version(std::string& s);
    
    static std::string get_compile_info(uint32_t s, uint32_t t);
    
    static std::string get_error_info(const std::string& c, const std::string& s);
    
    friend class VertexObject;
};

class VertexObject {
public:
    /**
     * Creates a new VertexObject object.
     */
    VertexObject();
    
    /**
     * Deletes this VertexObject object.
     */
    ~VertexObject();
    
    /**
     * VertexObject is non-copyable. The copy constructor is deleted.
     */
    VertexObject(const VertexObject&) = delete;
    
    /**
     * VertexObject is non-copyable. The copy assignment operator is deleted.
     */
    VertexObject& operator=(const VertexObject&) = delete;
    
    /**
     * Loads the specified mesh to this vertex object.
     *
     * \param m mesh
     * \param g material group
     */
    void load(const Mesh& m, const MeshGroup& g);
    
    /**
     * Attaches this vertex object to the target shader to automatically match
     * the input data with the shader locations.
     *
     * \param s target shader
     */
    void attach(const Shader& s) const;
    
    /**
     * Renders the vertex object to the current render target.
     */
    void render() const;
    
private:
    uint32_t id = 0;
    uint32_t buffer_id = 0;
    
    int length = 0;
    
    std::vector<std::string> names;
    std::vector<int> sizes;
    std::vector<int> locations;
};

class Texture {
public:
    /**
     * Creates a new Texture object.
     */
    Texture();
    
    /**
     * Deletes this Texture object.
     */
    ~Texture();
    
    /**
     * Texture is non-copyable. The copy constructor is deleted.
     */
    Texture(const Texture&) = delete;
    
    /**
     * Texture is non-copyable. The copy assignment operator is deleted.
     */
    Texture& operator=(const Texture&) = delete;
    
    /**
     * Initializes the texture as an empty 1D texture.
     *
     * \param w the width of the texture
     * \param f texture format
     * \param t image data type
     */
    void init_1d(int w, TextureFormat f, ImageType t = IMAGE_UBYTE);
    
    /**
     * Initializes the texture as an empty 2D texture.
     *
     * \param w the width of the texture
     * \param h the height of the texture
     * \param f texture format
     * \param t image data type
     */
    void init_2d(int w, int h, TextureFormat f, ImageType t = IMAGE_UBYTE);
    
    /**
     * Initializes the texture as a 2D texture with the specified image.
     *
     * \param i image
     * \param f texture format
     * \param t image data format
     */
    void init_2d(const Image& i, TextureFormat f, ImageFormat t = IMAGE_COLOR);
    
    /**
     * Initializes the texture as an empty 3D texture.
     *
     * \param w the width of the texture
     * \param h the height of the texture
     * \param d the depth of the texture
     * \param f texture format
     * \param t image data type
     */
    void init_3d(int w, int h, int d, TextureFormat f, ImageType t = IMAGE_UBYTE);
    
    /**
     * Initializes the texture as an empty cube texture.
     *
     * \param w the width of the texture
     * \param h the height of the texture
     * \param f texture format
     * \param t image data type
     */
    void init_cube(int w, int h, TextureFormat f, ImageType t = IMAGE_UBYTE);
    
    /**
     * Initializes the texture as a cube texture with the specified images from
     * different sides.
     *
     * \param px right (+X) side of the cube image
     * \param nx left  (-X) side of the cube image
     * \param py upper (+Y) side of the cube image
     * \param ny lower (-Y) side of the cube image
     * \param pz front (+Z) side of the cube image
     * \param nz back  (-Z) side of the cube image
     * \param f texture format
     * \param t image data format
     */
    void init_cube(const Image& px, const Image& nx, const Image& py, const Image& ny,
                   const Image& pz, const Image& nz, TextureFormat f, ImageFormat t = IMAGE_COLOR);
    
    /**
     * Initializes the texture as an empty 1D texture array.
     *
     * \param w the width of the texture
     * \param l the layer of the texture
     * \param f texture format
     * \param t image data type
     */
    void init_1d_array(int w, int l, TextureFormat f, ImageType t = IMAGE_UBYTE);
    
    /**
     * Initializes the texture as an empty 2D texture array.
     *
     * \param w the width of the texture
     * \param h the height of the texture
     * \param l the layer of the texture
     * \param f texture format
     * \param t image data type
     */
    void init_2d_array(int w, int h, int l, TextureFormat f, ImageType t = IMAGE_UBYTE);
    
    /**
     * Initializes the texture as an empty cube texture array.
     *
     * \param w the width of the texture
     * \param h the height of the texture
     * \param l the layer of the texture
     * \param f texture format
     * \param t image data type
     */
    void init_cube_array(int w, int h, int l, TextureFormat f, ImageType t = IMAGE_UBYTE);
    
    /**
     * Returns the width of the texture, or 0 if the texture is uninitialized.
     */
    int get_width() const;
    
    /**
     * Returns the height of the texture, or 0 if the texture is uninitialized.
     */
    int get_height() const;
    
    /**
     * Returns the depth of the texture, or 0 if the texture is uninitialized.
     */
    int get_depth() const;
    
    /**
     * Returns the layer of the texture, or 0 if the texture is uninitialized.
     * The type must be one of TEXTURE_1D_ARRAY, TEXTURE_2D_ARRAY,
     * TEXTURE_CUBE_ARRAY.
     */
    int get_layer() const;
    
    /**
     * Returns the type of the texture.
     */
    TextureType get_type() const;
    
    /**
     * Returns the format of the texture.
     */
    TextureFormat get_format() const;
    
    /**
     * Copies the texture to the specified image. The type must be TEXTURE_2D.
     */
    void copy_to_image(Image& i) const;
    
    /**
     * Generates mipmaps for the texture.
     */
    void generate_mipmap() const;
    
    /**
     * Sets the wrapping mode on S coordinate. The default is TEXTURE_REPEAT.
     *
     * \param m wrapping mode
     */
    void set_wrap_s(TextureWrappingMode m) const;
    
    /**
     * Sets the wrapping mode on T coordinate. The default is TEXTURE_REPEAT.
     *
     * \param m wrapping mode
     */
    void set_wrap_t(TextureWrappingMode m) const;
    
    /**
     * Sets the wrapping mode on R coordinate. The default is TEXTURE_REPEAT.
     *
     * \param m wrapping mode
     */
    void set_wrap_r(TextureWrappingMode m) const;
    
    /**
     * Sets the wrapping modes on S, T and R coordinates.
     *
     * \param m wrapping mode
     */
    void set_wrap_all(TextureWrappingMode m) const;
    
    /**
     * Sets the magnification and minification filters of the texture. The
     * default is TEXTURE_LINEAR and TEXTURE_NEAREST_MIPMAP_LINEAR.
     *
     * \param mag magnification filter
     * \param min minification filter
     */
    void set_filters(TextureFilter mag, TextureFilter min) const;
    
    /**
     * Sets the border color for border texels. The default is (0, 0, 0, 0).
     *
     * \param c border color
     */
    void set_border_color(const Vec4& c) const;
    
    /**
     * Sets the range of LODs for the texture with mipmaps. The default is 0 and
     * 1000.
     *
     * \param min minimal defined level
     * \param max maximal defined level
     */
    void set_lod_range(int min, int max) const;
    
    /**
     * Sets the bias of LOD for the texture with mipmaps. The default is 0.
     *
     * \param b bias level
     */
    void set_lod_bias(float b) const;
    
    /**
     * Activates this texture at the specified location.
     *
     * \param l the location of the texture
     */
    int activate(int l) const;
    
    /**
     * Returns the default texture format for the specified channel and byte.
     *
     * \param c channel
     * \param b byte
     */
    static TextureFormat default_format(int c, int b);
    
    /**
     * Returns the default texture format for the specified image.
     *
     * \param i image
     */
    static TextureFormat default_format(const Image& i);
    
private:
    uint32_t id = 0;
    
    int width = 0;
    int height = 0;
    int depth = 0;
    
    TextureType type = TEXTURE_2D;
    
    TextureFormat format = TEXTURE_R8G8B8A8_UNORM;
    
    void set_dimensions(int w, int h, int d);
    
    void set_parameters(TextureType t, TextureFormat f);
    
    friend class RenderTarget;
};

class Renderbuffer {
public:
    /**
     * Creates a new Renderbuffer object.
     */
    Renderbuffer();
    
    /**
     * Deletes this Renderbuffer object.
     */
    ~Renderbuffer();
    
    /**
     * Renderbuffer is non-copyable. The copy constructor is deleted.
     */
    Renderbuffer(const Renderbuffer&) = delete;
    
    /**
     * Renderbuffer is non-copyable. The copy assignment operator is deleted.
     */
    Renderbuffer& operator=(const Renderbuffer&) = delete;
    
    /**
     * Initializes the empty render buffer with width, height, format.
     *
     * \param w the width of the render buffer
     * \param h the height of the render buffer
     * \param f texture format
     */
    void init(int w, int h, TextureFormat f) const;
    
private:
    uint32_t id = 0;
    
    friend class RenderTarget;
};

class RenderTarget {
public:
    /**
     * Creates a new RenderTarget object.
     */
    RenderTarget();
    
    /**
     * Deletes this RenderTarget object.
     */
    ~RenderTarget();
    
    /**
     * RenderTarget is non-copyable. The copy constructor is deleted.
     */
    RenderTarget(const RenderTarget&) = delete;
    
    /**
     * RenderTarget is non-copyable. The copy assignment operator is deleted.
     */
    RenderTarget& operator=(const RenderTarget&) = delete;
    
    /**
     * Sets the 2D texture, the cube texture, the 2D texture array or the 3D
     * texture to the render target.
     *
     * \param t 2D texture, cube texture, 2D texture array or 3D texture
     * \param i the index of the texture
     * \param l mipmap level
     * \param p if there is a cube texture, specifies the face of the cube texture
     *          if there is a 2D texture array, specifies the layer of the texture
     *          if there is a 3D texture, specifies the layer of the texture
     */
    void set_texture(const Texture& t, unsigned int i, int l = 0, int p = 0) const;
    
    /**
     * Sets the 2D texture or the cube texture as the depth texture of render
     * target.
     *
     * \param t 2D texture or cube texture
     * \param l mipmap level
     * \param p if there is a cube texture, specifies the face of the cube texture
     *          if there is a 2D texture array, specifies the layer of the texture
     */
    void set_depth_texture(const Texture& t, int l = 0, int p = 0) const;
    
    /**
     * Sets the 2D texture or the cube texture as the stencil texture of render
     * target.
     *
     * \param t 2D texture or cube texture
     * \param l mipmap level
     * \param p if there is a cube texture, specifies the face of the cube texture
     *          if there is a 2D texture array, specifies the layer of the texture
     */
    void set_stencil_texture(const Texture& t, int l = 0, int p = 0) const;
    
    /**
     * Sets the 2D texture or the cube texture as the depth stencil texture of
     * render target.
     *
     * \param t 2D texture or cube texture
     * \param l mipmap level
     * \param p if there is a cube texture, specifies the face of the cube texture
     *          if there is a 2D texture array, specifies the layer of the texture
     */
    void set_depth_stencil_texture(const Texture& t, int l = 0, int p = 0) const;
    
    /**
     * Sets the render buffer as the depth buffer of the render target.
     *
     * \param r render buffer
     */
    void set_depth_buffer(const Renderbuffer& r) const;
    
    /**
     * Sets the render buffer as the stencil buffer of the render target.
     *
     * \param r render buffer
     */
    void set_stencil_buffer(const Renderbuffer& r) const;
    
    /**
     * Sets the render buffer as the depth stencil buffer of the render target.
     *
     * \param r render buffer
     */
    void set_depth_stencil_buffer(const Renderbuffer& r) const;
    
    /**
     * Sets the number of targets to be rendered into. The default is 1.
     *
     * \param n the number of targets
     */
    void set_target_number(int n) const;
    
    /**
     * Activates this render target.
     */
    void activate() const;
    
    /**
     * Activates the specified render target. If the render target is nullptr,
     * falls back to the default render target.
     *
     * \param t render target
     */
    static void activate(const RenderTarget* t);
    
private:
    uint32_t id = 0;
    
    std::vector<uint32_t> attachments;
    
    void set_framebuffer(const Texture& t, uint32_t a, int l, int p) const;
    
    void set_framebuffer(const Renderbuffer& r, uint32_t a) const;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/ShaderCache.h ------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class ShaderCache {
public:
    /**
     * Loads the contents of the specified vertex shader to the shader.
     *
     * \param n shader name
     * \param s vertex shader
     */
    static void load_vert(const std::string& n, const char* s);
    
    /**
     * Loads the contents of the specified vertex shader to the shader.
     *
     * \param n shader name
     * \param s vertex shader
     */
    static void load_vert(const std::string& n, const std::string& s);
    
    /**
     * Loads the contents of the specified geometry shader to the shader.
     *
     * \param n shader name
     * \param s geometry shader
     */
    static void load_geom(const std::string& n, const char* s);
    
    /**
     * Loads the contents of the specified geometry shader to the shader.
     *
     * \param n shader name
     * \param s geometry shader
     */
    static void load_geom(const std::string& n, const std::string& s);
    
    /**
     * Loads the contents of the specified fragment shader to the shader.
     *
     * \param n shader name
     * \param s fragment shader
     */
    static void load_frag(const std::string& n, const char* s);
    
    /**
     * Loads the contents of the specified fragment shader to the shader.
     *
     * \param n shader name
     * \param s fragment shader
     */
    static void load_frag(const std::string& n, const std::string& s);
    
    /**
     * Loads the contents of the specified included shader to the shader.
     *
     * \param n shader name
     * \param s included shader
     */
    static void load_include(const std::string& n, const char* s);
    
    /**
     * Loads the contents of the specified included shader to the shader.
     *
     * \param n shader name
     * \param s included shader
     */
    static void load_include(const std::string& n, const std::string& s);
    
    /**
     * Loads the vertex shader from the specified GLSL file to the shader.
     *
     * \param n shader name
     * \param p the path to vertex shader file
     */
    static void load_vert_file(const std::string& n, const std::string& p);
    
    /**
     * Loads the geometry shader from the specified GLSL file to the shader.
     *
     * \param n shader name
     * \param p the path to geometry shader file
     */
    static void load_geom_file(const std::string& n, const std::string& p);
    
    /**
     * Loads the fragment shader from the specified GLSL file to the shader.
     *
     * \param n shader name
     * \param p the path to fragment shader file
     */
    static void load_frag_file(const std::string& n, const std::string& p);
    
    /**
     * Loads the included shader from the specified GLSL file to the shader.
     *
     * \param n shader name
     * \param p the path to included shader file
     */
    static void load_include_file(const std::string& n, const std::string& p);
    
    /**
     * Returns true if the specified vertex shader is loaded.
     *
     * \param n shader name
     */
    static bool has_vert(const std::string& n);
    
    /**
     * Returns true if the specified geometry shader is loaded.
     *
     * \param n shader name
     */
    static bool has_geom(const std::string& n);
    
    /**
     * Returns true if the specified fragment shader is loaded.
     *
     * \param n shader name
     */
    static bool has_frag(const std::string& n);
    
    /**
     * Returns true if the specified included shader is loaded.
     *
     * \param n shader name
     */
    static bool has_include(const std::string& n);
    
    /**
     * Returns the shader with the specified name from shader cache.
     *
     * \param n shader name
     */
    static const gpu::Shader* fetch(const std::string& n);
    
    /**
     * Returns the shader with the specified name and defines from shader cache.
     *
     * \param n shader name
     * \param d defines
     */
    static const gpu::Shader* fetch(const std::string& n, const Defines& d);
    
    /**
     * Clears the shader cache with the specified name.
     *
     * \param n shader name
     */
    static void clear_cache(const std::string& n);
    
    /**
     * Clears the shader cache with the specified name and defines.
     *
     * \param n shader name
     * \param d defines
     */
    static void clear_cache(const std::string& n, const Defines& d);
    
    /**
     * Clears the shader cache.
     */
    static void clear_caches();
    
    /**
     * Returns the path to find the included shaders.
     */
    static std::string get_include_path();
    
    /**
     * Sets the path to find the included shaders. The default is "ink/shaders/
     * include".
     *
     * \param p include path
     */
    static void set_include_path(const std::string& p);
    
private:
    static std::string to_lower(const std::string& s);
    
    static void resolve_includes(std::string& s);
    
    static std::string include_path;
    
    static std::unordered_map<std::string, std::string> vert_shaders;
    static std::unordered_map<std::string, std::string> geom_shaders;
    static std::unordered_map<std::string, std::string> frag_shaders;
    static std::unordered_map<std::string, std::string> include_shaders;
    
    static std::unordered_set<std::string> include_set;
    
    static std::unordered_map<std::string, std::unique_ptr<gpu::Shader>> cache;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/ShaderLib.h --------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class ShaderLib {
public:
    /**
     * Returns the shader with the specified name from shader cache. You can
     * clear the cache in ShaderCache.
     *
     * \param n shader name
     */
    static const gpu::Shader* fetch(const std::string& n);
    
    /**
     * Returns the shader with the specified name and defines from shader cache.
     * You can clear the cache in ShaderCache.
     *
     * \param n shader name
     * \param d defines
     */
    static const gpu::Shader* fetch(const std::string& n, const Defines& d);
    
    /**
     * Returns the path to find the shaders.
     */
    static std::string get_library_path();
    
    /**
     * Sets the path to find the shaders. The default is "ink/shaders/library".
     *
     * \param p library path
     */
    static void set_library_path(const std::string& p);
    
private:
    static std::string library_path;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/lights/Light.h -------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class Light {
public:
    bool visible = true;       /**< whether the light is visible */
    float intensity = 1;       /**< the intensity of the light */
    Vec3 color = {1, 1, 1};    /**< the color of the light */
    
    /**
     * Creates a new Light object and initializes it with color and intensity.
     *
     * \param c the color of the light
     * \param i the intensity of the light
     */
    Light(const Vec3& c = {1, 1, 1}, float i = 1);
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/lights/Shadow.h ------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

enum ShadowType {
    SHADOW_HARD,
    SHADOW_PCF,
    SHADOW_PCSS,
};

class Shadow {
public:
    ShadowType type = SHADOW_PCF;    /**< the type of the shadow */
    int map_id = 0;                  /**< the ID of the shadow map */
    float bias = 0;                  /**< shadow bias, should be close to 0 */
    float normal_bias = 0;           /**< shadow bias along the object normal, range is 0 to 1 */
    float radius = 3;                /**< the radius of the blurred edge, only used in PCF or PCSS */
    Camera camera;                   /**< light's view camera of the shadow */
    
    /**
     * Sets the resolution of the shadow map and the max number of shadows.
     * Initializes shadows.
     *
     * \param w the width of the shadow map
     * \param h the height of the shadow map
     * \param n the max number of shadows
     */
    static void init(int w = 1024, int h = 1024, int n = 16);
    
    /**
     * Creates a new Shadow object.
     */
    Shadow() = default;
    
    /**
     * Returns the sample numbers when using PCF / PCSS shadow.
     */
    static int get_samples();
    
    /**
     * Sets the sample numbers when using PCF / PCSS shadow. Must be 16, 32 or
     * 64. The default is 32.
     *
     * \param s sample numbers
     */
    static void set_samples(int s);
    
    /**
     * Returns the resolution of the shadow texture (shadow map).
     */
    static Vec2 get_resolution();
    
    /**
     * Activates the shadow texture (shadow map) at the specified location.
     *
     * \param l the location of the texture
     */
    static int activate_texture(int l);
    
    /**
     * Returns the render target of the shadow texture (shadow map).
     */
    const gpu::RenderTarget* get_target() const;
    
private:
    static int samples;
    
    static Vec2 resolution;
    
    static std::unique_ptr<gpu::Texture> shadow_map;
    
    static std::unique_ptr<gpu::RenderTarget> shadow_target;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/lights/DirectionalLight.h --------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class DirectionalLight : public Light {
public:
    Vec3 position = {0, 0, 0};      /**< the position where the light casts from */
    Vec3 direction = {0, -1, 0};    /**< the direction where the light casts through */
    bool cast_shadow = false;       /**< whether the light will cast shadow */
    Shadow shadow;                  /**< the shadow of the light */
    
    /**
     * Creates a new DirectionalLight object and initializes it with color and
     * intensity.
     *
     * \param c the color of the light
     * \param i the intensity of the light
     */
    DirectionalLight(const Vec3& c = {1, 1, 1}, float i = 1);
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/lights/Exp2Fog.h ------------------------------------------------ */
/* -------------------------------------------------------------------------- */

namespace ink {

class Exp2Fog {
public:
    bool visible = true;       /**< whether the fog is visible */
    Vec3 color = {1, 1, 1};    /**< the color of the fog */
    float near = 1;            /**< the nearest distance of the fog */
    float density = 0.001;     /**< the density of the fog */
    
    /**
     * Creates a new Exp2Fog object and initializes it with color, distance and
     * density.
     *
     * \param c the color of the fog
     * \param n the nearest distance of the fog
     * \param d the density of the fog
     */
    Exp2Fog(const Vec3& c = {1, 1, 1}, float n = 1, float d = 0.001);
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/lights/HemisphereLight.h ---------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class HemisphereLight : public Light {
public:
    Vec3 ground_color = {1, 1, 1};    /**< the ground color */
    Vec3 direction = {0, 1, 0};       /**< the direction from the ground to the sky */
    
    /**
     * Creates a new HemisphereLight object and initializes it with colors and
     * intensity.
     *
     * \param s the sky color
     * \param g the ground color
     * \param i the intensity of the light
     */
    HemisphereLight(const Vec3& s = {1, 1, 1},
                    const Vec3& g = {1, 1, 1}, float i = 1);
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/lights/LinearFog.h ---------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class LinearFog {
public:
    bool visible = true;       /**< whether the fog is visible */
    Vec3 color = {1, 1, 1};    /**< the color of the fog */
    float near = 1;            /**< the nearest distance of the fog */
    float far = 1000;          /**< the farthest distance of the fog */
    
    /**
     * Creates a new LinearFog object and initializes it with color and
     * distances.
     *
     * \param c the color of the fog
     * \param n the nearest distance of the fog
     * \param f the farthest distance of the fog
     */
    LinearFog(const Vec3& c = {1, 1, 1}, float n = 1, float f = 1000);
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/lights/PointLight.h --------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class PointLight : public Light {
public:
    Vec3 position = {0, 0, 0};    /**< the position where the light casts from */
    float decay = 1;              /**< the decay rate of light */
    float distance = 0;           /**< the maximum distance affected by the light */
    
    /**
     * Creates a new PointLight object and initializes it with color and
     * intensity.
     *
     * \param c the color of the light
     * \param i the intensity of the light
     */
    PointLight(const Vec3& c = {1, 1, 1}, float i = 1);
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/lights/SpotLight.h ---------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class SpotLight : public Light {
public:
    Vec3 position = {0, 0, 0};      /**< the position where the light casts from */
    Vec3 direction = {0, -1, 0};    /**< the direction where the light casts through */
    float angle = PI / 3;           /**< the maximum angle of the light */
    float decay = 1;                /**< the decay rate of the light */
    float distance = 0;             /**< the maximum distance affected by the light */
    float penumbra = 0;             /**< the attenuation of the spot light cone */
    bool cast_shadow = false;       /**< whether the light will cast shadow */
    Shadow shadow;                  /**< the shadow of the light */
    
    /**
     * Creates a new SpotLight object and initializes it with color and
     * intensity.
     *
     * \param c the color of the light
     * \param i the intensity of the light
     */
    SpotLight(const Vec3& c = {1, 1, 1}, float i = 1);
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/probes/IBLFilter.h ---------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class IBLFilter {
public:
    /**
     * Loads the specified cube image set to the prefiltered radiance
     * environment map.
     *
     * \param px right (+X) side of the cube image
     * \param nx left  (-X) side of the cube image
     * \param py upper (+Y) side of the cube image
     * \param ny lower (-Y) side of the cube image
     * \param pz front (+Z) side of the cube image
     * \param nz back  (-Z) side of the cube image
     * \param m prefiltered radiance environment map
     * \param s size of the radiance environment map
     */
    static void load_cubemap(const Image& px, const Image& nx,
                             const Image& py, const Image& ny,
                             const Image& pz, const Image& nz,
                             gpu::Texture& m, int s = 256);
    
    /**
     * Loads the specified equirectangular image to the prefiltered radiance
     * environment map.
     *
     * \param i equirectangular image
     * \param m prefiltered radiance environment map
     * \param s size of the radiance environment map
     */
    static void load_equirect(const Image& i, gpu::Texture& m, int s = 256);
    
    /**
     * Loads a cube texture or 2D texture to the prefiltered radiance
     * environment map.
     *
     * \param t source texture
     * \param m prefiltered radiance environment map
     * \param s size of the radiance environment map
     */
    static void load_texture(const gpu::Texture& t, gpu::Texture& m, int s = 256);
    
private:
    static std::unique_ptr<gpu::VertexObject> fullscreen_plane;
    
    static std::unique_ptr<gpu::Texture> blur_map;
    
    static std::unique_ptr<gpu::RenderTarget> cubemap_target;
    
    static std::unique_ptr<gpu::RenderTarget> blur_target;
    
    static void init_fullscreen_plane();
    
    static void gaussian_weights(float s, int n, float* w);
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/probes/ReflectionProbe.h ---------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class ReflectionProbe {
public:
    int resolution = 256;    /**< the resolution of the reflection probe */
    float intensity = 1;     /**< the intensity of the reflection probe */
    Vec3 position;           /**< the position of the reflection probe */
    
    /**
     * Creates a new ReflectionProbe object and initializes it with intensity,
     * resolution, and position.
     *
     * \param i the intensity of the reflection probe
     * \param r the resolution of the reflection probe
     * \param p the position of the reflection probe
     */
    ReflectionProbe(float i = 1, int r = 256, const Vec3& p = {0, 0, 0});
    
    /**
     * Loads the specified cube image set to the reflection probe.
     *
     * \param px right (+X) side of the cube image
     * \param nx left  (-X) side of the cube image
     * \param py upper (+Y) side of the cube image
     * \param ny lower (-Y) side of the cube image
     * \param pz front (+Z) side of the cube image
     * \param nz back  (-Z) side of the cube image
     */
    void load_cubemap(const Image& px, const Image& nx,
                      const Image& py, const Image& ny,
                      const Image& pz, const Image& nz);
    
    /**
     * Loads the specified equirectangular image to the reflection probe.
     *
     * \param i equirectangular image
     */
    void load_equirect(const Image& i);
    
    /**
     * Loads a specified cube texture or 2D texture to the reflection probe.
     *
     * \param t cube texture
     */
    void load_texture(const gpu::Texture& t);
    
    /**
     * Activates the texture at the specified location of reflection probe.
     *
     * \param l the location of the texture
     */
    int activate(int l) const;
    
private:
    std::unique_ptr<gpu::Texture> reflection_map;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/scene/Scene.h --------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class Scene : public Instance {
public:
    /**
     * Creates a new Scene object and initializes it with name.
     *
     * \param n scene name
     */
    Scene(const std::string& n = "");
    
    /**
     * Returns the material that matches the specified name.
     *
     * \param n material name
     */
    Material* get_material(const std::string& n) const;
    
    /**
     * Returns the material that matches the specified name and is linked to the
     * specified mesh.
     *
     * \param n material name
     * \param s mesh
     */
    Material* get_material(const std::string& n, const Mesh& s) const;
    
    /**
     * Returns the material that matches the specified name and is linked to the
     * specified instance.
     *
     * \param n material name
     * \param s instance
     */
    Material* get_material(const std::string& n, const Instance& s) const;
    
    /**
     * Sets the specified material with name to the scene.
     *
     * \param n material name
     * \param m material
     */
    void set_material(const std::string& n, Material* m);
    
    /**
     * Sets the specified material with name to the scene. Links it to the
     * specified mesh.
     *
     * \param n material name
     * \param s mesh
     * \param m material
     */
    void set_material(const std::string& n, const Mesh& s, Material* m);
    
    /**
     * Sets the specified material with name to the scene. Links it to the
     * specified instance.
     *
     * \param n material name
     * \param s instance
     * \param m material
     */
    void set_material(const std::string& n, const Instance& s, Material* m);
    
    /**
     * Removes the specified material that matches the specified name from the
     * scene.
     *
     * \param n material name
     */
    void remove_material(const std::string& n);
    
    /**
     * Removes the specified material that matches the specified name and is
     * linked to the specified mesh from the scene.
     *
     * \param n material name
     * \param s mesh
     */
    void remove_material(const std::string& n, const Mesh& s);
    
    /**
     * Removes the specified material that matches the specified name and is
     * linked to the specified instance from the scene.
     *
     * \param n material name
     * \param s instance
     */
    void remove_material(const std::string& n, const Instance& s);
    
    /**
     * Removes all the materials from the scene.
     */
    void clear_materials();
    
    /**
     * Returns all the materials in the material library.
     */
    std::vector<Material*> get_materials() const;
    
    /**
     * Returns the linear fog in the scene if there is one, returns nullptr
     * otherwise.
     */
    LinearFog* get_linear_fog() const;
    
    /**
     * Sets the specified linear fog to the scene. Only one fog can be set in a
     * scene.
     *
     * \param f linear fog
     */
    void set_fog(LinearFog* f);
    
    /**
     * Returns the exp square fog in the scene if there is one, returns nullptr
     * otherwise.
     */
    Exp2Fog* get_exp2_fog() const;
    
    /**
     * Sets the specified exp square fog to the scene. Only one fog can be set
     * in a scene.
     *
     * \param f exp square fog
     */
    void set_fog(Exp2Fog* f);
    
    /**
     * Adds the specified point light to the scene. The light number should not
     * exceed the limit.
     *
     * \param l point light
     */
    void add_light(PointLight* l);
    
    /**
     * Removes the specified point light from the scene.
     * 
     * \param l point light
     */
    void remove_light(PointLight* l);
    
    /**
     * Returns the number of point lights in the scene.
     */
    size_t get_point_light_count() const;
    
    /**
     * Returns the point light at the specified index in the scene.
     *
     * \param i the index of the light
     */
    PointLight* get_point_light(int i) const;
    
    /**
     * Adds the specified spot light to the scene. The light number should not
     * exceed the limit.
     *
     * \param l spot light
     */
    void add_light(SpotLight* l);
    
    /**
     * Removes the specified spot light from the scene.
     *
     * \param l spot light
     */
    void remove_light(SpotLight* l);
    
    /**
     * Returns the number of spot lights in the scene.
     */
    size_t get_spot_light_count() const;
    
    /**
     * Returns the spot light at the specified index in the scene.
     *
     * \param i the index of the light
     */
    SpotLight* get_spot_light(int i) const;
    
    /**
     * Adds the specified directional light to the scene. The light number
     * should not exceed the limit.
     *
     * \param l directional light
     */
    void add_light(DirectionalLight* l);
    
    /**
     * Removes the specified directional light from the scene.
     *
     * \param l directional light
     */
    void remove_light(DirectionalLight* l);
    
    /**
     * Returns the number of directional lights in the scene.
     */
    size_t get_directional_light_count() const;
    
    /**
     * Returns the directional light at the specified index in the scene.
     *
     * \param i the index of the light
     */
    DirectionalLight* get_directional_light(int i) const;
    
    /**
     * Adds the specified hemisphere light to the scene. The light number should
     * not exceed the limit.
     *
     * \param l hemisphere light
     */
    void add_light(HemisphereLight* l);
    
    /**
     * Removes the specified hemisphere light from the scene.
     *
     * \param l hemisphere light
     */
    void remove_light(HemisphereLight* l);
    
    /**
     * Returns the number of hemisphere lights in the scene.
     */
    size_t get_hemisphere_light_count() const;
    
    /**
     * Returns the hemisphere light at the specified index in the scene.
     *
     * \param i the index of the light
     */
    HemisphereLight* get_hemisphere_light(int i) const;
    
    /**
     * Removes all the lights from the scene.
     */
    void clear_lights();
    
    /**
     * Updates the local and global matrices of all the instances in the scene.
     */
    void update_instances();
    
    /**
     * Returns an instance list of all the instances in the scene.
     */
    std::vector<const Instance*> to_instances() const;
    
    /**
     * Returns an instance list of all the instances in the scene, excluding
     * invisible ones.
     */
    std::vector<const Instance*> to_visible_instances() const;
    
private:
    LinearFog* linear_fog = nullptr;
    Exp2Fog* exp2_fog = nullptr;
    
    std::vector<PointLight*> point_lights;
    std::vector<SpotLight*> spot_lights;
    std::vector<DirectionalLight*> directional_lights;
    std::vector<HemisphereLight*> hemisphere_lights;
    
    std::unordered_map<std::string, Material*> material_library;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/renderer/Renderer.h --------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

enum RenderingMode {
    FORWARD_RENDERING,
    DEFERRED_RENDERING,
};

class Renderer {
public:
    using TextureCallback = std::function<void(gpu::Texture&)>;
    
    /**
     * Creates a new Renderer object.
     */
    Renderer() = default;
    
    /**
     * Returns the color (.xyz) and alpha (.w) for clearing.
     */
    Vec4 get_clear_color() const;
    
    /**
     * Sets the specified color (.xyz) and alpha (.w) for clearing. The default
     * is (0, 0, 0, 0).
     *
     * \param c color with alpha
     */
    void set_clear_color(const Vec4& c);
    
    /**
     * Clears the color, depth or stencil buffers. Fills the color buffer with
     * the current color clear value.
     *
     * \param c whether to clear the color buffer
     * \param d whether to clear the depth buffer
     * \param s whether to clear the stencil buffer
     */
    void clear(bool c = true, bool d = true, bool s = true) const;
    
    /**
     * Returns the viewport region.
     */
    gpu::Rect get_viewport() const;
    
    /**
     * Sets the viewport region to render from (x, y) to (x + width, y +
     * height). The default is (0, 0, 0, 0).
     *
     * \param v viewport region
     */
    void set_viewport(const gpu::Rect& v);
    
    /**
     * Returns true if scissor test is enabled.
     */
    bool get_scissor_test() const;
    
    /**
     * Determines whether to enable scissor test. The default is false.
     *
     * \param t enable scissor test
     */
    void set_scissor_test(bool t);
    
    /**
     * Returns the scissor region in scissor test.
     */
    gpu::Rect get_scissor() const;
    
    /**
     * Sets the scissor region from (x, y) to (x + width, y + height) in scissor
     * test. The default is (0, 0, 0, 0).
     *
     * \param s scissor region
     */
    void set_scissor(const gpu::Rect& s);
    
    /**
     * Returns the rendering mode. If the mode is DEFERRED_RENDERING, there
     * should be 4 render targets.
     */
    RenderingMode get_rendering_mode() const;
    
    /**
     * Sets the specified rendering mode. The default is DEFERRED_RENDERING.
     *
     * \param m rendering mode
     */
    void set_rendering_mode(RenderingMode m);
    
    /**
     * Returns the current render target if there is one, returns nullptr
     * otherwise.
     */
    const gpu::RenderTarget* get_target() const;
    
    /**
     * Sets the render target. If the render target is nullptr, falls back to
     * the default render target.
     *
     * \param t render target
     */
    void set_target(const gpu::RenderTarget* t);
    
    /**
     * Sets the texture callback which will be called when an image is loaded.
     *
     * \param f texture callback function
     */
    void set_texture_callback(const TextureCallback& f);
    
    /**
     * Returns the intensity of the skybox.
     */
    float get_skybox_intensity() const;
    
    /**
     * Sets the intensity of the skybox. The default is 1.
     *
     * \param i intensity
     */
    void set_skybox_intensity(float i);
    
    /**
     * Loads the specified image set, one for each side of the skybox cubemap.
     *
     * \param px right (+X) side of the skybox cubemap
     * \param nx left  (-X) side of the skybox cubemap
     * \param py upper (+Y) side of the skybox cubemap
     * \param ny lower (-Y) side of the skybox cubemap
     * \param pz front (+Z) side of the skybox cubemap
     * \param nz back  (-Z) side of the skybox cubemap
     */
    void load_skybox(const Image& px, const Image& nx,
                     const Image& py, const Image& ny,
                     const Image& pz, const Image& nz);
    
    /**
     * Loads the specified equirectangular image to the skybox.
     *
     * \param i equirectangular image
     */
    void load_skybox(const Image& i);
    
    /**
     * Renders skybox using a camera. This function should be called before
     * the scene is rendered.
     *
     * \param c camera
     */
    void render_skybox(const Camera& c) const;
    
    /**
     * Loads the specified mesh and creates corresponding vertex object.
     *
     * \param m mesh
     */
    void load_mesh(const Mesh& m);
    
    /**
     * Unloads the specified mesh and deletes corresponding vertex object.
     * 
     * \param m mesh
     */
    void unload_mesh(const Mesh& m);
    
    /**
     * Clears the mesh cache. The caches are generated automatically when
     * loading meshes.
     */
    void clear_mesh_caches();
    
    /**
     * Loads the specified image and creates corresponding texture. This
     * function will invoke the texture callback.
     *
     * \param i image
     */
    void load_image(const Image& i);
    
    /**
     * Unloads the specified image and deletes corresponding texture.
     *
     * \param i image
     */
    void unload_image(const Image& i);
    
    /**
     * Clears the image cache. The caches are generated automatically when
     * loading images.
     */
    void clear_image_caches();
    
    /**
     * Loads all the meshes and images in the scene.
     *
     * \param s scene
     */
    void load_scene(const Scene& s);
    
    /**
     * Unloads all the meshes and images in the scene.
     *
     * \param s scene
     */
    void unload_scene(const Scene& s);
    
    /**
     * Clears the scene cache. The caches are generated automatically when
     * loading meshes, images or scenes.
     */
    void clear_scene_caches();
    
    /**
     * Renders a scene using a camera. The results will be rendered to the
     * current render target.
     *
     * \param s scene
     * \param c camera
     */
    void render(const Scene& s, const Camera& c) const;
    
    /**
     * Renders the transparent objects in a scene using a camera. The results
     * will be rendered to the current render target.
     *
     * \param s scene
     * \param c camera
     */
    void render_transparent(const Scene& s, const Camera& c) const;
    
    /**
     * Renders a scene using the shadow camera. The results will be rendered to
     * the shadow map.
     *
     * \param s scene
     * \param t target shadow
     */
    void render_shadow(const Scene& s, const Shadow& t) const;
    
    /**
     * Updates the shadow of the spot light. This function will update the
     * shadow camera and render the results to shadow map.
     *
     * \param s scene
     * \param l spot light
     */
    void update_shadow(const Scene& s, SpotLight& l) const;
    
    /**
     * Updates the shadow of the directional light. This function will update
     * the shadow camera and render the results to shadow map.
     *
     * \param s scene
     * \param l directional light
     */
    void update_shadow(const Scene& s, DirectionalLight& l) const;
    
    /**
     * Updates the reflection probe at its position. The scene will be rendered
     * in forward rendering mode.
     *
     * \param s scene
     * \param r reflection probe
     */
    void update_probe(const Scene& s, ReflectionProbe& r) const;
    
    /**
     * Updates all the instances in the scene before the scene is rendered.
     *
     * \param s scene
     */
    static void update_scene(Scene& s);
    
    /**
     * Sets the defines object for the material.
     *
     * \param m material
     * \param d defines
     */
    static void set_material_defines(const Material& m, Defines& d);
    
    /**
     * Sets the defines object for the scene.
     *
     * \param s scene
     * \param d defines
     */
    static void set_scene_defines(const Scene& s, Defines& d);
    
    /**
     * Sets the uniforms object for the lights.
     *
     * \param s scene
     * \param shader shader
     */
    static void set_light_uniforms(const Scene& s, const gpu::Shader& shader);
    
private:
    Vec4 clear_color = {0, 0, 0, 0};
    
    bool scissor_test = false;
    
    gpu::Rect scissor = gpu::Rect(0, 0, 0, 0);
    
    gpu::Rect viewport = gpu::Rect(0, 0, 0, 0);
    
    const gpu::RenderTarget* target = nullptr;
    
    TextureCallback texture_callback = [](gpu::Texture& t) -> void {
        t.generate_mipmap(); /* generate mipmap for every texture */
    };
    
    RenderingMode rendering_mode = DEFERRED_RENDERING;
    
    float skybox_intensity = 1;
    
    std::unique_ptr<gpu::Texture> skybox_map;
    
    std::unordered_map<const Mesh*, std::unique_ptr<gpu::VertexObject[]>> mesh_cache;
    
    std::unordered_map<const Image*, std::unique_ptr<gpu::Texture>> image_cache;
    
    static std::unique_ptr<gpu::VertexObject> cube;
    
    static std::unique_ptr<gpu::Texture> probe_map;
    
    static std::unique_ptr<gpu::Renderbuffer> probe_buffer;
    
    static std::unique_ptr<gpu::RenderTarget> probe_target;
    
    void render_skybox_to_buffer(const Camera& c, RenderingMode r) const;
    
    void render_to_buffer(const Scene& s, const Camera& c, RenderingMode r, bool t) const;
    
    void render_to_shadow(const Scene& s, const Camera& c) const;
    
    static void init_cube();
    
    static void sort_instances(const Camera& c, std::vector<const Instance*>& l, bool t);
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/RenderPass.h ---------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class RenderPass {
public:
    /**
     * Creates a new RenderPass object.
     */
    RenderPass() = default;
    
    /**
     * Initializes the render pass and prepares the resources for rendering.
     */
    virtual void init() = 0;
    
    /**
     * Compiles the required shaders and renders to the render target.
     */
    virtual void render() = 0;
    
    /**
     * Returns the current render target if there is one, returns nullptr
     * otherwise.
     */
    const gpu::RenderTarget* get_target() const;
    
    /**
     * Sets the render target. If the render target is nullptr, falls back to
     * the default render target.
     *
     * \param t render target
     */
    void set_target(const gpu::RenderTarget* t);
    
    /**
     * Returns the viewport region.
     */
    static gpu::Rect get_viewport();
    
    /**
     * Sets the viewport region to render from (x, y) to (x + width, y +
     * height).
     *
     * \param v viewport region
     */
    static void set_viewport(const gpu::Rect& v);
    
    /**
     * Renders the full screen triangle. The result will be rendered to the
     * specified render target.
     *
     * \param s shader
     * \param t render target
     */
    static void render_to(const gpu::Shader* s, const gpu::RenderTarget* t);
    
protected:
    const gpu::RenderTarget* target = nullptr;
    
    static gpu::Rect viewport;
    
    static std::unique_ptr<gpu::VertexObject> fullscreen_plane;
    
    static void init_fullscreen_plane();
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/BlendPass.h ----------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class BlendPass : public RenderPass {
public:
    /**
     * Creates a new BlendPass object.
     */
    BlendPass() = default;
    
    /**
     * Initializes the render pass and prepares the resources for rendering.
     */
    void init() override;
    
    /**
     * Compiles the required shaders and renders to the render target.
     */
    void render() override;
    
    /**
     * Returns the 2D texture A that represents the input of the render pass.
     */
    const gpu::Texture* get_texture_a() const;
    
    /**
     * Sets the specified 2D texture A as the input of the render pass.
     *
     * \param t texture A
     */
    void set_texture_a(const gpu::Texture* t);
    
    /**
     * Returns the 2D texture B that represents the input of the render pass.
     */
    const gpu::Texture* get_texture_b() const;
    
    /**
     * Sets the specified 2D texture B as the input of the render pass.
     *
     * \param t texture B
     */
    void set_texture_b(const gpu::Texture* t);
    
    /**
     * Returns the operations when applying blending.
     */
    std::string get_operation() const;
    
    /**
     * Sets the operations when applying blending. The default is "a * b".
     *
     * \param o operation
     */
    void set_operation(const std::string& o);
    
    /**
     * Returns the swizzle of the texture A that represents the multiplication
     * factor in blending.
     */
    std::string get_swizzle_a() const;
    
    /**
     * Sets the swizzle of the texture A as the multiplication factor in
     * blending. The default is ".xyzw".
     *
     * \param s swizzle A
     */
    void set_swizzle_a(const std::string& s);
    
    /**
     * Returns the swizzle of the texture B that represents the multiplication
     * factor in blending.
     */
    std::string get_swizzle_b() const;
    
    /**
     * Sets the swizzle of the texture B as the multiplication factor in
     * blending. The default is ".xyzw".
     *
     * \param s swizzle B
     */
    void set_swizzle_b(const std::string& s);
    
private:
    std::string operation = "a * b";
    std::string swizzle_a = ".xyzw";
    std::string swizzle_b = ".xyzw";
    
    const gpu::Texture* map_a = nullptr;
    const gpu::Texture* map_b = nullptr;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/BloomPass.h ----------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class BloomPass : public RenderPass {
public:
    int width = 0;            /**< the width of the screen */
    int height = 0;           /**< the height of the screen */
    float threshold = 1;      /**< the threshold of the luminance to affect bloom */
    float intensity = 1;      /**< the intensity of the bloom effect */
    float radius = 0.5;       /**< the radius of the bloom effect, range is 0 to 1 */
    Vec3 tint = {1, 1, 1};    /**< the tint which modifies the bloom color */
    
    /**
     * Creates a new BloomPass object.
     */
    BloomPass() = default;
    
    /**
     * Creates a new BloomPass object and initializes it with the specified
     * parameters.
     *
     * \param w the width of the screen
     * \param h the height of the screen
     * \param t the threshold of the luminance to affect bloom
     * \param i the intensity of the bloom effect
     * \param r the radius of the bloom effect, range is 0 to 1
     */
    BloomPass(int w, int h, float t = 1, float i = 1, float r = 0.5);
    
    /**
     * Initializes the render pass and prepares the resources for rendering.
     */
    void init() override;
    
    /**
     * Compiles the required shaders and renders to the render target.
     */
    void render() override;
    
    /**
     * Returns the 2D texture that represents the input of the render pass.
     */
    const gpu::Texture* get_texture() const;
    
    /**
     * Sets the specified 2D texture as the input of the render pass.
     *
     * \param t input texture
     */
    void set_texture(const gpu::Texture* t);
    
private:
    const gpu::Texture* map = nullptr;
    
    std::unique_ptr<gpu::Texture> bloom_map_1;
    std::unique_ptr<gpu::Texture> bloom_map_2;
    
    std::unique_ptr<gpu::RenderTarget> bloom_target;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/BlurPass.h ------------------------------------------ */
/* -------------------------------------------------------------------------- */

namespace ink {

enum BlurType {
    BLUR_BOX,
    BLUR_GAUSSIAN,
    BLUR_BILATERAL,
};

class BlurPass : public RenderPass {
public:
    BlurType type = BLUR_BOX;    /**< the type of the blur */
    int width = 0;               /**< the width of the screen */
    int height = 0;              /**< the height of the screen */
    int channel = 4;             /**< the channel number of the texture */
    int radius = 3;              /**< the radius of the blur */
    float sigma_s = 2.0;         /**< the spatial sigma, for Gaussian or bilateral blur */
    float sigma_r = 0.25;        /**< the range sigma, only for bilateral blur */
    
    /**
     * Creates a new BlurPass object.
     */
    BlurPass() = default;
    
    /**
     * Creates a new BlurPass and initializes it with the size of the screen.
     *
     * \param w the width of the screen
     * \param h the height of the screen
     */
    BlurPass(int w, int h);
    
    /**
     * Initializes the render pass and prepares the resources for rendering.
     */
    void init() override;
    
    /**
     * Compiles the required shaders and renders to the render target.
     */
    void render() override;
    
    /**
     * Returns the 2D texture that represents the input of the render pass.
     */
    const gpu::Texture* get_texture() const;
    
    /**
     * Sets the specified 2D texture as the input of the render pass.
     *
     * \param t input texture
     */
    void set_texture(const gpu::Texture* t);
    
private:
    const gpu::Texture* map = nullptr;
    
    std::unique_ptr<gpu::Texture> blur_map_1;
    std::unique_ptr<gpu::Texture> blur_map_2;
    
    std::unique_ptr<gpu::RenderTarget> blur_target_1;
    std::unique_ptr<gpu::RenderTarget> blur_target_2;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/ColorGradePass.h ------------------------------------ */
/* -------------------------------------------------------------------------- */

namespace ink {

class ColorGradePass : public RenderPass {
public:
    Vec3 saturation = {1, 1, 1};    /**< the color purity */
    Vec3 contrast = {1, 1, 1};      /**< the tonal range */
    Vec3 gamma = {1, 1, 1};         /**< the midtones */
    Vec3 gain = {1, 1, 1};          /**< the highlights */
    Vec3 offset = {0, 0, 0};        /**< the shadows */
    
    /**
     * Creates a new ColorGradePass object.
     */
    ColorGradePass() = default;
    
    /**
     * Initializes the render pass and prepares the resources for rendering.
     */
    void init() override;
    
    /**
     * Compiles the required shaders and renders to the render target.
     */
    void render() override;
    
    /**
     * Returns the 2D texture that represents the input of the render pass.
     */
    const gpu::Texture* get_texture() const;
    
    /**
     * Sets the specified 2D texture as the input of the render pass.
     *
     * \param t input texture
     */
    void set_texture(const gpu::Texture* t);
    
private:
    const gpu::Texture* map = nullptr;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/CopyPass.h ------------------------------------------ */
/* -------------------------------------------------------------------------- */

namespace ink {

class CopyPass : public RenderPass {
public:
    /**
     * Creates a new CopyPass object.
     */
    CopyPass() = default;
    
    /**
     * Initializes the render pass and prepares the resources for rendering.
     */
    void init() override;
    
    /**
     * Compiles the required shaders and renders to the render target.
     */
    void render() override;
    
    /**
     * Returns the 2D texture that represents the input of the render pass.
     */
    const gpu::Texture* get_texture() const;
    
    /**
     * Sets the specified 2D texture as the input of the render pass.
     *
     * \param t source texture
     */
    void set_texture(const gpu::Texture* t);
    
private:
    const gpu::Texture* map = nullptr;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/FXAAPass.h ------------------------------------------ */
/* -------------------------------------------------------------------------- */

namespace ink {

class FXAAPass : public RenderPass {
public:
    /**
     * Creates a new FXAAPass (Fast Approximate Anti-Aliasing) object.
     */
    FXAAPass() = default;
    
    /**
     * Initializes the render pass and prepares the resources for rendering.
     */
    void init() override;
    
    /**
     * Compiles the required shaders and renders to the render target.
     */
    void render() override;
    
    /**
     * Returns the 2D texture that represents the input of the render pass.
     */
    const gpu::Texture* get_texture() const;
    
    /**
     * Sets the specified 2D texture as the input of the render pass. The
     * texture should be set to linear filtering.
     *
     * \param t input texture
     */
    void set_texture(const gpu::Texture* t);
    
private:
    const gpu::Texture* map = nullptr;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/GrainPass.h ----------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class GrainPass : public RenderPass {
public:
    float intensity = 0.5;    /**< the intensity of the grain */
    
    /**
     * Creates a new GrainPass object.
     */
    GrainPass() = default;
    
    /**
     * Initializes the render pass and prepares the resources for rendering.
     */
    void init() override;
    
    /**
     * Compiles the required shaders and renders to the render target.
     */
    void render() override;
    
    /**
     * Returns the 2D texture that represents the input of the render pass.
     */
    const gpu::Texture* get_texture() const;
    
    /**
     * Sets the specified 2D texture as the input of the render pass.
     *
     * \param t input texture
     */
    void set_texture(const gpu::Texture* t);
    
private:
    const gpu::Texture* map = nullptr;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/LightPass.h ----------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class LightPass : public RenderPass {
public:
    /**
     * Creates a new LightPass object.
     */
    LightPass() = default;
    
    /**
     * Initializes the render pass and prepares the resources for rendering.
     */
    void init() override;
    
    /**
     * Compiles the required shaders and renders to the render target.
     */
    void render() override;
    
    /**
     * Returns the scene that represents the input of the render pass.
     */
    const Scene* get_scene() const;
    
    /**
     * Sets the specified scene as the input of the render pass.
     *
     * \param s scene
     */
    void set_scene(const Scene* s);
    
    /**
     * Returns the camera that represents the input of the render pass.
     */
    const Camera* get_camera() const;
    
    /**
     * Sets the specified camera as the input of the render pass.
     *
     * \param c camera
     */
    void set_camera(const Camera* c);
    
    /**
     * Returns the 2D texture that represents the base color buffer of G-Buffer.
     */
    const gpu::Texture* get_texture_color() const;
    
    /**
     * Sets the specified 2D texture as the base color buffer of G-Buffer.
     *
     * \param t base color texture
     */
    void set_texture_color(const gpu::Texture* t);
    
    /**
     * Returns the 2D texture that represents the world normal buffer of
     * G-Buffer.
     */
    const gpu::Texture* get_texture_normal() const;
    
    /**
     * Sets the specified 2D texture as the world normal buffer of G-Buffer.
     *
     * \param t world normal texture
     */
    void set_texture_normal(const gpu::Texture* t);
    
    /**
     * Returns the 2D texture that represents the material data buffer of
     * G-Buffer.
     */
    const gpu::Texture* get_texture_material() const;
    
    /**
     * Sets the specified 2D texture as the material data buffer of G-Buffer.
     *
     * \param t material data texture
     */
    void set_texture_material(const gpu::Texture* t);
    
    /**
     * Returns the 2D texture that represents the indirect light buffer of
     * G-Buffer.
     */
    const gpu::Texture* get_texture_light() const;
    
    /**
     * Sets the specified 2D texture as the indirect light buffer of G-Buffer.
     *
     * \param t indirect light texture
     */
    void set_texture_light(const gpu::Texture* t);
    
    /**
     * Returns the 2D texture that represents the depth map.
     */
    const gpu::Texture* get_texture_depth() const;
    
    /**
     * Sets the specified 2D texture as the depth map (should be set to linear
     * filtering).
     *
     * \param t depth texture
     */
    void set_texture_depth(const gpu::Texture* t);
    
private:
    const Scene* scene = nullptr;
    
    const Camera* camera = nullptr;
    
    const gpu::Texture* g_color = nullptr;
    const gpu::Texture* g_normal = nullptr;
    const gpu::Texture* g_material = nullptr;
    const gpu::Texture* g_light = nullptr;
    const gpu::Texture* z_buffer = nullptr;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/SSAOPass.h ------------------------------------------ */
/* -------------------------------------------------------------------------- */

namespace ink {

class SSAOPass : public RenderPass {
public:
    int width = 0;            /**< the width of the screen */
    int height = 0;           /**< the height of the screen */
    int samples = 32;         /**< sample number, must be 16, 32 or 64 */
    float radius = 0;         /**< radius to search for occluders */
    float max_radius = 0;     /**< the maximum radius from occluders to the pixel */
    float max_z = 100;        /**< the maximum depth to render ambient occlusion */
    float intensity = 1;      /**< the intensity of ambient occlusion, range is 0 to 1 */
    
    /**
     * Creates a new SSAOPass (Screen Space Ambient Occlusion) object.
     */
    SSAOPass() = default;
    
    /**
     * Creates a new SSAOPass (Screen Space Ambient Occlusion) object and
     * initializes it with the specified parameters.
     *
     * \param w the width of the screen
     * \param h the height of the screen
     * \param r radius to search for occluders
     * \param m the maximum radius from occluders to the pixel
     * \param i the intensity of ambient occlusion, range is 0 to 1
     */
    SSAOPass(int w, int h, float r, float m, float i = 1);
    
    /**
     * Initializes the render pass and prepares the resources for rendering.
     */
    void init() override;
    
    /**
     * Compiles the required shaders and renders to the render target.
     */
    void render() override;
    
    /**
     * Returns the camera that represents the input of the render pass.
     */
    const Camera* get_camera() const;
    
    /**
     * Sets the specified camera as the input of the render pass.
     *
     * \param c camera
     */
    void set_camera(const Camera* c);
    
    /**
     * Returns the 2D texture that represents the input of the render pass.
     */
    const gpu::Texture* get_texture() const;
    
    /**
     * Sets the specified 2D texture as the input of the render pass.
     *
     * \param t input texture
     */
    void set_texture(const gpu::Texture* t);
    
    /**
     * Returns the 2D texture that represents the world normal buffer of
     * G-Buffer.
     */
    const gpu::Texture* get_texture_normal() const;
    
    /**
     * Sets the specified 2D texture as the world normal buffer of G-Buffer.
     *
     * \param t world normal texture
     */
    void set_texture_normal(const gpu::Texture* t);
    
    /**
     * Returns the 2D texture that represents the depth map.
     */
    const gpu::Texture* get_texture_depth() const;
    
    /**
     * Sets the specified 2D texture as the depth map (should be set to linear
     * filtering).
     *
     * \param t depth texture
     */
    void set_texture_depth(const gpu::Texture* t);
    
private:
    const Camera* camera = nullptr;
    
    const gpu::Texture* map = nullptr;
    const gpu::Texture* g_normal = nullptr;
    const gpu::Texture* z_buffer = nullptr;
    
    std::unique_ptr<gpu::Texture> blur_map_1;
    std::unique_ptr<gpu::Texture> blur_map_2;
    
    std::unique_ptr<gpu::RenderTarget> blur_target_1;
    std::unique_ptr<gpu::RenderTarget> blur_target_2;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/SSRPass.h ------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

class SSRPass : public RenderPass {
public:
    int max_steps = 50;           /**< the maximum steps of ray marching */
    float thickness = 0.02;       /**< the thickness of the objects on screen */
    float intensity = 0.5;        /**< the intensity of the reflections, range is 0 to 1 */
    float max_roughness = 0.8;    /**< the maximum roughness to apply the reflection */
    
    /**
     * Creates a new SSRPass (Screen Space Reflection) object.
     */
    SSRPass() = default;
    
    /**
     * Creates a new SSRPass (Screen Space Reflection) object and initializes it
     * with parameters.
     *
     * \param t the thickness of the objects on screen
     * \param i the intensity of the reflections, range is 0 to 1
     */
    SSRPass(float t = 0.02, float i = 0.5);
    
    /**
     * Initializes the render pass and prepares the resources for rendering.
     */
    void init() override;
    
    /**
     * Compiles the required shaders and renders to the render target.
     */
    void render() override;
    
    /**
     * Returns the camera that represents the input of the render pass.
     */
    const Camera* get_camera() const;
    
    /**
     * Sets the specified camera as the input of the render pass.
     *
     * \param c camera
     */
    void set_camera(const Camera* c);
    
    /**
     * Returns the 2D texture that represents the input of the render pass.
     */
    const gpu::Texture* get_texture() const;
    
    /**
     * Sets the specified 2D texture as the input of the render pass.
     *
     * \param t input texture
     */
    void set_texture(const gpu::Texture* t);
    
    /**
     * Returns the 2D texture that represents the world normal buffer of
     * G-Buffer.
     */
    const gpu::Texture* get_texture_normal() const;
    
    /**
     * Sets the specified 2D texture as the world normal buffer of G-Buffer.
     *
     * \param t world normal texture
     */
    void set_texture_normal(const gpu::Texture* t);
    
    /**
     * Returns the 2D texture that represents the material data buffer of
     * G-Buffer.
     */
    const gpu::Texture* get_texture_material() const;
    
    /**
     * Sets the specified 2D texture as the material data buffer in G-Buffers.
     *
     * \param t material data texture
     */
    void set_texture_material(const gpu::Texture* t);
    
    /**
     * Returns the 2D texture that represents the depth map.
     */
    const gpu::Texture* get_texture_depth() const;
    
    /**
     * Sets the specified 2D texture as the depth map (should be set to linear
     * filtering).
     *
     * \param t depth texture
     */
    void set_texture_depth(const gpu::Texture* t);
    
private:
    const Camera* camera = nullptr;
    
    const gpu::Texture* map = nullptr;
    const gpu::Texture* g_normal = nullptr;
    const gpu::Texture* g_material = nullptr;
    const gpu::Texture* z_buffer = nullptr;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/ToneMapPass.h --------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

enum ToneMapMode {
    LINEAR_TONE_MAP,
    REINHARD_TONE_MAP,
    OPTIMIZED_TONE_MAP,
    ACES_FILMIC_TONE_MAP,
};

class ToneMapPass : public RenderPass {
public:
    ToneMapMode mode = LINEAR_TONE_MAP;    /**< tone mapping mode */
    float exposure = 1;                    /**< tone mapping exposure */
    
    /**
     * Creates a new ToneMapPass object.
     */
    ToneMapPass() = default;
    
    /**
     * Initializes the render pass and prepares the resources for rendering.
     */
    void init() override;
    
    /**
     * Compiles the required shaders and renders to the render target.
     */
    void render() override;
    
    /**
     * Returns the 2D texture that represents the input of the render pass.
     */
    const gpu::Texture* get_texture() const;
    
    /**
     * Sets the specified 2D texture as the input of the render pass.
     *
     * \param t input texture
     */
    void set_texture(const gpu::Texture* t);
    
private:
    const gpu::Texture* map = nullptr;
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/LoadShaders.h --------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

/**
 * Loads all the include and library shaders to ShaderCache.
 */
void load_shaders();

}
