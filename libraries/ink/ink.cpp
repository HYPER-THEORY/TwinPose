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

#include "ink.h"

#include "opengl/glad.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <algorithm>
#include <bit>
#include <format>
#include <fstream>
#include <limits>

/* -------------------------------------------------------------------------- */
/* ---- ink/core/Error.cpp -------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

void Error::set(const std::string& m) {
    const std::string& message = m;
    if (callback) std::invoke(callback, message);
}

void Error::set(const std::string& l, const std::string& m) {
    std::string message = l + " Error: " + m;
    if (callback) std::invoke(callback, message);
}

void Error::set_callback(const ErrorCallback& f) {
    callback = f;
}

Error::ErrorCallback Error::callback;

}

/* -------------------------------------------------------------------------- */
/* ---- ink/core/File.cpp --------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

std::string File::read(const std::string& p) {
    std::string content;
    std::ifstream stream(p, std::fstream::in);
    if (stream.fail()) {
        Error::set("File", "Failed to read from file");
        return content;
    }
    stream.ignore(std::numeric_limits<std::streamsize>::max());
    std::streamsize length = stream.gcount();
    content.resize(length);
    stream.seekg(0, std::fstream::beg);
    stream.read(content.data(), length);
    stream.close();
    return content;
}

void File::write(const std::string& p, const std::string& c) {
    std::ofstream stream(p, std::fstream::out);
    stream.write(c.data(), c.size());
    if (stream.fail()) {
        Error::set("File", "Failed to write to file");
    }
    stream.close();
}

void File::write(const std::string& p, const char* c) {
    std::ofstream stream(p, std::fstream::out);
    stream.write(c, std::strlen(c));
    if (stream.fail()) {
        Error::set("File", "Failed to write to file");
    }
    stream.close();
}

void File::append(const std::string& p, const std::string& c) {
    std::ofstream stream(p, std::fstream::out | std::fstream::app);
    stream.write(c.data(), c.size());
    if (stream.fail()) {
        Error::set("File", "Failed to append to file");
    }
    stream.close();
}

void File::append(const std::string& p, const char* c) {
    std::ofstream stream(p, std::fstream::out | std::fstream::app);
    stream.write(c, std::strlen(c));
    if (stream.fail()) {
        Error::set("File", "Failed to append to file");
    }
    stream.close();
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/math/Color.cpp -------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

const Mat3 RGB_TO_XYZ = {
     0.4124564,  0.3575761,  0.1804375,
     0.2126729,  0.7151522,  0.0721750,
     0.0193339,  0.1191920,  0.9503041,
};

const Mat3 XYZ_TO_RGB = {
     3.2404542, -1.5371385, -0.4985314,
    -0.9692660,  1.8760108,  0.0415560,
     0.0556434, -0.2040259,  1.0572252,
};

static float saturate(float v) {
    return v < 0 ? 0 : v > 1 ? 1 : v;
}

static float rgb_to_srgb(float v) {
    return v <= 0.0031308f ? v * 12.92f : powf(v, 1.f / 2.4f) * 1.055f - 0.055f;
}

static float srgb_to_rgb(float v) {
    return v <= 0.04045f ? v / 12.92f : powf((v + 0.055f) / 1.055f, 2.4f);
}

static Vec3 hue_to_rgb(float h) {
    float r = fabsf(h * 6.f - 3.f) - 1.f;
    float g = 2.f - fabsf(h * 6.f - 2.f);
    float b = 2.f - fabsf(h * 6.f - 4.f);
    return {saturate(r), saturate(g), saturate(b)};
}

static Vec3 rgb_to_hcv(const Vec3& c) {
    Vec4 p = c.y < c.z ? Vec4(c.z, c.y, -1.f, 2.f / 3.f) : Vec4(c.y, c.z, 0.f, -1.f / 3.f);
    Vec4 q = c.x < p.x ? Vec4(p.x, p.y, p.w, c.x) : Vec4(c.x, p.y, p.z, p.x);
    float chr = q.x - fminf(q.w, q.y);
    float hue = fabsf((q.w - q.y) / (6.f * chr + 0.000001f) + q.z);
    return {hue, chr, q.x};
}

Vec3 Color::hex_to_rgb(unsigned int c) {
    float r = (c / 0x10000) / 255.f;
    float g = (c / 0x00100 % 0x100) / 255.f;
    float b = (c / 0x00001 % 0x100) / 255.f;
    return {r, g, b};
}

unsigned int Color::rgb_to_hex(const Vec3& c) {
    unsigned int r = roundf(c.x * 0xff) * 0x10000;
    unsigned int g = roundf(c.y * 0xff) * 0x00100;
    unsigned int b = roundf(c.z * 0xff) * 0x00001;
    return r + g + b;
}

Vec3 Color::rgb_to_srgb(const Vec3& c) {
    return {ink::rgb_to_srgb(c.x), ink::rgb_to_srgb(c.y), ink::rgb_to_srgb(c.z)};
}

Vec3 Color::srgb_to_rgb(const Vec3& c) {
    return {ink::srgb_to_rgb(c.x), ink::srgb_to_rgb(c.y), ink::srgb_to_rgb(c.z)};
}

Vec3 Color::rgb_to_xyz(const Vec3& c) {
    return RGB_TO_XYZ * c;
}

Vec3 Color::xyz_to_rgb(const Vec3& c) {
    return XYZ_TO_RGB * c;
}

Vec3 Color::rgb_to_hsv(const Vec3& c) {
    Vec3 hcv = rgb_to_hcv(c);
    return {hcv.x, hcv.y / (hcv.z + 0.000001f), hcv.z};
}

Vec3 Color::hsv_to_rgb(const Vec3& c) {
    return ((hue_to_rgb(c.x) - 1.f) * c.y + 1.f) * c.z;
}

Vec3 Color::rgb_to_hsl(const Vec3& c) {
    Vec3 hcv = rgb_to_hcv(c);
    float lum = hcv.z - hcv.y * 0.5f;
    float sat = hcv.y / (1.000001f - fabsf(lum * 2.f - 1.f));
    return {hcv.x, sat, lum};
}

Vec3 Color::hsl_to_rgb(const Vec3& c) {
    return (hue_to_rgb(c.x) - 0.5f) * (1.f - fabsf(2.f * c.z - 1.f)) * c.y + c.z;
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/math/Euler.cpp -------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

Euler::Euler(float x, float y, float z, EulerOrder o) :
x(x), y(y), z(z), order(o) {}

Euler::Euler(const Vec3& r, EulerOrder o) :
x(r.x), y(r.y), z(r.z), order(o) {}

Mat3 Euler::to_rotation_matrix() const {
    Mat3 rotation_x = {
        1       , 0       , 0       ,
        0       , cosf(x) , -sinf(x),
        0       , sinf(x) , cosf(x) ,
    };
    Mat3 rotation_y = {
        cosf(y) , 0       , sinf(y) ,
        0       , 1       , 0       ,
        -sinf(y), 0       , cosf(y) ,
    };
    Mat3 rotation_z = {
        cosf(z) , -sinf(z), 0       ,
        sinf(z) , cosf(z) , 0       ,
        0       , 0       , 1       ,
    };
    if (order == EULER_XYZ) {
        return rotation_x * rotation_y * rotation_z;
    }
    if (order == EULER_XZY) {
        return rotation_x * rotation_z * rotation_y;
    }
    if (order == EULER_YXZ) {
        return rotation_y * rotation_x * rotation_z;
    }
    if (order == EULER_YZX) {
        return rotation_y * rotation_z * rotation_x;
    }
    if (order == EULER_ZXY) {
        return rotation_z * rotation_x * rotation_y;
    }
    /*       ... EULER_ZYX*/{
        return rotation_z * rotation_y * rotation_x;
    }
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/math/Random.cpp ------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

double Random::random() {
    return std::generate_canonical<double, std::numeric_limits<double>::digits>(generator);
}

float Random::random_f() {
    return std::generate_canonical<float, std::numeric_limits<float>::digits>(generator);
}

void Random::set_seed(unsigned int s) {
    generator.seed(s);
}

std::mt19937 Random::generator;

}

/* -------------------------------------------------------------------------- */
/* ---- ink/math/Ray.cpp ---------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

Ray::Ray(const Vec3& o, const Vec3& d) : origin(o), direction(d) {}

float Ray::intersect_box(const Vec3& l, const Vec3& u) const {
    Vec3 inv = 1 / direction;
    Vec3 tmin = (l - origin) * inv;
    Vec3 tmax = (u - origin) * inv;
    if (inv.x < 0) std::swap(tmin.x, tmax.x);
    if (inv.y < 0) std::swap(tmin.y, tmax.y);
    if (inv.z < 0) std::swap(tmin.z, tmax.z);
    float tmin_v = tmin.x;
    float tmax_v = tmax.x;
    if (tmin_v > tmax.y || tmin.y > tmax_v) return -1;
    if (tmin.y > tmin_v || std::isnan(tmin_v)) tmin_v = tmin.y;
    if (tmax.y < tmax_v || std::isnan(tmax_v)) tmax_v = tmax.y;
    if (tmin_v > tmax.z || tmin.z > tmax_v) return -1;
    if (tmin.z > tmin_v || std::isnan(tmin_v)) tmin_v = tmin.z;
    if (tmax.z < tmax_v || std::isnan(tmax_v)) tmax_v = tmax.z;
    return tmax_v < 0 ? -1 : tmin_v >= 0 ? tmin_v : tmax_v;
}

float Ray::intersect_plane(const Vec3& n, float d) const {
    float cos_nd = n.dot(direction);
    if (cos_nd == 0) {
        return n.dot(origin) - d == 0 ? 0 : -1;
    }
    float t = -(n.dot(origin) - d) / cos_nd;
    return t < 0 ? -1 : t;
}

float Ray::intersect_sphere(const Vec3& c, float r) const {
    Vec3 ray_to_sphere = c - origin;
    float t = ray_to_sphere.dot(direction);
    float dt_2 = r * r - ray_to_sphere.dot(ray_to_sphere) + t * t;
    if (dt_2 < 0) return -1;
    float dt = sqrtf(dt_2);
    return t + dt < 0 ? -1 : t - dt < 0 ? t + dt : t - dt;
}

float Ray::intersect_triangle(const Vec3& a, const Vec3& b, const Vec3& c) const {
    Vec3 ab = b - a;
    Vec3 ac = c - a;
    Vec3 ao = origin - a;
    Vec3 p = direction.cross(ac);
    Vec3 q = ao.cross(ab);
    float d = ab.dot(p);
    if (d == 0) return -1;
    float inv = 1 / d;
    float u = ao.dot(p) * inv;
    float v = direction.dot(q) * inv;
    float t = ac.dot(q) * inv;
    return t < 0 || u < 0 || u > 1 || v < 0 || u + v > 1 ? -1 : t;
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/objects/Defines.cpp --------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

std::string Defines::get() const {
    return defines;
}

void Defines::set(const Defines& d) {
    defines += d.get();
}

void Defines::set(const std::string& n) {
    defines += std::format("#define {}\n", n);
}

void Defines::set(const std::string& n, const std::string& v) {
    defines += std::format("#define {} {}\n", n, v);
}

void Defines::set_i(const std::string& n, int v) {
    defines += std::format("#define {} {}\n", n, v);
}

void Defines::set_l(const std::string& n, long v) {
    defines += std::format("#define {} {}\n", n, v);
}

void Defines::set_ll(const std::string& n, long long v) {
    defines += std::format("#define {} {}\n", n, v);
}

void Defines::set_if(const std::string& n, bool f) {
    if (f) defines += std::format("#define {}\n", n);
}

void Defines::clear() {
    defines.clear();
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/objects/Image.cpp ----------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

template <typename Type>
static float unpack(Type v) {
    if constexpr (std::is_same_v<Type, float>) return v;
    return v / 255.f;
}

template <typename Type>
static Type pack(float v) {
    if constexpr (std::is_same_v<Type, float>) return v;
    return v < 0 ? 0 : v > 1 ? 255 : roundf(v * 255.f);
}

Image::Image(int w, int h, int c, int b) :
width(w), height(h), channel(c), bytes(b) {
    data.resize(w * h * c * b);
}

Image Image::subimage(int x1, int y1, int x2, int y2) const {
    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);
    
    /* check whether the sub-image region is legal */
    if (x1 < 0 || x2 > width || y1 < 0 || y2 > height) {
        Error::set("Image", "Illegal slicing region");
        return Image();
    }
    
    /* create a new image */
    Image image = Image(x2 - x1, y2 - y1, channel, bytes);
    
    /* copy image's data to subimage */
    int bpp = channel * bytes;
    int row_bytes = image.width * bpp;
    auto* data_ptr = data.data();
    auto* image_ptr = image.data.data();
    for (int r = 0; r < image.height; ++r) {
        auto* ptr_1 = data_ptr + (x1 + (r + y1) * width) * bpp;
        auto* ptr_2 = image_ptr + r * row_bytes;
        std::copy_n(ptr_1, row_bytes, ptr_2);
    }
    
    return image; /* return the sub-image */
}

void Image::flip_vertical() {
    int bpp = channel * bytes;
    int row_bytes = width * bpp;
    std::vector<uint8_t> temp(row_bytes);
    uint8_t* temp_ptr = temp.data();
    uint8_t* data_ptr = data.data();
    int median = height / 2;
    for (int y = 0; y < median; ++y) {
        uint8_t* ptr_1 = data_ptr + y * row_bytes;
        uint8_t* ptr_2 = data_ptr + (height - y - 1) * row_bytes;
        std::copy_n(ptr_1, row_bytes, temp_ptr);
        std::copy_n(ptr_2, row_bytes, ptr_1);
        std::copy_n(temp_ptr, row_bytes, ptr_2);
    }
}

void Image::flip_horizontal() {
    int bpp = channel * bytes;
    std::vector<uint8_t> temp(bpp);
    uint8_t* temp_ptr = temp.data();
    uint8_t* data_ptr = data.data();
    int median = width / 2;
    for (int x = 0; x < median; ++x) {
        for (int y = 0; y < height; ++y) {
            uint8_t* ptr_1 = data_ptr + (x + y * width) * bpp;
            uint8_t* ptr_2 = data_ptr + (width - x - 1 + y * width) * bpp;
            std::copy_n(ptr_1, bpp, temp_ptr);
            std::copy_n(ptr_2, bpp, ptr_1);
            std::copy_n(temp_ptr, bpp, ptr_2);
        }
    }
}

std::vector<Image> Image::split() const {
    int bpp = channel * bytes;
    std::vector<Image> images(channel);
    std::vector<uint8_t*> image_ptrs(channel);
    for (int i = 0; i < channel; ++i) {
        images[i] = Image(width, height, 1, bytes);
        image_ptrs[i] = images[i].data.data();
    }
    const uint8_t* data_ptr = data.data();
    int pixel = width * height;
    while (pixel --> 0) {
        const uint8_t* ptr_1 = data_ptr + pixel * bpp;
        for (int i = 0; i < channel; ++i) {
            uint8_t* ptr_2 = image_ptrs[i] + pixel * bytes;
            std::copy_n(ptr_1 + i * bytes, bytes, ptr_2);
        }
    }
    return images;
}

void Image::convert(ColorConversion c) {
    /* check the number of channels */
    if (channel != 3 && channel != 4) {
        return Error::set("Image", "Image's channel must be 3 or 4");
    }
    
    /* convert from RGB color space to BGR color space */
    if (c == COLOR_RGB_TO_BGR) {
        if (bytes == 1) {
            convert_rgb_to_bgr<uint8_t>();
        } else {
            convert_rgb_to_bgr<float>();
        }
    }
    
    /* convert from BGR color space to RGB color space */
    else if (c == COLOR_BGR_TO_RGB) {
        if (bytes == 1) {
            convert_bgr_to_rgb<uint8_t>();
        } else {
            convert_bgr_to_rgb<float>();
        }
    }
    
    /* convert from RGB color space to SRGB color space */
    else if (c == COLOR_RGB_TO_SRGB) {
        if (bytes == 1) {
            convert_rgb_to_srgb<uint8_t>();
        } else {
            convert_rgb_to_srgb<float>();
        }
    }
    
    /* convert from SRGB color space to RGB color space */
    else if (c == COLOR_SRGB_TO_RGB) {
        if (bytes == 1) {
            convert_srgb_to_rgb<uint8_t>();
        } else {
            convert_srgb_to_rgb<float>();
        }
    }
    
    /* convert from RGB color space to XYZ color space */
    else if (c == COLOR_RGB_TO_XYZ) {
        if (bytes == 1) {
            convert_rgb_to_xyz<uint8_t>();
        } else {
            convert_rgb_to_xyz<float>();
        }
    }
    
    /* convert from XYZ color space to RGB color space */
    else if (c == COLOR_XYZ_TO_RGB) {
        if (bytes == 1) {
            convert_xyz_to_rgb<uint8_t>();
        } else {
            convert_xyz_to_rgb<float>();
        }
    }
    
    /* convert from RGB color space to HSV color space */
    else if (c == COLOR_RGB_TO_HSV) {
        if (bytes == 1) {
            convert_rgb_to_hsv<uint8_t>();
        } else {
            convert_rgb_to_hsv<float>();
        }
    }
    
    /* convert from HSV color space to RGB color space */
    else if (c == COLOR_HSV_TO_RGB) {
        if (bytes == 1) {
            convert_hsv_to_rgb<uint8_t>();
        } else {
            convert_hsv_to_rgb<float>();
        }
    }
    
    /* convert from RGB color space to HSL color space */
    else if (c == COLOR_RGB_TO_HSL) {
        if (bytes == 1) {
            convert_rgb_to_hsl<uint8_t>();
        } else {
            convert_rgb_to_hsl<float>();
        }
    }
    
    /* convert from HSL color space to RGB color space */
    else if (c == COLOR_HSL_TO_RGB) {
        if (bytes == 1) {
            convert_hsl_to_rgb<uint8_t>();
        } else {
            convert_hsl_to_rgb<float>();
        }
    }
}

template <typename Type>
void Image::convert_rgb_to_bgr() {
    Type* ptr_r = reinterpret_cast<Type*>(data.data());
    Type* ptr_b = ptr_r + 2;
    
    /* swap the red and blue channel */
    int i = width * height;
    while (i --> 0) {
        std::swap(ptr_r[channel * i], ptr_b[channel * i]);
    }
}

template <typename Type>
void Image::convert_bgr_to_rgb() {
    Type* ptr_r = reinterpret_cast<Type*>(data.data());
    Type* ptr_b = ptr_r + 2;
    
    /* swap the red and blue channel */
    int i = width * height;
    while (i --> 0) {
        std::swap(ptr_r[channel * i], ptr_b[channel * i]);
    }
}

template <typename Type>
void Image::convert_rgb_to_srgb() {
    Type* ptr_r = reinterpret_cast<Type*>(data.data());
    Type* ptr_g = ptr_r + 1;
    Type* ptr_b = ptr_g + 1;
    
    /* convert colorspace */
    Vec3 color;
    int i = width * height;
    while (i --> 0) {
        /* unpack RGB information from data */
        color.x = unpack<Type>(ptr_r[channel * i]);
        color.y = unpack<Type>(ptr_g[channel * i]);
        color.z = unpack<Type>(ptr_b[channel * i]);
        
        /* convert to SRGB color space */
        color = Color::rgb_to_srgb(color);
        
        /* pack SRGB color to data */
        ptr_r[channel * i] = pack<Type>(color.x);
        ptr_g[channel * i] = pack<Type>(color.y);
        ptr_b[channel * i] = pack<Type>(color.z);
    }
}

template <typename Type>
void Image::convert_srgb_to_rgb() {
    Type* ptr_r = reinterpret_cast<Type*>(data.data());
    Type* ptr_g = ptr_r + 1;
    Type* ptr_b = ptr_g + 1;
    
    /* convert colorspace */
    Vec3 color;
    int i = width * height;
    while (i --> 0) {
        /* unpack SRGB information from data */
        color.x = unpack<Type>(ptr_r[channel * i]);
        color.y = unpack<Type>(ptr_g[channel * i]);
        color.z = unpack<Type>(ptr_b[channel * i]);
        
        /* convert to RGB color space */
        color = Color::srgb_to_rgb(color);
        
        /* pack RGB color to data */
        ptr_r[channel * i] = pack<Type>(color.x);
        ptr_g[channel * i] = pack<Type>(color.y);
        ptr_b[channel * i] = pack<Type>(color.z);
    }
}

template <typename Type>
void Image::convert_rgb_to_xyz() {
    Type* ptr_r = reinterpret_cast<Type*>(data.data());
    Type* ptr_g = ptr_r + 1;
    Type* ptr_b = ptr_g + 1;
    
    /* convert colorspace */
    Vec3 color;
    int i = width * height;
    while (i --> 0) {
        /* unpack RGB information from data */
        color.x = unpack<Type>(ptr_r[channel * i]);
        color.y = unpack<Type>(ptr_g[channel * i]);
        color.z = unpack<Type>(ptr_b[channel * i]);
        
        /* convert to XYZ color space */
        color = Color::rgb_to_xyz(color);
        
        /* pack XYZ color to data */
        ptr_r[channel * i] = pack<Type>(color.x);
        ptr_g[channel * i] = pack<Type>(color.y);
        ptr_b[channel * i] = pack<Type>(color.z);
    }
}

template <typename Type>
void Image::convert_xyz_to_rgb() {
    Type* ptr_r = reinterpret_cast<Type*>(data.data());
    Type* ptr_g = ptr_r + 1;
    Type* ptr_b = ptr_g + 1;
    
    /* convert colorspace */
    Vec3 color;
    int i = width * height;
    while (i --> 0) {
        /* unpack XYZ information from data */
        color.x = unpack<Type>(ptr_r[channel * i]);
        color.y = unpack<Type>(ptr_g[channel * i]);
        color.z = unpack<Type>(ptr_b[channel * i]);
        
        /* convert to RGB color space */
        color = Color::xyz_to_rgb(color);
        
        /* pack RGB color to data */
        ptr_r[channel * i] = pack<Type>(color.x);
        ptr_g[channel * i] = pack<Type>(color.y);
        ptr_b[channel * i] = pack<Type>(color.z);
    }
}

template <typename Type>
void Image::convert_rgb_to_hsv() {
    Type* ptr_r = reinterpret_cast<Type*>(data.data());
    Type* ptr_g = ptr_r + 1;
    Type* ptr_b = ptr_g + 1;
    
    /* convert colorspace */
    Vec3 color;
    int i = width * height;
    while (i --> 0) {
        /* unpack RGB information from data */
        color.x = unpack<Type>(ptr_r[channel * i]);
        color.y = unpack<Type>(ptr_g[channel * i]);
        color.z = unpack<Type>(ptr_b[channel * i]);
        
        /* convert to HSV color space */
        color = Color::rgb_to_hsv(color);
        
        /* pack HSV color to data */
        ptr_r[channel * i] = pack<Type>(color.x);
        ptr_g[channel * i] = pack<Type>(color.y);
        ptr_b[channel * i] = pack<Type>(color.z);
    }
}

template <typename Type>
void Image::convert_hsv_to_rgb() {
    Type* ptr_r = reinterpret_cast<Type*>(data.data());
    Type* ptr_g = ptr_r + 1;
    Type* ptr_b = ptr_g + 1;
    
    /* convert colorspace */
    Vec3 color;
    int i = width * height;
    while (i --> 0) {
        /* unpack HSV information from data */
        color.x = unpack<Type>(ptr_r[channel * i]);
        color.y = unpack<Type>(ptr_g[channel * i]);
        color.z = unpack<Type>(ptr_b[channel * i]);
        
        /* convert to RGB color space */
        color = Color::hsv_to_rgb(color);
        
        /* pack RGB color to data */
        ptr_r[channel * i] = pack<Type>(color.x);
        ptr_g[channel * i] = pack<Type>(color.y);
        ptr_b[channel * i] = pack<Type>(color.z);
    }
}

template <typename Type>
void Image::convert_rgb_to_hsl() {
    Type* ptr_r = reinterpret_cast<Type*>(data.data());
    Type* ptr_g = ptr_r + 1;
    Type* ptr_b = ptr_g + 1;
    
    /* convert colorspace */
    Vec3 color;
    int i = width * height;
    while (i --> 0) {
        /* unpack RGB information from data */
        color.x = unpack<Type>(ptr_r[channel * i]);
        color.y = unpack<Type>(ptr_g[channel * i]);
        color.z = unpack<Type>(ptr_b[channel * i]);
        
        /* convert to HSL color space */
        color = Color::rgb_to_hsl(color);
        
        /* pack HSL color to data */
        ptr_r[channel * i] = pack<Type>(color.x);
        ptr_g[channel * i] = pack<Type>(color.y);
        ptr_b[channel * i] = pack<Type>(color.z);
    }
}

template <typename Type>
void Image::convert_hsl_to_rgb() {
    Type* ptr_r = reinterpret_cast<Type*>(data.data());
    Type* ptr_g = ptr_r + 1;
    Type* ptr_b = ptr_g + 1;
    
    /* convert colorspace */
    Vec3 color;
    int i = width * height;
    while (i --> 0) {
        /* unpack HSL information from data */
        color.x = unpack<Type>(ptr_r[channel * i]);
        color.y = unpack<Type>(ptr_g[channel * i]);
        color.z = unpack<Type>(ptr_b[channel * i]);
        
        /* convert to RGB color space */
        color = Color::hsl_to_rgb(color);
        
        /* pack RGB color to data */
        ptr_r[channel * i] = pack<Type>(color.x);
        ptr_g[channel * i] = pack<Type>(color.y);
        ptr_b[channel * i] = pack<Type>(color.z);
    }
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/objects/Mesh.cpp ------------------------------------------------ */
/* -------------------------------------------------------------------------- */

namespace ink {

Mesh::Mesh(const std::string& n) : name(n) {}

void Mesh::translate(float x, float y, float z) {
    for (auto& v : vertex) {
        v.x += x;
        v.y += y;
        v.z += z;
    }
}

void Mesh::translate(const Vec3& t) {
    for (auto& v : vertex) {
        v += t;
    }
}

void Mesh::rotate_x(float a) {
    for (auto& v : vertex) {
        float v_y = v.y;
        float v_z = v.z;
        v.y = cosf(a) * v_y - sinf(a) * v_z;
        v.z = sinf(a) * v_y + cosf(a) * v_z;
    }
    for (auto& n : normal) {
        float n_y = n.y;
        float n_z = n.z;
        n.y = cosf(a) * n_y - sinf(a) * n_z;
        n.z = sinf(a) * n_y + cosf(a) * n_z;
        n = n.normalize();
    }
    for (auto& t : tangent) {
        Vec3 t_xyz = {t.x, t.y, t.z};
        t_xyz.y = cosf(a) * t.y - sinf(a) * t.z;
        t_xyz.z = sinf(a) * t.y + cosf(a) * t.z;
        t = {t_xyz.normalize(), t.w};
    }
}

void Mesh::rotate_y(float a) {
    for (auto& v : vertex) {
        float v_x = v.x;
        float v_z = v.z;
        v.x = cosf(a) * v_x + sinf(a) * v_z;
        v.z = -sinf(a) * v_x + cosf(a) * v_z;
    }
    for (auto& n : normal) {
        float n_x = n.x;
        float n_z = n.z;
        n.x = cosf(a) * n_x + sinf(a) * n_z;
        n.z = -sinf(a) * n_x + cosf(a) * n_z;
        n = n.normalize();
    }
    for (auto& t : tangent) {
        Vec3 t_xyz = {t.x, t.y, t.z};
        t_xyz.x = cosf(a) * t.x + sinf(a) * t.z;
        t_xyz.z = -sinf(a) * t.x + cosf(a) * t.z;
        t = {t_xyz.normalize(), t.w};
    }
}

void Mesh::rotate_z(float a) {
    for (auto& v : vertex) {
        float v_x = v.x;
        float v_y = v.y;
        v.x = cosf(a) * v_x - sinf(a) * v_y;
        v.y = sinf(a) * v_x + cosf(a) * v_y;
    }
    for (auto& n : normal) {
        float n_x = n.x;
        float n_y = n.y;
        n.x = cosf(a) * n_x - sinf(a) * n_y;
        n.y = sinf(a) * n_x + cosf(a) * n_y;
        n = n.normalize();
    }
    for (auto& t : tangent) {
        Vec3 t_xyz = {t.x, t.y, t.z};
        t_xyz.x = cosf(a) * t.x - sinf(a) * t.y;
        t_xyz.y = sinf(a) * t.x + cosf(a) * t.y;
        t = {t_xyz.normalize(), t.w};
    }
}

void Mesh::rotate(const Euler& e) {
    Mat3 rotation_matrix = e.to_rotation_matrix();
    for (auto& v : vertex) {
        v = rotation_matrix * v;
    }
    for (auto& n : normal) {
        n = Vec3(rotation_matrix * n).normalize();
    }
    for (auto& t : tangent) {
        Vec3 t_xyz = rotation_matrix * Vec3(t.x, t.y, t.z);
        t = {t_xyz.normalize(), t.w};
    }
}

void Mesh::scale(float x, float y, float z) {
    for (auto& v : vertex) {
        v.x *= x;
        v.y *= y;
        v.z *= z;
    }
    for (auto& n : normal) {
        n.x /= x;
        n.y /= y;
        n.z /= z;
        n = n.normalize();
    }
    for (auto& t : tangent) {
        Vec3 t_xyz = {t.x * x, t.y * y, t.z * z};
        t = {t_xyz.normalize(), t.w};
    }
}

void Mesh::scale(const Vec3& s) {
    for (auto& v : vertex) {
        v *= s;
    }
    for (auto& n : normal) {
        n = (n / s).normalize();
    }
    for (auto& t : tangent) {
        Vec3 t_xyz = {t.x * s.x, t.y * s.y, t.z * s.z};
        t = {t_xyz.normalize(), t.w};
    }
}

void Mesh::normalize() {
    size_t size = normal.size();
    for (int i = 0; i < size; ++i) {
        normal[i] = normal[i].normalize();
    }
}

void Mesh::create_normals() {
    if (vertex.empty()) {
        return Error::set("Mesh", "Vertex information is missing");
    }
    size_t size = vertex.size();
    normal.resize(size);
    std::unordered_map<std::string, Vec3> normals;
    for (int i = 0; i < size; i += 3) {
        Vec3 v1 = vertex[i + 1] - vertex[i];
        Vec3 v2 = vertex[i + 2] - vertex[i];
        Vec3 face_normal = v1.cross(v2);
        for (int j = i; j < i + 3; ++j) {
            std::string hash = (vertex[j] + Vec3(0.005)).to_string(2);
            if (normals.count(hash) == 0) normals.insert({hash, {}});
            normals[hash] += face_normal.normalize();
        }
    }
    for (auto& [k, v] : normals) {
        v = v.normalize();
    }
    for (int i = 0; i < size; ++i) {
        std::string hash = (vertex[i] + Vec3(0.005)).to_string(2);
        normal[i] = normals[hash];
    }
}

void Mesh::create_tangents() {
    if (vertex.empty()) {
        return Error::set("Mesh", "Vertex information is missing");
    }
    if (uv.empty()) {
        return Error::set("Mesh", "UV information is missing");
    }
    if (normal.empty()) {
        return Error::set("Mesh", "Normal information is missing");
    }
    size_t size = vertex.size();
    tangent.resize(size);
    for (int i = 0; i < size; i += 3) {
        Vec3 v1 = vertex[i + 1] - vertex[i];
        Vec3 v2 = vertex[i + 2] - vertex[i];
        Vec2 uv1 = uv[i + 1] - uv[i];
        Vec2 uv2 = uv[i + 2] - uv[i];
        float r = 1 / (uv1.x * uv2.y - uv2.x * uv1.y);
        Vec3 t = (v1 * uv2.y - v2 * uv1.y) * r;
        Vec3 b = (v2 * uv1.x - v1 * uv2.x) * r;
        for (int j = i; j < i + 3; ++j) {
            Vec3& n = normal[j];
            Vec3 ortho_t = (t - n * n.dot(t)).normalize();
            float m = n.cross(ortho_t).dot(b) < 0 ? -1 : 1;
            tangent[j] = Vec4(ortho_t, m);
        }
    }
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/objects/Instance.cpp -------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

Instance::Instance(const std::string& n) : name(n) {}

void Instance::add(Instance* i) {
    i->parent = this;
    children.emplace_back(i);
}

void Instance::add(const std::initializer_list<Instance*>& l) {
    for (auto& instance : l) {
        instance->parent = this;
    }
    children.insert(children.end(), l);
}

void Instance::remove(Instance* i) {
    i->parent = nullptr;
    std::erase(children, i);
}

void Instance::remove(const std::initializer_list<Instance*>& l) {
    for (auto& instance : l) {
        instance->parent = nullptr;
        std::erase(children, instance);
    }
}

void Instance::clear() {
    for (auto& child : children) {
        child->parent = nullptr;
    }
    children.clear();
}

Instance* Instance::get_child(int i) const {
    return children[i];
}

Instance* Instance::get_child(const std::string& n) const {
    size_t size = children.size();
    for (int c = 0; c < size; ++c) {
        if (children[c]->name == n) return children[c];
    }
    return nullptr;
}

size_t Instance::get_child_count() const {
    return children.size();
}

Instance* Instance::get_parent() const {
    return parent;
}

void Instance::set_transform(const Vec3& p, const Euler& r, const Vec3& s) {
    position = p;
    rotation = r;
    scale = s;
}

void Instance::update_matrix_local() {
    matrix_local = transform();
}

void Instance::update_matrix_global() {
    matrix_global = transform_global();
}

Vec3 Instance::global_to_local(const Vec3& v) const {
    return inverse_4x4(matrix_global) * Vec4(v, 1);
}

Vec3 Instance::local_to_global(const Vec3& v) const {
    return matrix_global * Vec4(v, 1);
}

Mat4 Instance::transform() const {
    return transform(position, rotation, scale);
}

Mat4 Instance::transform_global() const {
    const Instance* instance = this;
    Mat4 matrix = Mat4::identity();
    while (instance != nullptr) {
        matrix = instance->transform() * matrix;
        instance = instance->parent;
    }
    return matrix;
}

Mat4 Instance::transform(const Vec3& p, const Euler& r, const Vec3& s) {
    Mat3 m = r.to_rotation_matrix();
    return {
        m[0][0] * s.x, m[0][1] * s.y, m[0][2] * s.z, p.x          ,
        m[1][0] * s.x, m[1][1] * s.y, m[1][2] * s.z, p.y          ,
        m[2][0] * s.x, m[2][1] * s.y, m[2][2] * s.z, p.z          ,
        0            , 0            , 0            , 1            ,
    };
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/objects/Uniforms.cpp -------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

size_t Uniforms::get_count() const {
    return uniforms.size();
}

std::string Uniforms::get_name(int i) const {
    return std::get<0>(uniforms[i]);
}

int Uniforms::get_type(int i) const {
    return std::get<1>(uniforms[i]);
}

int Uniforms::get_location(int i) const {
    return std::get<2>(uniforms[i]);
}

float* Uniforms::get_data() {
    return data.data();
}

const float* Uniforms::get_data() const {
    return data.data();
}

void Uniforms::set_i(const std::string& n, int v) {
    size_t size = data.size();
    uniforms.emplace_back(std::make_tuple(n, 0, int(size)));
    data.emplace_back(std::bit_cast<float>(v));
}

void Uniforms::set_u(const std::string& n, unsigned int v) {
    size_t size = data.size();
    uniforms.emplace_back(std::make_tuple(n, 1, int(size)));
    data.emplace_back(std::bit_cast<float>(v));
}

void Uniforms::set_f(const std::string& n, float v) {
    size_t size = data.size();
    uniforms.emplace_back(std::make_tuple(n, 2, int(size)));
    data.emplace_back(v);
}

void Uniforms::set_v2(const std::string& n, const Vec2& v) {
    size_t size = data.size();
    uniforms.emplace_back(std::make_tuple(n, 3, int(size)));
    data.resize(size + 2);
    std::copy_n(&v.x, 2, data.data() + size);
}

void Uniforms::set_v3(const std::string& n, const Vec3& v) {
    size_t size = data.size();
    uniforms.emplace_back(std::make_tuple(n, 4, int(size)));
    data.resize(size + 3);
    std::copy_n(&v.x, 3, data.data() + size);
}

void Uniforms::set_v4(const std::string& n, const Vec4& v) {
    size_t size = data.size();
    uniforms.emplace_back(std::make_tuple(n, 5, int(size)));
    data.resize(size + 4);
    std::copy_n(&v.x, 4, data.data() + size);
}

void Uniforms::set_m2(const std::string& n, const Mat2& v) {
    size_t size = data.size();
    uniforms.emplace_back(std::make_tuple(n, 6, int(size)));
    data.resize(size + 4);
    std::copy_n(&v[0][0], 4, data.data() + size);
}

void Uniforms::set_m3(const std::string& n, const Mat3& v) {
    size_t size = data.size();
    uniforms.emplace_back(std::make_tuple(n, 7, int(size)));
    data.resize(size + 9);
    std::copy_n(&v[0][0], 9, data.data() + size);
}

void Uniforms::set_m4(const std::string& n, const Mat4& v) {
    size_t size = data.size();
    uniforms.emplace_back(std::make_tuple(n, 8, int(size)));
    data.resize(size + 16);
    std::copy_n(&v[0][0], 16, data.data() + size);
}

void Uniforms::clear() {
    uniforms.clear();
    data.clear();
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/objects/Material.cpp -------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

Material::Material(const std::string& n) : name(n) {
    custom_maps[0]  = nullptr;
    custom_maps[1]  = nullptr;
    custom_maps[2]  = nullptr;
    custom_maps[3]  = nullptr;
    custom_maps[4]  = nullptr;
    custom_maps[5]  = nullptr;
    custom_maps[6]  = nullptr;
    custom_maps[7]  = nullptr;
    custom_maps[8]  = nullptr;
    custom_maps[9]  = nullptr;
    custom_maps[10] = nullptr;
    custom_maps[11] = nullptr;
    custom_maps[12] = nullptr;
    custom_maps[13] = nullptr;
    custom_maps[14] = nullptr;
    custom_maps[15] = nullptr;
    side            = FRONT_SIDE;
    shadow_side     = BACK_SIDE;
    depth_func      = FUNC_LEQUAL;
    stencil_func    = FUNC_ALWAYS;
    stencil_fail    = STENCIL_KEEP;
    stencil_zfail   = STENCIL_KEEP;
    stencil_zpass   = STENCIL_KEEP;
    blend_op_rgb    = BLEND_ADD;
    blend_op_alpha  = BLEND_ADD;
    blend_src_rgb   = FACTOR_SRC_ALPHA;
    blend_src_alpha = FACTOR_SRC_ALPHA;
    blend_dst_rgb   = FACTOR_ONE_MINUS_SRC_ALPHA;
    blend_dst_alpha = FACTOR_ONE_MINUS_SRC_ALPHA;
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/loader/Loader.cpp ----------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

Image Loader::load_image(const std::string& p) {
    /* create a new image */
    Image image = Image(0, 0, 0, 1);
    
    /* get image data from file */
    auto* image_ptr = stbi_load(p.c_str(), &image.width, &image.height, &image.channel, 0);
    if (image_ptr == nullptr) {
        Error::set("Loader", "Failed to read from image");
        return Image();
    }
    
    /* copy image data to new image */
    image.data.resize(image.width * image.height * image.channel * image.bytes);
    memcpy(image.data.data(), image_ptr, image.data.size());
    
    /* release image data */
    stbi_image_free(image_ptr);
    return image;
}

Image Loader::load_image_hdr(const std::string& p) {
    /* create a new image */
    Image image = Image(0, 0, 0, 4);
    
    /* get image data from file */
    auto* image_ptr = stbi_loadf(p.c_str(), &image.width, &image.height, &image.channel, 0);
    if (image_ptr == nullptr) {
        Error::set("Loader", "Failed to read from image");
        return Image();
    }
    
    /* copy image data to new image */
    image.data.resize(image.width * image.height * image.channel * image.bytes);
    memcpy(image.data.data(), image_ptr, image.data.size());
    
    /* release image data */
    stbi_image_free(image_ptr);
    return image;
}

LoadObject Loader::load_mtl(const std::string& p) {
    /* prepare the file stream */
    std::ifstream stream;
    stream.open(p, std::ifstream::in);
    if (stream.fail()) {
        Error::set("Loader", "Failed to read from mtl file");
        return LoadObject();
    }
    
    /* initialize load object */
    LoadObject object;
    Material* current_material = nullptr;
    
    /* read data by line */
    std::string keyword;
    while (stream >> keyword) {
        
        /* create a new material */
        if (keyword == "newmtl") {
            std::string name;
            stream >> name;
            current_material = &object.material.emplace_back(Material(name));
        }
        
        /* material is not declared */
        else if (current_material == nullptr) {}
        
        /* Kd: diffuse color */
        else if (keyword == "Kd") {
            Vec3 kd;
            stream >> kd.x >> kd.y >> kd.z;
            current_material->color = kd;
        }
        
        /* Ke: emissive color */
        else if (keyword == "Ke") {
            Vec3 ke;
            stream >> ke.x >> ke.y >> ke.z;
            current_material->emissive = ke;
        }
        
        /* d: dissolve factor */
        else if (keyword == "d") {
            float d;
            stream >> d;
            current_material->alpha = d;
        }
        
        /* tr: transparency factor */
        else if (keyword == "tr") {
            float tr;
            stream >> tr;
            current_material->alpha = 1 - tr;
        }
        
        /* ignore unknown keyword */
        else {
            stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    
    /* close file stream */
    stream.close();
    
    /* return the load object */
    return object;
}

LoadObject Loader::load_obj(const std::string& p, const LoadObjOptions& o) {
    /* prepare the file stream */
    std::ifstream stream;
    stream.open(p, std::ifstream::in);
    if (stream.fail()) {
        Error::set("Loader", "Failed to read from obj file");
        return LoadObject();
    }
    
    /* temporary data */
    std::vector<Vec3> vertex;
    std::vector<Vec3> normal;
    std::vector<Vec2> uv;
    std::vector<Vec3> color;
    
    /* initialize load object */
    LoadObject object;
    Mesh* current_mesh = &object.mesh.emplace_back(Mesh("default"));
    
    /* initialize mesh group pointer */
    current_mesh->groups.emplace_back(MeshGroup{"default", 0, 0});
    MeshGroup* current_group = &current_mesh->groups.back();
    
    /* initialize total length */
    int total_length = 0;
    
    /* read data by line */
    std::string keyword;
    while (stream >> keyword) {
        
        /* add vertex to temporary array */
        if (keyword == "v") {
            Vec3 v;
            stream >> v.x >> v.y >> v.z;
            vertex.emplace_back(v);
            if (o.vertex_color) {
                stream >> v.x >> v.y >> v.z;
                color.emplace_back(v);
            }
        }
        
        /* add normal to temporary array */
        else if (keyword == "vn") {
            Vec3 vn;
            stream >> vn.x >> vn.y >> vn.z;
            normal.emplace_back(vn);
        }
        
        /* add uv to temporary array */
        else if (keyword == "vt") {
            Vec2 vt;
            stream >> vt.x >> vt.y;
            uv.emplace_back(vt);
        }
        
        /* search for data by index and add it to current mesh */
        else if (keyword == "f") {
            int index = 0;
            for (int i = 0; i < 3; ++i) {
                stream >> index;
                current_mesh->vertex.emplace_back(vertex[index - 1]);
                if (o.vertex_color) {
                    current_mesh->color.emplace_back(color[index - 1]);
                }
                
                /* check whether uv is omitted */
                if (stream.peek() != '/') continue;
                stream.get();
                if (std::isdigit(stream.peek())) {
                    stream >> index;
                    current_mesh->uv.emplace_back(uv[index - 1]);
                }
                
                /* check whether normal is omitted */
                if (stream.peek() != '/') continue;
                stream.get();
                stream >> index;
                current_mesh->normal.emplace_back(normal[index - 1]);
            }
            
            /* increase the length of current_group */
            current_group->length += 3;
            total_length += 3;
        }
        
        /* create new mesh object and initialize everything */
        else if (keyword == o.group) {
            std::string name;
            stream >> name;
            
            /* if current mesh has no data, replace its name */
            if (current_mesh->vertex.empty()) {
                current_mesh->name = name;
                continue;
            }
            
            /* initialize total length */
            total_length = 0;
            
            /* create new mesh object */
            current_mesh = &object.mesh.emplace_back(Mesh(name));
            
            /* create new mesh group */
            current_group = &current_mesh->groups.emplace_back(MeshGroup{name, total_length, 0});
        }
        
        /* create new mesh group */
        else if (keyword == "usemtl") {
            std::string name;
            stream >> name;
            
            /* if current mesh group has no data, replace its name */
            if (current_group->length == 0) {
                current_group->name = name;
                continue;
            }
            
            /* create new mesh group */
            current_group = &current_mesh->groups.emplace_back(MeshGroup{name, total_length, 0});
        }
        
        /* ignore unknown keyword */
        else {
            stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    
    /* close file stream */
    stream.close();
    
    /* return the load object */
    return object;
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/camera/Camera.cpp ----------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

bool Camera::is_perspective() const {
    return projection[3][2] == -1;
}

void Camera::lookat(const Vec3& p, const Vec3& d, const Vec3& u) {
    position = p;
    direction = d.normalize();
    up = u.normalize();
    Vec3& dn = direction;
    Vec3& un = up;
    Vec3 rn = -dn.cross(un).normalize();
    viewing = {
              rn.x,       rn.y,       rn.z, -p.dot(rn),
              un.x,       un.y,       un.z, -p.dot(un),
              dn.x,       dn.y,       dn.z, -p.dot(dn),
                 0,          0,          0,          1,
    };
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/camera/OrthoCamera.cpp ------------------------------------------ */
/* -------------------------------------------------------------------------- */

namespace ink {

OrthoCamera::OrthoCamera(float le, float ri, float lo, float up, float n, float f) {
    set(le, ri, lo, up, n, f);
}

void OrthoCamera::set(float le, float ri, float lo, float uh, float n, float f) {
    left = le;
    right = ri;
    lower = lo;
    upper = uh;
    far = f;
    near = n;
    projection = {
                2 / (ri - le),                     0,                     0, (ri + le) / (le - ri),
                            0,         2 / (uh - lo),                     0, (uh + lo) / (lo - uh),
                            0,                     0,           2 / (n - f),     (f + n) / (n - f),
                            0,                     0,                     0,                     1,
    };
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/camera/PerspCamera.cpp ------------------------------------------ */
/* -------------------------------------------------------------------------- */

namespace ink {

PerspCamera::PerspCamera(float fov, float a, float n, float f) {
    set(fov, a, n, f);
}

void PerspCamera::set(float fov, float a, float n, float f) {
    fov_y = fov;
    aspect = a;
    near = n;
    far = f;
    projection = {
        1 / tanf(fov / 2) / a,                     0,                     0,                     0,
                            0,     1 / tanf(fov / 2),                     0,                     0,
                            0,                     0,     (n + f) / (n - f),   2 * f * n / (n - f),
                            0,                     0,                    -1,                     0,
    };
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/meshes/BoxMesh.cpp ---------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

Mesh BoxMesh::create() {
    Mesh mesh = Mesh("Box");
    mesh.groups = {{"default", 0, 36}};
    mesh.vertex = vertex;
    mesh.uv = uv;
    mesh.normal = normal;
    return mesh;
}

std::vector<Vec3> BoxMesh::vertex = {
    { 0.500000000,  0.500000000,  0.500000000},
    { 0.500000000, -0.500000000,  0.500000000},
    { 0.500000000,  0.500000000, -0.500000000},
    { 0.500000000, -0.500000000,  0.500000000},
    { 0.500000000, -0.500000000, -0.500000000},
    { 0.500000000,  0.500000000, -0.500000000},
    {-0.500000000,  0.500000000, -0.500000000},
    {-0.500000000, -0.500000000, -0.500000000},
    {-0.500000000,  0.500000000,  0.500000000},
    {-0.500000000, -0.500000000, -0.500000000},
    {-0.500000000, -0.500000000,  0.500000000},
    {-0.500000000,  0.500000000,  0.500000000},
    {-0.500000000,  0.500000000, -0.500000000},
    {-0.500000000,  0.500000000,  0.500000000},
    { 0.500000000,  0.500000000, -0.500000000},
    {-0.500000000,  0.500000000,  0.500000000},
    { 0.500000000,  0.500000000,  0.500000000},
    { 0.500000000,  0.500000000, -0.500000000},
    {-0.500000000, -0.500000000,  0.500000000},
    {-0.500000000, -0.500000000, -0.500000000},
    { 0.500000000, -0.500000000,  0.500000000},
    {-0.500000000, -0.500000000, -0.500000000},
    { 0.500000000, -0.500000000, -0.500000000},
    { 0.500000000, -0.500000000,  0.500000000},
    {-0.500000000,  0.500000000,  0.500000000},
    {-0.500000000, -0.500000000,  0.500000000},
    { 0.500000000,  0.500000000,  0.500000000},
    {-0.500000000, -0.500000000,  0.500000000},
    { 0.500000000, -0.500000000,  0.500000000},
    { 0.500000000,  0.500000000,  0.500000000},
    { 0.500000000,  0.500000000, -0.500000000},
    { 0.500000000, -0.500000000, -0.500000000},
    {-0.500000000,  0.500000000, -0.500000000},
    { 0.500000000, -0.500000000, -0.500000000},
    {-0.500000000, -0.500000000, -0.500000000},
    {-0.500000000,  0.500000000, -0.500000000},
};

std::vector<Vec2> BoxMesh::uv = {
    { 0.000000000,  1.000000000},
    { 0.000000000,  0.000000000},
    { 1.000000000,  1.000000000},
    { 0.000000000,  0.000000000},
    { 1.000000000,  0.000000000},
    { 1.000000000,  1.000000000},
    { 0.000000000,  1.000000000},
    { 0.000000000,  0.000000000},
    { 1.000000000,  1.000000000},
    { 0.000000000,  0.000000000},
    { 1.000000000,  0.000000000},
    { 1.000000000,  1.000000000},
    { 0.000000000,  1.000000000},
    { 0.000000000,  0.000000000},
    { 1.000000000,  1.000000000},
    { 0.000000000,  0.000000000},
    { 1.000000000,  0.000000000},
    { 1.000000000,  1.000000000},
    { 0.000000000,  1.000000000},
    { 0.000000000,  0.000000000},
    { 1.000000000,  1.000000000},
    { 0.000000000,  0.000000000},
    { 1.000000000,  0.000000000},
    { 1.000000000,  1.000000000},
    { 0.000000000,  1.000000000},
    { 0.000000000,  0.000000000},
    { 1.000000000,  1.000000000},
    { 0.000000000,  0.000000000},
    { 1.000000000,  0.000000000},
    { 1.000000000,  1.000000000},
    { 0.000000000,  1.000000000},
    { 0.000000000,  0.000000000},
    { 1.000000000,  1.000000000},
    { 0.000000000,  0.000000000},
    { 1.000000000,  0.000000000},
    { 1.000000000,  1.000000000},
};

std::vector<Vec3> BoxMesh::normal = {
    { 1.000000000,  0.000000000,  0.000000000},
    { 1.000000000,  0.000000000,  0.000000000},
    { 1.000000000,  0.000000000,  0.000000000},
    { 1.000000000,  0.000000000,  0.000000000},
    { 1.000000000,  0.000000000,  0.000000000},
    { 1.000000000,  0.000000000,  0.000000000},
    {-1.000000000,  0.000000000,  0.000000000},
    {-1.000000000,  0.000000000,  0.000000000},
    {-1.000000000,  0.000000000,  0.000000000},
    {-1.000000000,  0.000000000,  0.000000000},
    {-1.000000000,  0.000000000,  0.000000000},
    {-1.000000000,  0.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000,  0.000000000,  1.000000000},
    { 0.000000000,  0.000000000,  1.000000000},
    { 0.000000000,  0.000000000,  1.000000000},
    { 0.000000000,  0.000000000,  1.000000000},
    { 0.000000000,  0.000000000,  1.000000000},
    { 0.000000000,  0.000000000,  1.000000000},
    { 0.000000000,  0.000000000, -1.000000000},
    { 0.000000000,  0.000000000, -1.000000000},
    { 0.000000000,  0.000000000, -1.000000000},
    { 0.000000000,  0.000000000, -1.000000000},
    { 0.000000000,  0.000000000, -1.000000000},
    { 0.000000000,  0.000000000, -1.000000000},
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/meshes/CylinderMesh.cpp ----------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

Mesh CylinderMesh::create() {
    Mesh mesh = Mesh("Cylinder");
    mesh.groups = {{"default", 0, 192}};
    mesh.vertex = vertex;
    mesh.uv = uv;
    mesh.normal = normal;
    return mesh;
}

std::vector<Vec3> CylinderMesh::vertex = {
    { 0.000000000,  0.500000000,  0.500000000},
    { 0.000000000, -0.500000000,  0.500000000},
    { 0.191341713,  0.500000000,  0.461939752},
    { 0.000000000, -0.500000000,  0.500000000},
    { 0.191341713, -0.500000000,  0.461939752},
    { 0.191341713,  0.500000000,  0.461939752},
    { 0.191341713,  0.500000000,  0.461939752},
    { 0.191341713, -0.500000000,  0.461939752},
    { 0.353553385,  0.500000000,  0.353553385},
    { 0.191341713, -0.500000000,  0.461939752},
    { 0.353553385, -0.500000000,  0.353553385},
    { 0.353553385,  0.500000000,  0.353553385},
    { 0.353553385,  0.500000000,  0.353553385},
    { 0.353553385, -0.500000000,  0.353553385},
    { 0.461939752,  0.500000000,  0.191341713},
    { 0.353553385, -0.500000000,  0.353553385},
    { 0.461939752, -0.500000000,  0.191341713},
    { 0.461939752,  0.500000000,  0.191341713},
    { 0.461939752,  0.500000000,  0.191341713},
    { 0.461939752, -0.500000000,  0.191341713},
    { 0.500000000,  0.500000000,  0.000000000},
    { 0.461939752, -0.500000000,  0.191341713},
    { 0.500000000, -0.500000000,  0.000000000},
    { 0.500000000,  0.500000000,  0.000000000},
    { 0.500000000,  0.500000000,  0.000000000},
    { 0.500000000, -0.500000000,  0.000000000},
    { 0.461939752,  0.500000000, -0.191341713},
    { 0.500000000, -0.500000000,  0.000000000},
    { 0.461939752, -0.500000000, -0.191341713},
    { 0.461939752,  0.500000000, -0.191341713},
    { 0.461939752,  0.500000000, -0.191341713},
    { 0.461939752, -0.500000000, -0.191341713},
    { 0.353553385,  0.500000000, -0.353553385},
    { 0.461939752, -0.500000000, -0.191341713},
    { 0.353553385, -0.500000000, -0.353553385},
    { 0.353553385,  0.500000000, -0.353553385},
    { 0.353553385,  0.500000000, -0.353553385},
    { 0.353553385, -0.500000000, -0.353553385},
    { 0.191341713,  0.500000000, -0.461939752},
    { 0.353553385, -0.500000000, -0.353553385},
    { 0.191341713, -0.500000000, -0.461939752},
    { 0.191341713,  0.500000000, -0.461939752},
    { 0.191341713,  0.500000000, -0.461939752},
    { 0.191341713, -0.500000000, -0.461939752},
    { 0.000000000,  0.500000000, -0.500000000},
    { 0.191341713, -0.500000000, -0.461939752},
    { 0.000000000, -0.500000000, -0.500000000},
    { 0.000000000,  0.500000000, -0.500000000},
    { 0.000000000,  0.500000000, -0.500000000},
    { 0.000000000, -0.500000000, -0.500000000},
    {-0.191341713,  0.500000000, -0.461939752},
    { 0.000000000, -0.500000000, -0.500000000},
    {-0.191341713, -0.500000000, -0.461939752},
    {-0.191341713,  0.500000000, -0.461939752},
    {-0.191341713,  0.500000000, -0.461939752},
    {-0.191341713, -0.500000000, -0.461939752},
    {-0.353553385,  0.500000000, -0.353553385},
    {-0.191341713, -0.500000000, -0.461939752},
    {-0.353553385, -0.500000000, -0.353553385},
    {-0.353553385,  0.500000000, -0.353553385},
    {-0.353553385,  0.500000000, -0.353553385},
    {-0.353553385, -0.500000000, -0.353553385},
    {-0.461939752,  0.500000000, -0.191341713},
    {-0.353553385, -0.500000000, -0.353553385},
    {-0.461939752, -0.500000000, -0.191341713},
    {-0.461939752,  0.500000000, -0.191341713},
    {-0.461939752,  0.500000000, -0.191341713},
    {-0.461939752, -0.500000000, -0.191341713},
    {-0.500000000,  0.500000000, -0.000000000},
    {-0.461939752, -0.500000000, -0.191341713},
    {-0.500000000, -0.500000000, -0.000000000},
    {-0.500000000,  0.500000000, -0.000000000},
    {-0.500000000,  0.500000000, -0.000000000},
    {-0.500000000, -0.500000000, -0.000000000},
    {-0.461939752,  0.500000000,  0.191341713},
    {-0.500000000, -0.500000000, -0.000000000},
    {-0.461939752, -0.500000000,  0.191341713},
    {-0.461939752,  0.500000000,  0.191341713},
    {-0.461939752,  0.500000000,  0.191341713},
    {-0.461939752, -0.500000000,  0.191341713},
    {-0.353553385,  0.500000000,  0.353553385},
    {-0.461939752, -0.500000000,  0.191341713},
    {-0.353553385, -0.500000000,  0.353553385},
    {-0.353553385,  0.500000000,  0.353553385},
    {-0.353553385,  0.500000000,  0.353553385},
    {-0.353553385, -0.500000000,  0.353553385},
    {-0.191341713,  0.500000000,  0.461939752},
    {-0.353553385, -0.500000000,  0.353553385},
    {-0.191341713, -0.500000000,  0.461939752},
    {-0.191341713,  0.500000000,  0.461939752},
    {-0.191341713,  0.500000000,  0.461939752},
    {-0.191341713, -0.500000000,  0.461939752},
    {-0.000000000,  0.500000000,  0.500000000},
    {-0.191341713, -0.500000000,  0.461939752},
    {-0.000000000, -0.500000000,  0.500000000},
    {-0.000000000,  0.500000000,  0.500000000},
    { 0.000000000,  0.500000000,  0.500000000},
    { 0.191341713,  0.500000000,  0.461939752},
    { 0.000000000,  0.500000000,  0.000000000},
    { 0.191341713,  0.500000000,  0.461939752},
    { 0.353553385,  0.500000000,  0.353553385},
    { 0.000000000,  0.500000000,  0.000000000},
    { 0.353553385,  0.500000000,  0.353553385},
    { 0.461939752,  0.500000000,  0.191341713},
    { 0.000000000,  0.500000000,  0.000000000},
    { 0.461939752,  0.500000000,  0.191341713},
    { 0.500000000,  0.500000000,  0.000000000},
    { 0.000000000,  0.500000000,  0.000000000},
    { 0.500000000,  0.500000000,  0.000000000},
    { 0.461939752,  0.500000000, -0.191341713},
    { 0.000000000,  0.500000000,  0.000000000},
    { 0.461939752,  0.500000000, -0.191341713},
    { 0.353553385,  0.500000000, -0.353553385},
    { 0.000000000,  0.500000000,  0.000000000},
    { 0.353553385,  0.500000000, -0.353553385},
    { 0.191341713,  0.500000000, -0.461939752},
    { 0.000000000,  0.500000000,  0.000000000},
    { 0.191341713,  0.500000000, -0.461939752},
    { 0.000000000,  0.500000000, -0.500000000},
    { 0.000000000,  0.500000000,  0.000000000},
    { 0.000000000,  0.500000000, -0.500000000},
    {-0.191341713,  0.500000000, -0.461939752},
    { 0.000000000,  0.500000000,  0.000000000},
    {-0.191341713,  0.500000000, -0.461939752},
    {-0.353553385,  0.500000000, -0.353553385},
    { 0.000000000,  0.500000000,  0.000000000},
    {-0.353553385,  0.500000000, -0.353553385},
    {-0.461939752,  0.500000000, -0.191341713},
    { 0.000000000,  0.500000000,  0.000000000},
    {-0.461939752,  0.500000000, -0.191341713},
    {-0.500000000,  0.500000000, -0.000000000},
    { 0.000000000,  0.500000000,  0.000000000},
    {-0.500000000,  0.500000000, -0.000000000},
    {-0.461939752,  0.500000000,  0.191341713},
    { 0.000000000,  0.500000000,  0.000000000},
    {-0.461939752,  0.500000000,  0.191341713},
    {-0.353553385,  0.500000000,  0.353553385},
    { 0.000000000,  0.500000000,  0.000000000},
    {-0.353553385,  0.500000000,  0.353553385},
    {-0.191341713,  0.500000000,  0.461939752},
    { 0.000000000,  0.500000000,  0.000000000},
    {-0.191341713,  0.500000000,  0.461939752},
    {-0.000000000,  0.500000000,  0.500000000},
    { 0.000000000,  0.500000000,  0.000000000},
    { 0.191341713, -0.500000000,  0.461939752},
    { 0.000000000, -0.500000000,  0.500000000},
    { 0.000000000, -0.500000000,  0.000000000},
    { 0.353553385, -0.500000000,  0.353553385},
    { 0.191341713, -0.500000000,  0.461939752},
    { 0.000000000, -0.500000000,  0.000000000},
    { 0.461939752, -0.500000000,  0.191341713},
    { 0.353553385, -0.500000000,  0.353553385},
    { 0.000000000, -0.500000000,  0.000000000},
    { 0.500000000, -0.500000000,  0.000000000},
    { 0.461939752, -0.500000000,  0.191341713},
    { 0.000000000, -0.500000000,  0.000000000},
    { 0.461939752, -0.500000000, -0.191341713},
    { 0.500000000, -0.500000000,  0.000000000},
    { 0.000000000, -0.500000000,  0.000000000},
    { 0.353553385, -0.500000000, -0.353553385},
    { 0.461939752, -0.500000000, -0.191341713},
    { 0.000000000, -0.500000000,  0.000000000},
    { 0.191341713, -0.500000000, -0.461939752},
    { 0.353553385, -0.500000000, -0.353553385},
    { 0.000000000, -0.500000000,  0.000000000},
    { 0.000000000, -0.500000000, -0.500000000},
    { 0.191341713, -0.500000000, -0.461939752},
    { 0.000000000, -0.500000000,  0.000000000},
    {-0.191341713, -0.500000000, -0.461939752},
    { 0.000000000, -0.500000000, -0.500000000},
    { 0.000000000, -0.500000000,  0.000000000},
    {-0.353553385, -0.500000000, -0.353553385},
    {-0.191341713, -0.500000000, -0.461939752},
    { 0.000000000, -0.500000000,  0.000000000},
    {-0.461939752, -0.500000000, -0.191341713},
    {-0.353553385, -0.500000000, -0.353553385},
    { 0.000000000, -0.500000000,  0.000000000},
    {-0.500000000, -0.500000000, -0.000000000},
    {-0.461939752, -0.500000000, -0.191341713},
    { 0.000000000, -0.500000000,  0.000000000},
    {-0.461939752, -0.500000000,  0.191341713},
    {-0.500000000, -0.500000000, -0.000000000},
    { 0.000000000, -0.500000000,  0.000000000},
    {-0.353553385, -0.500000000,  0.353553385},
    {-0.461939752, -0.500000000,  0.191341713},
    { 0.000000000, -0.500000000,  0.000000000},
    {-0.191341713, -0.500000000,  0.461939752},
    {-0.353553385, -0.500000000,  0.353553385},
    { 0.000000000, -0.500000000,  0.000000000},
    {-0.000000000, -0.500000000,  0.500000000},
    {-0.191341713, -0.500000000,  0.461939752},
    { 0.000000000, -0.500000000,  0.000000000},
};

std::vector<Vec2> CylinderMesh::uv = {
    { 0.000000000,  1.000000000},
    { 0.000000000,  0.000000000},
    { 0.062500000,  1.000000000},
    { 0.000000000,  0.000000000},
    { 0.062500000,  0.000000000},
    { 0.062500000,  1.000000000},
    { 0.062500000,  1.000000000},
    { 0.062500000,  0.000000000},
    { 0.125000000,  1.000000000},
    { 0.062500000,  0.000000000},
    { 0.125000000,  0.000000000},
    { 0.125000000,  1.000000000},
    { 0.125000000,  1.000000000},
    { 0.125000000,  0.000000000},
    { 0.187500000,  1.000000000},
    { 0.125000000,  0.000000000},
    { 0.187500000,  0.000000000},
    { 0.187500000,  1.000000000},
    { 0.187500000,  1.000000000},
    { 0.187500000,  0.000000000},
    { 0.250000000,  1.000000000},
    { 0.187500000,  0.000000000},
    { 0.250000000,  0.000000000},
    { 0.250000000,  1.000000000},
    { 0.250000000,  1.000000000},
    { 0.250000000,  0.000000000},
    { 0.312500000,  1.000000000},
    { 0.250000000,  0.000000000},
    { 0.312500000,  0.000000000},
    { 0.312500000,  1.000000000},
    { 0.312500000,  1.000000000},
    { 0.312500000,  0.000000000},
    { 0.375000000,  1.000000000},
    { 0.312500000,  0.000000000},
    { 0.375000000,  0.000000000},
    { 0.375000000,  1.000000000},
    { 0.375000000,  1.000000000},
    { 0.375000000,  0.000000000},
    { 0.437500000,  1.000000000},
    { 0.375000000,  0.000000000},
    { 0.437500000,  0.000000000},
    { 0.437500000,  1.000000000},
    { 0.437500000,  1.000000000},
    { 0.437500000,  0.000000000},
    { 0.500000000,  1.000000000},
    { 0.437500000,  0.000000000},
    { 0.500000000,  0.000000000},
    { 0.500000000,  1.000000000},
    { 0.500000000,  1.000000000},
    { 0.500000000,  0.000000000},
    { 0.562500000,  1.000000000},
    { 0.500000000,  0.000000000},
    { 0.562500000,  0.000000000},
    { 0.562500000,  1.000000000},
    { 0.562500000,  1.000000000},
    { 0.562500000,  0.000000000},
    { 0.625000000,  1.000000000},
    { 0.562500000,  0.000000000},
    { 0.625000000,  0.000000000},
    { 0.625000000,  1.000000000},
    { 0.625000000,  1.000000000},
    { 0.625000000,  0.000000000},
    { 0.687500000,  1.000000000},
    { 0.625000000,  0.000000000},
    { 0.687500000,  0.000000000},
    { 0.687500000,  1.000000000},
    { 0.687500000,  1.000000000},
    { 0.687500000,  0.000000000},
    { 0.750000000,  1.000000000},
    { 0.687500000,  0.000000000},
    { 0.750000000,  0.000000000},
    { 0.750000000,  1.000000000},
    { 0.750000000,  1.000000000},
    { 0.750000000,  0.000000000},
    { 0.812500000,  1.000000000},
    { 0.750000000,  0.000000000},
    { 0.812500000,  0.000000000},
    { 0.812500000,  1.000000000},
    { 0.812500000,  1.000000000},
    { 0.812500000,  0.000000000},
    { 0.875000000,  1.000000000},
    { 0.812500000,  0.000000000},
    { 0.875000000,  0.000000000},
    { 0.875000000,  1.000000000},
    { 0.875000000,  1.000000000},
    { 0.875000000,  0.000000000},
    { 0.937500000,  1.000000000},
    { 0.875000000,  0.000000000},
    { 0.937500000,  0.000000000},
    { 0.937500000,  1.000000000},
    { 0.937500000,  1.000000000},
    { 0.937500000,  0.000000000},
    { 1.000000000,  1.000000000},
    { 0.937500000,  0.000000000},
    { 1.000000000,  0.000000000},
    { 1.000000000,  1.000000000},
    { 1.000000000,  0.500000000},
    { 0.961939752,  0.691341698},
    { 0.500000000,  0.500000000},
    { 0.961939752,  0.691341698},
    { 0.853553414,  0.853553414},
    { 0.500000000,  0.500000000},
    { 0.853553414,  0.853553414},
    { 0.691341698,  0.961939752},
    { 0.500000000,  0.500000000},
    { 0.691341698,  0.961939752},
    { 0.500000000,  1.000000000},
    { 0.500000000,  0.500000000},
    { 0.500000000,  1.000000000},
    { 0.308658272,  0.961939752},
    { 0.500000000,  0.500000000},
    { 0.308658272,  0.961939752},
    { 0.146446615,  0.853553414},
    { 0.500000000,  0.500000000},
    { 0.146446615,  0.853553414},
    { 0.038060233,  0.691341698},
    { 0.500000000,  0.500000000},
    { 0.038060233,  0.691341698},
    { 0.000000000,  0.500000000},
    { 0.500000000,  0.500000000},
    { 0.000000000,  0.500000000},
    { 0.038060233,  0.308658272},
    { 0.500000000,  0.500000000},
    { 0.038060233,  0.308658272},
    { 0.146446615,  0.146446615},
    { 0.500000000,  0.500000000},
    { 0.146446615,  0.146446615},
    { 0.308658272,  0.038060233},
    { 0.500000000,  0.500000000},
    { 0.308658272,  0.038060233},
    { 0.500000000,  0.000000000},
    { 0.500000000,  0.500000000},
    { 0.500000000,  0.000000000},
    { 0.691341698,  0.038060233},
    { 0.500000000,  0.500000000},
    { 0.691341698,  0.038060233},
    { 0.853553414,  0.146446615},
    { 0.500000000,  0.500000000},
    { 0.853553414,  0.146446615},
    { 0.961939752,  0.308658272},
    { 0.500000000,  0.500000000},
    { 0.961939752,  0.308658272},
    { 1.000000000,  0.500000000},
    { 0.500000000,  0.500000000},
    { 0.961939752,  0.308658272},
    { 1.000000000,  0.500000000},
    { 0.500000000,  0.500000000},
    { 0.853553414,  0.146446615},
    { 0.961939752,  0.308658272},
    { 0.500000000,  0.500000000},
    { 0.691341698,  0.038060233},
    { 0.853553414,  0.146446615},
    { 0.500000000,  0.500000000},
    { 0.500000000,  0.000000000},
    { 0.691341698,  0.038060233},
    { 0.500000000,  0.500000000},
    { 0.308658272,  0.038060233},
    { 0.500000000,  0.000000000},
    { 0.500000000,  0.500000000},
    { 0.146446615,  0.146446615},
    { 0.308658272,  0.038060233},
    { 0.500000000,  0.500000000},
    { 0.038060233,  0.308658272},
    { 0.146446615,  0.146446615},
    { 0.500000000,  0.500000000},
    { 0.000000000,  0.500000000},
    { 0.038060233,  0.308658272},
    { 0.500000000,  0.500000000},
    { 0.038060233,  0.691341698},
    { 0.000000000,  0.500000000},
    { 0.500000000,  0.500000000},
    { 0.146446615,  0.853553414},
    { 0.038060233,  0.691341698},
    { 0.500000000,  0.500000000},
    { 0.308658272,  0.961939752},
    { 0.146446615,  0.853553414},
    { 0.500000000,  0.500000000},
    { 0.500000000,  1.000000000},
    { 0.308658272,  0.961939752},
    { 0.500000000,  0.500000000},
    { 0.691341698,  0.961939752},
    { 0.500000000,  1.000000000},
    { 0.500000000,  0.500000000},
    { 0.853553414,  0.853553414},
    { 0.691341698,  0.961939752},
    { 0.500000000,  0.500000000},
    { 0.961939752,  0.691341698},
    { 0.853553414,  0.853553414},
    { 0.500000000,  0.500000000},
    { 1.000000000,  0.500000000},
    { 0.961939752,  0.691341698},
    { 0.500000000,  0.500000000},
};

std::vector<Vec3> CylinderMesh::normal = {
    { 0.000000000,  0.000000000,  1.000000000},
    { 0.000000000,  0.000000000,  1.000000000},
    { 0.382683426,  0.000000000,  0.923879504},
    { 0.000000000,  0.000000000,  1.000000000},
    { 0.382683426,  0.000000000,  0.923879504},
    { 0.382683426,  0.000000000,  0.923879504},
    { 0.382683426,  0.000000000,  0.923879504},
    { 0.382683426,  0.000000000,  0.923879504},
    { 0.707106769,  0.000000000,  0.707106769},
    { 0.382683426,  0.000000000,  0.923879504},
    { 0.707106769,  0.000000000,  0.707106769},
    { 0.707106769,  0.000000000,  0.707106769},
    { 0.707106769,  0.000000000,  0.707106769},
    { 0.707106769,  0.000000000,  0.707106769},
    { 0.923879504,  0.000000000,  0.382683426},
    { 0.707106769,  0.000000000,  0.707106769},
    { 0.923879504,  0.000000000,  0.382683426},
    { 0.923879504,  0.000000000,  0.382683426},
    { 0.923879504,  0.000000000,  0.382683426},
    { 0.923879504,  0.000000000,  0.382683426},
    { 1.000000000,  0.000000000,  0.000000000},
    { 0.923879504,  0.000000000,  0.382683426},
    { 1.000000000,  0.000000000,  0.000000000},
    { 1.000000000,  0.000000000,  0.000000000},
    { 1.000000000,  0.000000000,  0.000000000},
    { 1.000000000,  0.000000000,  0.000000000},
    { 0.923879504,  0.000000000, -0.382683426},
    { 1.000000000,  0.000000000,  0.000000000},
    { 0.923879504,  0.000000000, -0.382683426},
    { 0.923879504,  0.000000000, -0.382683426},
    { 0.923879504,  0.000000000, -0.382683426},
    { 0.923879504,  0.000000000, -0.382683426},
    { 0.707106769,  0.000000000, -0.707106769},
    { 0.923879504,  0.000000000, -0.382683426},
    { 0.707106769,  0.000000000, -0.707106769},
    { 0.707106769,  0.000000000, -0.707106769},
    { 0.707106769,  0.000000000, -0.707106769},
    { 0.707106769,  0.000000000, -0.707106769},
    { 0.382683426,  0.000000000, -0.923879504},
    { 0.707106769,  0.000000000, -0.707106769},
    { 0.382683426,  0.000000000, -0.923879504},
    { 0.382683426,  0.000000000, -0.923879504},
    { 0.382683426,  0.000000000, -0.923879504},
    { 0.382683426,  0.000000000, -0.923879504},
    { 0.000000000,  0.000000000, -1.000000000},
    { 0.382683426,  0.000000000, -0.923879504},
    { 0.000000000,  0.000000000, -1.000000000},
    { 0.000000000,  0.000000000, -1.000000000},
    { 0.000000000,  0.000000000, -1.000000000},
    { 0.000000000,  0.000000000, -1.000000000},
    {-0.382683426,  0.000000000, -0.923879504},
    { 0.000000000,  0.000000000, -1.000000000},
    {-0.382683426,  0.000000000, -0.923879504},
    {-0.382683426,  0.000000000, -0.923879504},
    {-0.382683426,  0.000000000, -0.923879504},
    {-0.382683426,  0.000000000, -0.923879504},
    {-0.707106769,  0.000000000, -0.707106769},
    {-0.382683426,  0.000000000, -0.923879504},
    {-0.707106769,  0.000000000, -0.707106769},
    {-0.707106769,  0.000000000, -0.707106769},
    {-0.707106769,  0.000000000, -0.707106769},
    {-0.707106769,  0.000000000, -0.707106769},
    {-0.923879504,  0.000000000, -0.382683426},
    {-0.707106769,  0.000000000, -0.707106769},
    {-0.923879504,  0.000000000, -0.382683426},
    {-0.923879504,  0.000000000, -0.382683426},
    {-0.923879504,  0.000000000, -0.382683426},
    {-0.923879504,  0.000000000, -0.382683426},
    {-1.000000000,  0.000000000, -0.000000000},
    {-0.923879504,  0.000000000, -0.382683426},
    {-1.000000000,  0.000000000, -0.000000000},
    {-1.000000000,  0.000000000, -0.000000000},
    {-1.000000000,  0.000000000, -0.000000000},
    {-1.000000000,  0.000000000, -0.000000000},
    {-0.923879504,  0.000000000,  0.382683426},
    {-1.000000000,  0.000000000, -0.000000000},
    {-0.923879504,  0.000000000,  0.382683426},
    {-0.923879504,  0.000000000,  0.382683426},
    {-0.923879504,  0.000000000,  0.382683426},
    {-0.923879504,  0.000000000,  0.382683426},
    {-0.707106769,  0.000000000,  0.707106769},
    {-0.923879504,  0.000000000,  0.382683426},
    {-0.707106769,  0.000000000,  0.707106769},
    {-0.707106769,  0.000000000,  0.707106769},
    {-0.707106769,  0.000000000,  0.707106769},
    {-0.707106769,  0.000000000,  0.707106769},
    {-0.382683426,  0.000000000,  0.923879504},
    {-0.707106769,  0.000000000,  0.707106769},
    {-0.382683426,  0.000000000,  0.923879504},
    {-0.382683426,  0.000000000,  0.923879504},
    {-0.382683426,  0.000000000,  0.923879504},
    {-0.382683426,  0.000000000,  0.923879504},
    {-0.000000000,  0.000000000,  1.000000000},
    {-0.382683426,  0.000000000,  0.923879504},
    {-0.000000000,  0.000000000,  1.000000000},
    {-0.000000000,  0.000000000,  1.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.000000000, -1.000000000,  0.000000000},
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/meshes/PlaneMesh.cpp -------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

Mesh PlaneMesh::create() {
    Mesh mesh = Mesh("Plane");
    mesh.groups = {{"default", 0, 6}};
    mesh.vertex = vertex;
    mesh.uv = uv;
    mesh.normal = normal;
    return mesh;
}

std::vector<Vec3> PlaneMesh::vertex = {
    {-0.500000000,  0.500000000,  0.000000000},
    {-0.500000000, -0.500000000,  0.000000000},
    { 0.500000000,  0.500000000,  0.000000000},
    {-0.500000000, -0.500000000,  0.000000000},
    { 0.500000000, -0.500000000,  0.000000000},
    { 0.500000000,  0.500000000,  0.000000000},
};

std::vector<Vec2> PlaneMesh::uv = {
    { 0.000000000,  1.000000000},
    { 0.000000000,  0.000000000},
    { 1.000000000,  1.000000000},
    { 0.000000000,  0.000000000},
    { 1.000000000,  0.000000000},
    { 1.000000000,  1.000000000},
};

std::vector<Vec3> PlaneMesh::normal = {
    { 0.000000000,  0.000000000,  1.000000000},
    { 0.000000000,  0.000000000,  1.000000000},
    { 0.000000000,  0.000000000,  1.000000000},
    { 0.000000000,  0.000000000,  1.000000000},
    { 0.000000000,  0.000000000,  1.000000000},
    { 0.000000000,  0.000000000,  1.000000000},
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/meshes/SphereMesh.cpp ------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

Mesh SphereMesh::create() {
    Mesh mesh = Mesh("Sphere");
    mesh.groups = {{"default", 0, 672}};
    mesh.vertex = vertex;
    mesh.uv = uv;
    mesh.normal = normal;
    return mesh;
}

std::vector<Vec3> SphereMesh::vertex = {
    { 0.000000000,  0.500000000,  0.000000000},
    {-0.191341713,  0.461939752,  0.000000000},
    {-0.176776692,  0.461939752,  0.073223308},
    { 0.000000000,  0.500000000,  0.000000000},
    {-0.176776692,  0.461939752,  0.073223308},
    {-0.135299027,  0.461939752,  0.135299027},
    { 0.000000000,  0.500000000,  0.000000000},
    {-0.135299027,  0.461939752,  0.135299027},
    {-0.073223308,  0.461939752,  0.176776692},
    { 0.000000000,  0.500000000,  0.000000000},
    {-0.073223308,  0.461939752,  0.176776692},
    {-0.000000000,  0.461939752,  0.191341713},
    { 0.000000000,  0.500000000,  0.000000000},
    {-0.000000000,  0.461939752,  0.191341713},
    { 0.073223308,  0.461939752,  0.176776692},
    { 0.000000000,  0.500000000,  0.000000000},
    { 0.073223308,  0.461939752,  0.176776692},
    { 0.135299027,  0.461939752,  0.135299027},
    { 0.000000000,  0.500000000,  0.000000000},
    { 0.135299027,  0.461939752,  0.135299027},
    { 0.176776692,  0.461939752,  0.073223308},
    { 0.000000000,  0.500000000,  0.000000000},
    { 0.176776692,  0.461939752,  0.073223308},
    { 0.191341713,  0.461939752,  0.000000000},
    { 0.000000000,  0.500000000,  0.000000000},
    { 0.191341713,  0.461939752,  0.000000000},
    { 0.176776692,  0.461939752, -0.073223308},
    { 0.000000000,  0.500000000,  0.000000000},
    { 0.176776692,  0.461939752, -0.073223308},
    { 0.135299027,  0.461939752, -0.135299027},
    { 0.000000000,  0.500000000,  0.000000000},
    { 0.135299027,  0.461939752, -0.135299027},
    { 0.073223308,  0.461939752, -0.176776692},
    { 0.000000000,  0.500000000,  0.000000000},
    { 0.073223308,  0.461939752, -0.176776692},
    { 0.000000000,  0.461939752, -0.191341713},
    { 0.000000000,  0.500000000,  0.000000000},
    { 0.000000000,  0.461939752, -0.191341713},
    {-0.073223308,  0.461939752, -0.176776692},
    { 0.000000000,  0.500000000,  0.000000000},
    {-0.073223308,  0.461939752, -0.176776692},
    {-0.135299027,  0.461939752, -0.135299027},
    { 0.000000000,  0.500000000,  0.000000000},
    {-0.135299027,  0.461939752, -0.135299027},
    {-0.176776692,  0.461939752, -0.073223308},
    { 0.000000000,  0.500000000,  0.000000000},
    {-0.176776692,  0.461939752, -0.073223308},
    {-0.191341713,  0.461939752, -0.000000000},
    {-0.176776692,  0.461939752,  0.073223308},
    {-0.191341713,  0.461939752,  0.000000000},
    {-0.326640755,  0.353553385,  0.135299027},
    {-0.191341713,  0.461939752,  0.000000000},
    {-0.353553385,  0.353553385,  0.000000000},
    {-0.326640755,  0.353553385,  0.135299027},
    {-0.135299027,  0.461939752,  0.135299027},
    {-0.176776692,  0.461939752,  0.073223308},
    {-0.250000000,  0.353553385,  0.250000000},
    {-0.176776692,  0.461939752,  0.073223308},
    {-0.326640755,  0.353553385,  0.135299027},
    {-0.250000000,  0.353553385,  0.250000000},
    {-0.073223308,  0.461939752,  0.176776692},
    {-0.135299027,  0.461939752,  0.135299027},
    {-0.135299027,  0.353553385,  0.326640755},
    {-0.135299027,  0.461939752,  0.135299027},
    {-0.250000000,  0.353553385,  0.250000000},
    {-0.135299027,  0.353553385,  0.326640755},
    {-0.000000000,  0.461939752,  0.191341713},
    {-0.073223308,  0.461939752,  0.176776692},
    {-0.000000000,  0.353553385,  0.353553385},
    {-0.073223308,  0.461939752,  0.176776692},
    {-0.135299027,  0.353553385,  0.326640755},
    {-0.000000000,  0.353553385,  0.353553385},
    { 0.073223308,  0.461939752,  0.176776692},
    {-0.000000000,  0.461939752,  0.191341713},
    { 0.135299027,  0.353553385,  0.326640755},
    {-0.000000000,  0.461939752,  0.191341713},
    {-0.000000000,  0.353553385,  0.353553385},
    { 0.135299027,  0.353553385,  0.326640755},
    { 0.135299027,  0.461939752,  0.135299027},
    { 0.073223308,  0.461939752,  0.176776692},
    { 0.250000000,  0.353553385,  0.250000000},
    { 0.073223308,  0.461939752,  0.176776692},
    { 0.135299027,  0.353553385,  0.326640755},
    { 0.250000000,  0.353553385,  0.250000000},
    { 0.176776692,  0.461939752,  0.073223308},
    { 0.135299027,  0.461939752,  0.135299027},
    { 0.326640755,  0.353553385,  0.135299027},
    { 0.135299027,  0.461939752,  0.135299027},
    { 0.250000000,  0.353553385,  0.250000000},
    { 0.326640755,  0.353553385,  0.135299027},
    { 0.191341713,  0.461939752,  0.000000000},
    { 0.176776692,  0.461939752,  0.073223308},
    { 0.353553385,  0.353553385,  0.000000000},
    { 0.176776692,  0.461939752,  0.073223308},
    { 0.326640755,  0.353553385,  0.135299027},
    { 0.353553385,  0.353553385,  0.000000000},
    { 0.176776692,  0.461939752, -0.073223308},
    { 0.191341713,  0.461939752,  0.000000000},
    { 0.326640755,  0.353553385, -0.135299027},
    { 0.191341713,  0.461939752,  0.000000000},
    { 0.353553385,  0.353553385,  0.000000000},
    { 0.326640755,  0.353553385, -0.135299027},
    { 0.135299027,  0.461939752, -0.135299027},
    { 0.176776692,  0.461939752, -0.073223308},
    { 0.250000000,  0.353553385, -0.250000000},
    { 0.176776692,  0.461939752, -0.073223308},
    { 0.326640755,  0.353553385, -0.135299027},
    { 0.250000000,  0.353553385, -0.250000000},
    { 0.073223308,  0.461939752, -0.176776692},
    { 0.135299027,  0.461939752, -0.135299027},
    { 0.135299027,  0.353553385, -0.326640755},
    { 0.135299027,  0.461939752, -0.135299027},
    { 0.250000000,  0.353553385, -0.250000000},
    { 0.135299027,  0.353553385, -0.326640755},
    { 0.000000000,  0.461939752, -0.191341713},
    { 0.073223308,  0.461939752, -0.176776692},
    { 0.000000000,  0.353553385, -0.353553385},
    { 0.073223308,  0.461939752, -0.176776692},
    { 0.135299027,  0.353553385, -0.326640755},
    { 0.000000000,  0.353553385, -0.353553385},
    {-0.073223308,  0.461939752, -0.176776692},
    { 0.000000000,  0.461939752, -0.191341713},
    {-0.135299027,  0.353553385, -0.326640755},
    { 0.000000000,  0.461939752, -0.191341713},
    { 0.000000000,  0.353553385, -0.353553385},
    {-0.135299027,  0.353553385, -0.326640755},
    {-0.135299027,  0.461939752, -0.135299027},
    {-0.073223308,  0.461939752, -0.176776692},
    {-0.250000000,  0.353553385, -0.250000000},
    {-0.073223308,  0.461939752, -0.176776692},
    {-0.135299027,  0.353553385, -0.326640755},
    {-0.250000000,  0.353553385, -0.250000000},
    {-0.176776692,  0.461939752, -0.073223308},
    {-0.135299027,  0.461939752, -0.135299027},
    {-0.326640755,  0.353553385, -0.135299027},
    {-0.135299027,  0.461939752, -0.135299027},
    {-0.250000000,  0.353553385, -0.250000000},
    {-0.326640755,  0.353553385, -0.135299027},
    {-0.191341713,  0.461939752, -0.000000000},
    {-0.176776692,  0.461939752, -0.073223308},
    {-0.353553385,  0.353553385, -0.000000000},
    {-0.176776692,  0.461939752, -0.073223308},
    {-0.326640755,  0.353553385, -0.135299027},
    {-0.353553385,  0.353553385, -0.000000000},
    {-0.326640755,  0.353553385,  0.135299027},
    {-0.353553385,  0.353553385,  0.000000000},
    {-0.426776707,  0.191341713,  0.176776692},
    {-0.353553385,  0.353553385,  0.000000000},
    {-0.461939752,  0.191341713,  0.000000000},
    {-0.426776707,  0.191341713,  0.176776692},
    {-0.250000000,  0.353553385,  0.250000000},
    {-0.326640755,  0.353553385,  0.135299027},
    {-0.326640755,  0.191341713,  0.326640755},
    {-0.326640755,  0.353553385,  0.135299027},
    {-0.426776707,  0.191341713,  0.176776692},
    {-0.326640755,  0.191341713,  0.326640755},
    {-0.135299027,  0.353553385,  0.326640755},
    {-0.250000000,  0.353553385,  0.250000000},
    {-0.176776692,  0.191341713,  0.426776707},
    {-0.250000000,  0.353553385,  0.250000000},
    {-0.326640755,  0.191341713,  0.326640755},
    {-0.176776692,  0.191341713,  0.426776707},
    {-0.000000000,  0.353553385,  0.353553385},
    {-0.135299027,  0.353553385,  0.326640755},
    {-0.000000000,  0.191341713,  0.461939752},
    {-0.135299027,  0.353553385,  0.326640755},
    {-0.176776692,  0.191341713,  0.426776707},
    {-0.000000000,  0.191341713,  0.461939752},
    { 0.135299027,  0.353553385,  0.326640755},
    {-0.000000000,  0.353553385,  0.353553385},
    { 0.176776692,  0.191341713,  0.426776707},
    {-0.000000000,  0.353553385,  0.353553385},
    {-0.000000000,  0.191341713,  0.461939752},
    { 0.176776692,  0.191341713,  0.426776707},
    { 0.250000000,  0.353553385,  0.250000000},
    { 0.135299027,  0.353553385,  0.326640755},
    { 0.326640755,  0.191341713,  0.326640755},
    { 0.135299027,  0.353553385,  0.326640755},
    { 0.176776692,  0.191341713,  0.426776707},
    { 0.326640755,  0.191341713,  0.326640755},
    { 0.326640755,  0.353553385,  0.135299027},
    { 0.250000000,  0.353553385,  0.250000000},
    { 0.426776707,  0.191341713,  0.176776692},
    { 0.250000000,  0.353553385,  0.250000000},
    { 0.326640755,  0.191341713,  0.326640755},
    { 0.426776707,  0.191341713,  0.176776692},
    { 0.353553385,  0.353553385,  0.000000000},
    { 0.326640755,  0.353553385,  0.135299027},
    { 0.461939752,  0.191341713,  0.000000000},
    { 0.326640755,  0.353553385,  0.135299027},
    { 0.426776707,  0.191341713,  0.176776692},
    { 0.461939752,  0.191341713,  0.000000000},
    { 0.326640755,  0.353553385, -0.135299027},
    { 0.353553385,  0.353553385,  0.000000000},
    { 0.426776707,  0.191341713, -0.176776692},
    { 0.353553385,  0.353553385,  0.000000000},
    { 0.461939752,  0.191341713,  0.000000000},
    { 0.426776707,  0.191341713, -0.176776692},
    { 0.250000000,  0.353553385, -0.250000000},
    { 0.326640755,  0.353553385, -0.135299027},
    { 0.326640755,  0.191341713, -0.326640755},
    { 0.326640755,  0.353553385, -0.135299027},
    { 0.426776707,  0.191341713, -0.176776692},
    { 0.326640755,  0.191341713, -0.326640755},
    { 0.135299027,  0.353553385, -0.326640755},
    { 0.250000000,  0.353553385, -0.250000000},
    { 0.176776692,  0.191341713, -0.426776707},
    { 0.250000000,  0.353553385, -0.250000000},
    { 0.326640755,  0.191341713, -0.326640755},
    { 0.176776692,  0.191341713, -0.426776707},
    { 0.000000000,  0.353553385, -0.353553385},
    { 0.135299027,  0.353553385, -0.326640755},
    { 0.000000000,  0.191341713, -0.461939752},
    { 0.135299027,  0.353553385, -0.326640755},
    { 0.176776692,  0.191341713, -0.426776707},
    { 0.000000000,  0.191341713, -0.461939752},
    {-0.135299027,  0.353553385, -0.326640755},
    { 0.000000000,  0.353553385, -0.353553385},
    {-0.176776692,  0.191341713, -0.426776707},
    { 0.000000000,  0.353553385, -0.353553385},
    { 0.000000000,  0.191341713, -0.461939752},
    {-0.176776692,  0.191341713, -0.426776707},
    {-0.250000000,  0.353553385, -0.250000000},
    {-0.135299027,  0.353553385, -0.326640755},
    {-0.326640755,  0.191341713, -0.326640755},
    {-0.135299027,  0.353553385, -0.326640755},
    {-0.176776692,  0.191341713, -0.426776707},
    {-0.326640755,  0.191341713, -0.326640755},
    {-0.326640755,  0.353553385, -0.135299027},
    {-0.250000000,  0.353553385, -0.250000000},
    {-0.426776707,  0.191341713, -0.176776692},
    {-0.250000000,  0.353553385, -0.250000000},
    {-0.326640755,  0.191341713, -0.326640755},
    {-0.426776707,  0.191341713, -0.176776692},
    {-0.353553385,  0.353553385, -0.000000000},
    {-0.326640755,  0.353553385, -0.135299027},
    {-0.461939752,  0.191341713, -0.000000000},
    {-0.326640755,  0.353553385, -0.135299027},
    {-0.426776707,  0.191341713, -0.176776692},
    {-0.461939752,  0.191341713, -0.000000000},
    {-0.426776707,  0.191341713,  0.176776692},
    {-0.461939752,  0.191341713,  0.000000000},
    {-0.461939752,  0.000000000,  0.191341713},
    {-0.461939752,  0.191341713,  0.000000000},
    {-0.500000000,  0.000000000,  0.000000000},
    {-0.461939752,  0.000000000,  0.191341713},
    {-0.326640755,  0.191341713,  0.326640755},
    {-0.426776707,  0.191341713,  0.176776692},
    {-0.353553385,  0.000000000,  0.353553385},
    {-0.426776707,  0.191341713,  0.176776692},
    {-0.461939752,  0.000000000,  0.191341713},
    {-0.353553385,  0.000000000,  0.353553385},
    {-0.176776692,  0.191341713,  0.426776707},
    {-0.326640755,  0.191341713,  0.326640755},
    {-0.191341713,  0.000000000,  0.461939752},
    {-0.326640755,  0.191341713,  0.326640755},
    {-0.353553385,  0.000000000,  0.353553385},
    {-0.191341713,  0.000000000,  0.461939752},
    {-0.000000000,  0.191341713,  0.461939752},
    {-0.176776692,  0.191341713,  0.426776707},
    {-0.000000000,  0.000000000,  0.500000000},
    {-0.176776692,  0.191341713,  0.426776707},
    {-0.191341713,  0.000000000,  0.461939752},
    {-0.000000000,  0.000000000,  0.500000000},
    { 0.176776692,  0.191341713,  0.426776707},
    {-0.000000000,  0.191341713,  0.461939752},
    { 0.191341713,  0.000000000,  0.461939752},
    {-0.000000000,  0.191341713,  0.461939752},
    {-0.000000000,  0.000000000,  0.500000000},
    { 0.191341713,  0.000000000,  0.461939752},
    { 0.326640755,  0.191341713,  0.326640755},
    { 0.176776692,  0.191341713,  0.426776707},
    { 0.353553385,  0.000000000,  0.353553385},
    { 0.176776692,  0.191341713,  0.426776707},
    { 0.191341713,  0.000000000,  0.461939752},
    { 0.353553385,  0.000000000,  0.353553385},
    { 0.426776707,  0.191341713,  0.176776692},
    { 0.326640755,  0.191341713,  0.326640755},
    { 0.461939752,  0.000000000,  0.191341713},
    { 0.326640755,  0.191341713,  0.326640755},
    { 0.353553385,  0.000000000,  0.353553385},
    { 0.461939752,  0.000000000,  0.191341713},
    { 0.461939752,  0.191341713,  0.000000000},
    { 0.426776707,  0.191341713,  0.176776692},
    { 0.500000000,  0.000000000,  0.000000000},
    { 0.426776707,  0.191341713,  0.176776692},
    { 0.461939752,  0.000000000,  0.191341713},
    { 0.500000000,  0.000000000,  0.000000000},
    { 0.426776707,  0.191341713, -0.176776692},
    { 0.461939752,  0.191341713,  0.000000000},
    { 0.461939752,  0.000000000, -0.191341713},
    { 0.461939752,  0.191341713,  0.000000000},
    { 0.500000000,  0.000000000,  0.000000000},
    { 0.461939752,  0.000000000, -0.191341713},
    { 0.326640755,  0.191341713, -0.326640755},
    { 0.426776707,  0.191341713, -0.176776692},
    { 0.353553385,  0.000000000, -0.353553385},
    { 0.426776707,  0.191341713, -0.176776692},
    { 0.461939752,  0.000000000, -0.191341713},
    { 0.353553385,  0.000000000, -0.353553385},
    { 0.176776692,  0.191341713, -0.426776707},
    { 0.326640755,  0.191341713, -0.326640755},
    { 0.191341713,  0.000000000, -0.461939752},
    { 0.326640755,  0.191341713, -0.326640755},
    { 0.353553385,  0.000000000, -0.353553385},
    { 0.191341713,  0.000000000, -0.461939752},
    { 0.000000000,  0.191341713, -0.461939752},
    { 0.176776692,  0.191341713, -0.426776707},
    { 0.000000000,  0.000000000, -0.500000000},
    { 0.176776692,  0.191341713, -0.426776707},
    { 0.191341713,  0.000000000, -0.461939752},
    { 0.000000000,  0.000000000, -0.500000000},
    {-0.176776692,  0.191341713, -0.426776707},
    { 0.000000000,  0.191341713, -0.461939752},
    {-0.191341713,  0.000000000, -0.461939752},
    { 0.000000000,  0.191341713, -0.461939752},
    { 0.000000000,  0.000000000, -0.500000000},
    {-0.191341713,  0.000000000, -0.461939752},
    {-0.326640755,  0.191341713, -0.326640755},
    {-0.176776692,  0.191341713, -0.426776707},
    {-0.353553385,  0.000000000, -0.353553385},
    {-0.176776692,  0.191341713, -0.426776707},
    {-0.191341713,  0.000000000, -0.461939752},
    {-0.353553385,  0.000000000, -0.353553385},
    {-0.426776707,  0.191341713, -0.176776692},
    {-0.326640755,  0.191341713, -0.326640755},
    {-0.461939752,  0.000000000, -0.191341713},
    {-0.326640755,  0.191341713, -0.326640755},
    {-0.353553385,  0.000000000, -0.353553385},
    {-0.461939752,  0.000000000, -0.191341713},
    {-0.461939752,  0.191341713, -0.000000000},
    {-0.426776707,  0.191341713, -0.176776692},
    {-0.500000000,  0.000000000, -0.000000000},
    {-0.426776707,  0.191341713, -0.176776692},
    {-0.461939752,  0.000000000, -0.191341713},
    {-0.500000000,  0.000000000, -0.000000000},
    {-0.461939752,  0.000000000,  0.191341713},
    {-0.500000000,  0.000000000,  0.000000000},
    {-0.426776707, -0.191341713,  0.176776692},
    {-0.500000000,  0.000000000,  0.000000000},
    {-0.461939752, -0.191341713,  0.000000000},
    {-0.426776707, -0.191341713,  0.176776692},
    {-0.353553385,  0.000000000,  0.353553385},
    {-0.461939752,  0.000000000,  0.191341713},
    {-0.326640755, -0.191341713,  0.326640755},
    {-0.461939752,  0.000000000,  0.191341713},
    {-0.426776707, -0.191341713,  0.176776692},
    {-0.326640755, -0.191341713,  0.326640755},
    {-0.191341713,  0.000000000,  0.461939752},
    {-0.353553385,  0.000000000,  0.353553385},
    {-0.176776692, -0.191341713,  0.426776707},
    {-0.353553385,  0.000000000,  0.353553385},
    {-0.326640755, -0.191341713,  0.326640755},
    {-0.176776692, -0.191341713,  0.426776707},
    {-0.000000000,  0.000000000,  0.500000000},
    {-0.191341713,  0.000000000,  0.461939752},
    {-0.000000000, -0.191341713,  0.461939752},
    {-0.191341713,  0.000000000,  0.461939752},
    {-0.176776692, -0.191341713,  0.426776707},
    {-0.000000000, -0.191341713,  0.461939752},
    { 0.191341713,  0.000000000,  0.461939752},
    {-0.000000000,  0.000000000,  0.500000000},
    { 0.176776692, -0.191341713,  0.426776707},
    {-0.000000000,  0.000000000,  0.500000000},
    {-0.000000000, -0.191341713,  0.461939752},
    { 0.176776692, -0.191341713,  0.426776707},
    { 0.353553385,  0.000000000,  0.353553385},
    { 0.191341713,  0.000000000,  0.461939752},
    { 0.326640755, -0.191341713,  0.326640755},
    { 0.191341713,  0.000000000,  0.461939752},
    { 0.176776692, -0.191341713,  0.426776707},
    { 0.326640755, -0.191341713,  0.326640755},
    { 0.461939752,  0.000000000,  0.191341713},
    { 0.353553385,  0.000000000,  0.353553385},
    { 0.426776707, -0.191341713,  0.176776692},
    { 0.353553385,  0.000000000,  0.353553385},
    { 0.326640755, -0.191341713,  0.326640755},
    { 0.426776707, -0.191341713,  0.176776692},
    { 0.500000000,  0.000000000,  0.000000000},
    { 0.461939752,  0.000000000,  0.191341713},
    { 0.461939752, -0.191341713,  0.000000000},
    { 0.461939752,  0.000000000,  0.191341713},
    { 0.426776707, -0.191341713,  0.176776692},
    { 0.461939752, -0.191341713,  0.000000000},
    { 0.461939752,  0.000000000, -0.191341713},
    { 0.500000000,  0.000000000,  0.000000000},
    { 0.426776707, -0.191341713, -0.176776692},
    { 0.500000000,  0.000000000,  0.000000000},
    { 0.461939752, -0.191341713,  0.000000000},
    { 0.426776707, -0.191341713, -0.176776692},
    { 0.353553385,  0.000000000, -0.353553385},
    { 0.461939752,  0.000000000, -0.191341713},
    { 0.326640755, -0.191341713, -0.326640755},
    { 0.461939752,  0.000000000, -0.191341713},
    { 0.426776707, -0.191341713, -0.176776692},
    { 0.326640755, -0.191341713, -0.326640755},
    { 0.191341713,  0.000000000, -0.461939752},
    { 0.353553385,  0.000000000, -0.353553385},
    { 0.176776692, -0.191341713, -0.426776707},
    { 0.353553385,  0.000000000, -0.353553385},
    { 0.326640755, -0.191341713, -0.326640755},
    { 0.176776692, -0.191341713, -0.426776707},
    { 0.000000000,  0.000000000, -0.500000000},
    { 0.191341713,  0.000000000, -0.461939752},
    { 0.000000000, -0.191341713, -0.461939752},
    { 0.191341713,  0.000000000, -0.461939752},
    { 0.176776692, -0.191341713, -0.426776707},
    { 0.000000000, -0.191341713, -0.461939752},
    {-0.191341713,  0.000000000, -0.461939752},
    { 0.000000000,  0.000000000, -0.500000000},
    {-0.176776692, -0.191341713, -0.426776707},
    { 0.000000000,  0.000000000, -0.500000000},
    { 0.000000000, -0.191341713, -0.461939752},
    {-0.176776692, -0.191341713, -0.426776707},
    {-0.353553385,  0.000000000, -0.353553385},
    {-0.191341713,  0.000000000, -0.461939752},
    {-0.326640755, -0.191341713, -0.326640755},
    {-0.191341713,  0.000000000, -0.461939752},
    {-0.176776692, -0.191341713, -0.426776707},
    {-0.326640755, -0.191341713, -0.326640755},
    {-0.461939752,  0.000000000, -0.191341713},
    {-0.353553385,  0.000000000, -0.353553385},
    {-0.426776707, -0.191341713, -0.176776692},
    {-0.353553385,  0.000000000, -0.353553385},
    {-0.326640755, -0.191341713, -0.326640755},
    {-0.426776707, -0.191341713, -0.176776692},
    {-0.500000000,  0.000000000, -0.000000000},
    {-0.461939752,  0.000000000, -0.191341713},
    {-0.461939752, -0.191341713, -0.000000000},
    {-0.461939752,  0.000000000, -0.191341713},
    {-0.426776707, -0.191341713, -0.176776692},
    {-0.461939752, -0.191341713, -0.000000000},
    {-0.426776707, -0.191341713,  0.176776692},
    {-0.461939752, -0.191341713,  0.000000000},
    {-0.326640755, -0.353553385,  0.135299027},
    {-0.461939752, -0.191341713,  0.000000000},
    {-0.353553385, -0.353553385,  0.000000000},
    {-0.326640755, -0.353553385,  0.135299027},
    {-0.326640755, -0.191341713,  0.326640755},
    {-0.426776707, -0.191341713,  0.176776692},
    {-0.250000000, -0.353553385,  0.250000000},
    {-0.426776707, -0.191341713,  0.176776692},
    {-0.326640755, -0.353553385,  0.135299027},
    {-0.250000000, -0.353553385,  0.250000000},
    {-0.176776692, -0.191341713,  0.426776707},
    {-0.326640755, -0.191341713,  0.326640755},
    {-0.135299027, -0.353553385,  0.326640755},
    {-0.326640755, -0.191341713,  0.326640755},
    {-0.250000000, -0.353553385,  0.250000000},
    {-0.135299027, -0.353553385,  0.326640755},
    {-0.000000000, -0.191341713,  0.461939752},
    {-0.176776692, -0.191341713,  0.426776707},
    {-0.000000000, -0.353553385,  0.353553385},
    {-0.176776692, -0.191341713,  0.426776707},
    {-0.135299027, -0.353553385,  0.326640755},
    {-0.000000000, -0.353553385,  0.353553385},
    { 0.176776692, -0.191341713,  0.426776707},
    {-0.000000000, -0.191341713,  0.461939752},
    { 0.135299027, -0.353553385,  0.326640755},
    {-0.000000000, -0.191341713,  0.461939752},
    {-0.000000000, -0.353553385,  0.353553385},
    { 0.135299027, -0.353553385,  0.326640755},
    { 0.326640755, -0.191341713,  0.326640755},
    { 0.176776692, -0.191341713,  0.426776707},
    { 0.250000000, -0.353553385,  0.250000000},
    { 0.176776692, -0.191341713,  0.426776707},
    { 0.135299027, -0.353553385,  0.326640755},
    { 0.250000000, -0.353553385,  0.250000000},
    { 0.426776707, -0.191341713,  0.176776692},
    { 0.326640755, -0.191341713,  0.326640755},
    { 0.326640755, -0.353553385,  0.135299027},
    { 0.326640755, -0.191341713,  0.326640755},
    { 0.250000000, -0.353553385,  0.250000000},
    { 0.326640755, -0.353553385,  0.135299027},
    { 0.461939752, -0.191341713,  0.000000000},
    { 0.426776707, -0.191341713,  0.176776692},
    { 0.353553385, -0.353553385,  0.000000000},
    { 0.426776707, -0.191341713,  0.176776692},
    { 0.326640755, -0.353553385,  0.135299027},
    { 0.353553385, -0.353553385,  0.000000000},
    { 0.426776707, -0.191341713, -0.176776692},
    { 0.461939752, -0.191341713,  0.000000000},
    { 0.326640755, -0.353553385, -0.135299027},
    { 0.461939752, -0.191341713,  0.000000000},
    { 0.353553385, -0.353553385,  0.000000000},
    { 0.326640755, -0.353553385, -0.135299027},
    { 0.326640755, -0.191341713, -0.326640755},
    { 0.426776707, -0.191341713, -0.176776692},
    { 0.250000000, -0.353553385, -0.250000000},
    { 0.426776707, -0.191341713, -0.176776692},
    { 0.326640755, -0.353553385, -0.135299027},
    { 0.250000000, -0.353553385, -0.250000000},
    { 0.176776692, -0.191341713, -0.426776707},
    { 0.326640755, -0.191341713, -0.326640755},
    { 0.135299027, -0.353553385, -0.326640755},
    { 0.326640755, -0.191341713, -0.326640755},
    { 0.250000000, -0.353553385, -0.250000000},
    { 0.135299027, -0.353553385, -0.326640755},
    { 0.000000000, -0.191341713, -0.461939752},
    { 0.176776692, -0.191341713, -0.426776707},
    { 0.000000000, -0.353553385, -0.353553385},
    { 0.176776692, -0.191341713, -0.426776707},
    { 0.135299027, -0.353553385, -0.326640755},
    { 0.000000000, -0.353553385, -0.353553385},
    {-0.176776692, -0.191341713, -0.426776707},
    { 0.000000000, -0.191341713, -0.461939752},
    {-0.135299027, -0.353553385, -0.326640755},
    { 0.000000000, -0.191341713, -0.461939752},
    { 0.000000000, -0.353553385, -0.353553385},
    {-0.135299027, -0.353553385, -0.326640755},
    {-0.326640755, -0.191341713, -0.326640755},
    {-0.176776692, -0.191341713, -0.426776707},
    {-0.250000000, -0.353553385, -0.250000000},
    {-0.176776692, -0.191341713, -0.426776707},
    {-0.135299027, -0.353553385, -0.326640755},
    {-0.250000000, -0.353553385, -0.250000000},
    {-0.426776707, -0.191341713, -0.176776692},
    {-0.326640755, -0.191341713, -0.326640755},
    {-0.326640755, -0.353553385, -0.135299027},
    {-0.326640755, -0.191341713, -0.326640755},
    {-0.250000000, -0.353553385, -0.250000000},
    {-0.326640755, -0.353553385, -0.135299027},
    {-0.461939752, -0.191341713, -0.000000000},
    {-0.426776707, -0.191341713, -0.176776692},
    {-0.353553385, -0.353553385, -0.000000000},
    {-0.426776707, -0.191341713, -0.176776692},
    {-0.326640755, -0.353553385, -0.135299027},
    {-0.353553385, -0.353553385, -0.000000000},
    {-0.326640755, -0.353553385,  0.135299027},
    {-0.353553385, -0.353553385,  0.000000000},
    {-0.176776692, -0.461939752,  0.073223308},
    {-0.353553385, -0.353553385,  0.000000000},
    {-0.191341713, -0.461939752,  0.000000000},
    {-0.176776692, -0.461939752,  0.073223308},
    {-0.250000000, -0.353553385,  0.250000000},
    {-0.326640755, -0.353553385,  0.135299027},
    {-0.135299027, -0.461939752,  0.135299027},
    {-0.326640755, -0.353553385,  0.135299027},
    {-0.176776692, -0.461939752,  0.073223308},
    {-0.135299027, -0.461939752,  0.135299027},
    {-0.135299027, -0.353553385,  0.326640755},
    {-0.250000000, -0.353553385,  0.250000000},
    {-0.073223308, -0.461939752,  0.176776692},
    {-0.250000000, -0.353553385,  0.250000000},
    {-0.135299027, -0.461939752,  0.135299027},
    {-0.073223308, -0.461939752,  0.176776692},
    {-0.000000000, -0.353553385,  0.353553385},
    {-0.135299027, -0.353553385,  0.326640755},
    {-0.000000000, -0.461939752,  0.191341713},
    {-0.135299027, -0.353553385,  0.326640755},
    {-0.073223308, -0.461939752,  0.176776692},
    {-0.000000000, -0.461939752,  0.191341713},
    { 0.135299027, -0.353553385,  0.326640755},
    {-0.000000000, -0.353553385,  0.353553385},
    { 0.073223308, -0.461939752,  0.176776692},
    {-0.000000000, -0.353553385,  0.353553385},
    {-0.000000000, -0.461939752,  0.191341713},
    { 0.073223308, -0.461939752,  0.176776692},
    { 0.250000000, -0.353553385,  0.250000000},
    { 0.135299027, -0.353553385,  0.326640755},
    { 0.135299027, -0.461939752,  0.135299027},
    { 0.135299027, -0.353553385,  0.326640755},
    { 0.073223308, -0.461939752,  0.176776692},
    { 0.135299027, -0.461939752,  0.135299027},
    { 0.326640755, -0.353553385,  0.135299027},
    { 0.250000000, -0.353553385,  0.250000000},
    { 0.176776692, -0.461939752,  0.073223308},
    { 0.250000000, -0.353553385,  0.250000000},
    { 0.135299027, -0.461939752,  0.135299027},
    { 0.176776692, -0.461939752,  0.073223308},
    { 0.353553385, -0.353553385,  0.000000000},
    { 0.326640755, -0.353553385,  0.135299027},
    { 0.191341713, -0.461939752,  0.000000000},
    { 0.326640755, -0.353553385,  0.135299027},
    { 0.176776692, -0.461939752,  0.073223308},
    { 0.191341713, -0.461939752,  0.000000000},
    { 0.326640755, -0.353553385, -0.135299027},
    { 0.353553385, -0.353553385,  0.000000000},
    { 0.176776692, -0.461939752, -0.073223308},
    { 0.353553385, -0.353553385,  0.000000000},
    { 0.191341713, -0.461939752,  0.000000000},
    { 0.176776692, -0.461939752, -0.073223308},
    { 0.250000000, -0.353553385, -0.250000000},
    { 0.326640755, -0.353553385, -0.135299027},
    { 0.135299027, -0.461939752, -0.135299027},
    { 0.326640755, -0.353553385, -0.135299027},
    { 0.176776692, -0.461939752, -0.073223308},
    { 0.135299027, -0.461939752, -0.135299027},
    { 0.135299027, -0.353553385, -0.326640755},
    { 0.250000000, -0.353553385, -0.250000000},
    { 0.073223308, -0.461939752, -0.176776692},
    { 0.250000000, -0.353553385, -0.250000000},
    { 0.135299027, -0.461939752, -0.135299027},
    { 0.073223308, -0.461939752, -0.176776692},
    { 0.000000000, -0.353553385, -0.353553385},
    { 0.135299027, -0.353553385, -0.326640755},
    { 0.000000000, -0.461939752, -0.191341713},
    { 0.135299027, -0.353553385, -0.326640755},
    { 0.073223308, -0.461939752, -0.176776692},
    { 0.000000000, -0.461939752, -0.191341713},
    {-0.135299027, -0.353553385, -0.326640755},
    { 0.000000000, -0.353553385, -0.353553385},
    {-0.073223308, -0.461939752, -0.176776692},
    { 0.000000000, -0.353553385, -0.353553385},
    { 0.000000000, -0.461939752, -0.191341713},
    {-0.073223308, -0.461939752, -0.176776692},
    {-0.250000000, -0.353553385, -0.250000000},
    {-0.135299027, -0.353553385, -0.326640755},
    {-0.135299027, -0.461939752, -0.135299027},
    {-0.135299027, -0.353553385, -0.326640755},
    {-0.073223308, -0.461939752, -0.176776692},
    {-0.135299027, -0.461939752, -0.135299027},
    {-0.326640755, -0.353553385, -0.135299027},
    {-0.250000000, -0.353553385, -0.250000000},
    {-0.176776692, -0.461939752, -0.073223308},
    {-0.250000000, -0.353553385, -0.250000000},
    {-0.135299027, -0.461939752, -0.135299027},
    {-0.176776692, -0.461939752, -0.073223308},
    {-0.353553385, -0.353553385, -0.000000000},
    {-0.326640755, -0.353553385, -0.135299027},
    {-0.191341713, -0.461939752, -0.000000000},
    {-0.326640755, -0.353553385, -0.135299027},
    {-0.176776692, -0.461939752, -0.073223308},
    {-0.191341713, -0.461939752, -0.000000000},
    {-0.176776692, -0.461939752,  0.073223308},
    {-0.191341713, -0.461939752,  0.000000000},
    {-0.000000000, -0.500000000,  0.000000000},
    {-0.135299027, -0.461939752,  0.135299027},
    {-0.176776692, -0.461939752,  0.073223308},
    {-0.000000000, -0.500000000,  0.000000000},
    {-0.073223308, -0.461939752,  0.176776692},
    {-0.135299027, -0.461939752,  0.135299027},
    {-0.000000000, -0.500000000,  0.000000000},
    {-0.000000000, -0.461939752,  0.191341713},
    {-0.073223308, -0.461939752,  0.176776692},
    {-0.000000000, -0.500000000,  0.000000000},
    { 0.073223308, -0.461939752,  0.176776692},
    {-0.000000000, -0.461939752,  0.191341713},
    { 0.000000000, -0.500000000,  0.000000000},
    { 0.135299027, -0.461939752,  0.135299027},
    { 0.073223308, -0.461939752,  0.176776692},
    { 0.000000000, -0.500000000,  0.000000000},
    { 0.176776692, -0.461939752,  0.073223308},
    { 0.135299027, -0.461939752,  0.135299027},
    { 0.000000000, -0.500000000,  0.000000000},
    { 0.191341713, -0.461939752,  0.000000000},
    { 0.176776692, -0.461939752,  0.073223308},
    { 0.000000000, -0.500000000,  0.000000000},
    { 0.176776692, -0.461939752, -0.073223308},
    { 0.191341713, -0.461939752,  0.000000000},
    { 0.000000000, -0.500000000, -0.000000000},
    { 0.135299027, -0.461939752, -0.135299027},
    { 0.176776692, -0.461939752, -0.073223308},
    { 0.000000000, -0.500000000, -0.000000000},
    { 0.073223308, -0.461939752, -0.176776692},
    { 0.135299027, -0.461939752, -0.135299027},
    { 0.000000000, -0.500000000, -0.000000000},
    { 0.000000000, -0.461939752, -0.191341713},
    { 0.073223308, -0.461939752, -0.176776692},
    { 0.000000000, -0.500000000, -0.000000000},
    {-0.073223308, -0.461939752, -0.176776692},
    { 0.000000000, -0.461939752, -0.191341713},
    {-0.000000000, -0.500000000, -0.000000000},
    {-0.135299027, -0.461939752, -0.135299027},
    {-0.073223308, -0.461939752, -0.176776692},
    {-0.000000000, -0.500000000, -0.000000000},
    {-0.176776692, -0.461939752, -0.073223308},
    {-0.135299027, -0.461939752, -0.135299027},
    {-0.000000000, -0.500000000, -0.000000000},
    {-0.191341713, -0.461939752, -0.000000000},
    {-0.176776692, -0.461939752, -0.073223308},
    {-0.000000000, -0.500000000, -0.000000000},
};

std::vector<Vec2> SphereMesh::uv = {
    { 0.031250000,  1.000000000},
    { 0.000000000,  0.875000000},
    { 0.062500000,  0.875000000},
    { 0.093750000,  1.000000000},
    { 0.062500000,  0.875000000},
    { 0.125000000,  0.875000000},
    { 0.156250000,  1.000000000},
    { 0.125000000,  0.875000000},
    { 0.187500000,  0.875000000},
    { 0.218750000,  1.000000000},
    { 0.187500000,  0.875000000},
    { 0.250000000,  0.875000000},
    { 0.281250000,  1.000000000},
    { 0.250000000,  0.875000000},
    { 0.312500000,  0.875000000},
    { 0.343750000,  1.000000000},
    { 0.312500000,  0.875000000},
    { 0.375000000,  0.875000000},
    { 0.406250000,  1.000000000},
    { 0.375000000,  0.875000000},
    { 0.437500000,  0.875000000},
    { 0.468750000,  1.000000000},
    { 0.437500000,  0.875000000},
    { 0.500000000,  0.875000000},
    { 0.531250000,  1.000000000},
    { 0.500000000,  0.875000000},
    { 0.562500000,  0.875000000},
    { 0.593750000,  1.000000000},
    { 0.562500000,  0.875000000},
    { 0.625000000,  0.875000000},
    { 0.656250000,  1.000000000},
    { 0.625000000,  0.875000000},
    { 0.687500000,  0.875000000},
    { 0.718750000,  1.000000000},
    { 0.687500000,  0.875000000},
    { 0.750000000,  0.875000000},
    { 0.781250000,  1.000000000},
    { 0.750000000,  0.875000000},
    { 0.812500000,  0.875000000},
    { 0.843750000,  1.000000000},
    { 0.812500000,  0.875000000},
    { 0.875000000,  0.875000000},
    { 0.906250000,  1.000000000},
    { 0.875000000,  0.875000000},
    { 0.937500000,  0.875000000},
    { 0.968750000,  1.000000000},
    { 0.937500000,  0.875000000},
    { 1.000000000,  0.875000000},
    { 0.062500000,  0.875000000},
    { 0.000000000,  0.875000000},
    { 0.062500000,  0.750000000},
    { 0.000000000,  0.875000000},
    { 0.000000000,  0.750000000},
    { 0.062500000,  0.750000000},
    { 0.125000000,  0.875000000},
    { 0.062500000,  0.875000000},
    { 0.125000000,  0.750000000},
    { 0.062500000,  0.875000000},
    { 0.062500000,  0.750000000},
    { 0.125000000,  0.750000000},
    { 0.187500000,  0.875000000},
    { 0.125000000,  0.875000000},
    { 0.187500000,  0.750000000},
    { 0.125000000,  0.875000000},
    { 0.125000000,  0.750000000},
    { 0.187500000,  0.750000000},
    { 0.250000000,  0.875000000},
    { 0.187500000,  0.875000000},
    { 0.250000000,  0.750000000},
    { 0.187500000,  0.875000000},
    { 0.187500000,  0.750000000},
    { 0.250000000,  0.750000000},
    { 0.312500000,  0.875000000},
    { 0.250000000,  0.875000000},
    { 0.312500000,  0.750000000},
    { 0.250000000,  0.875000000},
    { 0.250000000,  0.750000000},
    { 0.312500000,  0.750000000},
    { 0.375000000,  0.875000000},
    { 0.312500000,  0.875000000},
    { 0.375000000,  0.750000000},
    { 0.312500000,  0.875000000},
    { 0.312500000,  0.750000000},
    { 0.375000000,  0.750000000},
    { 0.437500000,  0.875000000},
    { 0.375000000,  0.875000000},
    { 0.437500000,  0.750000000},
    { 0.375000000,  0.875000000},
    { 0.375000000,  0.750000000},
    { 0.437500000,  0.750000000},
    { 0.500000000,  0.875000000},
    { 0.437500000,  0.875000000},
    { 0.500000000,  0.750000000},
    { 0.437500000,  0.875000000},
    { 0.437500000,  0.750000000},
    { 0.500000000,  0.750000000},
    { 0.562500000,  0.875000000},
    { 0.500000000,  0.875000000},
    { 0.562500000,  0.750000000},
    { 0.500000000,  0.875000000},
    { 0.500000000,  0.750000000},
    { 0.562500000,  0.750000000},
    { 0.625000000,  0.875000000},
    { 0.562500000,  0.875000000},
    { 0.625000000,  0.750000000},
    { 0.562500000,  0.875000000},
    { 0.562500000,  0.750000000},
    { 0.625000000,  0.750000000},
    { 0.687500000,  0.875000000},
    { 0.625000000,  0.875000000},
    { 0.687500000,  0.750000000},
    { 0.625000000,  0.875000000},
    { 0.625000000,  0.750000000},
    { 0.687500000,  0.750000000},
    { 0.750000000,  0.875000000},
    { 0.687500000,  0.875000000},
    { 0.750000000,  0.750000000},
    { 0.687500000,  0.875000000},
    { 0.687500000,  0.750000000},
    { 0.750000000,  0.750000000},
    { 0.812500000,  0.875000000},
    { 0.750000000,  0.875000000},
    { 0.812500000,  0.750000000},
    { 0.750000000,  0.875000000},
    { 0.750000000,  0.750000000},
    { 0.812500000,  0.750000000},
    { 0.875000000,  0.875000000},
    { 0.812500000,  0.875000000},
    { 0.875000000,  0.750000000},
    { 0.812500000,  0.875000000},
    { 0.812500000,  0.750000000},
    { 0.875000000,  0.750000000},
    { 0.937500000,  0.875000000},
    { 0.875000000,  0.875000000},
    { 0.937500000,  0.750000000},
    { 0.875000000,  0.875000000},
    { 0.875000000,  0.750000000},
    { 0.937500000,  0.750000000},
    { 1.000000000,  0.875000000},
    { 0.937500000,  0.875000000},
    { 1.000000000,  0.750000000},
    { 0.937500000,  0.875000000},
    { 0.937500000,  0.750000000},
    { 1.000000000,  0.750000000},
    { 0.062500000,  0.750000000},
    { 0.000000000,  0.750000000},
    { 0.062500000,  0.625000000},
    { 0.000000000,  0.750000000},
    { 0.000000000,  0.625000000},
    { 0.062500000,  0.625000000},
    { 0.125000000,  0.750000000},
    { 0.062500000,  0.750000000},
    { 0.125000000,  0.625000000},
    { 0.062500000,  0.750000000},
    { 0.062500000,  0.625000000},
    { 0.125000000,  0.625000000},
    { 0.187500000,  0.750000000},
    { 0.125000000,  0.750000000},
    { 0.187500000,  0.625000000},
    { 0.125000000,  0.750000000},
    { 0.125000000,  0.625000000},
    { 0.187500000,  0.625000000},
    { 0.250000000,  0.750000000},
    { 0.187500000,  0.750000000},
    { 0.250000000,  0.625000000},
    { 0.187500000,  0.750000000},
    { 0.187500000,  0.625000000},
    { 0.250000000,  0.625000000},
    { 0.312500000,  0.750000000},
    { 0.250000000,  0.750000000},
    { 0.312500000,  0.625000000},
    { 0.250000000,  0.750000000},
    { 0.250000000,  0.625000000},
    { 0.312500000,  0.625000000},
    { 0.375000000,  0.750000000},
    { 0.312500000,  0.750000000},
    { 0.375000000,  0.625000000},
    { 0.312500000,  0.750000000},
    { 0.312500000,  0.625000000},
    { 0.375000000,  0.625000000},
    { 0.437500000,  0.750000000},
    { 0.375000000,  0.750000000},
    { 0.437500000,  0.625000000},
    { 0.375000000,  0.750000000},
    { 0.375000000,  0.625000000},
    { 0.437500000,  0.625000000},
    { 0.500000000,  0.750000000},
    { 0.437500000,  0.750000000},
    { 0.500000000,  0.625000000},
    { 0.437500000,  0.750000000},
    { 0.437500000,  0.625000000},
    { 0.500000000,  0.625000000},
    { 0.562500000,  0.750000000},
    { 0.500000000,  0.750000000},
    { 0.562500000,  0.625000000},
    { 0.500000000,  0.750000000},
    { 0.500000000,  0.625000000},
    { 0.562500000,  0.625000000},
    { 0.625000000,  0.750000000},
    { 0.562500000,  0.750000000},
    { 0.625000000,  0.625000000},
    { 0.562500000,  0.750000000},
    { 0.562500000,  0.625000000},
    { 0.625000000,  0.625000000},
    { 0.687500000,  0.750000000},
    { 0.625000000,  0.750000000},
    { 0.687500000,  0.625000000},
    { 0.625000000,  0.750000000},
    { 0.625000000,  0.625000000},
    { 0.687500000,  0.625000000},
    { 0.750000000,  0.750000000},
    { 0.687500000,  0.750000000},
    { 0.750000000,  0.625000000},
    { 0.687500000,  0.750000000},
    { 0.687500000,  0.625000000},
    { 0.750000000,  0.625000000},
    { 0.812500000,  0.750000000},
    { 0.750000000,  0.750000000},
    { 0.812500000,  0.625000000},
    { 0.750000000,  0.750000000},
    { 0.750000000,  0.625000000},
    { 0.812500000,  0.625000000},
    { 0.875000000,  0.750000000},
    { 0.812500000,  0.750000000},
    { 0.875000000,  0.625000000},
    { 0.812500000,  0.750000000},
    { 0.812500000,  0.625000000},
    { 0.875000000,  0.625000000},
    { 0.937500000,  0.750000000},
    { 0.875000000,  0.750000000},
    { 0.937500000,  0.625000000},
    { 0.875000000,  0.750000000},
    { 0.875000000,  0.625000000},
    { 0.937500000,  0.625000000},
    { 1.000000000,  0.750000000},
    { 0.937500000,  0.750000000},
    { 1.000000000,  0.625000000},
    { 0.937500000,  0.750000000},
    { 0.937500000,  0.625000000},
    { 1.000000000,  0.625000000},
    { 0.062500000,  0.625000000},
    { 0.000000000,  0.625000000},
    { 0.062500000,  0.500000000},
    { 0.000000000,  0.625000000},
    { 0.000000000,  0.500000000},
    { 0.062500000,  0.500000000},
    { 0.125000000,  0.625000000},
    { 0.062500000,  0.625000000},
    { 0.125000000,  0.500000000},
    { 0.062500000,  0.625000000},
    { 0.062500000,  0.500000000},
    { 0.125000000,  0.500000000},
    { 0.187500000,  0.625000000},
    { 0.125000000,  0.625000000},
    { 0.187500000,  0.500000000},
    { 0.125000000,  0.625000000},
    { 0.125000000,  0.500000000},
    { 0.187500000,  0.500000000},
    { 0.250000000,  0.625000000},
    { 0.187500000,  0.625000000},
    { 0.250000000,  0.500000000},
    { 0.187500000,  0.625000000},
    { 0.187500000,  0.500000000},
    { 0.250000000,  0.500000000},
    { 0.312500000,  0.625000000},
    { 0.250000000,  0.625000000},
    { 0.312500000,  0.500000000},
    { 0.250000000,  0.625000000},
    { 0.250000000,  0.500000000},
    { 0.312500000,  0.500000000},
    { 0.375000000,  0.625000000},
    { 0.312500000,  0.625000000},
    { 0.375000000,  0.500000000},
    { 0.312500000,  0.625000000},
    { 0.312500000,  0.500000000},
    { 0.375000000,  0.500000000},
    { 0.437500000,  0.625000000},
    { 0.375000000,  0.625000000},
    { 0.437500000,  0.500000000},
    { 0.375000000,  0.625000000},
    { 0.375000000,  0.500000000},
    { 0.437500000,  0.500000000},
    { 0.500000000,  0.625000000},
    { 0.437500000,  0.625000000},
    { 0.500000000,  0.500000000},
    { 0.437500000,  0.625000000},
    { 0.437500000,  0.500000000},
    { 0.500000000,  0.500000000},
    { 0.562500000,  0.625000000},
    { 0.500000000,  0.625000000},
    { 0.562500000,  0.500000000},
    { 0.500000000,  0.625000000},
    { 0.500000000,  0.500000000},
    { 0.562500000,  0.500000000},
    { 0.625000000,  0.625000000},
    { 0.562500000,  0.625000000},
    { 0.625000000,  0.500000000},
    { 0.562500000,  0.625000000},
    { 0.562500000,  0.500000000},
    { 0.625000000,  0.500000000},
    { 0.687500000,  0.625000000},
    { 0.625000000,  0.625000000},
    { 0.687500000,  0.500000000},
    { 0.625000000,  0.625000000},
    { 0.625000000,  0.500000000},
    { 0.687500000,  0.500000000},
    { 0.750000000,  0.625000000},
    { 0.687500000,  0.625000000},
    { 0.750000000,  0.500000000},
    { 0.687500000,  0.625000000},
    { 0.687500000,  0.500000000},
    { 0.750000000,  0.500000000},
    { 0.812500000,  0.625000000},
    { 0.750000000,  0.625000000},
    { 0.812500000,  0.500000000},
    { 0.750000000,  0.625000000},
    { 0.750000000,  0.500000000},
    { 0.812500000,  0.500000000},
    { 0.875000000,  0.625000000},
    { 0.812500000,  0.625000000},
    { 0.875000000,  0.500000000},
    { 0.812500000,  0.625000000},
    { 0.812500000,  0.500000000},
    { 0.875000000,  0.500000000},
    { 0.937500000,  0.625000000},
    { 0.875000000,  0.625000000},
    { 0.937500000,  0.500000000},
    { 0.875000000,  0.625000000},
    { 0.875000000,  0.500000000},
    { 0.937500000,  0.500000000},
    { 1.000000000,  0.625000000},
    { 0.937500000,  0.625000000},
    { 1.000000000,  0.500000000},
    { 0.937500000,  0.625000000},
    { 0.937500000,  0.500000000},
    { 1.000000000,  0.500000000},
    { 0.062500000,  0.500000000},
    { 0.000000000,  0.500000000},
    { 0.062500000,  0.375000000},
    { 0.000000000,  0.500000000},
    { 0.000000000,  0.375000000},
    { 0.062500000,  0.375000000},
    { 0.125000000,  0.500000000},
    { 0.062500000,  0.500000000},
    { 0.125000000,  0.375000000},
    { 0.062500000,  0.500000000},
    { 0.062500000,  0.375000000},
    { 0.125000000,  0.375000000},
    { 0.187500000,  0.500000000},
    { 0.125000000,  0.500000000},
    { 0.187500000,  0.375000000},
    { 0.125000000,  0.500000000},
    { 0.125000000,  0.375000000},
    { 0.187500000,  0.375000000},
    { 0.250000000,  0.500000000},
    { 0.187500000,  0.500000000},
    { 0.250000000,  0.375000000},
    { 0.187500000,  0.500000000},
    { 0.187500000,  0.375000000},
    { 0.250000000,  0.375000000},
    { 0.312500000,  0.500000000},
    { 0.250000000,  0.500000000},
    { 0.312500000,  0.375000000},
    { 0.250000000,  0.500000000},
    { 0.250000000,  0.375000000},
    { 0.312500000,  0.375000000},
    { 0.375000000,  0.500000000},
    { 0.312500000,  0.500000000},
    { 0.375000000,  0.375000000},
    { 0.312500000,  0.500000000},
    { 0.312500000,  0.375000000},
    { 0.375000000,  0.375000000},
    { 0.437500000,  0.500000000},
    { 0.375000000,  0.500000000},
    { 0.437500000,  0.375000000},
    { 0.375000000,  0.500000000},
    { 0.375000000,  0.375000000},
    { 0.437500000,  0.375000000},
    { 0.500000000,  0.500000000},
    { 0.437500000,  0.500000000},
    { 0.500000000,  0.375000000},
    { 0.437500000,  0.500000000},
    { 0.437500000,  0.375000000},
    { 0.500000000,  0.375000000},
    { 0.562500000,  0.500000000},
    { 0.500000000,  0.500000000},
    { 0.562500000,  0.375000000},
    { 0.500000000,  0.500000000},
    { 0.500000000,  0.375000000},
    { 0.562500000,  0.375000000},
    { 0.625000000,  0.500000000},
    { 0.562500000,  0.500000000},
    { 0.625000000,  0.375000000},
    { 0.562500000,  0.500000000},
    { 0.562500000,  0.375000000},
    { 0.625000000,  0.375000000},
    { 0.687500000,  0.500000000},
    { 0.625000000,  0.500000000},
    { 0.687500000,  0.375000000},
    { 0.625000000,  0.500000000},
    { 0.625000000,  0.375000000},
    { 0.687500000,  0.375000000},
    { 0.750000000,  0.500000000},
    { 0.687500000,  0.500000000},
    { 0.750000000,  0.375000000},
    { 0.687500000,  0.500000000},
    { 0.687500000,  0.375000000},
    { 0.750000000,  0.375000000},
    { 0.812500000,  0.500000000},
    { 0.750000000,  0.500000000},
    { 0.812500000,  0.375000000},
    { 0.750000000,  0.500000000},
    { 0.750000000,  0.375000000},
    { 0.812500000,  0.375000000},
    { 0.875000000,  0.500000000},
    { 0.812500000,  0.500000000},
    { 0.875000000,  0.375000000},
    { 0.812500000,  0.500000000},
    { 0.812500000,  0.375000000},
    { 0.875000000,  0.375000000},
    { 0.937500000,  0.500000000},
    { 0.875000000,  0.500000000},
    { 0.937500000,  0.375000000},
    { 0.875000000,  0.500000000},
    { 0.875000000,  0.375000000},
    { 0.937500000,  0.375000000},
    { 1.000000000,  0.500000000},
    { 0.937500000,  0.500000000},
    { 1.000000000,  0.375000000},
    { 0.937500000,  0.500000000},
    { 0.937500000,  0.375000000},
    { 1.000000000,  0.375000000},
    { 0.062500000,  0.375000000},
    { 0.000000000,  0.375000000},
    { 0.062500000,  0.250000000},
    { 0.000000000,  0.375000000},
    { 0.000000000,  0.250000000},
    { 0.062500000,  0.250000000},
    { 0.125000000,  0.375000000},
    { 0.062500000,  0.375000000},
    { 0.125000000,  0.250000000},
    { 0.062500000,  0.375000000},
    { 0.062500000,  0.250000000},
    { 0.125000000,  0.250000000},
    { 0.187500000,  0.375000000},
    { 0.125000000,  0.375000000},
    { 0.187500000,  0.250000000},
    { 0.125000000,  0.375000000},
    { 0.125000000,  0.250000000},
    { 0.187500000,  0.250000000},
    { 0.250000000,  0.375000000},
    { 0.187500000,  0.375000000},
    { 0.250000000,  0.250000000},
    { 0.187500000,  0.375000000},
    { 0.187500000,  0.250000000},
    { 0.250000000,  0.250000000},
    { 0.312500000,  0.375000000},
    { 0.250000000,  0.375000000},
    { 0.312500000,  0.250000000},
    { 0.250000000,  0.375000000},
    { 0.250000000,  0.250000000},
    { 0.312500000,  0.250000000},
    { 0.375000000,  0.375000000},
    { 0.312500000,  0.375000000},
    { 0.375000000,  0.250000000},
    { 0.312500000,  0.375000000},
    { 0.312500000,  0.250000000},
    { 0.375000000,  0.250000000},
    { 0.437500000,  0.375000000},
    { 0.375000000,  0.375000000},
    { 0.437500000,  0.250000000},
    { 0.375000000,  0.375000000},
    { 0.375000000,  0.250000000},
    { 0.437500000,  0.250000000},
    { 0.500000000,  0.375000000},
    { 0.437500000,  0.375000000},
    { 0.500000000,  0.250000000},
    { 0.437500000,  0.375000000},
    { 0.437500000,  0.250000000},
    { 0.500000000,  0.250000000},
    { 0.562500000,  0.375000000},
    { 0.500000000,  0.375000000},
    { 0.562500000,  0.250000000},
    { 0.500000000,  0.375000000},
    { 0.500000000,  0.250000000},
    { 0.562500000,  0.250000000},
    { 0.625000000,  0.375000000},
    { 0.562500000,  0.375000000},
    { 0.625000000,  0.250000000},
    { 0.562500000,  0.375000000},
    { 0.562500000,  0.250000000},
    { 0.625000000,  0.250000000},
    { 0.687500000,  0.375000000},
    { 0.625000000,  0.375000000},
    { 0.687500000,  0.250000000},
    { 0.625000000,  0.375000000},
    { 0.625000000,  0.250000000},
    { 0.687500000,  0.250000000},
    { 0.750000000,  0.375000000},
    { 0.687500000,  0.375000000},
    { 0.750000000,  0.250000000},
    { 0.687500000,  0.375000000},
    { 0.687500000,  0.250000000},
    { 0.750000000,  0.250000000},
    { 0.812500000,  0.375000000},
    { 0.750000000,  0.375000000},
    { 0.812500000,  0.250000000},
    { 0.750000000,  0.375000000},
    { 0.750000000,  0.250000000},
    { 0.812500000,  0.250000000},
    { 0.875000000,  0.375000000},
    { 0.812500000,  0.375000000},
    { 0.875000000,  0.250000000},
    { 0.812500000,  0.375000000},
    { 0.812500000,  0.250000000},
    { 0.875000000,  0.250000000},
    { 0.937500000,  0.375000000},
    { 0.875000000,  0.375000000},
    { 0.937500000,  0.250000000},
    { 0.875000000,  0.375000000},
    { 0.875000000,  0.250000000},
    { 0.937500000,  0.250000000},
    { 1.000000000,  0.375000000},
    { 0.937500000,  0.375000000},
    { 1.000000000,  0.250000000},
    { 0.937500000,  0.375000000},
    { 0.937500000,  0.250000000},
    { 1.000000000,  0.250000000},
    { 0.062500000,  0.250000000},
    { 0.000000000,  0.250000000},
    { 0.062500000,  0.125000000},
    { 0.000000000,  0.250000000},
    { 0.000000000,  0.125000000},
    { 0.062500000,  0.125000000},
    { 0.125000000,  0.250000000},
    { 0.062500000,  0.250000000},
    { 0.125000000,  0.125000000},
    { 0.062500000,  0.250000000},
    { 0.062500000,  0.125000000},
    { 0.125000000,  0.125000000},
    { 0.187500000,  0.250000000},
    { 0.125000000,  0.250000000},
    { 0.187500000,  0.125000000},
    { 0.125000000,  0.250000000},
    { 0.125000000,  0.125000000},
    { 0.187500000,  0.125000000},
    { 0.250000000,  0.250000000},
    { 0.187500000,  0.250000000},
    { 0.250000000,  0.125000000},
    { 0.187500000,  0.250000000},
    { 0.187500000,  0.125000000},
    { 0.250000000,  0.125000000},
    { 0.312500000,  0.250000000},
    { 0.250000000,  0.250000000},
    { 0.312500000,  0.125000000},
    { 0.250000000,  0.250000000},
    { 0.250000000,  0.125000000},
    { 0.312500000,  0.125000000},
    { 0.375000000,  0.250000000},
    { 0.312500000,  0.250000000},
    { 0.375000000,  0.125000000},
    { 0.312500000,  0.250000000},
    { 0.312500000,  0.125000000},
    { 0.375000000,  0.125000000},
    { 0.437500000,  0.250000000},
    { 0.375000000,  0.250000000},
    { 0.437500000,  0.125000000},
    { 0.375000000,  0.250000000},
    { 0.375000000,  0.125000000},
    { 0.437500000,  0.125000000},
    { 0.500000000,  0.250000000},
    { 0.437500000,  0.250000000},
    { 0.500000000,  0.125000000},
    { 0.437500000,  0.250000000},
    { 0.437500000,  0.125000000},
    { 0.500000000,  0.125000000},
    { 0.562500000,  0.250000000},
    { 0.500000000,  0.250000000},
    { 0.562500000,  0.125000000},
    { 0.500000000,  0.250000000},
    { 0.500000000,  0.125000000},
    { 0.562500000,  0.125000000},
    { 0.625000000,  0.250000000},
    { 0.562500000,  0.250000000},
    { 0.625000000,  0.125000000},
    { 0.562500000,  0.250000000},
    { 0.562500000,  0.125000000},
    { 0.625000000,  0.125000000},
    { 0.687500000,  0.250000000},
    { 0.625000000,  0.250000000},
    { 0.687500000,  0.125000000},
    { 0.625000000,  0.250000000},
    { 0.625000000,  0.125000000},
    { 0.687500000,  0.125000000},
    { 0.750000000,  0.250000000},
    { 0.687500000,  0.250000000},
    { 0.750000000,  0.125000000},
    { 0.687500000,  0.250000000},
    { 0.687500000,  0.125000000},
    { 0.750000000,  0.125000000},
    { 0.812500000,  0.250000000},
    { 0.750000000,  0.250000000},
    { 0.812500000,  0.125000000},
    { 0.750000000,  0.250000000},
    { 0.750000000,  0.125000000},
    { 0.812500000,  0.125000000},
    { 0.875000000,  0.250000000},
    { 0.812500000,  0.250000000},
    { 0.875000000,  0.125000000},
    { 0.812500000,  0.250000000},
    { 0.812500000,  0.125000000},
    { 0.875000000,  0.125000000},
    { 0.937500000,  0.250000000},
    { 0.875000000,  0.250000000},
    { 0.937500000,  0.125000000},
    { 0.875000000,  0.250000000},
    { 0.875000000,  0.125000000},
    { 0.937500000,  0.125000000},
    { 1.000000000,  0.250000000},
    { 0.937500000,  0.250000000},
    { 1.000000000,  0.125000000},
    { 0.937500000,  0.250000000},
    { 0.937500000,  0.125000000},
    { 1.000000000,  0.125000000},
    { 0.062500000,  0.125000000},
    { 0.000000000,  0.125000000},
    { 0.031250000,  0.000000000},
    { 0.125000000,  0.125000000},
    { 0.062500000,  0.125000000},
    { 0.093750000,  0.000000000},
    { 0.187500000,  0.125000000},
    { 0.125000000,  0.125000000},
    { 0.156250000,  0.000000000},
    { 0.250000000,  0.125000000},
    { 0.187500000,  0.125000000},
    { 0.218750000,  0.000000000},
    { 0.312500000,  0.125000000},
    { 0.250000000,  0.125000000},
    { 0.281250000,  0.000000000},
    { 0.375000000,  0.125000000},
    { 0.312500000,  0.125000000},
    { 0.343750000,  0.000000000},
    { 0.437500000,  0.125000000},
    { 0.375000000,  0.125000000},
    { 0.406250000,  0.000000000},
    { 0.500000000,  0.125000000},
    { 0.437500000,  0.125000000},
    { 0.468750000,  0.000000000},
    { 0.562500000,  0.125000000},
    { 0.500000000,  0.125000000},
    { 0.531250000,  0.000000000},
    { 0.625000000,  0.125000000},
    { 0.562500000,  0.125000000},
    { 0.593750000,  0.000000000},
    { 0.687500000,  0.125000000},
    { 0.625000000,  0.125000000},
    { 0.656250000,  0.000000000},
    { 0.750000000,  0.125000000},
    { 0.687500000,  0.125000000},
    { 0.718750000,  0.000000000},
    { 0.812500000,  0.125000000},
    { 0.750000000,  0.125000000},
    { 0.781250000,  0.000000000},
    { 0.875000000,  0.125000000},
    { 0.812500000,  0.125000000},
    { 0.843750000,  0.000000000},
    { 0.937500000,  0.125000000},
    { 0.875000000,  0.125000000},
    { 0.906250000,  0.000000000},
    { 1.000000000,  0.125000000},
    { 0.937500000,  0.125000000},
    { 0.968750000,  0.000000000},
};

std::vector<Vec3> SphereMesh::normal = {
    { 0.000000000,  1.000000000,  0.000000000},
    {-0.382683426,  0.923879504,  0.000000000},
    {-0.353553385,  0.923879504,  0.146446615},
    { 0.000000000,  1.000000000,  0.000000000},
    {-0.353553385,  0.923879504,  0.146446615},
    {-0.270598054,  0.923879504,  0.270598054},
    { 0.000000000,  1.000000000,  0.000000000},
    {-0.270598054,  0.923879504,  0.270598054},
    {-0.146446615,  0.923879504,  0.353553385},
    { 0.000000000,  1.000000000,  0.000000000},
    {-0.146446615,  0.923879504,  0.353553385},
    {-0.000000000,  0.923879504,  0.382683426},
    { 0.000000000,  1.000000000,  0.000000000},
    {-0.000000000,  0.923879504,  0.382683426},
    { 0.146446615,  0.923879504,  0.353553385},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.146446615,  0.923879504,  0.353553385},
    { 0.270598054,  0.923879504,  0.270598054},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.270598054,  0.923879504,  0.270598054},
    { 0.353553385,  0.923879504,  0.146446615},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.353553385,  0.923879504,  0.146446615},
    { 0.382683426,  0.923879504,  0.000000000},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.382683426,  0.923879504,  0.000000000},
    { 0.353553385,  0.923879504, -0.146446615},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.353553385,  0.923879504, -0.146446615},
    { 0.270598054,  0.923879504, -0.270598054},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.270598054,  0.923879504, -0.270598054},
    { 0.146446615,  0.923879504, -0.353553385},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.146446615,  0.923879504, -0.353553385},
    { 0.000000000,  0.923879504, -0.382683426},
    { 0.000000000,  1.000000000,  0.000000000},
    { 0.000000000,  0.923879504, -0.382683426},
    {-0.146446615,  0.923879504, -0.353553385},
    { 0.000000000,  1.000000000,  0.000000000},
    {-0.146446615,  0.923879504, -0.353553385},
    {-0.270598054,  0.923879504, -0.270598054},
    { 0.000000000,  1.000000000,  0.000000000},
    {-0.270598054,  0.923879504, -0.270598054},
    {-0.353553385,  0.923879504, -0.146446615},
    { 0.000000000,  1.000000000,  0.000000000},
    {-0.353553385,  0.923879504, -0.146446615},
    {-0.382683426,  0.923879504, -0.000000000},
    {-0.353553385,  0.923879504,  0.146446615},
    {-0.382683426,  0.923879504,  0.000000000},
    {-0.653281510,  0.707106769,  0.270598054},
    {-0.382683426,  0.923879504,  0.000000000},
    {-0.707106769,  0.707106769,  0.000000000},
    {-0.653281510,  0.707106769,  0.270598054},
    {-0.270598054,  0.923879504,  0.270598054},
    {-0.353553385,  0.923879504,  0.146446615},
    {-0.500000000,  0.707106769,  0.500000000},
    {-0.353553385,  0.923879504,  0.146446615},
    {-0.653281510,  0.707106769,  0.270598054},
    {-0.500000000,  0.707106769,  0.500000000},
    {-0.146446615,  0.923879504,  0.353553385},
    {-0.270598054,  0.923879504,  0.270598054},
    {-0.270598054,  0.707106769,  0.653281510},
    {-0.270598054,  0.923879504,  0.270598054},
    {-0.500000000,  0.707106769,  0.500000000},
    {-0.270598054,  0.707106769,  0.653281510},
    {-0.000000000,  0.923879504,  0.382683426},
    {-0.146446615,  0.923879504,  0.353553385},
    {-0.000000000,  0.707106769,  0.707106769},
    {-0.146446615,  0.923879504,  0.353553385},
    {-0.270598054,  0.707106769,  0.653281510},
    {-0.000000000,  0.707106769,  0.707106769},
    { 0.146446615,  0.923879504,  0.353553385},
    {-0.000000000,  0.923879504,  0.382683426},
    { 0.270598054,  0.707106769,  0.653281510},
    {-0.000000000,  0.923879504,  0.382683426},
    {-0.000000000,  0.707106769,  0.707106769},
    { 0.270598054,  0.707106769,  0.653281510},
    { 0.270598054,  0.923879504,  0.270598054},
    { 0.146446615,  0.923879504,  0.353553385},
    { 0.500000000,  0.707106769,  0.500000000},
    { 0.146446615,  0.923879504,  0.353553385},
    { 0.270598054,  0.707106769,  0.653281510},
    { 0.500000000,  0.707106769,  0.500000000},
    { 0.353553385,  0.923879504,  0.146446615},
    { 0.270598054,  0.923879504,  0.270598054},
    { 0.653281510,  0.707106769,  0.270598054},
    { 0.270598054,  0.923879504,  0.270598054},
    { 0.500000000,  0.707106769,  0.500000000},
    { 0.653281510,  0.707106769,  0.270598054},
    { 0.382683426,  0.923879504,  0.000000000},
    { 0.353553385,  0.923879504,  0.146446615},
    { 0.707106769,  0.707106769,  0.000000000},
    { 0.353553385,  0.923879504,  0.146446615},
    { 0.653281510,  0.707106769,  0.270598054},
    { 0.707106769,  0.707106769,  0.000000000},
    { 0.353553385,  0.923879504, -0.146446615},
    { 0.382683426,  0.923879504,  0.000000000},
    { 0.653281510,  0.707106769, -0.270598054},
    { 0.382683426,  0.923879504,  0.000000000},
    { 0.707106769,  0.707106769,  0.000000000},
    { 0.653281510,  0.707106769, -0.270598054},
    { 0.270598054,  0.923879504, -0.270598054},
    { 0.353553385,  0.923879504, -0.146446615},
    { 0.500000000,  0.707106769, -0.500000000},
    { 0.353553385,  0.923879504, -0.146446615},
    { 0.653281510,  0.707106769, -0.270598054},
    { 0.500000000,  0.707106769, -0.500000000},
    { 0.146446615,  0.923879504, -0.353553385},
    { 0.270598054,  0.923879504, -0.270598054},
    { 0.270598054,  0.707106769, -0.653281510},
    { 0.270598054,  0.923879504, -0.270598054},
    { 0.500000000,  0.707106769, -0.500000000},
    { 0.270598054,  0.707106769, -0.653281510},
    { 0.000000000,  0.923879504, -0.382683426},
    { 0.146446615,  0.923879504, -0.353553385},
    { 0.000000000,  0.707106769, -0.707106769},
    { 0.146446615,  0.923879504, -0.353553385},
    { 0.270598054,  0.707106769, -0.653281510},
    { 0.000000000,  0.707106769, -0.707106769},
    {-0.146446615,  0.923879504, -0.353553385},
    { 0.000000000,  0.923879504, -0.382683426},
    {-0.270598054,  0.707106769, -0.653281510},
    { 0.000000000,  0.923879504, -0.382683426},
    { 0.000000000,  0.707106769, -0.707106769},
    {-0.270598054,  0.707106769, -0.653281510},
    {-0.270598054,  0.923879504, -0.270598054},
    {-0.146446615,  0.923879504, -0.353553385},
    {-0.500000000,  0.707106769, -0.500000000},
    {-0.146446615,  0.923879504, -0.353553385},
    {-0.270598054,  0.707106769, -0.653281510},
    {-0.500000000,  0.707106769, -0.500000000},
    {-0.353553385,  0.923879504, -0.146446615},
    {-0.270598054,  0.923879504, -0.270598054},
    {-0.653281510,  0.707106769, -0.270598054},
    {-0.270598054,  0.923879504, -0.270598054},
    {-0.500000000,  0.707106769, -0.500000000},
    {-0.653281510,  0.707106769, -0.270598054},
    {-0.382683426,  0.923879504, -0.000000000},
    {-0.353553385,  0.923879504, -0.146446615},
    {-0.707106769,  0.707106769, -0.000000000},
    {-0.353553385,  0.923879504, -0.146446615},
    {-0.653281510,  0.707106769, -0.270598054},
    {-0.707106769,  0.707106769, -0.000000000},
    {-0.653281510,  0.707106769,  0.270598054},
    {-0.707106769,  0.707106769,  0.000000000},
    {-0.853553414,  0.382683426,  0.353553385},
    {-0.707106769,  0.707106769,  0.000000000},
    {-0.923879504,  0.382683426,  0.000000000},
    {-0.853553414,  0.382683426,  0.353553385},
    {-0.500000000,  0.707106769,  0.500000000},
    {-0.653281510,  0.707106769,  0.270598054},
    {-0.653281510,  0.382683426,  0.653281510},
    {-0.653281510,  0.707106769,  0.270598054},
    {-0.853553414,  0.382683426,  0.353553385},
    {-0.653281510,  0.382683426,  0.653281510},
    {-0.270598054,  0.707106769,  0.653281510},
    {-0.500000000,  0.707106769,  0.500000000},
    {-0.353553385,  0.382683426,  0.853553414},
    {-0.500000000,  0.707106769,  0.500000000},
    {-0.653281510,  0.382683426,  0.653281510},
    {-0.353553385,  0.382683426,  0.853553414},
    {-0.000000000,  0.707106769,  0.707106769},
    {-0.270598054,  0.707106769,  0.653281510},
    {-0.000000000,  0.382683426,  0.923879504},
    {-0.270598054,  0.707106769,  0.653281510},
    {-0.353553385,  0.382683426,  0.853553414},
    {-0.000000000,  0.382683426,  0.923879504},
    { 0.270598054,  0.707106769,  0.653281510},
    {-0.000000000,  0.707106769,  0.707106769},
    { 0.353553385,  0.382683426,  0.853553414},
    {-0.000000000,  0.707106769,  0.707106769},
    {-0.000000000,  0.382683426,  0.923879504},
    { 0.353553385,  0.382683426,  0.853553414},
    { 0.500000000,  0.707106769,  0.500000000},
    { 0.270598054,  0.707106769,  0.653281510},
    { 0.653281510,  0.382683426,  0.653281510},
    { 0.270598054,  0.707106769,  0.653281510},
    { 0.353553385,  0.382683426,  0.853553414},
    { 0.653281510,  0.382683426,  0.653281510},
    { 0.653281510,  0.707106769,  0.270598054},
    { 0.500000000,  0.707106769,  0.500000000},
    { 0.853553414,  0.382683426,  0.353553385},
    { 0.500000000,  0.707106769,  0.500000000},
    { 0.653281510,  0.382683426,  0.653281510},
    { 0.853553414,  0.382683426,  0.353553385},
    { 0.707106769,  0.707106769,  0.000000000},
    { 0.653281510,  0.707106769,  0.270598054},
    { 0.923879504,  0.382683426,  0.000000000},
    { 0.653281510,  0.707106769,  0.270598054},
    { 0.853553414,  0.382683426,  0.353553385},
    { 0.923879504,  0.382683426,  0.000000000},
    { 0.653281510,  0.707106769, -0.270598054},
    { 0.707106769,  0.707106769,  0.000000000},
    { 0.853553414,  0.382683426, -0.353553385},
    { 0.707106769,  0.707106769,  0.000000000},
    { 0.923879504,  0.382683426,  0.000000000},
    { 0.853553414,  0.382683426, -0.353553385},
    { 0.500000000,  0.707106769, -0.500000000},
    { 0.653281510,  0.707106769, -0.270598054},
    { 0.653281510,  0.382683426, -0.653281510},
    { 0.653281510,  0.707106769, -0.270598054},
    { 0.853553414,  0.382683426, -0.353553385},
    { 0.653281510,  0.382683426, -0.653281510},
    { 0.270598054,  0.707106769, -0.653281510},
    { 0.500000000,  0.707106769, -0.500000000},
    { 0.353553385,  0.382683426, -0.853553414},
    { 0.500000000,  0.707106769, -0.500000000},
    { 0.653281510,  0.382683426, -0.653281510},
    { 0.353553385,  0.382683426, -0.853553414},
    { 0.000000000,  0.707106769, -0.707106769},
    { 0.270598054,  0.707106769, -0.653281510},
    { 0.000000000,  0.382683426, -0.923879504},
    { 0.270598054,  0.707106769, -0.653281510},
    { 0.353553385,  0.382683426, -0.853553414},
    { 0.000000000,  0.382683426, -0.923879504},
    {-0.270598054,  0.707106769, -0.653281510},
    { 0.000000000,  0.707106769, -0.707106769},
    {-0.353553385,  0.382683426, -0.853553414},
    { 0.000000000,  0.707106769, -0.707106769},
    { 0.000000000,  0.382683426, -0.923879504},
    {-0.353553385,  0.382683426, -0.853553414},
    {-0.500000000,  0.707106769, -0.500000000},
    {-0.270598054,  0.707106769, -0.653281510},
    {-0.653281510,  0.382683426, -0.653281510},
    {-0.270598054,  0.707106769, -0.653281510},
    {-0.353553385,  0.382683426, -0.853553414},
    {-0.653281510,  0.382683426, -0.653281510},
    {-0.653281510,  0.707106769, -0.270598054},
    {-0.500000000,  0.707106769, -0.500000000},
    {-0.853553414,  0.382683426, -0.353553385},
    {-0.500000000,  0.707106769, -0.500000000},
    {-0.653281510,  0.382683426, -0.653281510},
    {-0.853553414,  0.382683426, -0.353553385},
    {-0.707106769,  0.707106769, -0.000000000},
    {-0.653281510,  0.707106769, -0.270598054},
    {-0.923879504,  0.382683426, -0.000000000},
    {-0.653281510,  0.707106769, -0.270598054},
    {-0.853553414,  0.382683426, -0.353553385},
    {-0.923879504,  0.382683426, -0.000000000},
    {-0.853553414,  0.382683426,  0.353553385},
    {-0.923879504,  0.382683426,  0.000000000},
    {-0.923879504,  0.000000000,  0.382683426},
    {-0.923879504,  0.382683426,  0.000000000},
    {-1.000000000,  0.000000000,  0.000000000},
    {-0.923879504,  0.000000000,  0.382683426},
    {-0.653281510,  0.382683426,  0.653281510},
    {-0.853553414,  0.382683426,  0.353553385},
    {-0.707106769,  0.000000000,  0.707106769},
    {-0.853553414,  0.382683426,  0.353553385},
    {-0.923879504,  0.000000000,  0.382683426},
    {-0.707106769,  0.000000000,  0.707106769},
    {-0.353553385,  0.382683426,  0.853553414},
    {-0.653281510,  0.382683426,  0.653281510},
    {-0.382683426,  0.000000000,  0.923879504},
    {-0.653281510,  0.382683426,  0.653281510},
    {-0.707106769,  0.000000000,  0.707106769},
    {-0.382683426,  0.000000000,  0.923879504},
    {-0.000000000,  0.382683426,  0.923879504},
    {-0.353553385,  0.382683426,  0.853553414},
    {-0.000000000,  0.000000000,  1.000000000},
    {-0.353553385,  0.382683426,  0.853553414},
    {-0.382683426,  0.000000000,  0.923879504},
    {-0.000000000,  0.000000000,  1.000000000},
    { 0.353553385,  0.382683426,  0.853553414},
    {-0.000000000,  0.382683426,  0.923879504},
    { 0.382683426,  0.000000000,  0.923879504},
    {-0.000000000,  0.382683426,  0.923879504},
    {-0.000000000,  0.000000000,  1.000000000},
    { 0.382683426,  0.000000000,  0.923879504},
    { 0.653281510,  0.382683426,  0.653281510},
    { 0.353553385,  0.382683426,  0.853553414},
    { 0.707106769,  0.000000000,  0.707106769},
    { 0.353553385,  0.382683426,  0.853553414},
    { 0.382683426,  0.000000000,  0.923879504},
    { 0.707106769,  0.000000000,  0.707106769},
    { 0.853553414,  0.382683426,  0.353553385},
    { 0.653281510,  0.382683426,  0.653281510},
    { 0.923879504,  0.000000000,  0.382683426},
    { 0.653281510,  0.382683426,  0.653281510},
    { 0.707106769,  0.000000000,  0.707106769},
    { 0.923879504,  0.000000000,  0.382683426},
    { 0.923879504,  0.382683426,  0.000000000},
    { 0.853553414,  0.382683426,  0.353553385},
    { 1.000000000,  0.000000000,  0.000000000},
    { 0.853553414,  0.382683426,  0.353553385},
    { 0.923879504,  0.000000000,  0.382683426},
    { 1.000000000,  0.000000000,  0.000000000},
    { 0.853553414,  0.382683426, -0.353553385},
    { 0.923879504,  0.382683426,  0.000000000},
    { 0.923879504,  0.000000000, -0.382683426},
    { 0.923879504,  0.382683426,  0.000000000},
    { 1.000000000,  0.000000000,  0.000000000},
    { 0.923879504,  0.000000000, -0.382683426},
    { 0.653281510,  0.382683426, -0.653281510},
    { 0.853553414,  0.382683426, -0.353553385},
    { 0.707106769,  0.000000000, -0.707106769},
    { 0.853553414,  0.382683426, -0.353553385},
    { 0.923879504,  0.000000000, -0.382683426},
    { 0.707106769,  0.000000000, -0.707106769},
    { 0.353553385,  0.382683426, -0.853553414},
    { 0.653281510,  0.382683426, -0.653281510},
    { 0.382683426,  0.000000000, -0.923879504},
    { 0.653281510,  0.382683426, -0.653281510},
    { 0.707106769,  0.000000000, -0.707106769},
    { 0.382683426,  0.000000000, -0.923879504},
    { 0.000000000,  0.382683426, -0.923879504},
    { 0.353553385,  0.382683426, -0.853553414},
    { 0.000000000,  0.000000000, -1.000000000},
    { 0.353553385,  0.382683426, -0.853553414},
    { 0.382683426,  0.000000000, -0.923879504},
    { 0.000000000,  0.000000000, -1.000000000},
    {-0.353553385,  0.382683426, -0.853553414},
    { 0.000000000,  0.382683426, -0.923879504},
    {-0.382683426,  0.000000000, -0.923879504},
    { 0.000000000,  0.382683426, -0.923879504},
    { 0.000000000,  0.000000000, -1.000000000},
    {-0.382683426,  0.000000000, -0.923879504},
    {-0.653281510,  0.382683426, -0.653281510},
    {-0.353553385,  0.382683426, -0.853553414},
    {-0.707106769,  0.000000000, -0.707106769},
    {-0.353553385,  0.382683426, -0.853553414},
    {-0.382683426,  0.000000000, -0.923879504},
    {-0.707106769,  0.000000000, -0.707106769},
    {-0.853553414,  0.382683426, -0.353553385},
    {-0.653281510,  0.382683426, -0.653281510},
    {-0.923879504,  0.000000000, -0.382683426},
    {-0.653281510,  0.382683426, -0.653281510},
    {-0.707106769,  0.000000000, -0.707106769},
    {-0.923879504,  0.000000000, -0.382683426},
    {-0.923879504,  0.382683426, -0.000000000},
    {-0.853553414,  0.382683426, -0.353553385},
    {-1.000000000,  0.000000000, -0.000000000},
    {-0.853553414,  0.382683426, -0.353553385},
    {-0.923879504,  0.000000000, -0.382683426},
    {-1.000000000,  0.000000000, -0.000000000},
    {-0.923879504,  0.000000000,  0.382683426},
    {-1.000000000,  0.000000000,  0.000000000},
    {-0.853553414, -0.382683426,  0.353553385},
    {-1.000000000,  0.000000000,  0.000000000},
    {-0.923879504, -0.382683426,  0.000000000},
    {-0.853553414, -0.382683426,  0.353553385},
    {-0.707106769,  0.000000000,  0.707106769},
    {-0.923879504,  0.000000000,  0.382683426},
    {-0.653281510, -0.382683426,  0.653281510},
    {-0.923879504,  0.000000000,  0.382683426},
    {-0.853553414, -0.382683426,  0.353553385},
    {-0.653281510, -0.382683426,  0.653281510},
    {-0.382683426,  0.000000000,  0.923879504},
    {-0.707106769,  0.000000000,  0.707106769},
    {-0.353553385, -0.382683426,  0.853553414},
    {-0.707106769,  0.000000000,  0.707106769},
    {-0.653281510, -0.382683426,  0.653281510},
    {-0.353553385, -0.382683426,  0.853553414},
    {-0.000000000,  0.000000000,  1.000000000},
    {-0.382683426,  0.000000000,  0.923879504},
    {-0.000000000, -0.382683426,  0.923879504},
    {-0.382683426,  0.000000000,  0.923879504},
    {-0.353553385, -0.382683426,  0.853553414},
    {-0.000000000, -0.382683426,  0.923879504},
    { 0.382683426,  0.000000000,  0.923879504},
    {-0.000000000,  0.000000000,  1.000000000},
    { 0.353553385, -0.382683426,  0.853553414},
    {-0.000000000,  0.000000000,  1.000000000},
    {-0.000000000, -0.382683426,  0.923879504},
    { 0.353553385, -0.382683426,  0.853553414},
    { 0.707106769,  0.000000000,  0.707106769},
    { 0.382683426,  0.000000000,  0.923879504},
    { 0.653281510, -0.382683426,  0.653281510},
    { 0.382683426,  0.000000000,  0.923879504},
    { 0.353553385, -0.382683426,  0.853553414},
    { 0.653281510, -0.382683426,  0.653281510},
    { 0.923879504,  0.000000000,  0.382683426},
    { 0.707106769,  0.000000000,  0.707106769},
    { 0.853553414, -0.382683426,  0.353553385},
    { 0.707106769,  0.000000000,  0.707106769},
    { 0.653281510, -0.382683426,  0.653281510},
    { 0.853553414, -0.382683426,  0.353553385},
    { 1.000000000,  0.000000000,  0.000000000},
    { 0.923879504,  0.000000000,  0.382683426},
    { 0.923879504, -0.382683426,  0.000000000},
    { 0.923879504,  0.000000000,  0.382683426},
    { 0.853553414, -0.382683426,  0.353553385},
    { 0.923879504, -0.382683426,  0.000000000},
    { 0.923879504,  0.000000000, -0.382683426},
    { 1.000000000,  0.000000000,  0.000000000},
    { 0.853553414, -0.382683426, -0.353553385},
    { 1.000000000,  0.000000000,  0.000000000},
    { 0.923879504, -0.382683426,  0.000000000},
    { 0.853553414, -0.382683426, -0.353553385},
    { 0.707106769,  0.000000000, -0.707106769},
    { 0.923879504,  0.000000000, -0.382683426},
    { 0.653281510, -0.382683426, -0.653281510},
    { 0.923879504,  0.000000000, -0.382683426},
    { 0.853553414, -0.382683426, -0.353553385},
    { 0.653281510, -0.382683426, -0.653281510},
    { 0.382683426,  0.000000000, -0.923879504},
    { 0.707106769,  0.000000000, -0.707106769},
    { 0.353553385, -0.382683426, -0.853553414},
    { 0.707106769,  0.000000000, -0.707106769},
    { 0.653281510, -0.382683426, -0.653281510},
    { 0.353553385, -0.382683426, -0.853553414},
    { 0.000000000,  0.000000000, -1.000000000},
    { 0.382683426,  0.000000000, -0.923879504},
    { 0.000000000, -0.382683426, -0.923879504},
    { 0.382683426,  0.000000000, -0.923879504},
    { 0.353553385, -0.382683426, -0.853553414},
    { 0.000000000, -0.382683426, -0.923879504},
    {-0.382683426,  0.000000000, -0.923879504},
    { 0.000000000,  0.000000000, -1.000000000},
    {-0.353553385, -0.382683426, -0.853553414},
    { 0.000000000,  0.000000000, -1.000000000},
    { 0.000000000, -0.382683426, -0.923879504},
    {-0.353553385, -0.382683426, -0.853553414},
    {-0.707106769,  0.000000000, -0.707106769},
    {-0.382683426,  0.000000000, -0.923879504},
    {-0.653281510, -0.382683426, -0.653281510},
    {-0.382683426,  0.000000000, -0.923879504},
    {-0.353553385, -0.382683426, -0.853553414},
    {-0.653281510, -0.382683426, -0.653281510},
    {-0.923879504,  0.000000000, -0.382683426},
    {-0.707106769,  0.000000000, -0.707106769},
    {-0.853553414, -0.382683426, -0.353553385},
    {-0.707106769,  0.000000000, -0.707106769},
    {-0.653281510, -0.382683426, -0.653281510},
    {-0.853553414, -0.382683426, -0.353553385},
    {-1.000000000,  0.000000000, -0.000000000},
    {-0.923879504,  0.000000000, -0.382683426},
    {-0.923879504, -0.382683426, -0.000000000},
    {-0.923879504,  0.000000000, -0.382683426},
    {-0.853553414, -0.382683426, -0.353553385},
    {-0.923879504, -0.382683426, -0.000000000},
    {-0.853553414, -0.382683426,  0.353553385},
    {-0.923879504, -0.382683426,  0.000000000},
    {-0.653281510, -0.707106769,  0.270598054},
    {-0.923879504, -0.382683426,  0.000000000},
    {-0.707106769, -0.707106769,  0.000000000},
    {-0.653281510, -0.707106769,  0.270598054},
    {-0.653281510, -0.382683426,  0.653281510},
    {-0.853553414, -0.382683426,  0.353553385},
    {-0.500000000, -0.707106769,  0.500000000},
    {-0.853553414, -0.382683426,  0.353553385},
    {-0.653281510, -0.707106769,  0.270598054},
    {-0.500000000, -0.707106769,  0.500000000},
    {-0.353553385, -0.382683426,  0.853553414},
    {-0.653281510, -0.382683426,  0.653281510},
    {-0.270598054, -0.707106769,  0.653281510},
    {-0.653281510, -0.382683426,  0.653281510},
    {-0.500000000, -0.707106769,  0.500000000},
    {-0.270598054, -0.707106769,  0.653281510},
    {-0.000000000, -0.382683426,  0.923879504},
    {-0.353553385, -0.382683426,  0.853553414},
    {-0.000000000, -0.707106769,  0.707106769},
    {-0.353553385, -0.382683426,  0.853553414},
    {-0.270598054, -0.707106769,  0.653281510},
    {-0.000000000, -0.707106769,  0.707106769},
    { 0.353553385, -0.382683426,  0.853553414},
    {-0.000000000, -0.382683426,  0.923879504},
    { 0.270598054, -0.707106769,  0.653281510},
    {-0.000000000, -0.382683426,  0.923879504},
    {-0.000000000, -0.707106769,  0.707106769},
    { 0.270598054, -0.707106769,  0.653281510},
    { 0.653281510, -0.382683426,  0.653281510},
    { 0.353553385, -0.382683426,  0.853553414},
    { 0.500000000, -0.707106769,  0.500000000},
    { 0.353553385, -0.382683426,  0.853553414},
    { 0.270598054, -0.707106769,  0.653281510},
    { 0.500000000, -0.707106769,  0.500000000},
    { 0.853553414, -0.382683426,  0.353553385},
    { 0.653281510, -0.382683426,  0.653281510},
    { 0.653281510, -0.707106769,  0.270598054},
    { 0.653281510, -0.382683426,  0.653281510},
    { 0.500000000, -0.707106769,  0.500000000},
    { 0.653281510, -0.707106769,  0.270598054},
    { 0.923879504, -0.382683426,  0.000000000},
    { 0.853553414, -0.382683426,  0.353553385},
    { 0.707106769, -0.707106769,  0.000000000},
    { 0.853553414, -0.382683426,  0.353553385},
    { 0.653281510, -0.707106769,  0.270598054},
    { 0.707106769, -0.707106769,  0.000000000},
    { 0.853553414, -0.382683426, -0.353553385},
    { 0.923879504, -0.382683426,  0.000000000},
    { 0.653281510, -0.707106769, -0.270598054},
    { 0.923879504, -0.382683426,  0.000000000},
    { 0.707106769, -0.707106769,  0.000000000},
    { 0.653281510, -0.707106769, -0.270598054},
    { 0.653281510, -0.382683426, -0.653281510},
    { 0.853553414, -0.382683426, -0.353553385},
    { 0.500000000, -0.707106769, -0.500000000},
    { 0.853553414, -0.382683426, -0.353553385},
    { 0.653281510, -0.707106769, -0.270598054},
    { 0.500000000, -0.707106769, -0.500000000},
    { 0.353553385, -0.382683426, -0.853553414},
    { 0.653281510, -0.382683426, -0.653281510},
    { 0.270598054, -0.707106769, -0.653281510},
    { 0.653281510, -0.382683426, -0.653281510},
    { 0.500000000, -0.707106769, -0.500000000},
    { 0.270598054, -0.707106769, -0.653281510},
    { 0.000000000, -0.382683426, -0.923879504},
    { 0.353553385, -0.382683426, -0.853553414},
    { 0.000000000, -0.707106769, -0.707106769},
    { 0.353553385, -0.382683426, -0.853553414},
    { 0.270598054, -0.707106769, -0.653281510},
    { 0.000000000, -0.707106769, -0.707106769},
    {-0.353553385, -0.382683426, -0.853553414},
    { 0.000000000, -0.382683426, -0.923879504},
    {-0.270598054, -0.707106769, -0.653281510},
    { 0.000000000, -0.382683426, -0.923879504},
    { 0.000000000, -0.707106769, -0.707106769},
    {-0.270598054, -0.707106769, -0.653281510},
    {-0.653281510, -0.382683426, -0.653281510},
    {-0.353553385, -0.382683426, -0.853553414},
    {-0.500000000, -0.707106769, -0.500000000},
    {-0.353553385, -0.382683426, -0.853553414},
    {-0.270598054, -0.707106769, -0.653281510},
    {-0.500000000, -0.707106769, -0.500000000},
    {-0.853553414, -0.382683426, -0.353553385},
    {-0.653281510, -0.382683426, -0.653281510},
    {-0.653281510, -0.707106769, -0.270598054},
    {-0.653281510, -0.382683426, -0.653281510},
    {-0.500000000, -0.707106769, -0.500000000},
    {-0.653281510, -0.707106769, -0.270598054},
    {-0.923879504, -0.382683426, -0.000000000},
    {-0.853553414, -0.382683426, -0.353553385},
    {-0.707106769, -0.707106769, -0.000000000},
    {-0.853553414, -0.382683426, -0.353553385},
    {-0.653281510, -0.707106769, -0.270598054},
    {-0.707106769, -0.707106769, -0.000000000},
    {-0.653281510, -0.707106769,  0.270598054},
    {-0.707106769, -0.707106769,  0.000000000},
    {-0.353553385, -0.923879504,  0.146446615},
    {-0.707106769, -0.707106769,  0.000000000},
    {-0.382683426, -0.923879504,  0.000000000},
    {-0.353553385, -0.923879504,  0.146446615},
    {-0.500000000, -0.707106769,  0.500000000},
    {-0.653281510, -0.707106769,  0.270598054},
    {-0.270598054, -0.923879504,  0.270598054},
    {-0.653281510, -0.707106769,  0.270598054},
    {-0.353553385, -0.923879504,  0.146446615},
    {-0.270598054, -0.923879504,  0.270598054},
    {-0.270598054, -0.707106769,  0.653281510},
    {-0.500000000, -0.707106769,  0.500000000},
    {-0.146446615, -0.923879504,  0.353553385},
    {-0.500000000, -0.707106769,  0.500000000},
    {-0.270598054, -0.923879504,  0.270598054},
    {-0.146446615, -0.923879504,  0.353553385},
    {-0.000000000, -0.707106769,  0.707106769},
    {-0.270598054, -0.707106769,  0.653281510},
    {-0.000000000, -0.923879504,  0.382683426},
    {-0.270598054, -0.707106769,  0.653281510},
    {-0.146446615, -0.923879504,  0.353553385},
    {-0.000000000, -0.923879504,  0.382683426},
    { 0.270598054, -0.707106769,  0.653281510},
    {-0.000000000, -0.707106769,  0.707106769},
    { 0.146446615, -0.923879504,  0.353553385},
    {-0.000000000, -0.707106769,  0.707106769},
    {-0.000000000, -0.923879504,  0.382683426},
    { 0.146446615, -0.923879504,  0.353553385},
    { 0.500000000, -0.707106769,  0.500000000},
    { 0.270598054, -0.707106769,  0.653281510},
    { 0.270598054, -0.923879504,  0.270598054},
    { 0.270598054, -0.707106769,  0.653281510},
    { 0.146446615, -0.923879504,  0.353553385},
    { 0.270598054, -0.923879504,  0.270598054},
    { 0.653281510, -0.707106769,  0.270598054},
    { 0.500000000, -0.707106769,  0.500000000},
    { 0.353553385, -0.923879504,  0.146446615},
    { 0.500000000, -0.707106769,  0.500000000},
    { 0.270598054, -0.923879504,  0.270598054},
    { 0.353553385, -0.923879504,  0.146446615},
    { 0.707106769, -0.707106769,  0.000000000},
    { 0.653281510, -0.707106769,  0.270598054},
    { 0.382683426, -0.923879504,  0.000000000},
    { 0.653281510, -0.707106769,  0.270598054},
    { 0.353553385, -0.923879504,  0.146446615},
    { 0.382683426, -0.923879504,  0.000000000},
    { 0.653281510, -0.707106769, -0.270598054},
    { 0.707106769, -0.707106769,  0.000000000},
    { 0.353553385, -0.923879504, -0.146446615},
    { 0.707106769, -0.707106769,  0.000000000},
    { 0.382683426, -0.923879504,  0.000000000},
    { 0.353553385, -0.923879504, -0.146446615},
    { 0.500000000, -0.707106769, -0.500000000},
    { 0.653281510, -0.707106769, -0.270598054},
    { 0.270598054, -0.923879504, -0.270598054},
    { 0.653281510, -0.707106769, -0.270598054},
    { 0.353553385, -0.923879504, -0.146446615},
    { 0.270598054, -0.923879504, -0.270598054},
    { 0.270598054, -0.707106769, -0.653281510},
    { 0.500000000, -0.707106769, -0.500000000},
    { 0.146446615, -0.923879504, -0.353553385},
    { 0.500000000, -0.707106769, -0.500000000},
    { 0.270598054, -0.923879504, -0.270598054},
    { 0.146446615, -0.923879504, -0.353553385},
    { 0.000000000, -0.707106769, -0.707106769},
    { 0.270598054, -0.707106769, -0.653281510},
    { 0.000000000, -0.923879504, -0.382683426},
    { 0.270598054, -0.707106769, -0.653281510},
    { 0.146446615, -0.923879504, -0.353553385},
    { 0.000000000, -0.923879504, -0.382683426},
    {-0.270598054, -0.707106769, -0.653281510},
    { 0.000000000, -0.707106769, -0.707106769},
    {-0.146446615, -0.923879504, -0.353553385},
    { 0.000000000, -0.707106769, -0.707106769},
    { 0.000000000, -0.923879504, -0.382683426},
    {-0.146446615, -0.923879504, -0.353553385},
    {-0.500000000, -0.707106769, -0.500000000},
    {-0.270598054, -0.707106769, -0.653281510},
    {-0.270598054, -0.923879504, -0.270598054},
    {-0.270598054, -0.707106769, -0.653281510},
    {-0.146446615, -0.923879504, -0.353553385},
    {-0.270598054, -0.923879504, -0.270598054},
    {-0.653281510, -0.707106769, -0.270598054},
    {-0.500000000, -0.707106769, -0.500000000},
    {-0.353553385, -0.923879504, -0.146446615},
    {-0.500000000, -0.707106769, -0.500000000},
    {-0.270598054, -0.923879504, -0.270598054},
    {-0.353553385, -0.923879504, -0.146446615},
    {-0.707106769, -0.707106769, -0.000000000},
    {-0.653281510, -0.707106769, -0.270598054},
    {-0.382683426, -0.923879504, -0.000000000},
    {-0.653281510, -0.707106769, -0.270598054},
    {-0.353553385, -0.923879504, -0.146446615},
    {-0.382683426, -0.923879504, -0.000000000},
    {-0.353553385, -0.923879504,  0.146446615},
    {-0.382683426, -0.923879504,  0.000000000},
    {-0.000000000, -1.000000000,  0.000000000},
    {-0.270598054, -0.923879504,  0.270598054},
    {-0.353553385, -0.923879504,  0.146446615},
    {-0.000000000, -1.000000000,  0.000000000},
    {-0.146446615, -0.923879504,  0.353553385},
    {-0.270598054, -0.923879504,  0.270598054},
    {-0.000000000, -1.000000000,  0.000000000},
    {-0.000000000, -0.923879504,  0.382683426},
    {-0.146446615, -0.923879504,  0.353553385},
    {-0.000000000, -1.000000000,  0.000000000},
    { 0.146446615, -0.923879504,  0.353553385},
    {-0.000000000, -0.923879504,  0.382683426},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.270598054, -0.923879504,  0.270598054},
    { 0.146446615, -0.923879504,  0.353553385},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.353553385, -0.923879504,  0.146446615},
    { 0.270598054, -0.923879504,  0.270598054},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.382683426, -0.923879504,  0.000000000},
    { 0.353553385, -0.923879504,  0.146446615},
    { 0.000000000, -1.000000000,  0.000000000},
    { 0.353553385, -0.923879504, -0.146446615},
    { 0.382683426, -0.923879504,  0.000000000},
    { 0.000000000, -1.000000000, -0.000000000},
    { 0.270598054, -0.923879504, -0.270598054},
    { 0.353553385, -0.923879504, -0.146446615},
    { 0.000000000, -1.000000000, -0.000000000},
    { 0.146446615, -0.923879504, -0.353553385},
    { 0.270598054, -0.923879504, -0.270598054},
    { 0.000000000, -1.000000000, -0.000000000},
    { 0.000000000, -0.923879504, -0.382683426},
    { 0.146446615, -0.923879504, -0.353553385},
    { 0.000000000, -1.000000000, -0.000000000},
    {-0.146446615, -0.923879504, -0.353553385},
    { 0.000000000, -0.923879504, -0.382683426},
    {-0.000000000, -1.000000000, -0.000000000},
    {-0.270598054, -0.923879504, -0.270598054},
    {-0.146446615, -0.923879504, -0.353553385},
    {-0.000000000, -1.000000000, -0.000000000},
    {-0.353553385, -0.923879504, -0.146446615},
    {-0.270598054, -0.923879504, -0.270598054},
    {-0.000000000, -1.000000000, -0.000000000},
    {-0.382683426, -0.923879504, -0.000000000},
    {-0.353553385, -0.923879504, -0.146446615},
    {-0.000000000, -1.000000000, -0.000000000},
};

}

/* -------------------------------------------------------------------------- */
/* ---- ink/graphics/Gpu.cpp ------------------------------------------------ */
/* -------------------------------------------------------------------------- */

namespace ink::gpu {

constexpr uint32_t GL_COMPARISON_FUNCTIONS[] = {
    GL_NEVER,                                                 /**< FUNC_NEVER */
    GL_LESS,                                                  /**< FUNC_LESS */
    GL_EQUAL,                                                 /**< FUNC_EQUAL */
    GL_LEQUAL,                                                /**< FUNC_LEQUAL */
    GL_GREATER,                                               /**< FUNC_GREATER */
    GL_NOTEQUAL,                                              /**< FUNC_NOTEQUAL */
    GL_GEQUAL,                                                /**< FUNC_GEQUAL */
    GL_ALWAYS,                                                /**< FUNC_ALWAYS */
};

constexpr uint32_t GL_STENCIL_OPERATIONS[] = {
    GL_ZERO,                                                  /**< STENCIL_ZERO */
    GL_KEEP,                                                  /**< STENCIL_KEEP */
    GL_REPLACE,                                               /**< STENCIL_REPLACE */
    GL_INCR,                                                  /**< STENCIL_INCR */
    GL_DECR,                                                  /**< STENCIL_DECR */
    GL_INCR_WRAP,                                             /**< STENCIL_INCR_WRAP */
    GL_DECR_WRAP,                                             /**< STENCIL_DECR_WRAP */
    GL_INVERT,                                                /**< STENCIL_INVERT */
};

constexpr uint32_t GL_BLEND_OPERATIONS[] = {
    GL_FUNC_ADD,                                              /**< BLEND_ADD */
    GL_FUNC_SUBTRACT,                                         /**< BLEND_SUBTRACT */
    GL_FUNC_REVERSE_SUBTRACT,                                 /**< BLEND_REVERSE_SUBTRACT */
    GL_MIN,                                                   /**< BLEND_MIN */
    GL_MAX,                                                   /**< BLEND_MAX */
};

constexpr uint32_t GL_BLEND_FACTORS[] = {
    GL_ZERO,                                                  /**< FACTOR_ZERO */
    GL_ONE,                                                   /**< FACTOR_ONE */
    GL_SRC_COLOR,                                             /**< FACTOR_SRC_COLOR */
    GL_ONE_MINUS_SRC_COLOR,                                   /**< FACTOR_ONE_MINUS_SRC_COLOR */
    GL_DST_COLOR,                                             /**< FACTOR_DST_COLOR */
    GL_ONE_MINUS_DST_COLOR,                                   /**< FACTOR_ONE_MINUS_DST_COLOR */
    GL_SRC_ALPHA,                                             /**< FACTOR_SRC_ALPHA */
    GL_ONE_MINUS_SRC_ALPHA,                                   /**< FACTOR_ONE_MINUS_SRC_ALPHA */
    GL_DST_ALPHA,                                             /**< FACTOR_DST_ALPHA */
    GL_ONE_MINUS_DST_ALPHA,                                   /**< FACTOR_ONE_MINUS_DST_ALPHA */
};

constexpr uint32_t GL_RENDER_SIDES[] = {
    GL_FRONT,                                                 /**< FRONT_SIDE */
    GL_BACK,                                                  /**< BACK_SIDE */
    GL_FRONT_AND_BACK,                                        /**< DOUBLE_SIDE */
};

constexpr uint32_t GL_IMAGE_TYPES[] = {
    GL_UNSIGNED_BYTE,                                         /**< IMAGE_UBYTE */
    GL_BYTE,                                                  /**< IMAGE_BYTE */
    GL_UNSIGNED_SHORT,                                        /**< IMAGE_USHORT */
    GL_SHORT,                                                 /**< IMAGE_SHORT */
    GL_UNSIGNED_INT,                                          /**< IMAGE_UINT */
    GL_INT,                                                   /**< IMAGE_INT */
    GL_HALF_FLOAT,                                            /**< IMAGE_HALF_FLOAT */
    GL_FLOAT,                                                 /**< IMAGE_FLOAT */
    GL_UNSIGNED_INT_24_8,                                     /**< IMAGE_UINT_24_8 */
};

constexpr uint32_t GL_IMAGE_COLORS[] = {
    GL_RED,                                                   /**< CHANNEL_NUMBER_1 */
    GL_RG,                                                    /**< CHANNEL_NUMBER_2 */
    GL_RGB,                                                   /**< CHANNEL_NUMBER_3 */
    GL_RGBA,                                                  /**< CHANNEL_NUMBER_4 */
};

constexpr uint32_t GL_IMAGE_COLOR_INTEGERS[] = {
    GL_RED_INTEGER,                                           /**< CHANNEL_NUMBER_1 */
    GL_RG_INTEGER,                                            /**< CHANNEL_NUMBER_2 */
    GL_RGB_INTEGER,                                           /**< CHANNEL_NUMBER_3 */
    GL_RGBA_INTEGER,                                          /**< CHANNEL_NUMBER_4 */
};

constexpr uint32_t GL_IMAGE_FORMATS[] = {
    GL_RGBA,                                                  /**< IMAGE_COLOR */
    GL_RGBA_INTEGER,                                          /**< IMAGE_COLOR_INTEGER */
    GL_DEPTH_COMPONENT,                                       /**< IMAGE_DEPTH */
    GL_STENCIL_INDEX,                                         /**< IMAGE_STENCIL */
    GL_DEPTH_STENCIL,                                         /**< IMAGE_DEPTH_STENCIL */
};

constexpr uint32_t GL_TEXTURE_TYPES[] = {
    GL_TEXTURE_1D,                                            /**< TEXTURE_1D */
    GL_TEXTURE_2D,                                            /**< TEXTURE_2D */
    GL_TEXTURE_3D,                                            /**< TEXTURE_3D */
    GL_TEXTURE_CUBE_MAP,                                      /**< TEXTURE_CUBE */
    GL_TEXTURE_1D_ARRAY,                                      /**< TEXTURE_1D_ARRAY */
    GL_TEXTURE_2D_ARRAY,                                      /**< TEXTURE_2D_ARRAY */
    GL_TEXTURE_CUBE_MAP_ARRAY,                                /**< TEXTURE_CUBE_ARRAY */
};

constexpr int32_t GL_TEXTURE_SIZED_INTERNAL_FORMATS[] = {
    GL_R8,                                                    /**< TEXTURE_R8_UNORM */
    GL_R8_SNORM,                                              /**< TEXTURE_R8_SNORM */
    GL_R16,                                                   /**< TEXTURE_R16_UNORM */
    GL_R16_SNORM,                                             /**< TEXTURE_R16_SNORM */
    GL_RG8,                                                   /**< TEXTURE_R8G8_UNORM */
    GL_RG8_SNORM,                                             /**< TEXTURE_R8G8_SNORM */
    GL_RG16,                                                  /**< TEXTURE_R16G16_UNORM */
    GL_RG16_SNORM,                                            /**< TEXTURE_R16G16_SNORM */
    GL_R3_G3_B2,                                              /**< TEXTURE_R3G3B2_UNORM */
    GL_RGB4,                                                  /**< TEXTURE_R4G4B4_UNORM */
    GL_RGB5,                                                  /**< TEXTURE_R5G5B5_UNORM */
    GL_RGB8,                                                  /**< TEXTURE_R8G8B8_UNORM */
    GL_RGB8_SNORM,                                            /**< TEXTURE_R8G8B8_SNORM */
    GL_RGB10,                                                 /**< TEXTURE_R10G10B10_UNORM */
    GL_RGB12,                                                 /**< TEXTURE_R12G12B12_UNORM */
    GL_RGB16_SNORM,                                           /**< TEXTURE_R16G16B16_UNORM */
    GL_RGBA2,                                                 /**< TEXTURE_R2G2B2A2_UNORM */
    GL_RGBA4,                                                 /**< TEXTURE_R4G4B4A4_UNORM */
    GL_RGB5_A1,                                               /**< TEXTURE_R5G5B5A1_UNORM */
    GL_RGBA8,                                                 /**< TEXTURE_R8G8B8A8_UNORM */
    GL_RGBA8_SNORM,                                           /**< TEXTURE_R8G8B8A8_SNORM */
    GL_RGB10_A2,                                              /**< TEXTURE_R10G10B10A2_UNORM */
    GL_RGB10_A2UI,                                            /**< TEXTURE_R10G10B10A2_UINT */
    GL_RGBA12,                                                /**< TEXTURE_R12G12B12A12_UNORM */
    GL_RGBA16,                                                /**< TEXTURE_R16G16B16A16_UNORM */
    GL_SRGB8,                                                 /**< TEXTURE_R8G8B8_SRGB */
    GL_SRGB8_ALPHA8,                                          /**< TEXTURE_R8G8B8A8_SRGB */
    GL_R16F,                                                  /**< TEXTURE_R16_SFLOAT */
    GL_RG16F,                                                 /**< TEXTURE_R16G16_SFLOAT */
    GL_RGB16F,                                                /**< TEXTURE_R16G16B16_SFLOAT */
    GL_RGBA16F,                                               /**< TEXTURE_R16G16B16A16_SFLOAT */
    GL_R32F,                                                  /**< TEXTURE_R32_SFLOAT */
    GL_RG32F,                                                 /**< TEXTURE_R32G32_SFLOAT */
    GL_RGB32F,                                                /**< TEXTURE_R32G32B32_SFLOAT */
    GL_RGBA32F,                                               /**< TEXTURE_R32G32B32A32_SFLOAT */
    GL_R11F_G11F_B10F,                                        /**< TEXTURE_R11G11B10_SFLOAT */
    GL_RGB9_E5,                                               /**< TEXTURE_R9G9B9E5_UNORM */
    GL_R8I,                                                   /**< TEXTURE_R8_SINT */
    GL_R8UI,                                                  /**< TEXTURE_R8_UINT */
    GL_R16I,                                                  /**< TEXTURE_R16_SINT */
    GL_R16UI,                                                 /**< TEXTURE_R16_UINT */
    GL_R32I,                                                  /**< TEXTURE_R32_SINT */
    GL_R32UI,                                                 /**< TEXTURE_R32_UINT */
    GL_RG8I,                                                  /**< TEXTURE_R8G8_SINT */
    GL_RG8UI,                                                 /**< TEXTURE_R8G8_UINT */
    GL_RG16I,                                                 /**< TEXTURE_R16G16_SINT */
    GL_RG16UI,                                                /**< TEXTURE_R16G16_UINT */
    GL_RG32I,                                                 /**< TEXTURE_R32G32_SINT */
    GL_RG32UI,                                                /**< TEXTURE_R32G32_UINT */
    GL_RGB8I,                                                 /**< TEXTURE_R8G8B8_SINT */
    GL_RGB8UI,                                                /**< TEXTURE_R8G8B8_UINT */
    GL_RGB16I,                                                /**< TEXTURE_R16G16B16_SINT */
    GL_RGB16UI,                                               /**< TEXTURE_R16G16B16_UINT */
    GL_RGB32I,                                                /**< TEXTURE_R32G32B32_SINT */
    GL_RGB32UI,                                               /**< TEXTURE_R32G32B32_UINT */
    GL_RGBA8I,                                                /**< TEXTURE_R8G8B8A8_SINT */
    GL_RGBA8UI,                                               /**< TEXTURE_R8G8B8A8_UINT */
    GL_RGBA16I,                                               /**< TEXTURE_R16G16B16A16_SINT */
    GL_RGBA16UI,                                              /**< TEXTURE_R16G16B16A16_UINT */
    GL_RGBA32I,                                               /**< TEXTURE_R32G32B32A32_SINT */
    GL_RGBA32UI,                                              /**< TEXTURE_R32G32B32A32_UINT */
    GL_DEPTH_COMPONENT16,                                     /**< TEXTURE_D16_UNORM */
    GL_DEPTH_COMPONENT24,                                     /**< TEXTURE_D24_UNORM */
    GL_DEPTH_COMPONENT32F,                                    /**< TEXTURE_D32_SFLOAT */
    GL_DEPTH24_STENCIL8,                                      /**< TEXTURE_D24_UNORM_S8_UINT */
    GL_DEPTH32F_STENCIL8,                                     /**< TEXTURE_D32_SFLOAT_S8_UINT */
};

constexpr uint32_t GL_TEXTURE_BASE_INTERNAL_FORMATS[] = {
    GL_RED,                                                   /**< TEXTURE_R8_UNORM */
    GL_RED,                                                   /**< TEXTURE_R8_SNORM */
    GL_RED,                                                   /**< TEXTURE_R16_UNORM */
    GL_RED,                                                   /**< TEXTURE_R16_SNORM */
    GL_RG,                                                    /**< TEXTURE_R8G8_UNORM */
    GL_RG,                                                    /**< TEXTURE_R8G8_SNORM */
    GL_RG,                                                    /**< TEXTURE_R16G16_UNORM */
    GL_RG,                                                    /**< TEXTURE_R16G16_SNORM */
    GL_RGB,                                                   /**< TEXTURE_R3G3B2_UNORM */
    GL_RGB,                                                   /**< TEXTURE_R4G4B4_UNORM */
    GL_RGB,                                                   /**< TEXTURE_R5G5B5_UNORM */
    GL_RGB,                                                   /**< TEXTURE_R8G8B8_UNORM */
    GL_RGB,                                                   /**< TEXTURE_R8G8B8_SNORM */
    GL_RGB,                                                   /**< TEXTURE_R10G10B10_UNORM */
    GL_RGB,                                                   /**< TEXTURE_R12G12B12_UNORM */
    GL_RGB,                                                   /**< TEXTURE_R16G16B16_UNORM */
    GL_RGB,                                                   /**< TEXTURE_R2G2B2A2_UNORM */
    GL_RGB,                                                   /**< TEXTURE_R4G4B4A4_UNORM */
    GL_RGBA,                                                  /**< TEXTURE_R5G5B5A1_UNORM */
    GL_RGBA,                                                  /**< TEXTURE_R8G8B8A8_UNORM */
    GL_RGBA,                                                  /**< TEXTURE_R8G8B8A8_SNORM */
    GL_RGBA,                                                  /**< TEXTURE_R10G10B10A2_UNORM */
    GL_RGBA,                                                  /**< TEXTURE_R10G10B10A2_UINT */
    GL_RGBA,                                                  /**< TEXTURE_R12G12B12A12_UNORM */
    GL_RGBA,                                                  /**< TEXTURE_R16G16B16A16_UNORM */
    GL_RGB,                                                   /**< TEXTURE_R8G8B8_SRGB */
    GL_RGBA,                                                  /**< TEXTURE_R8G8B8A8_SRGB */
    GL_RED,                                                   /**< TEXTURE_R16_SFLOAT */
    GL_RG,                                                    /**< TEXTURE_R16G16_SFLOAT */
    GL_RGB,                                                   /**< TEXTURE_R16G16B16_SFLOAT */
    GL_RGBA,                                                  /**< TEXTURE_R16G16B16A16_SFLOAT */
    GL_RED,                                                   /**< TEXTURE_R32_SFLOAT */
    GL_RG,                                                    /**< TEXTURE_R32G32_SFLOAT */
    GL_RGB,                                                   /**< TEXTURE_R32G32B32_SFLOAT */
    GL_RGBA,                                                  /**< TEXTURE_R32G32B32A32_SFLOAT */
    GL_RGB,                                                   /**< TEXTURE_R11G11B10_SFLOAT */
    GL_RGB,                                                   /**< TEXTURE_R9G9B9E5_UNORM */
    GL_RED,                                                   /**< TEXTURE_R8_SINT */
    GL_RED,                                                   /**< TEXTURE_R8_UINT */
    GL_RED,                                                   /**< TEXTURE_R16_SINT */
    GL_RED,                                                   /**< TEXTURE_R16_UINT */
    GL_RED,                                                   /**< TEXTURE_R32_SINT */
    GL_RED,                                                   /**< TEXTURE_R32_UINT */
    GL_RG,                                                    /**< TEXTURE_R8G8_SINT */
    GL_RG,                                                    /**< TEXTURE_R8G8_UINT */
    GL_RG,                                                    /**< TEXTURE_R16G16_SINT */
    GL_RG,                                                    /**< TEXTURE_R16G16_UINT */
    GL_RG,                                                    /**< TEXTURE_R32G32_SINT */
    GL_RG,                                                    /**< TEXTURE_R32G32_UINT */
    GL_RGB,                                                   /**< TEXTURE_R8G8B8_SINT */
    GL_RGB,                                                   /**< TEXTURE_R8G8B8_UINT */
    GL_RGB,                                                   /**< TEXTURE_R16G16B16_SINT */
    GL_RGB,                                                   /**< TEXTURE_R16G16B16_UINT */
    GL_RGB,                                                   /**< TEXTURE_R32G32B32_SINT */
    GL_RGB,                                                   /**< TEXTURE_R32G32B32_UINT */
    GL_RGBA,                                                  /**< TEXTURE_R8G8B8A8_SINT */
    GL_RGBA,                                                  /**< TEXTURE_R8G8B8A8_UINT */
    GL_RGBA,                                                  /**< TEXTURE_R16G16B16A16_SINT */
    GL_RGBA,                                                  /**< TEXTURE_R16G16B16A16_UINT */
    GL_RGBA,                                                  /**< TEXTURE_R32G32B32A32_SINT */
    GL_RGBA,                                                  /**< TEXTURE_R32G32B32A32_UINT */
    GL_DEPTH_COMPONENT,                                       /**< TEXTURE_D16_UNORM */
    GL_DEPTH_COMPONENT,                                       /**< TEXTURE_D24_UNORM */
    GL_DEPTH_COMPONENT,                                       /**< TEXTURE_D32_SFLOAT */
    GL_DEPTH_STENCIL,                                         /**< TEXTURE_D24_UNORM_S8_UINT */
    GL_DEPTH_STENCIL,                                         /**< TEXTURE_D32_SFLOAT_S8_UINT */
};

constexpr int32_t GL_TEXTURE_WRAPPINGS[] = {
    GL_REPEAT,                                                /**< TEXTURE_REPEAT */
    GL_MIRRORED_REPEAT,                                       /**< TEXTURE_MIRRORED_REPEAT */
    GL_CLAMP_TO_EDGE,                                         /**< TEXTURE_CLAMP_TO_EDGE */
    GL_CLAMP_TO_BORDER,                                       /**< TEXTURE_CLAMP_TO_BORDER */
};

constexpr int32_t GL_TEXTURE_FILTERS[] = {
    GL_NEAREST,                                               /**< TEXTURE_NEAREST */
    GL_LINEAR,                                                /**< TEXTURE_LINEAR */
    GL_NEAREST_MIPMAP_NEAREST,                                /**< TEXTURE_NEAREST_MIPMAP_NEAREST */
    GL_LINEAR_MIPMAP_NEAREST,                                 /**< TEXTURE_LINEAR_MIPMAP_NEAREST */
    GL_NEAREST_MIPMAP_LINEAR,                                 /**< TEXTURE_NEAREST_MIPMAP_LINEAR */
    GL_LINEAR_MIPMAP_LINEAR,                                  /**< TEXTURE_LINEAR_MIPMAP_LINEAR */
};

constexpr uint32_t GL_COLOR_ATTACHMENTS[] = {
    GL_COLOR_ATTACHMENT0,                                     /**< TARGET_NUMBER_1 */
    GL_COLOR_ATTACHMENT1,                                     /**< TARGET_NUMBER_2 */
    GL_COLOR_ATTACHMENT2,                                     /**< TARGET_NUMBER_3 */
    GL_COLOR_ATTACHMENT3,                                     /**< TARGET_NUMBER_4 */
    GL_COLOR_ATTACHMENT4,                                     /**< TARGET_NUMBER_5 */
    GL_COLOR_ATTACHMENT5,                                     /**< TARGET_NUMBER_6 */
    GL_COLOR_ATTACHMENT6,                                     /**< TARGET_NUMBER_7 */
    GL_COLOR_ATTACHMENT7,                                     /**< TARGET_NUMBER_8 */
    GL_COLOR_ATTACHMENT8,                                     /**< TARGET_NUMBER_9 */
    GL_COLOR_ATTACHMENT9,                                     /**< TARGET_NUMBER_10 */
    GL_COLOR_ATTACHMENT10,                                    /**< TARGET_NUMBER_11 */
    GL_COLOR_ATTACHMENT11,                                    /**< TARGET_NUMBER_12 */
    GL_COLOR_ATTACHMENT12,                                    /**< TARGET_NUMBER_13 */
    GL_COLOR_ATTACHMENT13,                                    /**< TARGET_NUMBER_14 */
    GL_COLOR_ATTACHMENT14,                                    /**< TARGET_NUMBER_15 */
    GL_COLOR_ATTACHMENT15,                                    /**< TARGET_NUMBER_16 */
    GL_COLOR_ATTACHMENT16,                                    /**< TARGET_NUMBER_17 */
    GL_COLOR_ATTACHMENT17,                                    /**< TARGET_NUMBER_18 */
    GL_COLOR_ATTACHMENT18,                                    /**< TARGET_NUMBER_19 */
    GL_COLOR_ATTACHMENT19,                                    /**< TARGET_NUMBER_20 */
    GL_COLOR_ATTACHMENT20,                                    /**< TARGET_NUMBER_21 */
    GL_COLOR_ATTACHMENT21,                                    /**< TARGET_NUMBER_22 */
    GL_COLOR_ATTACHMENT22,                                    /**< TARGET_NUMBER_23 */
    GL_COLOR_ATTACHMENT23,                                    /**< TARGET_NUMBER_24 */
    GL_COLOR_ATTACHMENT24,                                    /**< TARGET_NUMBER_25 */
    GL_COLOR_ATTACHMENT25,                                    /**< TARGET_NUMBER_26 */
    GL_COLOR_ATTACHMENT26,                                    /**< TARGET_NUMBER_27 */
    GL_COLOR_ATTACHMENT27,                                    /**< TARGET_NUMBER_28 */
    GL_COLOR_ATTACHMENT28,                                    /**< TARGET_NUMBER_29 */
    GL_COLOR_ATTACHMENT29,                                    /**< TARGET_NUMBER_30 */
    GL_COLOR_ATTACHMENT30,                                    /**< TARGET_NUMBER_31 */
    GL_COLOR_ATTACHMENT31,                                    /**< TARGET_NUMBER_32 */
};

ComparisonFunc get_comparison_function(uint32_t v) {
    if (v == GL_NEVER   ) return FUNC_NEVER;
    if (v == GL_LESS    ) return FUNC_LESS;
    if (v == GL_EQUAL   ) return FUNC_EQUAL;
    if (v == GL_LEQUAL  ) return FUNC_LEQUAL;
    if (v == GL_GREATER ) return FUNC_GREATER;
    if (v == GL_NOTEQUAL) return FUNC_NOTEQUAL;
    if (v == GL_GEQUAL  ) return FUNC_GEQUAL;
    /*   ... GL_ALWAYS */ return FUNC_ALWAYS;
}

StencilOperation get_stencil_operation(uint32_t v) {
    if (v == GL_ZERO     ) return STENCIL_ZERO;
    if (v == GL_KEEP     ) return STENCIL_KEEP;
    if (v == GL_REPLACE  ) return STENCIL_REPLACE;
    if (v == GL_INCR     ) return STENCIL_INCR;
    if (v == GL_DECR     ) return STENCIL_DECR;
    if (v == GL_INCR_WRAP) return STENCIL_INCR_WRAP;
    if (v == GL_DECR_WRAP) return STENCIL_DECR_WRAP;
    /*   ... GL_INVERT  */ return STENCIL_INVERT;
}

BlendOperation get_blend_operation(uint32_t v) {
    if (v == GL_FUNC_ADD             ) return BLEND_ADD;
    if (v == GL_FUNC_SUBTRACT        ) return BLEND_SUBTRACT;
    if (v == GL_FUNC_REVERSE_SUBTRACT) return BLEND_REVERSE_SUBTRACT;
    if (v == GL_MIN                  ) return BLEND_MIN;
    /*   ... GL_MAX                 */ return BLEND_MAX;
}

BlendFactor get_blend_factor(uint32_t v) {
    if (v == GL_ZERO               ) return FACTOR_ZERO;
    if (v == GL_ONE                ) return FACTOR_ONE;
    if (v == GL_SRC_COLOR          ) return FACTOR_SRC_COLOR;
    if (v == GL_ONE_MINUS_SRC_COLOR) return FACTOR_ONE_MINUS_SRC_COLOR;
    if (v == GL_DST_COLOR          ) return FACTOR_DST_COLOR;
    if (v == GL_ONE_MINUS_DST_COLOR) return FACTOR_ONE_MINUS_DST_COLOR;
    if (v == GL_SRC_ALPHA          ) return FACTOR_SRC_ALPHA;
    if (v == GL_ONE_MINUS_SRC_ALPHA) return FACTOR_ONE_MINUS_SRC_ALPHA;
    if (v == GL_DST_ALPHA          ) return FACTOR_DST_ALPHA;
    /*   ... GL_ONE_MINUS_DST_ALPHA*/return FACTOR_ONE_MINUS_DST_ALPHA;
}

RenderSide get_render_side(uint32_t v) {
    if (v == GL_FRONT         ) return FRONT_SIDE;
    if (v == GL_BACK          ) return BACK_SIDE;
    /*   ... GL_FRONT_AND_BACK*/return DOUBLE_SIDE;
}

Rect::Rect(int w, int h) : width(w), height(h) {}

Rect::Rect(int x, int y, int w, int h) : x(x), y(y), width(w), height(h) {}

std::string State::get_device_info() {
    std::string info = "Vendor: ";
    const uint8_t* vendor = glGetString(GL_VENDOR);
    info += reinterpret_cast<const char*>(vendor);
    info += "\nRenderer: ";
    const uint8_t* renderer = glGetString(GL_RENDERER);
    info += reinterpret_cast<const char*>(renderer);
    info += "\nVersion: ";
    const uint8_t* version = glGetString(GL_VERSION);
    info += reinterpret_cast<const char*>(version);
    info += "\nGLSL Version: ";
    const uint8_t* glsl = glGetString(GL_SHADING_LANGUAGE_VERSION);
    info += reinterpret_cast<const char*>(glsl);
    return info + "\n";
}

void State::finish() {
    glFinish();
}

void State::flush() {
    glFlush();
}

std::string State::get_error() {
    std::string info;
    uint32_t error = glGetError();
    while (error != GL_NO_ERROR) {
        info += "OpenGL Error: " + std::to_string(error) + ": ";
        if (error == GL_INVALID_ENUM) {
            info += "An unacceptable value is specified for an enumerated argument.\n";
        } else if (error == GL_INVALID_VALUE) {
            info += "A numeric argument is out of range.\n";
        } else if (error == GL_INVALID_OPERATION) {
            info += "The specified operation is not allowed in the current state.\n";
        } else if (error == GL_OUT_OF_MEMORY) {
            info += "There is not enough memory left to execute the command.\n";
        } else {
            info += "An unknown error has occurred.\n";
        }
        error = glGetError();
    }
    return info;
}

void State::clear(bool c, bool d, bool s) {
    GLbitfield flag = 0;
    flag |= GL_COLOR_BUFFER_BIT * c;
    flag |= GL_DEPTH_BUFFER_BIT * d;
    flag |= GL_STENCIL_BUFFER_BIT * s;
    glClear(flag);
}

Vec4 State::get_clear_color() {
    Vec4 clear_color;
    glGetFloatv(GL_COLOR_CLEAR_VALUE, &clear_color.x);
    return clear_color;
}

void State::set_clear_color(const Vec3& c, float a) {
    glClearColor(c.x, c.y, c.z, a);
}

void State::set_clear_color(const Vec4& c) {
    glClearColor(c.x, c.y, c.z, c.w);
}

Vec4 State::get_color_writemask() {
    int color_writemasks[4];
    glGetIntegerv(GL_COLOR_WRITEMASK, &color_writemasks[0]);
    return Vec4(color_writemasks[0], color_writemasks[1],
                color_writemasks[2], color_writemasks[3]);
}

void State::set_color_writemask(bool r, bool g, bool b, bool a) {
    glColorMask(r, g, b, a);
}

void State::enable_depth_test() {
    glEnable(GL_DEPTH_TEST);
}

void State::disable_depth_test() {
    glDisable(GL_DEPTH_TEST);
}

double State::get_clear_depth() {
    double clear_depth = 0;
    glGetDoublev(GL_DEPTH_CLEAR_VALUE, &clear_depth);
    return clear_depth;
}

void State::set_clear_depth(double d) {
    glClearDepth(d);
}

bool State::get_depth_writemask() {
    int depth_writemask = 0;
    glGetIntegerv(GL_DEPTH_WRITEMASK, &depth_writemask);
    return depth_writemask;
}

void State::set_depth_writemask(bool m) {
    glDepthMask(m);
}

ComparisonFunc State::get_depth_func() {
    int depth_func = 0;
    glGetIntegerv(GL_DEPTH_FUNC, &depth_func);
    return get_comparison_function(depth_func);
}

void State::set_depth_func(ComparisonFunc f) {
    glDepthFunc(GL_COMPARISON_FUNCTIONS[f]);
}

void State::enable_stencil_test() {
    glEnable(GL_STENCIL_TEST);
}

void State::disable_stencil_test() {
    glDisable(GL_STENCIL_TEST);
}

int State::get_clear_stencil() {
    int clear_stencil = 0;
    glGetIntegerv(GL_STENCIL_CLEAR_VALUE, &clear_stencil);
    return clear_stencil;
}

void State::set_clear_stencil(int s) {
    glClearStencil(s);
}

unsigned int State::get_stencil_writemask() {
    int stencil_writemask = 0;
    glGetIntegerv(GL_STENCIL_WRITEMASK, &stencil_writemask);
    return stencil_writemask;
}

void State::set_stencil_writemask(unsigned int m) {
    glStencilMask(m);
}

ComparisonFunc State::get_stencil_func() {
    int stencil_func = 0;
    glGetIntegerv(GL_STENCIL_FUNC, &stencil_func);
    return get_comparison_function(stencil_func);
}

int State::get_stencil_ref() {
    int stencil_ref = 0;
    glGetIntegerv(GL_STENCIL_REF, &stencil_ref);
    return stencil_ref;
}

int State::get_stencil_mask() {
    int stencil_mask = 0;
    glGetIntegerv(GL_STENCIL_VALUE_MASK, &stencil_mask);
    return stencil_mask;
}

void State::set_stencil_func(ComparisonFunc f, int r, int m) {
    glStencilFunc(GL_COMPARISON_FUNCTIONS[f], r, m);
}

StencilOperation State::get_stencil_fail() {
    int stencil_fail = 0;
    glGetIntegerv(GL_STENCIL_FAIL, &stencil_fail);
    return get_stencil_operation(stencil_fail);
}

StencilOperation State::get_stencil_zfail() {
    int stencil_zfail = 0;
    glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &stencil_zfail);
    return get_stencil_operation(stencil_zfail);
}

StencilOperation State::get_stencil_zpass() {
    int stencil_zpass = 0;
    glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &stencil_zpass);
    return get_stencil_operation(stencil_zpass);
}

void State::set_stencil_op(StencilOperation f,
                           StencilOperation zf,
                           StencilOperation zp) {
    glStencilOp(GL_STENCIL_OPERATIONS[f],
                GL_STENCIL_OPERATIONS[zf],
                GL_STENCIL_OPERATIONS[zp]);
}

void State::enable_blending() {
    glEnable(GL_BLEND);
}

void State::disable_blending() {
    glDisable(GL_BLEND);
}

BlendOperation State::get_blend_op_rgb() {
    int blend_op_rgb = 0;
    glGetIntegerv(GL_BLEND_EQUATION_RGB, &blend_op_rgb);
    return get_blend_operation(blend_op_rgb);
}

BlendOperation State::get_blend_op_alpha() {
    int blend_op_alpha = 0;
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &blend_op_alpha);
    return get_blend_operation(blend_op_alpha);
}

void State::set_blend_op(BlendOperation o) {
    glBlendEquation(GL_BLEND_OPERATIONS[o]);
}

void State::set_blend_op(BlendOperation rgb, BlendOperation a) {
    glBlendEquationSeparate(GL_BLEND_OPERATIONS[rgb], GL_BLEND_OPERATIONS[a]);
}

BlendFactor State::get_blend_src_rgb() {
    int blend_src_rgb = 0;
    glGetIntegerv(GL_BLEND_SRC_RGB, &blend_src_rgb);
    return get_blend_factor(blend_src_rgb);
}

BlendFactor State::get_blend_src_alpha() {
    int blend_src_alpha = 0;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &blend_src_alpha);
    return get_blend_factor(blend_src_alpha);
}

BlendFactor State::get_blend_dst_rgb() {
    int blend_dst_rgb = 0;
    glGetIntegerv(GL_BLEND_DST_RGB, &blend_dst_rgb);
    return get_blend_factor(blend_dst_rgb);
}

BlendFactor State::get_blend_dst_alpha() {
    int blend_dst_alpha = 0;
    glGetIntegerv(GL_BLEND_DST_ALPHA, &blend_dst_alpha);
    return get_blend_factor(blend_dst_alpha);
}

void State::set_blend_factor(BlendFactor s, BlendFactor d) {
    glBlendFunc(GL_BLEND_FACTORS[s], GL_BLEND_FACTORS[d]);
}

void State::set_blend_factor(BlendFactor sr, BlendFactor dr,
                             BlendFactor sa, BlendFactor da) {
    glBlendFuncSeparate(GL_BLEND_FACTORS[sr], GL_BLEND_FACTORS[dr],
                        GL_BLEND_FACTORS[sa], GL_BLEND_FACTORS[da]);
}

Rect State::get_viewport() {
    Rect viewport_rect;
    glGetIntegerv(GL_VIEWPORT, &viewport_rect.x);
    return viewport_rect;
}

void State::set_viewport(const Rect& v) {
    glViewport(v.x, v.y, v.width, v.height);
}

void State::enable_scissor_test() {
    glEnable(GL_SCISSOR_TEST);
}

void State::disable_scissor_test() {
    glDisable(GL_SCISSOR_TEST);
}

Rect State::get_scissor() {
    Rect scissor_rect;
    glGetIntegerv(GL_SCISSOR_BOX, &scissor_rect.x);
    return scissor_rect;
}

void State::set_scissor(const Rect& s) {
    glScissor(s.x, s.y, s.width, s.height);
}

void State::enable_wireframe() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void State::disable_wireframe() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void State::enable_culling() {
    glEnable(GL_CULL_FACE);
}

void State::disable_culling() {
    glDisable(GL_CULL_FACE);
}

RenderSide State::get_cull_side() {
    int cull_side = 0;
    glGetIntegerv(GL_CULL_FACE_MODE, &cull_side);
    return get_render_side(cull_side);
}

void State::set_cull_side(RenderSide s) {
    glCullFace(GL_RENDER_SIDES[s]);
}

void State::enable_polygon_offset() {
    glEnable(GL_POLYGON_OFFSET_FILL);
}

void State::disable_polygon_offset() {
    glDisable(GL_POLYGON_OFFSET_FILL);
}

float State::get_polygon_offset_factor() {
    float polygon_offset_factor = 0;
    glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &polygon_offset_factor);
    return polygon_offset_factor;
}

float State::get_polygon_offset_units() {
    float polygon_offset_units = 0;
    glGetFloatv(GL_POLYGON_OFFSET_UNITS, &polygon_offset_units);
    return polygon_offset_units;
}

void State::set_polygon_offset(float f, float u) {
    glPolygonOffset(f, u);
}

void State::enable_dithering() {
    glEnable(GL_DITHER);
}

void State::disable_dithering() {
    glDisable(GL_DITHER);
}

void State::enable_multisample() {
    glEnable(GL_MULTISAMPLE);
}

void State::disable_multisample() {
    glDisable(GL_MULTISAMPLE);
}

void State::enable_alpha_to_coverage() {
    glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
}

void State::disable_alpha_to_coverage() {
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
}

void State::enable_texture_cube_seamless() {
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void State::disable_texture_cube_seamless() {
    glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void MaterialState::set_depth(const Material& m) {
    if (!m.depth_test) return State::disable_depth_test();
    State::enable_depth_test();
    State::set_depth_func(m.depth_func);
}

void MaterialState::set_stencil(const Material& m) {
    if (!m.stencil_test) return State::disable_stencil_test();
    State::enable_stencil_test();
    State::set_stencil_writemask(m.stencil_writemask);
    State::set_stencil_func(m.stencil_func, m.stencil_ref, m.stencil_mask);
    State::set_stencil_op(m.stencil_fail, m.stencil_zfail, m.stencil_zpass);
}

void MaterialState::set_blending(const Material& m) {
    if (!m.blending) return State::disable_blending();
    State::enable_blending();
    State::set_blend_op(m.blend_op_rgb, m.blend_op_alpha);
    State::set_blend_factor(m.blend_src_rgb, m.blend_dst_rgb, m.blend_src_alpha, m.blend_dst_alpha);
}

void MaterialState::set_wireframe(const Material& m) {
    if (!m.wireframe) return State::disable_wireframe();
    State::enable_wireframe();
}

void MaterialState::set_side(const Material& m) {
    if (m.side == FRONT_SIDE) {
        State::enable_culling();
        State::set_cull_side(BACK_SIDE);
    } else if (m.side == BACK_SIDE) {
        State::enable_culling();
        State::set_cull_side(FRONT_SIDE);
    } else if (m.side == DOUBLE_SIDE) {
        State::disable_culling();
    }
}

void MaterialState::set_shadow_side(const Material& m) {
    if (m.shadow_side == FRONT_SIDE) {
        State::enable_culling();
        State::set_cull_side(BACK_SIDE);
    } else if (m.shadow_side == BACK_SIDE) {
        State::enable_culling();
        State::set_cull_side(FRONT_SIDE);
    } else if (m.shadow_side == DOUBLE_SIDE) {
        State::disable_culling();
    }
}

Shader::Shader() {
    program = glCreateProgram();
}

Shader::~Shader() {
    glDeleteProgram(program);
}

void Shader::load_vert(const char* s) {
    vert_shader = s;
}

void Shader::load_vert(const std::string& s) {
    vert_shader = s;
}

void Shader::load_geom(const char* s) {
    geom_shader = s;
}

void Shader::load_geom(const std::string& s) {
    geom_shader = s;
}

void Shader::load_frag(const char* s) {
    frag_shader = s;
}

void Shader::load_frag(const std::string& s) {
    frag_shader = s;
}

void Shader::load_vert_file(const std::string& p) {
    vert_shader = File::read(p);
}

void Shader::load_geom_file(const std::string& p) {
    geom_shader = File::read(p);
}

void Shader::load_frag_file(const std::string& p) {
    frag_shader = File::read(p);
}

void Shader::compile() const {
    compile_shaders();
}

void Shader::use_program() const {
    glUseProgram(program);
}

void Shader::set_defines(const Defines& d) {
    defines = d.get();
}

void Shader::set_uniform_i(const std::string& n, int v) const {
    glUniform1i(glGetUniformLocation(program, n.c_str()), v);
}

void Shader::set_uniform_u(const std::string& n, unsigned int v) const {
    glUniform1ui(glGetUniformLocation(program, n.c_str()), v);
}

void Shader::set_uniform_f(const std::string& n, float v) const {
    glUniform1f(glGetUniformLocation(program, n.c_str()), v);
}

void Shader::set_uniform_v2(const std::string& n, const Vec2& v) const {
    glUniform2fv(glGetUniformLocation(program, n.c_str()), 1, &v.x);
}

void Shader::set_uniform_v3(const std::string& n, const Vec3& v) const {
    glUniform3fv(glGetUniformLocation(program, n.c_str()), 1, &v.x);
}

void Shader::set_uniform_v4(const std::string& n, const Vec4& v) const {
    glUniform4fv(glGetUniformLocation(program, n.c_str()), 1, &v.x);
}

void Shader::set_uniform_m2(const std::string& n, const Mat2& v) const {
    glUniformMatrix2fv(glGetUniformLocation(program, n.c_str()), 1, GL_TRUE, v[0]);
}

void Shader::set_uniform_m3(const std::string& n, const Mat3& v) const {
    glUniformMatrix3fv(glGetUniformLocation(program, n.c_str()), 1, GL_TRUE, v[0]);
}

void Shader::set_uniform_m4(const std::string& n, const Mat4& v) const {
    glUniformMatrix4fv(glGetUniformLocation(program, n.c_str()), 1, GL_TRUE, v[0]);
}

void Shader::set_uniforms(const Uniforms& u) const {
    auto* data_f = u.get_data();
    auto* data_i = reinterpret_cast<const int*>(data_f);
    auto* data_u = reinterpret_cast<const unsigned int*>(data_f);
    size_t uniform_count = u.get_count();
    for (int i = 0; i < uniform_count; ++i) {
        std::string name = u.get_name(i);
        int32_t gl_location = glGetUniformLocation(program, name.c_str());
        int type = u.get_type(i);
        int location = u.get_location(i);
        if (type == 0 /* int */ ) {
            glUniform1i(gl_location, data_i[location]);
        } else if (type == 1 /* unsigned int */ ) {
            glUniform1ui(gl_location, data_u[location]);
        } else if (type == 2 /* float */ ) {
            glUniform1f(gl_location, data_f[location]);
        } else if (type == 3 /* Vec2 */ ) {
            glUniform2fv(gl_location, 1, data_f + location);
        } else if (type == 4 /* Vec3 */ ) {
            glUniform3fv(gl_location, 1, data_f + location);
        } else if (type == 5 /* Vec4 */ ) {
            glUniform4fv(gl_location, 1, data_f + location);
        } else if (type == 6 /* Mat2 */ ) {
            glUniformMatrix2fv(gl_location, 1, GL_TRUE, data_f + location);
        } else if (type == 7 /* Mat3 */ ) {
            glUniformMatrix3fv(gl_location, 1, GL_TRUE, data_f + location);
        } else if (type == 8 /* Mat4 */ ) {
            glUniformMatrix4fv(gl_location, 1, GL_TRUE, data_f + location);
        } else {
            Error::set("Shader", "Unknown uniform variable type");
        }
    }
}

std::string Shader::get_glsl_version() {
    return glsl_version;
}

void Shader::set_glsl_version(const std::string& v) {
    glsl_version = v;
}

uint32_t Shader::compile_shader(const std::string& s, int32_t t) const {
    std::string shader_string = s;
    resolve_defines(shader_string);
    resolve_version(shader_string);
    uint32_t shader_id = glCreateShader(t);
    const char* shader_str = shader_string.c_str();
    glShaderSource(shader_id, 1, &shader_str, nullptr);
    glCompileShader(shader_id);
    std::string info = get_compile_info(shader_id, t);
    if (!info.empty()) {
        Error::set("Shader", get_error_info(info, shader_string));
    }
    glAttachShader(program, shader_id);
    return shader_id;
}

void Shader::compile_shaders() const {
    /* compile vertex shader */
    bool use_vert_shader = !vert_shader.empty();
    uint32_t vert_id = 0;
    if (use_vert_shader) {
        vert_id = compile_shader(vert_shader, GL_VERTEX_SHADER);
    } else {
        return Error::set("Shader", "Vertex shader is missing");
    }
    
    /* compile geometry shader */
    bool use_geom_shader = !geom_shader.empty();
    uint32_t geom_id = 0;
    if (use_geom_shader) {
        geom_id = compile_shader(geom_shader, GL_GEOMETRY_SHADER);
    }
    
    /* compile fragment shader */
    bool use_frag_shader = !frag_shader.empty();
    uint32_t frag_id = 0;
    if (use_frag_shader) {
        frag_id = compile_shader(frag_shader, GL_FRAGMENT_SHADER);
    } else {
        return Error::set("Shader", "Fragment shader is missing");
    }
    
    /* link shaders to program */
    glLinkProgram(program);
    std::string info = get_link_info();
    if (!info.empty()) Error::set("Shader", info);
    
    /* delete vertex shader */
    glDeleteShader(vert_id);
    
    /* delete geometry shader */
    if (use_geom_shader) glDeleteShader(geom_id);
    
    /* delete fragment shader */
    glDeleteShader(frag_id);
}

std::string Shader::get_link_info() const {
    int32_t success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_TRUE) return "";
    char info[1024];
    glGetProgramInfoLog(program, 1024, nullptr, info);
    return std::string("Link error\n") + info;
}

void Shader::resolve_defines(std::string& s) const {
    s = defines + s;
}

void Shader::resolve_version(std::string& s) {
    s = "#version " + glsl_version + "\n" + s;
}

std::string Shader::get_compile_info(uint32_t s, uint32_t t) {
    int32_t success;
    glGetShaderiv(s, GL_COMPILE_STATUS, &success);
    if (success == GL_TRUE) return "";
    char info[1024];
    glGetShaderInfoLog(s, 1024, nullptr, info);
    if (t == GL_VERTEX_SHADER) {
        return std::string("Shader: Vertex shader compile error\n") + info;
    } else if (t == GL_GEOMETRY_SHADER) {
        return std::string("Shader: Geometry shader compile error\n") + info;
    } else if (t == GL_FRAGMENT_SHADER) {
        return std::string("Shader: Fragment shader compile error\n") + info;
    }
    return "Shader: No such shader";
}

std::string Shader::get_error_info(const std::string& c, const std::string& s) {
    size_t line_begin = 0;
    size_t line_end = -1;
    
    /* initialize error information and error number */
    std::string info;
    int error_number = 0;
    
    while (true) {
        /* search the string of each line */
        line_begin = line_end + 1;
        line_end = c.find('\n', line_begin);
        if (line_end == -1) break;
        size_t line_length = line_end - line_begin;
        std::string line = c.substr(line_begin, line_length);
        info += line + '\n';
        if (line.substr(0, 9) != "ERROR: 0:") continue;
        
        /* get the line number from error information */
        size_t number_begin = line.find(':', 7) + 1;
        size_t number_end = line.find(':', number_begin);
        size_t number_length = number_end - number_begin;
        int line_number = std::stoi(line.substr(number_begin, number_length));
        
        /* search the code where the error occurred */
        size_t error_begin = 0;
        size_t error_end = -1;
        for (int i = 0; i < line_number; ++i) {
            error_begin = error_end + 1;
            error_end = s.find('\n', error_begin);
            if (error_end == -1) break;
        }
        size_t error_length = error_end - error_begin;
        std::string error_line = s.substr(error_begin, error_length);
        
        /* add code line to error information */
        info += error_line + "\n\n";
        ++error_number;
    }
    
    /* count how many errors have occurred */
    info += std::to_string(error_number);
    info += error_number == 1 ? " error generated.\n" : " errors generated.\n";
    
    return info; /* return error information */
}

std::string Shader::glsl_version = "410";

VertexObject::VertexObject() {
    glGenVertexArrays(1, &id);
    glGenBuffers(1, &buffer_id);
}

VertexObject::~VertexObject() {
    glDeleteVertexArrays(1, &id);
    glDeleteBuffers(1, &buffer_id);
}

void VertexObject::load(const Mesh& m, const MeshGroup& g) {
    auto& vertex = m.vertex;
    auto& normal = m.normal;
    auto& uv = m.uv;
    auto& tangent = m.tangent;
    auto& color = m.color;
    
    /* check whether to use attributes */
    bool has_normal = !normal.empty();
    bool has_uv = !uv.empty();
    bool has_tangent = !tangent.empty();
    bool has_color = !color.empty();
    
    /* calculate length and stride */
    length = g.length;
    int group_end = g.position + g.length;
    int stride = 3;
    if (has_normal) stride += 3;
    if (has_uv) stride += 2;
    if (has_tangent) stride += 4;
    if (has_color) stride += 3;
    
    /* pack attributes' data into one vector */
    std::vector<float> data(length * stride);
    /* has_vertex */ {
        names = {"vertex"};
        sizes = {3};
        locations = {0};
        auto* data_ptr = data.data() + locations.back();
        for (int i = g.position; i < group_end; ++i) {
            data_ptr[0] = vertex[i].x;
            data_ptr[1] = vertex[i].y;
            data_ptr[2] = vertex[i].z;
            data_ptr += stride;
        }
        locations.emplace_back(locations.back() + 3);
    }
    if (has_normal) {
        names.emplace_back("normal");
        sizes.emplace_back(3);
        auto* data_ptr = data.data() + locations.back();
        for (int i = g.position; i < group_end; ++i) {
            data_ptr[0] = normal[i].x;
            data_ptr[1] = normal[i].y;
            data_ptr[2] = normal[i].z;
            data_ptr += stride;
        }
        locations.emplace_back(locations.back() + 3);
    }
    if (has_uv) {
        names.emplace_back("uv");
        sizes.emplace_back(2);
        auto* data_ptr = data.data() + locations.back();
        for (int i = g.position; i < group_end; ++i) {
            data_ptr[0] = uv[i].x;
            data_ptr[1] = uv[i].y;
            data_ptr += stride;
        }
        locations.emplace_back(locations.back() + 2);
    }
    if (has_tangent) {
        names.emplace_back("tangent");
        sizes.emplace_back(4);
        auto* data_ptr = data.data() + locations.back();
        for (int i = g.position; i < group_end; ++i) {
            data_ptr[0] = tangent[i].x;
            data_ptr[1] = tangent[i].y;
            data_ptr[2] = tangent[i].z;
            data_ptr[3] = tangent[i].w;
            data_ptr += stride;
        }
        locations.emplace_back(locations.back() + 4);
    }
    if (has_color) {
        names.emplace_back("color");
        sizes.emplace_back(3);
        auto* data_ptr = data.data() + locations.back();
        for (int i = g.position; i < group_end; ++i) {
            data_ptr[0] = color[i].x;
            data_ptr[1] = color[i].y;
            data_ptr[2] = color[i].z;
            data_ptr += stride;
        }
        locations.emplace_back(locations.back() + 3);
    }
    
    /* upload data to GPU */
    glBindVertexArray(id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.size(), data.data(), GL_STATIC_DRAW);
}

void VertexObject::attach(const Shader& s) const {
    int stride = locations.back();
    glBindVertexArray(id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
    for (int i = 0; i < sizes.size(); ++i) {
        int32_t attrib = glGetAttribLocation(s.program, names[i].c_str());
        if (attrib == -1) continue;
        void* pointer = reinterpret_cast<void*>(sizeof(float) * locations[i]);
        glVertexAttribPointer(attrib, sizes[i], GL_FLOAT, GL_FALSE, sizeof(float) * stride, pointer);
        glEnableVertexAttribArray(attrib);
    }
}

void VertexObject::render() const {
    glBindVertexArray(id);
    glDrawArrays(GL_TRIANGLES, 0, length);
}

Texture::Texture() {
    glGenTextures(1, &id);
}

Texture::~Texture() {
    glDeleteTextures(1, &id);
}

void Texture::init_1d(int w, TextureFormat f, ImageType t) {
    int32_t sized = GL_TEXTURE_SIZED_INTERNAL_FORMATS[f];
    uint32_t base = GL_TEXTURE_BASE_INTERNAL_FORMATS[f];
    uint32_t data = GL_IMAGE_TYPES[t];
    glBindTexture(GL_TEXTURE_1D, id);
    glTexImage1D(GL_TEXTURE_1D, 0, sized, w, 0, base, data, nullptr);
    set_dimensions(w, 0, 0);
    set_parameters(TEXTURE_1D, f);
}

void Texture::init_2d(int w, int h, TextureFormat f, ImageType t) {
    int32_t sized = GL_TEXTURE_SIZED_INTERNAL_FORMATS[f];
    uint32_t base = GL_TEXTURE_BASE_INTERNAL_FORMATS[f];
    uint32_t data = GL_IMAGE_TYPES[t];
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, sized, w, h, 0, base, data, nullptr);
    set_dimensions(w, h, 0);
    set_parameters(TEXTURE_2D, f);
}

void Texture::init_2d(const Image& i, TextureFormat f, ImageFormat t) {
    int32_t sized = GL_TEXTURE_SIZED_INTERNAL_FORMATS[f];
    uint32_t base = GL_IMAGE_FORMATS[t];
    uint32_t data = GL_IMAGE_TYPES[i.bytes == 1 ? IMAGE_UBYTE : IMAGE_FLOAT];
    if (base == GL_RGBA) base = GL_IMAGE_COLORS[i.channel - 1];
    if (base == GL_RGBA_INTEGER) base = GL_IMAGE_COLOR_INTEGERS[i.channel - 1];
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, sized, i.width, i.height, 0, base, data, i.data.data());
    set_dimensions(i.width, i.height, 0);
    set_parameters(TEXTURE_2D, f);
}

void Texture::init_3d(int w, int h, int d, TextureFormat f, ImageType t) {
    int32_t sized = GL_TEXTURE_SIZED_INTERNAL_FORMATS[f];
    uint32_t base = GL_TEXTURE_BASE_INTERNAL_FORMATS[f];
    uint32_t data = GL_IMAGE_TYPES[t];
    glBindTexture(GL_TEXTURE_3D, id);
    glTexImage3D(GL_TEXTURE_3D, 0, sized, w, h, d, 0, base, data, nullptr);
    set_dimensions(w, h, d);
    set_parameters(TEXTURE_3D, f);
}

void Texture::init_cube(int w, int h, TextureFormat f, ImageType t) {
    int32_t sized = GL_TEXTURE_SIZED_INTERNAL_FORMATS[f];
    uint32_t base = GL_TEXTURE_BASE_INTERNAL_FORMATS[f];
    uint32_t data = GL_IMAGE_TYPES[t];
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    for (int i = 0; i < 6; ++i) {
        uint32_t target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
        glTexImage2D(target, 0, sized, w, h, 0, base, data, nullptr);
    }
    set_dimensions(w, h, 0);
    set_parameters(TEXTURE_CUBE, f);
}

void Texture::init_cube(const Image& px, const Image& nx, const Image& py, const Image& ny,
                        const Image& pz, const Image& nz, TextureFormat f, ImageFormat t) {
    int32_t sized = GL_TEXTURE_SIZED_INTERNAL_FORMATS[f];
    uint32_t base = GL_IMAGE_FORMATS[t];
    uint32_t data = GL_IMAGE_TYPES[px.bytes == 1 ? IMAGE_UBYTE : IMAGE_FLOAT];
    if (base == GL_RGBA) base = GL_IMAGE_COLORS[px.channel - 1];
    if (base == GL_RGBA_INTEGER) base = GL_IMAGE_COLOR_INTEGERS[px.channel - 1];
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    uint32_t target = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
    glTexImage2D(target, 0, sized, px.width, px.height, 0, base, data, px.data.data());
    target = GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
    glTexImage2D(target, 0, sized, nx.width, nx.height, 0, base, data, nx.data.data());
    target = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
    glTexImage2D(target, 0, sized, py.width, py.height, 0, base, data, py.data.data());
    target = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
    glTexImage2D(target, 0, sized, ny.width, ny.height, 0, base, data, ny.data.data());
    target = GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
    glTexImage2D(target, 0, sized, pz.width, pz.height, 0, base, data, pz.data.data());
    target = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
    glTexImage2D(target, 0, sized, nz.width, nz.height, 0, base, data, nz.data.data());
    set_dimensions(px.width, px.height, 0);
    set_parameters(TEXTURE_CUBE, f);
}

void Texture::init_1d_array(int w, int l, TextureFormat f, ImageType t) {
    int32_t sized = GL_TEXTURE_SIZED_INTERNAL_FORMATS[f];
    uint32_t base = GL_TEXTURE_BASE_INTERNAL_FORMATS[f];
    uint32_t data = GL_IMAGE_TYPES[t];
    glBindTexture(GL_TEXTURE_1D_ARRAY, id);
    glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, sized, w, l, 0, base, data, nullptr);
    set_dimensions(w, l, 0);
    set_parameters(TEXTURE_1D_ARRAY, f);
}

void Texture::init_2d_array(int w, int h, int l, TextureFormat f, ImageType t) {
    int32_t sized = GL_TEXTURE_SIZED_INTERNAL_FORMATS[f];
    uint32_t base = GL_TEXTURE_BASE_INTERNAL_FORMATS[f];
    uint32_t data = GL_IMAGE_TYPES[t];
    glBindTexture(GL_TEXTURE_2D_ARRAY, id);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, sized, w, h, l, 0, base, data, nullptr);
    set_dimensions(w, h, l);
    set_parameters(TEXTURE_2D_ARRAY, f);
}

void Texture::init_cube_array(int w, int h, int l, TextureFormat f, ImageType t) {
    int32_t sized = GL_TEXTURE_SIZED_INTERNAL_FORMATS[f];
    uint32_t base = GL_TEXTURE_BASE_INTERNAL_FORMATS[f];
    uint32_t data = GL_IMAGE_TYPES[t];
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, id);
    glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, sized, w, h, l * 6, 0, base, data, nullptr);
    set_dimensions(w, h, l);
    set_parameters(TEXTURE_CUBE_ARRAY, f);
}

int Texture::get_width() const {
    return width;
}

int Texture::get_height() const {
    return height;
}

int Texture::get_depth() const {
    return depth;
}

int Texture::get_layer() const {
    if (type == TEXTURE_1D_ARRAY  ) return height;
    if (type == TEXTURE_2D_ARRAY  ) return depth;
    if (type == TEXTURE_CUBE_ARRAY) return depth;
    return 0;
}

TextureType Texture::get_type() const {
    return type;
}

TextureFormat Texture::get_format() const {
    return format;
}

void Texture::copy_to_image(Image& i) const {
    /* check whether the texture is 2D */
    if (type != TEXTURE_2D) {
        return Error::set("Texture", "Cannot get image from non-2D texture");
    }
    
    /* get the base internal format of the texture */
    uint32_t base = GL_TEXTURE_BASE_INTERNAL_FORMATS[format];
    
    /* get image data from GPU */
    uint8_t* image_data = i.data.data();
    uint32_t image_type = i.bytes == 1 ? GL_UNSIGNED_BYTE : GL_FLOAT;
    glBindTexture(GL_TEXTURE_2D, id);
    glGetTexImage(GL_TEXTURE_2D, 0, base, image_type, image_data);
}

void Texture::generate_mipmap() const {
    uint32_t gl_type = GL_TEXTURE_TYPES[type];
    glBindTexture(gl_type, id);
    glGenerateMipmap(gl_type);
}

void Texture::set_wrap_s(TextureWrappingMode m) const {
    uint32_t gl_type = GL_TEXTURE_TYPES[type];
    glBindTexture(gl_type, id);
    glTexParameteri(gl_type, GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAPPINGS[m]);
}

void Texture::set_wrap_t(TextureWrappingMode m) const {
    uint32_t gl_type = GL_TEXTURE_TYPES[type];
    glBindTexture(gl_type, id);
    glTexParameteri(gl_type, GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAPPINGS[m]);
}

void Texture::set_wrap_r(TextureWrappingMode m) const {
    uint32_t gl_type = GL_TEXTURE_TYPES[type];
    glBindTexture(gl_type, id);
    glTexParameteri(gl_type, GL_TEXTURE_WRAP_R, GL_TEXTURE_WRAPPINGS[m]);
}

void Texture::set_wrap_all(TextureWrappingMode m) const {
    uint32_t gl_type = GL_TEXTURE_TYPES[type];
    glBindTexture(gl_type, id);
    glTexParameteri(gl_type, GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAPPINGS[m]);
    glTexParameteri(gl_type, GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAPPINGS[m]);
    glTexParameteri(gl_type, GL_TEXTURE_WRAP_R, GL_TEXTURE_WRAPPINGS[m]);
}

void Texture::set_filters(TextureFilter mag, TextureFilter min) const {
    uint32_t gl_type = GL_TEXTURE_TYPES[type];
    glBindTexture(gl_type, id);
    glTexParameteri(gl_type, GL_TEXTURE_MAG_FILTER, GL_TEXTURE_FILTERS[mag]);
    glTexParameteri(gl_type, GL_TEXTURE_MIN_FILTER, GL_TEXTURE_FILTERS[min]);
}

void Texture::set_border_color(const Vec4& c) const {
    uint32_t gl_type = GL_TEXTURE_TYPES[type];
    glBindTexture(gl_type, id);
    glTexParameterfv(gl_type, GL_TEXTURE_BORDER_COLOR, &c.x);
}

void Texture::set_lod_range(int min, int max) const {
    uint32_t gl_type = GL_TEXTURE_TYPES[type];
    glBindTexture(gl_type, id);
    glTexParameteri(gl_type, GL_TEXTURE_BASE_LEVEL, min);
    glTexParameteri(gl_type, GL_TEXTURE_MAX_LEVEL, max);
}

void Texture::set_lod_bias(float b) const {
    uint32_t gl_type = GL_TEXTURE_TYPES[type];
    glBindTexture(gl_type, id);
    glTexParameterf(gl_type, GL_TEXTURE_LOD_BIAS, b);
}

int Texture::activate(int l) const {
    uint32_t gl_type = GL_TEXTURE_TYPES[type];
    glActiveTexture(GL_TEXTURE0 + l);
    glBindTexture(gl_type, id);
    return l;
}

void Texture::set_dimensions(int w, int h, int d) {
    width = w;
    height = h;
    depth = d;
}

void Texture::set_parameters(TextureType t, TextureFormat f) {
    type = t;
    format = f;
}

TextureFormat Texture::default_format(int c, int b) {
    if (c == 1 && b == 1) return TEXTURE_R8_UNORM;
    if (c == 1 && b == 4) return TEXTURE_R16_SFLOAT;
    if (c == 2 && b == 1) return TEXTURE_R8G8_UNORM;
    if (c == 2 && b == 4) return TEXTURE_R16G16_SFLOAT;
    if (c == 3 && b == 1) return TEXTURE_R8G8B8_UNORM;
    if (c == 3 && b == 4) return TEXTURE_R16G16B16_SFLOAT;
    if (c == 4 && b == 1) return TEXTURE_R8G8B8A8_UNORM;
    /*  c == 4 && b == 4*/return TEXTURE_R16G16B16A16_SFLOAT;
}

TextureFormat Texture::default_format(const Image& i) {
    return default_format(i.channel, i.bytes);
}

Renderbuffer::Renderbuffer() {
    glGenRenderbuffers(1, &id);
}

Renderbuffer::~Renderbuffer() {
    glDeleteRenderbuffers(1, &id);
}

void Renderbuffer::init(int w, int h, TextureFormat f) const {
    int32_t sized = GL_TEXTURE_SIZED_INTERNAL_FORMATS[f];
    glBindRenderbuffer(GL_RENDERBUFFER, id);
    glRenderbufferStorage(GL_RENDERBUFFER, sized, w, h);
}

RenderTarget::RenderTarget() {
    glGenFramebuffers(1, &id);
}

RenderTarget::~RenderTarget() {
    glDeleteFramebuffers(1, &id);
}

void RenderTarget::set_texture(const Texture& t, unsigned int i, int l, int p) const {
    set_framebuffer(t, GL_COLOR_ATTACHMENT0 + i, l, p);
}

void RenderTarget::set_depth_texture(const Texture& t, int l, int p) const {
    set_framebuffer(t, GL_DEPTH_ATTACHMENT, l, p);
}

void RenderTarget::set_stencil_texture(const Texture& t, int l, int p) const {
    set_framebuffer(t, GL_STENCIL_ATTACHMENT, l, p);
}

void RenderTarget::set_depth_stencil_texture(const Texture& t, int l, int p) const {
    set_framebuffer(t, GL_DEPTH_STENCIL_ATTACHMENT, l, p);
}

void RenderTarget::set_depth_buffer(const Renderbuffer& r) const {
    set_framebuffer(r, GL_DEPTH_ATTACHMENT);
}

void RenderTarget::set_stencil_buffer(const Renderbuffer& r) const {
    set_framebuffer(r, GL_STENCIL_ATTACHMENT);
}

void RenderTarget::set_depth_stencil_buffer(const Renderbuffer& r) const {
    set_framebuffer(r, GL_DEPTH_STENCIL_ATTACHMENT);
}

void RenderTarget::set_target_number(int n) const {
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    glDrawBuffers(n, GL_COLOR_ATTACHMENTS);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTarget::activate() const {
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        Error::set("RenderTarget", "Render target is not complete");
    }
}

void RenderTarget::activate(const RenderTarget* t) {
    uint32_t id = t == nullptr ? 0 : t->id;
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        Error::set("RenderTarget", "Render target is not complete");
    }
}

void RenderTarget::set_framebuffer(const Texture& t, uint32_t a, int l, int p) const {
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    if (t.type == TEXTURE_2D) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, a, GL_TEXTURE_2D, t.id, l);
    } else if (t.type == TEXTURE_CUBE) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, a, GL_TEXTURE_CUBE_MAP_POSITIVE_X + p, t.id, l);
    } else if (t.type == TEXTURE_3D) {
        glFramebufferTexture3D(GL_FRAMEBUFFER, a, GL_TEXTURE_3D, t.id, l, p);
    } else if (t.type == TEXTURE_2D_ARRAY) {
        glFramebufferTextureLayer(GL_FRAMEBUFFER, a, t.id, l, p);
    } else if (t.type == TEXTURE_CUBE_ARRAY) {
        glFramebufferTextureLayer(GL_FRAMEBUFFER, a, t.id, l, p);
    } else {
        Error::set("RenderTarget", "Texture type is not supported");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTarget::set_framebuffer(const Renderbuffer& r, uint32_t a) const {
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, a, GL_RENDERBUFFER, r.id);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/ShaderCache.cpp ----------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

void ShaderCache::load_vert(const std::string& n, const char* s) {
    std::string name = to_lower(n);
    vert_shaders.insert_or_assign(name, s);
    resolve_includes(vert_shaders[name]);
}

void ShaderCache::load_vert(const std::string& n, const std::string& s) {
    std::string name = to_lower(n);
    vert_shaders.insert_or_assign(name, s);
    resolve_includes(vert_shaders[name]);
}

void ShaderCache::load_geom(const std::string& n, const char* s) {
    std::string name = to_lower(n);
    geom_shaders.insert_or_assign(name, s);
    resolve_includes(geom_shaders[name]);
}

void ShaderCache::load_geom(const std::string& n, const std::string& s) {
    std::string name = to_lower(n);
    geom_shaders.insert_or_assign(name, s);
    resolve_includes(geom_shaders[name]);
}

void ShaderCache::load_frag(const std::string& n, const char* s) {
    std::string name = to_lower(n);
    frag_shaders.insert_or_assign(name, s);
    resolve_includes(frag_shaders[name]);
}

void ShaderCache::load_frag(const std::string& n, const std::string& s) {
    std::string name = to_lower(n);
    frag_shaders.insert_or_assign(name, s);
    resolve_includes(frag_shaders[name]);
}

void ShaderCache::load_include(const std::string& n, const char* s) {
    std::string name = to_lower(n);
    include_shaders.insert_or_assign(name, s);
    resolve_includes(include_shaders[name]);
}

void ShaderCache::load_include(const std::string& n, const std::string& s) {
    std::string name = to_lower(n);
    include_shaders.insert_or_assign(name, s);
    resolve_includes(include_shaders[name]);
}

void ShaderCache::load_vert_file(const std::string& n, const std::string& p) {
    std::string name = to_lower(n);
    vert_shaders.insert_or_assign(name, File::read(p));
    resolve_includes(vert_shaders[name]);
}

void ShaderCache::load_geom_file(const std::string& n, const std::string& p) {
    std::string name = to_lower(n);
    geom_shaders.insert_or_assign(name, File::read(p));
    resolve_includes(geom_shaders[name]);
}

void ShaderCache::load_frag_file(const std::string& n, const std::string& p) {
    std::string name = to_lower(n);
    frag_shaders.insert_or_assign(name, File::read(p));
    resolve_includes(frag_shaders[name]);
}

void ShaderCache::load_include_file(const std::string& n, const std::string& p) {
    std::string name = to_lower(n);
    include_shaders.insert_or_assign(name, File::read(p));
    resolve_includes(include_shaders[name]);
}

bool ShaderCache::has_vert(const std::string& n) {
    std::string name = to_lower(n);
    return vert_shaders.count(name) != 0;
}

bool ShaderCache::has_geom(const std::string& n) {
    std::string name = to_lower(n);
    return geom_shaders.count(name) != 0;
}

bool ShaderCache::has_frag(const std::string& n) {
    std::string name = to_lower(n);
    return frag_shaders.count(name) != 0;
}

bool ShaderCache::has_include(const std::string& n) {
    std::string name = to_lower(n);
    return include_shaders.count(name) != 0;
}

const gpu::Shader* ShaderCache::fetch(const std::string& n) {
    /* set the name as cache key */
    std::string name = to_lower(n);
    std::string key = name;
    
    /* check whether the key exists */
    if (cache.count(key) != 0) return cache.at(key).get();
    
    /* insert key and shader to the cache */
    auto p = cache.insert({key, std::make_unique<gpu::Shader>()});
    auto* shader = p.first->second.get();
    
    /* load vertex, geometry and fragment shaders */
    if (vert_shaders.count(name) != 0) {
        shader->load_vert(vert_shaders[name]);
    } else {
        Error::set("ShaderCache", "Vertex shader is missing");
    }
    if (geom_shaders.count(name) != 0) {
        shader->load_geom(geom_shaders[name]);
    }
    if (frag_shaders.count(name) != 0) {
        shader->load_frag(frag_shaders[name]);
    } else {
        Error::set("ShaderCache", "Fragment shader is missing");
    }
    
    /* compile shader */
    shader->compile();
    
    return shader; /* return the shader */
}

const gpu::Shader* ShaderCache::fetch(const std::string& n, const Defines& d) {
    /* set the name and defines as cache key */
    std::string name = to_lower(n);
    std::string key = name + '\n' + d.get();
    
    /* check whether the key exists */
    if (cache.count(key) != 0) return cache.at(key).get();
    
    /* insert key and shader to the cache */
    auto p = cache.insert({key, std::make_unique<gpu::Shader>()});
    auto* shader = p.first->second.get();
    
    /* load vertex, geometry and fragment shaders */
    if (vert_shaders.count(name) != 0) {
        shader->load_vert(vert_shaders[name]);
    } else {
        Error::set("ShaderCache", "Vertex shader is missing");
    }
    if (geom_shaders.count(name) != 0) {
        shader->load_geom(geom_shaders[name]);
    }
    if (frag_shaders.count(name) != 0) {
        shader->load_frag(frag_shaders[name]);
    } else {
        Error::set("ShaderCache", "Fragment shader is missing");
    }
    
    /* set defines and compile shader */
    shader->set_defines(d);
    shader->compile();
    
    return shader; /* return the shader */
}

void ShaderCache::clear_cache(const std::string& n) {
    std::string name = to_lower(n);
    cache.erase(name);
}

void ShaderCache::clear_cache(const std::string& n, const Defines& d) {
    std::string name = to_lower(n);
    cache.erase(name + '\n' + d.get());
}

void ShaderCache::clear_caches() {
    cache.clear();
}

std::string ShaderCache::get_include_path() {
    return include_path;
}

void ShaderCache::set_include_path(const std::string& p) {
    include_path = p;
}

std::string ShaderCache::to_lower(const std::string& s) {
    std::string lower = s;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}

void ShaderCache::resolve_includes(std::string& s) {
    size_t line_begin = 0;
    size_t line_end = -1;
    
    while (line_end != s.length()) {
        /* search every line of the shader content */
        line_begin = line_end + 1;
        line_end = s.find('\n', line_begin);
        line_end = line_end == -1 ? s.length() : line_end;
        std::string line = s.substr(line_begin, line_end - line_begin);
        
        /* search for the include name */
        size_t char_1 = line.find_first_not_of(" \t");
        if (char_1 == -1 || line[char_1] != '#') {
            continue;
        }
        size_t char_2 = line.find_first_not_of(" \t", char_1 + 1);
        if (char_2 == -1 || line.substr(char_2, 7) != "include") {
            continue;
        }
        size_t char_3 = line.find_first_not_of(" \t", char_2 + 7);
        if (char_3 == -1 || line[char_3] != '<') {
            continue;
        }
        size_t char_4 = line.find('>', char_3 + 1);
        if (char_4 == -1) {
            Error::set("ShaderCache", "Invalid preprocessing directive");
            continue;
        }
        std::string include_name = to_lower(line.substr(char_3 + 1, char_4 - char_3 - 1));
        
        /* read the included file into content */
        if (include_set.count(include_name) != 0) {
            s = std::string();
            return Error::set("ShaderCache", "Circular include dependency");
        }
        if (!has_include(include_name)) {
            include_set.insert(include_name);
            load_include_file(include_name, include_path + "/" + include_name + ".glsl");
            include_set.erase(include_name);
        }
        std::string content = include_shaders[include_name];
        
        /* replace the line with content */
        s.replace(line_begin, line_end - line_begin, content);
        line_end = line_begin - 1;
    }
}

std::string ShaderCache::include_path = "ink/shaders/include";

std::unordered_map<std::string, std::string> ShaderCache::vert_shaders;
std::unordered_map<std::string, std::string> ShaderCache::geom_shaders;
std::unordered_map<std::string, std::string> ShaderCache::frag_shaders;
std::unordered_map<std::string, std::string> ShaderCache::include_shaders;

std::unordered_set<std::string> ShaderCache::include_set;

std::unordered_map<std::string, std::unique_ptr<gpu::Shader>> ShaderCache::cache;

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/ShaderLib.cpp ------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

const gpu::Shader* ShaderLib::fetch(const std::string& n) {
    std::string shader_file = library_path + "/" + n;
    if (!ShaderCache::has_vert(n) || !ShaderCache::has_frag(n)) {
        ShaderCache::load_vert(n, File::read(shader_file + ".vert.glsl"));
        ShaderCache::load_frag(n, File::read(shader_file + ".frag.glsl"));
    }
    return ShaderCache::fetch(n);
}

const gpu::Shader* ShaderLib::fetch(const std::string& n, const Defines& d) {
    std::string shader_file = library_path + "/" + n;
    if (!ShaderCache::has_vert(n) || !ShaderCache::has_frag(n)) {
        ShaderCache::load_vert(n, File::read(shader_file + ".vert.glsl"));
        ShaderCache::load_frag(n, File::read(shader_file + ".frag.glsl"));
    }
    return ShaderCache::fetch(n, d);
}

std::string ShaderLib::get_library_path() {
    return library_path;
}

void ShaderLib::set_library_path(const std::string& p) {
    library_path = p;
}

std::string ShaderLib::library_path = "ink/shaders/library";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/lights/Light.cpp ------------------------------------------------ */
/* -------------------------------------------------------------------------- */

namespace ink {

Light::Light(const Vec3& c, float i) : color(c), intensity(i) {}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/lights/Shadow.cpp ----------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

void Shadow::init(int w, int h, int n) {
    resolution = Vec2(w, h);
    shadow_map = std::make_unique<gpu::Texture>();
    shadow_map->init_2d_array(w, h, n, TEXTURE_D24_UNORM);
    shadow_map->set_wrap_all(TEXTURE_CLAMP_TO_BORDER);
    shadow_map->set_filters(TEXTURE_LINEAR, TEXTURE_NEAREST);
    shadow_map->set_border_color({1, 1, 1, 1});
    shadow_target = std::make_unique<gpu::RenderTarget>();
    shadow_target->set_target_number(0);
}

int Shadow::get_samples() {
    return samples;
}

void Shadow::set_samples(int s) {
    samples = s;
}

Vec2 Shadow::get_resolution() {
    return resolution;
}

int Shadow::activate_texture(int l) {
    return shadow_map->activate(l);
}

const gpu::RenderTarget* Shadow::get_target() const {
    shadow_target->set_depth_texture(*shadow_map, 0, map_id);
    return shadow_target.get();
}

int Shadow::samples = 32;

Vec2 Shadow::resolution;

std::unique_ptr<gpu::Texture> Shadow::shadow_map;

std::unique_ptr<gpu::RenderTarget> Shadow::shadow_target;

}

/* -------------------------------------------------------------------------- */
/* ---- ink/lights/DirectionalLight.cpp ------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

DirectionalLight::DirectionalLight(const Vec3& c, float i) : Light(c, i) {
    shadow.camera = OrthoCamera(-5, 5, -5, 5, 0.5, 500);
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/lights/Exp2Fog.cpp ---------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

Exp2Fog::Exp2Fog(const Vec3& c, float n, float d) :
color(c), near(n), density(d) {}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/lights/HemisphereLight.cpp -------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

HemisphereLight::HemisphereLight(const Vec3& s, const Vec3& g, float i) :
Light(s, i), ground_color(g) {}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/lights/LinearFog.cpp -------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

LinearFog::LinearFog(const Vec3& c, float n, float f) :
color(c), near(n), far(f) {}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/lights/PointLight.cpp ------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

PointLight::PointLight(const Vec3& c, float i) : Light(c, i) {}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/lights/SpotLight.cpp -------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

SpotLight::SpotLight(const Vec3& c, float i) : Light(c, i) {
    shadow.camera = PerspCamera(angle * 2, 1, 0.5, 500);
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/probes/IBLFilter.cpp -------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr Vec3 DODECAHEDRON_AXES[] = {
    { 1.000000000,  1.000000000,  1.000000000},
    {-1.000000000,  1.000000000,  1.000000000},
    { 1.000000000,  1.000000000, -1.000000000},
    {-1.000000000,  1.000000000, -1.000000000},
    { 0.000000000,  1.618033989,  0.618033989},
    { 0.000000000,  1.618033989, -0.618033989},
    { 0.618033989,  0.000000000,  1.618033989},
    {-0.618033989,  0.000000000,  1.618033989},
    { 1.618033989,  0.618033989,  0.000000000},
    {-1.618033989,  0.618033989,  0.000000000},
};

void IBLFilter::load_cubemap(const Image& px, const Image& nx,
                             const Image& py, const Image& ny,
                             const Image& pz, const Image& nz,
                             gpu::Texture& m, int s) {
    gpu::Texture source_map;
    source_map.init_cube(px, nx, py, ny, pz, nz,
                         gpu::Texture::default_format(px));
    source_map.set_filters(TEXTURE_LINEAR, TEXTURE_LINEAR);
    load_texture(source_map, m, s);
}

void IBLFilter::load_equirect(const Image& i, gpu::Texture& m, int s) {
    gpu::Texture source_map;
    source_map.init_2d(i, gpu::Texture::default_format(i));
    source_map.set_filters(TEXTURE_LINEAR, TEXTURE_LINEAR);
    load_texture(source_map, m, s);
}

void IBLFilter::load_texture(const gpu::Texture& t, gpu::Texture& m, int s) {
    /* initialize fullscreen plane vertex object */
    if (!fullscreen_plane) init_fullscreen_plane();
    
    /* set the states of GPU pipeline */
    gpu::State::disable_depth_test();
    gpu::State::disable_stencil_test();
    gpu::State::disable_scissor_test();
    gpu::State::disable_blending();
    gpu::State::disable_culling();
    gpu::State::disable_wireframe();
    gpu::State::enable_texture_cube_seamless();
    
    /* change the current viewport */
    gpu::State::set_viewport(gpu::Rect(s, s));
    
    /* initialize radiance environment map */
    m.init_cube(s, s, t.get_format());
    m.set_filters(TEXTURE_LINEAR, TEXTURE_LINEAR_MIPMAP_LINEAR);
    m.generate_mipmap();
    
    /* fetch the blur shader from shader lib */
    auto* blur_shader = ShaderLib::fetch("SphericalBlur");
    
    /* fetch the cubemap shader from shader lib */
    Defines cubemap_defines;
    int type = t.get_type();
    if (type == TEXTURE_2D) {
        cubemap_defines.set("USE_EQUIRECT");
    } else if (type == TEXTURE_CUBE) {
        cubemap_defines.set("USE_CUBEMAP");
    }
    auto* cubemap_shader = ShaderLib::fetch("Cubemap", cubemap_defines);
    
    /* prepare cubemap render target */
    if (!cubemap_target) {
        cubemap_target = std::make_unique<gpu::RenderTarget>();
    }
    
    /* render to cube texture */
    for (int i = 0; i < 6; ++i) {
        cubemap_target->set_texture(m, 0, 0, i);
        gpu::RenderTarget::activate(cubemap_target.get());
        cubemap_shader->use_program();
        cubemap_shader->set_uniform_i("face", i);
        cubemap_shader->set_uniform_i("map", t.activate(0));
        fullscreen_plane->attach(*cubemap_shader);
        fullscreen_plane->render();
    }
    
    /* prepare blur map */
    if (!blur_map) blur_map = std::make_unique<gpu::Texture>();
    blur_map->init_cube(s, s, t.get_format());
    blur_map->generate_mipmap();
    blur_map->set_filters(TEXTURE_LINEAR, TEXTURE_LINEAR_MIPMAP_LINEAR);
    
    /* prepare blur render target */
    if (!blur_target) {
        blur_target = std::make_unique<gpu::RenderTarget>();
    }
    
    /* blur cube texture latitudinally and longitudinally */
    float weights[20];
    int size_lod = s;
    int max_lod = log2f(size_lod);
    for (int lod = 1; lod <= max_lod; ++lod) {
        
        /* calculate sigma radians */
        float sigma_radians = sqrtf(3) / size_lod;
        if (lod == 1) sigma_radians = 2.f / size_lod;
        
        /* calculate blur parameters */
        Vec3 pole_axis = DODECAHEDRON_AXES[(lod - 1) % 10];
        float d_theta = PI / (size_lod * 2 - 2);
        if (std::isinf(d_theta)) d_theta = PI;
        float sigma = sigma_radians / d_theta;
        int samples = 1 + floorf(sigma * 3);
        gaussian_weights(sigma, 20, weights);
        
        /* change the current viewport */
        gpu::State::set_viewport(gpu::Rect(size_lod / 2, size_lod / 2));
        
        /* blur texture latitudinally */
        for (int i = 0; i < 6; ++i) {
            blur_target->set_texture(*blur_map, 0, lod, i);
            gpu::RenderTarget::activate(blur_target.get());
            blur_shader->use_program();
            blur_shader->set_uniform_f("lod", lod - 1);
            blur_shader->set_uniform_i("face", i);
            blur_shader->set_uniform_i("samples", samples);
            blur_shader->set_uniform_i("latitudinal", 1);
            blur_shader->set_uniform_f("d_theta", d_theta);
            blur_shader->set_uniform_v3("pole_axis", pole_axis);
            blur_shader->set_uniform_i("map", m.activate(0));
            for (int w = 0; w < 20; ++w) {
                std::string weights_i = std::format("weights[{}]", w);
                blur_shader->set_uniform_f(weights_i, weights[w]);
            }
            fullscreen_plane->attach(*blur_shader);
            fullscreen_plane->render();
        }
        
        /* calculate blur parameters */
        size_lod /= 2;
        d_theta = PI / (size_lod * 2 - 2);
        if (std::isinf(d_theta)) d_theta = PI;
        sigma = sigma_radians / d_theta;
        samples = 1 + floorf(sigma * 3);
        gaussian_weights(sigma, 20, weights);
        
        /* blur texture longitudinally */
        for (int f = 0; f < 6; ++f) {
            blur_target->set_texture(m, 0, lod, f);
            gpu::RenderTarget::activate(blur_target.get());
            blur_shader->use_program();
            blur_shader->set_uniform_f("lod", lod);
            blur_shader->set_uniform_i("face", f);
            blur_shader->set_uniform_i("samples", samples);
            blur_shader->set_uniform_i("latitudinal", 0);
            blur_shader->set_uniform_f("d_theta", d_theta);
            blur_shader->set_uniform_v3("pole_axis", pole_axis);
            blur_shader->set_uniform_i("map", blur_map->activate(0));
            for (int w = 0; w < 20; ++w) {
                std::string weights_i = std::format("weights[{}]", w);
                blur_shader->set_uniform_f(weights_i, weights[w]);
            }
            fullscreen_plane->attach(*blur_shader);
            fullscreen_plane->render();
        }
    }
    
    /* set back to the default render target */
    gpu::RenderTarget::activate(nullptr);
}

void IBLFilter::init_fullscreen_plane() {
    /* prepare triangle mesh */
    Mesh triangle_mesh = Mesh("fullscreen");
    triangle_mesh.groups = {{"default", 0, 3}};
    triangle_mesh.vertex = {{-1, 3, 0}, {-1, -1, 0}, {3, -1, 0}};
    triangle_mesh.uv = {{0, 2}, {0, 0}, {2, 0}};
    
    /* prepare fullscreen plane vertex object */
    fullscreen_plane = std::make_unique<gpu::VertexObject>();
    fullscreen_plane->load(triangle_mesh, triangle_mesh.groups[0]);
}

void IBLFilter::gaussian_weights(float s, int n, float* w) {
    float weight_sum = 0;
    for (int i = 0; i < n; ++i) {
        float x = i / s;
        float weight = expf(-x * x * .5f);
        w[i] = weight;
        if (i == 0) {
            weight_sum += weight;
        } else {
            weight_sum += weight * 2;
        }
    }
    for (int i = 0; i < n; ++i) {
        w[i] /= weight_sum;
    }
}

std::unique_ptr<gpu::VertexObject> IBLFilter::fullscreen_plane;

std::unique_ptr<gpu::Texture> IBLFilter::blur_map;

std::unique_ptr<gpu::RenderTarget> IBLFilter::cubemap_target;

std::unique_ptr<gpu::RenderTarget> IBLFilter::blur_target;

}

/* -------------------------------------------------------------------------- */
/* ---- ink/probes/ReflectionProbe.cpp -------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

ReflectionProbe::ReflectionProbe(float i, int r, const Vec3& p) :
intensity(i), resolution(r), position(p) {}

void ReflectionProbe::load_cubemap(const Image& px, const Image& nx,
                                   const Image& py, const Image& ny,
                                   const Image& pz, const Image& nz) {
    if (!reflection_map) {
        reflection_map = std::make_unique<gpu::Texture>();
    }
    IBLFilter::load_cubemap(px, nx, py, ny, pz, nz, *reflection_map, resolution);
}

void ReflectionProbe::load_equirect(const Image& i) {
    if (!reflection_map) {
        reflection_map = std::make_unique<gpu::Texture>();
    }
    IBLFilter::load_equirect(i, *reflection_map, resolution);
}

void ReflectionProbe::load_texture(const gpu::Texture& t) {
    if (!reflection_map) {
        reflection_map = std::make_unique<gpu::Texture>();
    }
    IBLFilter::load_texture(t, *reflection_map, resolution);
}

int ReflectionProbe::activate(int l) const {
    return reflection_map->activate(l);
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/scene/Scene.cpp ------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

Scene::Scene(const std::string& n) : Instance(n) {}

Material* Scene::get_material(const std::string& n) const {
    if (material_library.count(n) == 0) {
        return nullptr;
    }
    return material_library.at(n);
}

Material* Scene::get_material(const std::string& n, const Mesh& s) const {
    auto name = std::format("M{}#{}", reinterpret_cast<size_t>(&s), n);
    if (material_library.count(name) == 0) {
        return nullptr;
    }
    return material_library.at(name);
}

Material* Scene::get_material(const std::string& n, const Instance& s) const {
    auto name = std::format("I{}#{}", reinterpret_cast<size_t>(&s), n);
    if (material_library.count(name) == 0) {
        return nullptr;
    }
    return material_library.at(name);
}

void Scene::set_material(const std::string& n, Material* m) {
    material_library.insert_or_assign(n, m);
}

void Scene::set_material(const std::string& n, const Mesh& s, Material* m) {
    auto name = std::format("M{}#{}", reinterpret_cast<size_t>(&s), n);
    material_library.insert_or_assign(name, m);
}

void Scene::set_material(const std::string& n, const Instance& s, Material* m) {
    auto name = std::format("I{}#{}", reinterpret_cast<size_t>(&s), n);
    material_library.insert_or_assign(name, m);
}

void Scene::remove_material(const std::string& n) {
    material_library.erase(n);
}

void Scene::remove_material(const std::string& n, const Mesh& s) {
    auto name = std::format("M{}#{}", reinterpret_cast<size_t>(&s), n);
    material_library.erase(name);
}

void Scene::remove_material(const std::string& n, const Instance& s) {
    auto name = std::format("I{}#{}", reinterpret_cast<size_t>(&s), n);
    material_library.erase(name);
}

void Scene::clear_materials() {
    material_library.clear();
}

std::vector<Material*> Scene::get_materials() const {
    std::vector<Material*> materials;
    for (auto& [name, material] : material_library) {
        materials.emplace_back(material);
    }
    return materials;
}

LinearFog* Scene::get_linear_fog() const {
    return linear_fog;
}

void Scene::set_fog(LinearFog* f) {
    linear_fog = f;
    exp2_fog = nullptr;
}

Exp2Fog* Scene::get_exp2_fog() const {
    return exp2_fog;
}

void Scene::set_fog(Exp2Fog* f) {
    exp2_fog = f;
    linear_fog = nullptr;
}

void Scene::add_light(PointLight* l) {
    point_lights.emplace_back(l);
}

void Scene::remove_light(PointLight* l) {
    std::erase(point_lights, l);
}

size_t Scene::get_point_light_count() const {
    return point_lights.size();
}

PointLight* Scene::get_point_light(int i) const {
    return point_lights[i];
}

void Scene::add_light(SpotLight* l) {
    spot_lights.emplace_back(l);
}

void Scene::remove_light(SpotLight* l) {
    std::erase(spot_lights, l);
}

size_t Scene::get_spot_light_count() const {
    return spot_lights.size();
}

SpotLight* Scene::get_spot_light(int i) const {
    return spot_lights[i];
}

void Scene::add_light(DirectionalLight* l) {
    directional_lights.emplace_back(l);
}

void Scene::remove_light(DirectionalLight* l) {
    std::erase(directional_lights, l);
}

size_t Scene::get_directional_light_count() const {
    return directional_lights.size();
}

DirectionalLight* Scene::get_directional_light(int i) const {
    return directional_lights[i];
}

void Scene::add_light(HemisphereLight* l) {
    hemisphere_lights.emplace_back(l);
}

void Scene::remove_light(HemisphereLight* l) {
    std::erase(hemisphere_lights, l);
}

size_t Scene::get_hemisphere_light_count() const {
    return hemisphere_lights.size();
}

HemisphereLight* Scene::get_hemisphere_light(int i) const {
    return hemisphere_lights[i];
}

void Scene::clear_lights() {
    point_lights.clear();
    spot_lights.clear();
    directional_lights.clear();
    hemisphere_lights.clear();
}

void Scene::update_instances() {
    std::vector<Instance*> unvisited;
    matrix_local = transform();
    matrix_global = matrix_local;
    unvisited.emplace_back(this);
    while (!unvisited.empty()) {
        Instance* current = unvisited.back();
        unvisited.pop_back();
        size_t count = current->get_child_count();
        for (int i = 0; i < count; ++i) {
            Instance* child = current->get_child(i);
            child->matrix_local = child->transform();
            child->matrix_global = current->matrix_global * child->matrix_local;
            unvisited.emplace_back(child);
        }
    }
}

std::vector<const Instance*> Scene::to_instances() const {
    std::vector<const Instance*> unvisited;
    std::vector<const Instance*> instances;
    unvisited.emplace_back(this);
    while (!unvisited.empty()) {
        const Instance* current = unvisited.back();
        unvisited.pop_back();
        size_t count = current->get_child_count();
        for (int i = 0; i < count; ++i) {
            unvisited.emplace_back(current->get_child(i));
        }
        instances.emplace_back(current);
    }
    return instances;
}

std::vector<const Instance*> Scene::to_visible_instances() const {
    std::vector<const Instance*> unvisited;
    std::vector<const Instance*> instances;
    unvisited.emplace_back(this);
    while (!unvisited.empty()) {
        const Instance* current = unvisited.back();
        unvisited.pop_back();
        if (!current->visible) continue;
        size_t count = current->get_child_count();
        for (int i = 0; i < count; ++i) {
            unvisited.emplace_back(current->get_child(i));
        }
        if (current->mesh != nullptr) {
            instances.emplace_back(current);
        }
    }
    return instances;
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/renderer/Renderer.cpp ------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr Vec3 CUBE_DIRECTIONS[] = {
    {-1, 0, 0}, {1, 0, 0}, {0, -1, 0}, {0, 1, 0}, {0, 0, -1}, {0, 0, 1},
};

constexpr Vec3 CUBE_UPS[] = {
    {0, -1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}, {0, -1, 0}, {0, -1, 0},
};

Vec4 Renderer::get_clear_color() const {
    return clear_color;
}

void Renderer::set_clear_color(const Vec4& c) {
    clear_color = c;
}

void Renderer::clear(bool c, bool d, bool s) const {
    gpu::RenderTarget::activate(target);
    gpu::State::set_clear_color(clear_color);
    gpu::State::clear(c, d, s);
    gpu::RenderTarget::activate(nullptr);
}

gpu::Rect Renderer::get_viewport() const {
    return viewport;
}

void Renderer::set_viewport(const gpu::Rect& v) {
    viewport = v;
}

bool Renderer::get_scissor_test() const {
    return scissor_test;
}

void Renderer::set_scissor_test(bool t) {
    scissor_test = t;
}

gpu::Rect Renderer::get_scissor() const {
    return scissor;
}

void Renderer::set_scissor(const gpu::Rect& s) {
    scissor = s;
}

RenderingMode Renderer::get_rendering_mode() const {
    return rendering_mode;
}

void Renderer::set_rendering_mode(RenderingMode m) {
    rendering_mode = m;
}

const gpu::RenderTarget* Renderer::get_target() const {
    return target;
}

void Renderer::set_target(const gpu::RenderTarget *t) {
    target = t;
}

void Renderer::set_texture_callback(const TextureCallback& f) {
    texture_callback = f;
}

float Renderer::get_skybox_intensity() const {
    return skybox_intensity;
}

void Renderer::set_skybox_intensity(float i) {
    skybox_intensity = i;
}

void Renderer::load_skybox(const Image& px, const Image& nx,
                           const Image& py, const Image& ny,
                           const Image& pz, const Image& nz) {
    skybox_map = std::make_unique<gpu::Texture>();
    skybox_map->init_cube(px, nx, py, ny, pz, nz,
                          gpu::Texture::default_format(px));
    skybox_map->set_filters(TEXTURE_LINEAR, TEXTURE_LINEAR);
}

void Renderer::load_skybox(const Image& i) {
    skybox_map = std::make_unique<gpu::Texture>();
    skybox_map->init_2d(i, gpu::Texture::default_format(i));
    skybox_map->set_filters(TEXTURE_LINEAR, TEXTURE_LINEAR);
}

void Renderer::render_skybox(const Camera& c) const {
    /* activate the render target */
    gpu::RenderTarget::activate(target);
    
    /* set the viewport region */
    gpu::State::set_viewport(viewport);
    
    /* set the scissor test and region */
    if (scissor_test) {
        gpu::State::enable_scissor_test();
        gpu::State::set_scissor(scissor);
    } else {
        gpu::State::disable_scissor_test();
    }
    
    /* render the skybox to buffer */
    render_skybox_to_buffer(c, rendering_mode);
    
    /* set back to the default render target */
    gpu::RenderTarget::activate(nullptr);
}

void Renderer::load_mesh(const Mesh& m) {
    if (mesh_cache.count(&m) != 0) return;
    size_t size = m.groups.size();
    auto p = mesh_cache.insert({&m, std::make_unique<gpu::VertexObject[]>(size)});
    auto* vertex_object = p.first->second.get();
    for (int i = 0; i < size; ++i) {
        vertex_object[i].load(m, m.groups[i]);
    }
}

void Renderer::unload_mesh(const Mesh& m) {
    mesh_cache.erase(&m);
}

void Renderer::clear_mesh_caches() {
    mesh_cache.clear();
}

void Renderer::load_image(const Image& i) {
    if (image_cache.count(&i) != 0) return;
    auto p = image_cache.insert({&i, std::make_unique<gpu::Texture>()});
    auto* texture = p.first->second.get();
    texture->init_2d(i, gpu::Texture::default_format(i));
    if (texture_callback) {
        std::invoke(texture_callback, *texture);
    }
}

void Renderer::unload_image(const Image& i) {
    image_cache.erase(&i);
}

void Renderer::clear_image_caches() {
    image_cache.clear();
}

void Renderer::load_scene(const Scene& s) {
    /* load the meshes linked with instance */
    for (auto& instance : s.to_instances()) {
        auto* mesh = instance->mesh;
        if (mesh != nullptr) load_mesh(*mesh);
    }
    
    /* load the images linked with instance */
    for (auto& material : s.get_materials()) {
        if (material->normal_map != nullptr) {
            load_image(*material->normal_map);
        }
        if (material->displacement_map != nullptr) {
            load_image(*material->displacement_map);
        }
        if (material->color_map != nullptr) {
            load_image(*material->color_map);
        }
        if (material->alpha_map != nullptr) {
            load_image(*material->alpha_map);
        }
        if (material->emissive_map != nullptr) {
            load_image(*material->emissive_map);
        }
        if (material->ao_map != nullptr) {
            load_image(*material->ao_map);
        }
        if (material->roughness_map != nullptr) {
            load_image(*material->roughness_map);
        }
        if (material->metalness_map != nullptr) {
            load_image(*material->metalness_map);
        }
        if (material->specular_map != nullptr) {
            load_image(*material->specular_map);
        }
        for (int i = 0; i < 16; ++i) {
            auto* image = material->custom_maps[i];
            if (image != nullptr) load_image(*image);
        }
    }
}

void Renderer::unload_scene(const Scene& s) {
    /* unload the meshes linked with instance */
    for (auto& instance : s.to_instances()) {
        auto* mesh = instance->mesh;
        if (mesh != nullptr) unload_mesh(*mesh);
    }
    
    /* unload the images linked with instance */
    for (auto& material : s.get_materials()) {
        if (material->normal_map != nullptr) {
            unload_image(*material->normal_map);
        }
        if (material->displacement_map != nullptr) {
            unload_image(*material->displacement_map);
        }
        if (material->color_map != nullptr) {
            unload_image(*material->color_map);
        }
        if (material->alpha_map != nullptr) {
            unload_image(*material->alpha_map);
        }
        if (material->emissive_map != nullptr) {
            unload_image(*material->emissive_map);
        }
        if (material->ao_map != nullptr) {
            unload_image(*material->ao_map);
        }
        if (material->roughness_map != nullptr) {
            unload_image(*material->roughness_map);
        }
        if (material->metalness_map != nullptr) {
            unload_image(*material->metalness_map);
        }
        if (material->specular_map != nullptr) {
            unload_image(*material->specular_map);
        }
        for (int i = 0; i < 16; ++i) {
            auto* image = material->custom_maps[i];
            if (image != nullptr) unload_image(*image);
        }
    }
}

void Renderer::clear_scene_caches() {
    mesh_cache.clear();
    image_cache.clear();
}

void Renderer::render(const Scene& s, const Camera& c) const {
    /* activate the render target */
    gpu::RenderTarget::activate(target);
    
    /* set the viewport region */
    gpu::State::set_viewport(viewport);
    
    /* set the scissor test and region */
    if (scissor_test) {
        gpu::State::enable_scissor_test();
        gpu::State::set_scissor(scissor);
    } else {
        gpu::State::disable_scissor_test();
    }
    
    /* render the scene to buffer */
    render_to_buffer(s, c, rendering_mode, false);
    
    /* set back to the default render target */
    gpu::RenderTarget::activate(nullptr);
}

void Renderer::render_transparent(const Scene& s, const Camera& c) const {
    /* activate the render target */
    gpu::RenderTarget::activate(target);
    
    /* set the viewport region */
    gpu::State::set_viewport(viewport);
    
    /* set the scissor test and region */
    if (scissor_test) {
        gpu::State::enable_scissor_test();
        gpu::State::set_scissor(scissor);
    } else {
        gpu::State::disable_scissor_test();
    }
    
    /* render the scene to buffer */
    gpu::State::set_depth_writemask(false);
    render_to_buffer(s, c, rendering_mode, true);
    gpu::State::set_depth_writemask(true);
    
    /* set back to the default render target */
    gpu::RenderTarget::activate(nullptr);
}

void Renderer::render_shadow(const Scene& s, const Shadow& t) const {
    /* activate the render target */
    gpu::RenderTarget::activate(t.get_target());
    
    /* set the viewport to fit the resolution of shadow map */
    Vec2 resolution = Shadow::get_resolution();
    gpu::State::set_viewport(gpu::Rect(resolution.x, resolution.y));
    
    /* disable the scissor test */
    gpu::State::disable_scissor_test();
    
    /* clear the shadow map (depth only) */
    gpu::State::clear(false, true, false);
    
    /* render the scene to shadow */
    render_to_shadow(s, t.camera);
    
    /* set back to the default render target */
    gpu::RenderTarget::activate(nullptr);
}

void Renderer::update_shadow(const Scene& s, SpotLight& l) const {
    Vec3 up = l.direction.cross({0, 0, 1});
    if (up.magnitude() < 1E-4) up = {0, 1, 0};
    l.shadow.camera.lookat(l.position, -l.direction, up);
    render_shadow(s, l.shadow);
}

void Renderer::update_shadow(const Scene& s, DirectionalLight& l) const {
    Vec3 up = l.direction.cross({0, 0, 1});
    if (up.magnitude() < 1E-4) up = {0, 1, 0};
    l.shadow.camera.lookat(l.position, -l.direction, up);
    render_shadow(s, l.shadow);
}

void Renderer::update_probe(const Scene& s, ReflectionProbe& r) const {
    /* set the viewport with the resolution of probe */
    gpu::State::set_viewport(gpu::Rect(r.resolution, r.resolution));
    
    /* disable the scissor test */
    gpu::State::disable_scissor_test();
    
    /* prepare texture for probe */
    if (!probe_map) {
        probe_map = std::make_unique<gpu::Texture>();
    }
    probe_map->init_cube(r.resolution, r.resolution, TEXTURE_R8G8B8A8_UNORM);
    probe_map->set_filters(TEXTURE_LINEAR, TEXTURE_LINEAR);
    
    /* prepare depth texture for rendering */
    if (!probe_buffer) {
        probe_buffer = std::make_unique<gpu::Renderbuffer>();
    }
    probe_buffer->init(r.resolution, r.resolution, TEXTURE_D24_UNORM);
    
    /* prepare frame buffer for probe */
    if (!probe_target) {
        probe_target = std::make_unique<gpu::RenderTarget>();
    }
    probe_target->set_depth_buffer(*probe_buffer);
    
    /* initialize camera for probe */
    PerspCamera camera = PerspCamera(PI_2, 1, 0.1, 100);
    
    for (int i = 0; i < 6; ++i) {
        /* update camera for each side of cube */
        camera.lookat(r.position, CUBE_DIRECTIONS[i], CUBE_UPS[i]);
        
        /* update and activate render target */
        probe_target->set_texture(*probe_map, 0, 0, i);
        gpu::RenderTarget::activate(probe_target.get());
        
        /* clear depth buffer before rendering */
        gpu::State::set_clear_color(clear_color);
        gpu::State::clear(true, true, false);
        
        /* render skybox loaded by renderer */
        if (skybox_map) {
            render_skybox_to_buffer(camera, FORWARD_RENDERING);
        }
        
        /* render the opaque objects in scene */
        render_to_buffer(s, camera, FORWARD_RENDERING, false);
        
        /* render the transparent objects in scene */
        gpu::State::set_depth_writemask(false);
        render_to_buffer(s, camera, FORWARD_RENDERING, true);
        gpu::State::set_depth_writemask(true);
    }
    
    /* set back to the default render target */
    gpu::RenderTarget::activate(nullptr);
    
    /* update probe with probe texture */
    r.load_texture(*probe_map);
}

void Renderer::update_scene(Scene& s) {
    s.update_instances();
}

void Renderer::set_material_defines(const Material& m, Defines& d) {
    /* check whether to use vertex color */
    d.set_if("USE_VERTEX_COLOR", m.use_vertex_color);
    
    /* check whether to use normal map */
    d.set_if("USE_NORMAL_MAP", m.normal_map != nullptr);
    
    /* check whether to use normal map in tangent space */
    d.set_if("USE_TANGENT_SPACE", m.normal_map != nullptr && m.use_tangent_space);
    
    /* check whether to use normal map in object space */
    d.set_if("USE_OBJECT_SPACE", m.normal_map != nullptr && !m.use_tangent_space);
    
    /* check whether to use displacement map */
    d.set_if("USE_DISPLACEMENT_MAP", m.displacement_map != nullptr);
    
    /* check whether to use color map */
    d.set_if("USE_COLOR_MAP", m.color_map != nullptr && !m.use_map_with_alpha);
    
    /* check whether to use color map with alpha channel */
    d.set_if("USE_COLOR_ALPHA_MAP", m.color_map != nullptr && m.use_map_with_alpha);
    
    /* check whether to use alpha map */
    d.set_if("USE_ALPHA_MAP", m.alpha_map != nullptr);
    
    /* check whether to use emissive map */
    d.set_if("USE_EMISSIVE_MAP", m.emissive_map != nullptr);
    
    /* check whether to use ambient occlusion map */
    d.set_if("USE_AO_MAP", m.ao_map != nullptr);
    
    /* check whether to use metalness map */
    d.set_if("USE_METALNESS_MAP", m.metalness_map != nullptr);
    
    /* check whether to use roughness map */
    d.set_if("USE_ROUGHNESS_MAP", m.roughness_map != nullptr);
    
    /* check whether to use specular map */
    d.set_if("USE_SPECULAR_MAP", m.specular_map != nullptr);
    
    /* check whether to use reflection probe */
    d.set_if("USE_REFLECTION_PROBE", m.reflection_probe != nullptr);
}

void Renderer::set_scene_defines(const Scene& s, Defines& d) {
    /* set the sample numbers of shadow */
    d.set_i("SHADOW_SAMPLES", Shadow::get_samples());
    
    /* set the number of point lights */
    size_t light_count = s.get_point_light_count();
    d.set_l("NUM_POINT_LIGHT", light_count);
    
    /* set the number of spot lights */
    light_count = s.get_spot_light_count();
    d.set_l("NUM_SPOT_LIGHT", light_count);
    
    /* set the number of directional lights */
    light_count = s.get_directional_light_count();
    d.set_l("NUM_DIRECTIONAL_LIGHT", light_count);
    
    /* set the number of hemisphere lights */
    light_count = s.get_hemisphere_light_count();
    d.set_l("NUM_HEMISPHERE_LIGHT", light_count);
    
    /* set whether to use linear fog */
    d.set_if("USE_LINEAR_FOG", s.get_linear_fog() != nullptr);
    
    /* set whether to use exp square fog */
    d.set_if("USE_EXP2_FOG", s.get_exp2_fog() != nullptr);
}

void Renderer::set_light_uniforms(const Scene& s, const gpu::Shader& shader) {
    /* determines whether to enable shadow */
    bool enable_shadow = false;
    
    /* apply point lights in lighting */
    size_t point_light_count = s.get_point_light_count();
    for (int i = 0; i < point_light_count; ++i) {
        
        /* pass the light information to shader */
        auto& light = *s.get_point_light(i);
        auto lights_i = std::format("point_lights[{}]", i);
        Vec3 light_color = light.color * light.intensity * PI;
        shader.set_uniform_i(lights_i + ".visible", light.visible);
        shader.set_uniform_v3(lights_i + ".position", light.position);
        shader.set_uniform_v3(lights_i + ".color", light_color);
        shader.set_uniform_f(lights_i + ".distance", light.distance);
        shader.set_uniform_f(lights_i + ".decay", light.decay);
    }
    
    /* apply spot lights in lighting */
    size_t spot_light_count = s.get_spot_light_count();
    for (int i = 0; i < spot_light_count; ++i) {
        
        /* pass the light information to shader */
        auto& light = *s.get_spot_light(i);
        std::string lights_i = std::format("spot_lights[{}]", i);
        Vec3 light_direction = -light.direction.normalize();
        Vec3 light_color = light.color * light.intensity * PI;
        float light_angle = cosf(light.angle);
        float light_penumbra = cosf(light.angle * (1 - light.penumbra));
        shader.set_uniform_i(lights_i + ".visible", light.visible);
        shader.set_uniform_v3(lights_i + ".position", light.position);
        shader.set_uniform_v3(lights_i + ".direction", light_direction);
        shader.set_uniform_v3(lights_i + ".color", light_color);
        shader.set_uniform_f(lights_i + ".distance", light.distance);
        shader.set_uniform_f(lights_i + ".decay", light.decay);
        shader.set_uniform_f(lights_i + ".angle", light_angle);
        shader.set_uniform_f(lights_i + ".penumbra", light_penumbra);
        shader.set_uniform_i(lights_i + ".cast_shadow", light.cast_shadow);
        
        /* check whether the light cast shadow */
        if (!light.cast_shadow) continue;
        enable_shadow = true;
        
        /* pass the shadow information to shader */
        auto& shadow = light.shadow;
        std::string shadows_i = std::format("spot_lights[{}].shadow", i);
        Mat4 view_proj = shadow.camera.projection * shadow.camera.viewing;
        shader.set_uniform_i(shadows_i + ".type", shadow.type);
        shader.set_uniform_i(shadows_i + ".map_id", shadow.map_id);
        shader.set_uniform_f(shadows_i + ".bias", shadow.bias);
        shader.set_uniform_f(shadows_i + ".normal_bias", shadow.normal_bias);
        shader.set_uniform_f(shadows_i + ".radius", shadow.radius);
        shader.set_uniform_m4(shadows_i + ".view_proj", view_proj);
    }
    
    /* apply directional lights in lighting */
    size_t directional_light_count = s.get_directional_light_count();
    for (int i = 0; i < directional_light_count; ++i) {
        
        /* pass the light information to shader */
        auto& light = *s.get_directional_light(i);
        std::string lights_i = std::format("directional_lights[{}]", i);
        Vec3 light_direction = -light.direction.normalize();
        Vec3 light_color = light.color * light.intensity * PI;
        shader.set_uniform_i(lights_i + ".visible", light.visible);
        shader.set_uniform_v3(lights_i + ".direction", light_direction);
        shader.set_uniform_v3(lights_i + ".color", light_color);
        shader.set_uniform_i(lights_i + ".cast_shadow", light.cast_shadow);
        
        /* check whether the light cast shadow */
        if (!light.cast_shadow) continue;
        enable_shadow = true;
        
        /* pass the shadow information to shader */
        auto& shadow = light.shadow;
        std::string shadows_i = std::format("directional_lights[{}].shadow", i);
        Mat4 view_proj = shadow.camera.projection * shadow.camera.viewing;
        shader.set_uniform_i(shadows_i + ".type", shadow.type);
        shader.set_uniform_i(shadows_i + ".map_id", shadow.map_id);
        shader.set_uniform_f(shadows_i + ".bias", shadow.bias);
        shader.set_uniform_f(shadows_i + ".normal_bias", shadow.normal_bias);
        shader.set_uniform_f(shadows_i + ".radius", shadow.radius);
        shader.set_uniform_m4(shadows_i + ".view_proj", view_proj);
    }
    
    /* apply hemisphere lights in lighting */
    size_t hemisphere_light_count = s.get_hemisphere_light_count();
    for (int i = 0; i < hemisphere_light_count; ++i) {
        
        /* pass the light information to shader */
        auto& light = *s.get_hemisphere_light(i);
        std::string lights_i = std::format("hemisphere_lights[{}]", i);
        Vec3 light_sky_color = light.color * light.intensity * PI;
        Vec3 light_ground_color = light.ground_color * light.intensity * PI;
        shader.set_uniform_i(lights_i + ".visible", light.visible);
        shader.set_uniform_v3(lights_i + ".direction", light.direction);
        shader.set_uniform_v3(lights_i + ".sky_color", light_sky_color);
        shader.set_uniform_v3(lights_i + ".ground_color", light_ground_color);
    }
    
    /* pass the shadow parameters to shader */
    if (enable_shadow) Shadow::activate_texture(26);
    shader.set_uniform_i("global_shadow.map", 26);
    shader.set_uniform_v2("global_shadow.size", Shadow::get_resolution());
    
    /* pass the linear fog parameters to shader */
    auto* linear_fog = s.get_linear_fog();
    if (linear_fog != nullptr) {
        shader.set_uniform_i("fog.visible", linear_fog->visible);
        shader.set_uniform_v3("fog.color", linear_fog->color);
        shader.set_uniform_f("fog.near", linear_fog->near);
        shader.set_uniform_f("fog.far", linear_fog->far);
    }
    
    /* pass the exp square fog parameters to shader */
    auto* exp2_fog = s.get_exp2_fog();
    if (exp2_fog != nullptr) {
        shader.set_uniform_i("fog.visible", exp2_fog->visible);
        shader.set_uniform_v3("fog.color", exp2_fog->color);
        shader.set_uniform_f("fog.near", exp2_fog->near);
        shader.set_uniform_f("fog.density", exp2_fog->density);
    }
}

void Renderer::render_skybox_to_buffer(const Camera& c, RenderingMode r) const {
    /* initialize cube vertex object */
    if (!cube) init_cube();
    
    /* set the states of GPU pipeline */
    gpu::State::disable_depth_test();
    gpu::State::disable_stencil_test();
    gpu::State::disable_blending();
    gpu::State::enable_culling();
    gpu::State::set_cull_side(FRONT_SIDE);
    gpu::State::disable_wireframe();
    gpu::State::enable_texture_cube_seamless();
    
    /* fetch the cube shader from shader lib */
    Defines cube_defines;
    int type = skybox_map->get_type();
    if (type == TEXTURE_2D) {
        cube_defines.set("USE_EQUIRECT");
    } else if (type == TEXTURE_CUBE) {
        cube_defines.set("USE_CUBEMAP");
    }
    auto* cube_shader = ShaderLib::fetch("Cube", cube_defines);
    
    /* calculate transform matrices */
    Mat4 viewing = c.viewing;
    for (int i = 0; i < 3; ++i) {
        viewing[i][3] = 0;
    }
    Mat4 view_proj = c.projection * viewing;
    
    /* render to the render target */
    cube_shader->use_program();
    cube_shader->set_uniform_m4("view_proj", view_proj);
    cube_shader->set_uniform_f("intensity", skybox_intensity);
    cube_shader->set_uniform_i("map", skybox_map->activate(0));
    cube->attach(*cube_shader);
    cube->render();
}

void Renderer::render_to_buffer(const Scene& s, const Camera& c, RenderingMode r, bool t) const {
    /* create transform matrices & vectors */
    Mat4 model;
    Mat4 view = c.viewing;
    Mat4 proj = c.projection;
    Mat4 model_view;
    Mat4 model_view_proj;
    Mat3 normal_mat;
    Vec3 camera_pos = c.position;
    
    /* render all the visible instances in sorted order */
    auto visible_instances = s.to_visible_instances();
    sort_instances(c, visible_instances, t);
    for (auto& instance : visible_instances) {
        
        /* get matrices from instance */
        model = instance->matrix_global;
        model_view = view * model;
        model_view_proj = proj * model_view;
        normal_mat = inverse_3x3(Mat3{
            model[0][0], model[1][0], model[2][0],
            model[0][1], model[1][1], model[2][1],
            model[0][2], model[1][2], model[2][2],
        });
        
        /* get mesh from instance */
        auto* mesh = instance->mesh;
        
        /* check whether the scene is loaded */
        if (mesh_cache.count(mesh) == 0) {
            Error::set("Renderer", "Scene is not loaded");
            continue;
        }
        
        /* get vertex objects from mesh cache */
        auto* vertex_object = mesh_cache.at(mesh).get();
        size_t group_size = mesh->groups.size();
        for (int i = 0; i < group_size; ++i) {
            
            /* get material from material groups */
            auto& group = mesh->groups[i];
            auto* material = s.get_material(group.name, *instance);
            if (material == nullptr) {
                material = s.get_material(group.name, *mesh);
            }
            if (material == nullptr) {
                material = s.get_material(group.name);
            }
            if (material == nullptr) {
                Error::set("Renderer", "Material is not linked");
                continue;
            }
            
            /* check whether the material is visible */
            if (!material->visible) continue;
            
            /* check whether the material is transparent */
            bool is_transparent = material->blending;
            if (is_transparent != t) continue;
            
            /* fetch the standard shader from shader lib */
            auto* standard_shader = static_cast<const gpu::Shader*>(material->shader);
            if (standard_shader == nullptr) {
                Defines standard_defines;
                set_material_defines(*material, standard_defines);
                if (!t && r == DEFERRED_RENDERING) {
                    standard_defines.set("DEFERRED_RENDERING");
                } else {
                    standard_defines.set("FORWARD_RENDERING");
                    set_scene_defines(s, standard_defines);
                }
                standard_shader = ShaderLib::fetch("Standard", standard_defines);
            }
            
            /* render vertex object with shader */
            standard_shader->use_program();
            vertex_object[i].attach(*standard_shader);
            
            if (is_transparent || r == FORWARD_RENDERING) {
                /* pass the camera parameters to shader */
                Mat4 inv_view_proj = inverse_4x4(c.projection * c.viewing);
                standard_shader->set_uniform_m4("inv_view_proj", inv_view_proj);
                
                /* pass the lights & fogs parameters to shader */
                set_light_uniforms(s, *standard_shader);
            }
            
            /* pass the renderer parameters to shader */
            standard_shader->set_uniform_m4("model"          , model          );
            standard_shader->set_uniform_m4("view"           , view           );
            standard_shader->set_uniform_m4("proj"           , proj           );
            standard_shader->set_uniform_m4("model_view"     , model_view     );
            standard_shader->set_uniform_m4("model_view_proj", model_view_proj);
            standard_shader->set_uniform_m3("normal_mat"     , normal_mat     );
            standard_shader->set_uniform_v3("camera_pos"     , camera_pos     );
            
            /* pass the material parameters to shader */
            standard_shader->set_uniform_v3("color"      , material->color       );
            standard_shader->set_uniform_f("alpha_test"  , material->alpha_test  );
            standard_shader->set_uniform_f("alpha"       , material->alpha       );
            standard_shader->set_uniform_f("ao_intensity", material->ao_intensity);
            standard_shader->set_uniform_f("specular"    , material->specular    );
            standard_shader->set_uniform_f("metalness"   , material->metalness   );
            standard_shader->set_uniform_f("roughness"   , material->roughness   );
            
            /* pass the emissive parameter to shader */
            Vec3 emissive = material->emissive * material->emissive_intensity;
            standard_shader->set_uniform_v3("emissive", emissive);
            
            /* pass the normal scale if use normal map */
            if (material->normal_map != nullptr) {
                standard_shader->set_uniform_f("normal_scale", material->normal_scale);
            }
            
            /* pass the displacement scale if use displacement map */
            if (material->displacement_map != nullptr) {
                standard_shader->set_uniform_f("displacement_scale", material->displacement_scale);
            }
            
            /* pass the images linked with material */
            for (int j = 0; j < 16; ++j) {
                auto* image = material->custom_maps[j];
                if (image != nullptr) image_cache.at(image)->activate(j);
            }
            if (material->normal_map != nullptr) {
                auto& map = image_cache.at(material->normal_map);
                standard_shader->set_uniform_i("normal_map", map->activate(16));
            }
            if (material->displacement_map != nullptr) {
                auto& map = image_cache.at(material->displacement_map);
                standard_shader->set_uniform_i("displacement_map", map->activate(17));
            }
            if (material->color_map != nullptr) {
                auto& map = image_cache.at(material->color_map);
                standard_shader->set_uniform_i("color_map", map->activate(18));
            }
            if (material->alpha_map != nullptr) {
                auto& map = image_cache.at(material->alpha_map);
                standard_shader->set_uniform_i("alpha_map", map->activate(19));
            }
            if (material->emissive_map != nullptr) {
                auto& map = image_cache.at(material->emissive_map);
                standard_shader->set_uniform_i("emissive_map", map->activate(20));
            }
            if (material->ao_map != nullptr) {
                auto& map = image_cache.at(material->ao_map);
                standard_shader->set_uniform_i("ao_map", map->activate(21));
            }
            if (material->roughness_map != nullptr) {
                auto& map = image_cache.at(material->roughness_map);
                standard_shader->set_uniform_i("roughness_map", map->activate(22));
            }
            if (material->metalness_map != nullptr) {
                auto& map = image_cache.at(material->metalness_map);
                standard_shader->set_uniform_i("metalness_map", map->activate(23));
            }
            if (material->specular_map != nullptr) {
                auto& map = image_cache.at(material->specular_map);
                standard_shader->set_uniform_i("specular_map", map->activate(24));
            }
            
            /* pass the reflection probe linked with material */
            auto* ref_probe = static_cast<const ReflectionProbe*>(material->reflection_probe);
            if (ref_probe != nullptr) {
                float ref_lod = log2f(ref_probe->resolution);
                standard_shader->set_uniform_i("ref_map", ref_probe->activate(25));
                standard_shader->set_uniform_f("ref_lod", ref_lod);
                standard_shader->set_uniform_f("ref_intensity", ref_probe->intensity);
            }
            
            /* pass the custom uniforms linked with material */
            if (material->uniforms != nullptr) {
                standard_shader->set_uniforms(*material->uniforms);
            }
            
            /* set the states of GPU pipeline by material */
            gpu::MaterialState::set_depth(*material);
            gpu::MaterialState::set_stencil(*material);
            gpu::MaterialState::set_blending(*material);
            gpu::MaterialState::set_side(*material);
            gpu::MaterialState::set_wireframe(*material);
            
            /* cull back side of face if side is DOUBLE_SIDE */
            if (is_transparent && material->side == DOUBLE_SIDE) {
                gpu::State::enable_culling();
                gpu::State::set_cull_side(BACK_SIDE);
            }
            
            /* render the vertex object at group i */
            vertex_object[i].render();
        }
    }
}

void Renderer::render_to_shadow(const Scene& s, const Camera& c) const {
    /* create transform matrices & vectors */
    Mat4 model;
    Mat4 view = c.viewing;
    Mat4 proj = c.projection;
    Mat4 model_view;
    Mat4 model_view_proj;
    
    /* render all the visible instances in sorted order */
    auto visible_instances = s.to_visible_instances();
    sort_instances(c, visible_instances, false);
    for (auto& instance : visible_instances) {
        
        /* check whether the instance casts shadow */
        if (!instance->cast_shadow) continue;
        
        /* get matrices from instance */
        model = instance->matrix_global;
        model_view = view * model;
        model_view_proj = proj * model_view;
        
        /* get mesh from instance */
        auto* mesh = instance->mesh;
        
        /* check whether the scene is loaded */
        if (mesh_cache.count(mesh) == 0) {
            Error::set("Renderer", "Scene is not loaded");
            continue;
        }
        
        /* get vertex objects from cache */
        auto* vertex_object = mesh_cache.at(mesh).get();
        size_t group_size = mesh->groups.size();
        for (int i = 0; i < group_size; ++i) {
            
            /* get material from material groups */
            auto& group = mesh->groups[i];
            auto* material = s.get_material(group.name, *instance);
            if (material == nullptr) {
                material = s.get_material(group.name, *mesh);
            }
            if (material == nullptr) {
                material = s.get_material(group.name);
            }
            if (material == nullptr) {
                Error::set("Renderer", "Material is not linked");
                continue;
            }
            
            /* check whether the material is visible */
            if (!material->visible) continue;
            
            /* check whether the material is transparent */
            if (material->blending) continue;
            
            /* whether to use color map and alpha map */
            bool use_color_map = material->color_map != nullptr && material->use_map_with_alpha;
            bool use_alpha_map = material->alpha_map != nullptr;
            
            /* fetch the shadow shader from shader lib */
            Defines shadow_defines;
            shadow_defines.set_if("USE_COLOR_MAP", use_color_map);
            shadow_defines.set_if("USE_ALPHA_MAP", use_alpha_map);
            auto* shadow_shader = ShaderLib::fetch("Shadow", shadow_defines);
            
            /* render vertex object with shader */
            shadow_shader->use_program();
            vertex_object[i].attach(*shadow_shader);
            
            /* pass the renderer parameters to shader */
            shadow_shader->set_uniform_m4("model_view_proj", model_view_proj);
            
            /* pass the material parameters to shader */
            shadow_shader->set_uniform_f("alpha", material->alpha);
            shadow_shader->set_uniform_f("alpha_test", material->alpha_test);
            shadow_shader->set_uniform_i("color_map", 0);
            shadow_shader->set_uniform_i("alpha_map", 1);
            
            /* activate color map linked with material */
            if (use_color_map) {
                image_cache.at(material->color_map)->activate(0);
            }
            if (use_alpha_map) {
                image_cache.at(material->alpha_map)->activate(1);
            }
            
            /* set the states of GPU pipeline by material */
            gpu::MaterialState::set_depth(*material);
            gpu::MaterialState::set_stencil(*material);
            gpu::MaterialState::set_blending(*material);
            gpu::MaterialState::set_shadow_side(*material);
            gpu::MaterialState::set_wireframe(*material);
            
            /* render the vertex object for group i */
            vertex_object[i].render();
        }
    }
}

void Renderer::init_cube() {
    cube = std::make_unique<gpu::VertexObject>();
    Mesh box = BoxMesh::create();
    cube->load(box, box.groups[0]);
}

void Renderer::sort_instances(const Camera& c, std::vector<const Instance*>& l, bool t) {
    using InstanceItem = std::pair<const Instance*, float>;
    std::vector<InstanceItem> sorted_instances;
    auto instances_size = l.size();
    sorted_instances.resize(instances_size);
    for (int i = 0; i < instances_size; ++i) {
        Vec3 position = l[i]->local_to_global({});
        sorted_instances[i].first = l[i];
        sorted_instances[i].second = (c.position - position).dot(c.direction);
    }
    auto compare = t ?
    std::function([](const InstanceItem& a, const InstanceItem& b) -> bool {
        if (a.first->priority != b.first->priority) {
            return a.first->priority < b.first->priority;
        }
        return a.second > b.second;
    }) :
    std::function([](const InstanceItem& a, const InstanceItem& b) -> bool {
        if (a.first->priority != b.first->priority) {
            return a.first->priority < b.first->priority;
        }
        return a.second < b.second;
    });
    std::stable_sort(sorted_instances.begin(), sorted_instances.end(), compare);
    for (int i = 0; i < instances_size; ++i) {
        l[i] = sorted_instances[i].first;
    }
}

std::unique_ptr<gpu::VertexObject> Renderer::cube;

std::unique_ptr<gpu::Texture> Renderer::probe_map;

std::unique_ptr<gpu::Renderbuffer> Renderer::probe_buffer;

std::unique_ptr<gpu::RenderTarget> Renderer::probe_target;

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/RenderPass.cpp -------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

const gpu::RenderTarget* RenderPass::get_target() const {
    return target;
}

void RenderPass::set_target(const gpu::RenderTarget* t) {
    target = t;
}

gpu::Rect RenderPass::get_viewport() {
    return viewport;
}

void RenderPass::set_viewport(const gpu::Rect& v) {
    viewport = v;
}

void RenderPass::render_to(const gpu::Shader* s, const gpu::RenderTarget* t) {
    /* initialize fullscreen plane vertex object */
    if (!fullscreen_plane) init_fullscreen_plane();
    
    /* activate render target */
    gpu::RenderTarget::activate(t);
    
    /* set the states of GPU pipeline */
    gpu::State::disable_depth_test();
    gpu::State::disable_stencil_test();
    gpu::State::disable_scissor_test();
    gpu::State::disable_blending();
    gpu::State::disable_culling();
    gpu::State::disable_wireframe();
    
    /* set the viewport region */
    gpu::State::set_viewport(viewport);
    
    /* draw the fullscreen plane with shader */
    fullscreen_plane->attach(*s);
    fullscreen_plane->render();
    
    /* set to default render target */
    gpu::RenderTarget::activate(nullptr);
}

void RenderPass::init_fullscreen_plane() {
    /* prepare triangle mesh */
    Mesh triangle_mesh = Mesh("fullscreen");
    triangle_mesh.groups = {{"default", 0, 3}};
    triangle_mesh.vertex = {{-1, 3, 0}, {-1, -1, 0}, {3, -1, 0}};
    triangle_mesh.uv = {{0, 2}, {0, 0}, {2, 0}};
    
    /* prepare fullscreen plane vertex object */
    fullscreen_plane = std::make_unique<gpu::VertexObject>();
    fullscreen_plane->load(triangle_mesh, triangle_mesh.groups[0]);
}

gpu::Rect RenderPass::viewport;

std::unique_ptr<gpu::VertexObject> RenderPass::fullscreen_plane;

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/BlendPass.cpp --------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

void BlendPass::init() {}

void BlendPass::render() {
    Defines blend_defines;
    blend_defines.set("BLEND_OP(a, b)", operation);
    blend_defines.set("A_SWIZZLE", swizzle_a);
    blend_defines.set("B_SWIZZLE", swizzle_b);
    auto* blend_shader = ShaderLib::fetch("Blend", blend_defines);
    blend_shader->use_program();
    blend_shader->set_uniform_i("map_a", map_a->activate(0));
    blend_shader->set_uniform_i("map_b", map_b->activate(1));
    RenderPass::render_to(blend_shader, target);
}

const gpu::Texture* BlendPass::get_texture_a() const {
    return map_a;
}

void BlendPass::set_texture_a(const gpu::Texture* t) {
    map_a = t;
}

const gpu::Texture* BlendPass::get_texture_b() const {
    return map_b;
}

void BlendPass::set_texture_b(const gpu::Texture* t) {
    map_b = t;
}

std::string BlendPass::get_operation() const {
    return operation;
}

void BlendPass::set_operation(const std::string& o) {
    operation = o;
}

std::string BlendPass::get_swizzle_a() const {
    return swizzle_a;
}

void BlendPass::set_swizzle_a(const std::string& s) {
    swizzle_a = s;
}

std::string BlendPass::get_swizzle_b() const {
    return swizzle_b;
}

void BlendPass::set_swizzle_b(const std::string& s) {
    swizzle_b = s;
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/BloomPass.cpp --------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

BloomPass::BloomPass(int w, int h, float t, float i, float r) :
width(w), height(h), threshold(t), intensity(i), radius(r) {}

void BloomPass::init() {
    /* check the width and height */
    if (width == 0 || height == 0) {
        return Error::set("BloomPass", "Width and height should be greater than 0");
    }
    
    /* prepare bloom map 1 */
    bloom_map_1 = std::make_unique<gpu::Texture>();
    bloom_map_1->init_2d(width / 2, height / 2, TEXTURE_R16G16B16_SFLOAT);
    bloom_map_1->set_filters(TEXTURE_LINEAR, TEXTURE_LINEAR_MIPMAP_LINEAR);
    bloom_map_1->set_wrap_all(TEXTURE_CLAMP_TO_EDGE);
    bloom_map_1->generate_mipmap();
    
    /* prepare bloom map 2 */
    bloom_map_2 = std::make_unique<gpu::Texture>();
    bloom_map_2->init_2d(width / 2, height / 2, TEXTURE_R16G16B16_SFLOAT);
    bloom_map_2->set_filters(TEXTURE_LINEAR, TEXTURE_LINEAR_MIPMAP_LINEAR);
    bloom_map_2->set_wrap_all(TEXTURE_CLAMP_TO_EDGE);
    bloom_map_2->generate_mipmap();
    
    /* prepare bloom render target */
    bloom_target = std::make_unique<gpu::RenderTarget>();
}

void BloomPass::render() {
    /* fetch bright pass shader from shader lib */
    auto* bright_pass_shader = ShaderLib::fetch("BrightPass");
    
    /* fetch blur shader from shader lib */
    Defines blur_defines;
    blur_defines.set("TYPE", "vec3");
    blur_defines.set("SWIZZLE", ".xyz");
    auto* blur_shader = ShaderLib::fetch("GaussianBlur", blur_defines);
    
    /* fetch bloom shader from shader lib */
    auto* bloom_shader = ShaderLib::fetch("Bloom");
    
    /* change the current viewport */
    gpu::Rect viewport = RenderPass::get_viewport();
    RenderPass::set_viewport(gpu::Rect(width / 2, height / 2));
    
    /* 1. render bright pixels to blur map 1 */
    bright_pass_shader->use_program();
    bright_pass_shader->set_uniform_f("threshold", threshold);
    bright_pass_shader->set_uniform_i("map", map->activate(0));
    bloom_target->set_texture(*bloom_map_1, 0, 0);
    RenderPass::render_to(bright_pass_shader, bloom_target.get());
    
    /* initialize size lod */
    Vec2 size_lod = Vec2(width / 2, height / 2);
    
    /* 2. blur texture on the mipmap chain */
    for (int lod = 0; lod < 5; ++lod) {
        int sigma = lod * 2 + 3;
        
        /* set the viewport of size lod */
        RenderPass::set_viewport(gpu::Rect(size_lod.x, size_lod.y));
        
        /* blur texture horizontally */
        blur_shader->use_program();
        blur_shader->set_uniform_f("lod", fmax(0, lod - 1));
        blur_shader->set_uniform_v2("direction", {1 / size_lod.x, 0});
        blur_shader->set_uniform_i("radius", sigma * 3);
        blur_shader->set_uniform_f("sigma_s", sigma);
        blur_shader->set_uniform_i("map", bloom_map_1->activate(0));
        bloom_target->set_texture(*bloom_map_2, 0, lod);
        RenderPass::render_to(blur_shader, bloom_target.get());
        
        /* blur texture vertically */
        blur_shader->use_program();
        blur_shader->set_uniform_f("lod", lod);
        blur_shader->set_uniform_v2("direction", {0, 1 / size_lod.y});
        blur_shader->set_uniform_i("radius", sigma * 3);
        blur_shader->set_uniform_f("sigma_s", sigma);
        blur_shader->set_uniform_i("map", bloom_map_2->activate(0));
        bloom_target->set_texture(*bloom_map_1, 0, lod);
        RenderPass::render_to(blur_shader, bloom_target.get());
        
        /* update size lod to lower lod */
        size_lod.x = fmax(1, floorf(size_lod.x / 2));
        size_lod.y = fmax(1, floorf(size_lod.y / 2));
    }
    
    /* set back to the initial viewport */
    RenderPass::set_viewport(viewport);
    
    /* 3. blur texture on the mipmap chain */
    bloom_shader->use_program();
    bloom_shader->set_uniform_v3("tint", tint);
    bloom_shader->set_uniform_f("intensity", intensity);
    bloom_shader->set_uniform_f("radius", radius);
    bloom_shader->set_uniform_i("map", map->activate(0));
    bloom_shader->set_uniform_i("bloom_map", bloom_map_1->activate(1));
    RenderPass::render_to(bloom_shader, target);
}

const gpu::Texture* BloomPass::get_texture() const {
    return map;
}

void BloomPass::set_texture(const gpu::Texture* t) {
    map = t;
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/BlurPass.cpp ---------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

const std::string TYPES[] = {
    "float", "vec2", "vec3", "vec4"
};

const std::string SWIZZLES[] = {
    ".x", ".xy", ".xyz", ".xyzw"
};

BlurPass::BlurPass(int w, int h) : width(w), height(h) {}

void BlurPass::init() {
    /* check the width and height */
    if (width == 0 || height == 0) {
        return Error::set("BlurPass", "Width and height should be greater than 0");
    }
    
    /* get default format with channel */
    TextureFormat format = gpu::Texture::default_format(channel, 1);
    
    /* prepare blur map 1 */
    blur_map_1 = std::make_unique<gpu::Texture>();
    blur_map_1->init_2d(width / 2, height / 2, format);
    blur_map_1->set_filters(TEXTURE_NEAREST, TEXTURE_NEAREST);
    blur_map_1->set_wrap_all(TEXTURE_CLAMP_TO_EDGE);
    
    /* prepare blur map 2 */
    blur_map_2 = std::make_unique<gpu::Texture>();
    blur_map_2->init_2d(width / 2, height / 2, format);
    blur_map_2->set_filters(TEXTURE_LINEAR, TEXTURE_LINEAR);
    blur_map_2->set_wrap_all(TEXTURE_CLAMP_TO_EDGE);
    
    /* prepare blur render target 1 */
    blur_target_1 = std::make_unique<gpu::RenderTarget>();
    blur_target_1->set_texture(*blur_map_1, 0);
    
    /* prepare blur render target 2 */
    blur_target_2 = std::make_unique<gpu::RenderTarget>();
    blur_target_2->set_texture(*blur_map_2, 0);
}

void BlurPass::render() {
    /* fetch box / Gaussian / bilateral blur shader from shader lib */
    Defines blur_defines;
    blur_defines.set("TYPE", TYPES[channel - 1]);
    blur_defines.set("SWIZZLE", SWIZZLES[channel - 1]);
    std::string shader_name;
    if (type == BLUR_BOX) {
        shader_name = "BoxBlur";
    } else if (type == BLUR_GAUSSIAN) {
        shader_name = "GaussianBlur";
    } else if (type == BLUR_BILATERAL) {
        shader_name = "BilateralBlur";
    }
    auto* blur_shader = ShaderLib::fetch(shader_name, blur_defines);
    
    /* fetch copy shader from shader lib */
    auto* copy_shader = ShaderLib::fetch("Copy");
    
    /* calculate screen parameter */
    Vec2 screen_size = Vec2(width / 2, height / 2);
    
    /* change the current viewport */
    gpu::Rect viewport = RenderPass::get_viewport();
    RenderPass::set_viewport(gpu::Rect(width / 2, height / 2));
    
    /* 1. blur texture horizontally (down-sampling) */
    blur_shader->use_program();
    blur_shader->set_uniform_f("lod", 0);
    blur_shader->set_uniform_v2("direction", Vec2(1 / screen_size.x, 0));
    blur_shader->set_uniform_i("radius", radius);
    if (type == BLUR_GAUSSIAN || type == BLUR_BILATERAL) {
        blur_shader->set_uniform_f("sigma_s", sigma_s);
    }
    if (type == BLUR_BILATERAL) {
        blur_shader->set_uniform_f("sigma_r", sigma_r);
    }
    blur_shader->set_uniform_i("map", map->activate(0));
    RenderPass::render_to(blur_shader, blur_target_1.get());
    
    /* 2. blur texture vertically */
    blur_shader->use_program();
    blur_shader->set_uniform_f("lod", 0);
    blur_shader->set_uniform_v2("direction", Vec2(0, 1 / screen_size.y));
    blur_shader->set_uniform_i("radius", radius);
    if (type == BLUR_GAUSSIAN || type == BLUR_BILATERAL) {
        blur_shader->set_uniform_f("sigma_s", sigma_s);
    }
    if (type == BLUR_BILATERAL) {
        blur_shader->set_uniform_f("sigma_r", sigma_r);
    }
    blur_shader->set_uniform_i("map", blur_map_1->activate(0));
    RenderPass::render_to(blur_shader, blur_target_2.get());
    
    /* set back to the initial viewport */
    RenderPass::set_viewport(viewport);
    
    /* 3. render results to render target (up-sampling) */
    copy_shader->use_program();
    copy_shader->set_uniform_i("map", blur_map_2->activate(0));
    RenderPass::render_to(copy_shader, target);
}

const gpu::Texture* BlurPass::get_texture() const {
    return map;
}

void BlurPass::set_texture(const gpu::Texture* t) {
    map = t;
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/ColorGradePass.cpp ---------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

void ColorGradePass::init() {}

void ColorGradePass::render() {
    auto* color_grade_shader = ShaderLib::fetch("ColorGrading");
    color_grade_shader->use_program();
    color_grade_shader->set_uniform_v3("saturation", saturation);
    color_grade_shader->set_uniform_v3("contrast", contrast);
    color_grade_shader->set_uniform_v3("gamma", gamma);
    color_grade_shader->set_uniform_v3("gain", gain);
    color_grade_shader->set_uniform_v3("offset", offset);
    color_grade_shader->set_uniform_i("map", map->activate(0));
    RenderPass::render_to(color_grade_shader, target);
}

const gpu::Texture* ColorGradePass::get_texture() const {
    return map;
}

void ColorGradePass::set_texture(const gpu::Texture* t) {
    map = t;
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/CopyPass.cpp ---------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

void CopyPass::init() {}

void CopyPass::render() {
    auto* copy_shader = ShaderLib::fetch("Copy");
    copy_shader->use_program();
    copy_shader->set_uniform_f("lod", 0);
    copy_shader->set_uniform_i("map", map->activate(0));
    RenderPass::render_to(copy_shader, target);
}

const gpu::Texture* CopyPass::get_texture() const {
    return map;
}

void CopyPass::set_texture(const gpu::Texture* t) {
    map = t;
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/FXAAPass.cpp ---------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

void FXAAPass::init() {}

void FXAAPass::render() {
    auto* fxaa_shader = ShaderLib::fetch("FXAA");
    gpu::Rect viewport = RenderPass::get_viewport();
    Vec2 screen_size = Vec2(viewport.width, viewport.height);
    fxaa_shader->use_program();
    fxaa_shader->set_uniform_v2("screen_size", screen_size);
    fxaa_shader->set_uniform_i("map", map->activate(0));
    RenderPass::render_to(fxaa_shader, target);
}

const gpu::Texture* FXAAPass::get_texture() const {
    return map;
}

void FXAAPass::set_texture(const gpu::Texture* t) {
    map = t;
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/GrainPass.cpp --------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

void GrainPass::init() {}

void GrainPass::render() {
    auto* grain_shader = ShaderLib::fetch("Grain");
    grain_shader->use_program();
    grain_shader->set_uniform_f("intensity", intensity);
    grain_shader->set_uniform_f("seed", Random::random_f() + 1);
    grain_shader->set_uniform_i("map", map->activate(0));
    RenderPass::render_to(grain_shader, target);
}

const gpu::Texture* GrainPass::get_texture() const {
    return map;
}

void GrainPass::set_texture(const gpu::Texture* t) {
    map = t;
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/LightPass.cpp --------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

void LightPass::init() {}

void LightPass::render() {
    /* fetch light shader from shader lib */
    Defines light_defines;
    Renderer::set_scene_defines(*scene, light_defines);
    auto* light_shader = ShaderLib::fetch("Lighting", light_defines);
    
    /* pass parameters and G-Buffers to shader */
    light_shader->use_program();
    light_shader->set_uniform_v3("camera_pos", camera->position);
    light_shader->set_uniform_i("g_color", g_color->activate(0));
    light_shader->set_uniform_i("g_normal", g_normal->activate(1));
    light_shader->set_uniform_i("g_material", g_material->activate(2));
    light_shader->set_uniform_i("g_light", g_light->activate(3));
    light_shader->set_uniform_i("z_buffer", z_buffer->activate(4));
    
    /* pass camera parameter to shader */
    Mat4 inv_view_proj = inverse_4x4(camera->projection * camera->viewing);
    light_shader->set_uniform_m4("inv_view_proj", inv_view_proj);
    
    /* pass the lights & fogs parameters to shader */
    Renderer::set_light_uniforms(*scene, *light_shader);
    
    /* render results to render target */
    RenderPass::render_to(light_shader, target);
}

const Scene* LightPass::get_scene() const {
    return scene;
}

void LightPass::set_scene(const Scene* s) {
    scene = s;
}

const Camera* LightPass::get_camera() const {
    return camera;
}

void LightPass::set_camera(const Camera* c) {
    camera = c;
}

const gpu::Texture* LightPass::get_texture_color() const {
    return g_color;
}

void LightPass::set_texture_color(const gpu::Texture* t) {
    g_color = t;
}

const gpu::Texture* LightPass::get_texture_normal() const {
    return g_normal;
}

void LightPass::set_texture_normal(const gpu::Texture* t) {
    g_normal = t;
}

const gpu::Texture* LightPass::get_texture_material() const {
    return g_material;
}

void LightPass::set_texture_material(const gpu::Texture* t) {
    g_material = t;
}

const gpu::Texture* LightPass::get_texture_light() const {
    return g_light;
}

void LightPass::set_texture_light(const gpu::Texture* t) {
    g_light = t;
}

const gpu::Texture* LightPass::get_texture_depth() const {
    return z_buffer;
}

void LightPass::set_texture_depth(const gpu::Texture* t) {
    z_buffer = t;
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/SSAOPass.cpp ---------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

SSAOPass::SSAOPass(int w, int h, float r, float m, float i) :
width(w), height(h), radius(r), max_radius(m), intensity(i) {}

void SSAOPass::init() {
    /* check the width and height */
    if (width == 0 || height == 0) {
        return Error::set("SSAOPass", "Width and height should be greater than 0");
    }
    
    /* prepare blur map 1 */
    blur_map_1 = std::make_unique<gpu::Texture>();
    blur_map_1->init_2d(width / 2, height / 2, TEXTURE_R8_UNORM);
    blur_map_1->set_filters(TEXTURE_NEAREST, TEXTURE_NEAREST);
    blur_map_1->set_wrap_all(TEXTURE_CLAMP_TO_EDGE);
    
    /* prepare blur map 2 */
    blur_map_2 = std::make_unique<gpu::Texture>();
    blur_map_2->init_2d(width / 2, height / 2, TEXTURE_R8_UNORM);
    blur_map_2->set_filters(TEXTURE_LINEAR, TEXTURE_LINEAR);
    blur_map_2->set_wrap_all(TEXTURE_CLAMP_TO_EDGE);
    
    /* prepare blur render target 1 */
    blur_target_1 = std::make_unique<gpu::RenderTarget>();
    blur_target_1->set_texture(*blur_map_1, 0);
    
    /* prepare blur render target 2 */
    blur_target_2 = std::make_unique<gpu::RenderTarget>();
    blur_target_2->set_texture(*blur_map_2, 0);
}

void SSAOPass::render() {
    /* fetch SSAO shader from shader lib */
    Defines ssao_defines;
    ssao_defines.set("SAMPLES", std::to_string(samples));
    auto* ssao_shader = ShaderLib::fetch("SSAO", ssao_defines);
    
    /* fetch blur shader from shader lib */
    Defines blur_defines;
    blur_defines.set("TYPE", "float");
    blur_defines.set("SWIZZLE", ".x");
    auto* blur_shader = ShaderLib::fetch("BilateralBlur", blur_defines);
    
    /* fetch blend shader from shader lib */
    Defines blend_defines;
    blend_defines.set("BLEND_OP(a, b)", "a * b");
    blend_defines.set("A_SWIZZLE", ".xyzw");
    blend_defines.set("B_SWIZZLE", ".xxxx");
    auto* blend_shader = ShaderLib::fetch("Blend", blend_defines);
    
    /* calculate camera & screen parameters */
    Mat4 inv_proj = inverse_4x4(camera->projection);
    Vec2 screen_size = Vec2(width / 2, height / 2);
    
    /* change the current viewport */
    gpu::Rect viewport = RenderPass::get_viewport();
    RenderPass::set_viewport(gpu::Rect(width / 2, height / 2));
    
    /* 1. render SSAO to texture (down-sampling) */
    ssao_shader->use_program();
    ssao_shader->set_uniform_f("intensity", intensity);
    ssao_shader->set_uniform_f("radius", radius);
    ssao_shader->set_uniform_f("max_radius", max_radius);
    ssao_shader->set_uniform_f("max_z", max_z);
    ssao_shader->set_uniform_f("near", camera->near);
    ssao_shader->set_uniform_f("far", camera->far);
    ssao_shader->set_uniform_m4("view", camera->viewing);
    ssao_shader->set_uniform_m4("proj", camera->projection);
    ssao_shader->set_uniform_m4("inv_proj", inv_proj);
    ssao_shader->set_uniform_i("g_normal", g_normal->activate(0));
    ssao_shader->set_uniform_i("z_buffer", z_buffer->activate(1));
    RenderPass::render_to(ssao_shader, blur_target_1.get());
    
    /* 2. blur texture for two times */
    for (int i = 0; i < 2; ++i) {
        
        /* blur texture horizontally */
        blur_shader->use_program();
        blur_shader->set_uniform_f("lod", 0);
        blur_shader->set_uniform_v2("direction", Vec2(1 / screen_size.x, 0));
        blur_shader->set_uniform_i("radius", 7);
        blur_shader->set_uniform_f("sigma_s", 2);
        blur_shader->set_uniform_f("sigma_r", 0.25);
        blur_shader->set_uniform_i("map", blur_map_1->activate(0));
        RenderPass::render_to(blur_shader, blur_target_2.get());
        
        /* blur texture vertically */
        blur_shader->use_program();
        blur_shader->set_uniform_f("lod", 0);
        blur_shader->set_uniform_v2("direction", Vec2(0, 1 / screen_size.y));
        blur_shader->set_uniform_i("radius", 7);
        blur_shader->set_uniform_f("sigma_s", 2);
        blur_shader->set_uniform_f("sigma_r", 0.25);
        blur_shader->set_uniform_i("map", blur_map_2->activate(0));
        RenderPass::render_to(blur_shader, blur_target_1.get());
    }
    
    /* set back to the initial viewport */
    RenderPass::set_viewport(viewport);
    
    /* 3. render results to render target (up-sampling) */
    blend_shader->use_program();
    blend_shader->set_uniform_i("map_a", map->activate(1));
    blend_shader->set_uniform_i("map_b", blur_map_1->activate(0));
    RenderPass::render_to(blend_shader, target);
}

const Camera* SSAOPass::get_camera() const {
    return camera;
}

void SSAOPass::set_camera(const Camera* c) {
    camera = c;
}

const gpu::Texture* SSAOPass::get_texture() const {
    return map;
}

void SSAOPass::set_texture(const gpu::Texture* t) {
    map = t;
}

const gpu::Texture* SSAOPass::get_texture_normal() const {
    return g_normal;
}

void SSAOPass::set_texture_normal(const gpu::Texture* t) {
    g_normal = t;
}

const gpu::Texture* SSAOPass::get_texture_depth() const {
    return z_buffer;
}

void SSAOPass::set_texture_depth(const gpu::Texture* t) {
    z_buffer = t;
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/SSRPass.cpp ----------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

SSRPass::SSRPass(float t, float i) : thickness(t), intensity(i) {}

void SSRPass::init() {}

void SSRPass::render() {
    /* fetch SSR shader from shader lib */
    auto* ssr_shader = ShaderLib::fetch("SSR");
    
    /* calculate camera parameters */
    gpu::Rect viewport = RenderPass::get_viewport();
    Vec2 screen_size = Vec2(viewport.width, viewport.height);
    Mat4 inv_proj = inverse_4x4(camera->projection);
    
    /* render SSR results to render target */
    ssr_shader->use_program();
    ssr_shader->set_uniform_i("max_steps", max_steps);
    ssr_shader->set_uniform_f("intensity", intensity);
    ssr_shader->set_uniform_f("thickness", thickness);
    ssr_shader->set_uniform_f("max_roughness", max_roughness);
    ssr_shader->set_uniform_f("near", camera->near);
    ssr_shader->set_uniform_f("far", camera->far);
    ssr_shader->set_uniform_v2("screen_size", screen_size);
    ssr_shader->set_uniform_m4("view", camera->viewing);
    ssr_shader->set_uniform_m4("proj", camera->projection);
    ssr_shader->set_uniform_m4("inv_proj", inv_proj);
    ssr_shader->set_uniform_i("map", map->activate(0));
    ssr_shader->set_uniform_i("g_normal", g_normal->activate(1));
    ssr_shader->set_uniform_i("g_material", g_material->activate(2));
    ssr_shader->set_uniform_i("z_buffer", z_buffer->activate(3));
    RenderPass::render_to(ssr_shader, target);
}

const Camera* SSRPass::get_camera() const {
    return camera;
}

void SSRPass::set_camera(const Camera* c) {
    camera = c;
}

const gpu::Texture* SSRPass::get_texture() const {
    return map;
}

void SSRPass::set_texture(const gpu::Texture* t) {
    map = t;
}

const gpu::Texture* SSRPass::get_texture_normal() const {
    return g_normal;
}

void SSRPass::set_texture_normal(const gpu::Texture* t) {
    g_normal = t;
}

const gpu::Texture* SSRPass::get_texture_material() const {
    return g_material;
}

void SSRPass::set_texture_material(const gpu::Texture* t) {
    g_material = t;
}

const gpu::Texture* SSRPass::get_texture_depth() const {
    return z_buffer;
}

void SSRPass::set_texture_depth(const gpu::Texture* t) {
    z_buffer = t;
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/postprocess/ToneMapPass.cpp ------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

void ToneMapPass::init() {}

void ToneMapPass::render() {
    /* fetch tone map shader from shader lib */
    Defines tone_map_defines;
    if (mode == LINEAR_TONE_MAP) {
        tone_map_defines.set("TONE_MAP", "linear_tone_map");
    } else if (mode == REINHARD_TONE_MAP) {
        tone_map_defines.set("TONE_MAP", "reinhard_tone_map");
    } else if (mode == OPTIMIZED_TONE_MAP) {
        tone_map_defines.set("TONE_MAP", "optimized_tone_map");
    } else if (mode == ACES_FILMIC_TONE_MAP) {
        tone_map_defines.set("TONE_MAP", "aces_filmic_tone_map");
    }
    auto* tone_map_shader = ShaderLib::fetch("ToneMapping", tone_map_defines);
    
    /* render tone map results to render target */
    tone_map_shader->use_program();
    tone_map_shader->set_uniform_f("exposure", exposure);
    tone_map_shader->set_uniform_i("map", map->activate(0));
    RenderPass::render_to(tone_map_shader, target);
}

const gpu::Texture* ToneMapPass::get_texture() const {
    return map;
}

void ToneMapPass::set_texture(const gpu::Texture* t) {
    map = t;
}

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/include/Common.glsl ------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* COMMON_GLSL = 
"#ifndef COMMON_GLSL\n"
"#define COMMON_GLSL\n"
"\n"
"#define PI                  3.141592653589793\n"
"#define TWO_PI              6.283185307179586\n"
"#define HALF_PI             1.5707963267948966\n"
"#define INV_PI              0.3183098861837907\n"
"#define EPS                 0.000001\n"
"\n"
"#define saturate(a)         clamp(a, 0., 1.)\n"
"\n"
"#define min3(a, b, c)       min(a, min(b, c))\n"
"#define max3(a, b, c)       max(a, max(b, c))\n"
"#define min4(a, b, c, d)    min(a, min(b, min(c, d)))\n"
"#define max4(a, b, c, d)    max(a, max(b, max(c, d)))\n"
"\n"
"/* Returns the squared value of x */\n"
"float square(float x)       { return x * x; }\n"
"vec2  square(vec2  x)       { return x * x; }\n"
"vec3  square(vec3  x)       { return x * x; }\n"
"vec4  square(vec4  x)       { return x * x; }\n"
"\n"
"/* Returns the cubed value of x */\n"
"float cube(float x)         { return x * x * x; }\n"
"vec2  cube(vec2  x)         { return x * x * x; }\n"
"vec3  cube(vec3  x)         { return x * x * x; }\n"
"vec4  cube(vec4  x)         { return x * x * x; }\n"
"\n"
"/* Returns a pseudo-random value in the range from 0 to 1. */\n"
"highp float rand(vec2 s) {\n"
"    const highp float a = 12.9898;\n"
"    const highp float b = 78.233;\n"
"    const highp float c = 43758.5453;\n"
"    highp float dt = dot(s, vec2(a, b));\n"
"    highp float sn = mod(dt, PI);\n"
"    return fract(sin(sn) * c);\n"
"}\n"
"\n"
"/* Returns a pseudo-random value in the range from 0 to 1. */\n"
"highp float rand(vec3 s) {\n"
"    const highp float a = 12.9898;\n"
"    const highp float b = 78.233;\n"
"    const highp float c = 45.164;\n"
"    const highp float d = 43758.5453;\n"
"    highp float dt = dot(s, vec3(a, b, c));\n"
"    highp float sn = mod(dt, PI);\n"
"    return fract(sin(sn) * d);\n"
"}\n"
"\n"
"/* Converts color to relative luminance. */\n"
"float relative_luminance(vec3 color) {\n"
"    const vec3 weights = vec3(0.2126, 0.7152, 0.0722);\n"
"    return dot(weights, color);\n"
"}\n"
"\n"
"#endif\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/include/Transform.glsl ---------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* TRANSFORM_GLSL = 
"#ifndef TRANSFORM_GLSL\n"
"#define TRANSFORM_GLSL\n"
"\n"
"/* Converts from view Z to depth in perspective camera. */\n"
"float z_to_depth_persp(float z, float near, float far) {\n"
"    return (2. * near * far / z + near + far) / (far - near) * 0.5 + 0.5;\n"
"}\n"
"\n"
"/* Converts from depth to view Z in perspective camera. */\n"
"float depth_to_z_persp(float depth, float near, float far) {\n"
"    return -2. * near * far / (near + far + (depth * 2. - 1.) * (near - far));\n"
"}\n"
"\n"
"/* Converts from view Z to depth in orthographic camera. */\n"
"float z_to_depth_ortho(float z, float near, float far) {\n"
"    return (2. * z + near + far) / (near - far) * 0.5 + 0.5;\n"
"}\n"
"\n"
"/* Converts from depth to view Z in orthographic camera. */\n"
"float depth_to_z_ortho(float depth, float near, float far) {\n"
"    return -0.5 * (near + far - (depth * 2. - 1.) * (near - far));\n"
"}\n"
"\n"
"#endif\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/include/Packing.glsl ------------------------------------ */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* PACKING_GLSL = 
"#ifndef PACKING_GLSL\n"
"#define PACKING_GLSL\n"
"\n"
"/* Packs normal to RGB. */\n"
"vec3 pack_normal(vec3 normal) {\n"
"    return normal * 0.5 + 0.5;\n"
"}\n"
"\n"
"/* Unpacks RGB to normal. */\n"
"vec3 unpack_normal(vec3 rgb) {\n"
"    return rgb * 2. - 1.;\n"
"}\n"
"\n"
"/* Packs vec2 to RGBA. */\n"
"vec4 pack_vec2(vec2 v) {\n"
"    vec4 r = vec4(v.x, fract(v.x * 255.), v.y, fract(v.y * 255.));\n"
"    return vec4(r.x - r.y / 255., r.y, r.z - r.w / 255., r.w);\n"
"}\n"
"\n"
"/* Unpacks RGBA to vec2. */\n"
"vec2 unpack_vec2(vec4 v) {\n"
"    return vec2(v.x + (v.y / 255.), v.z + (v.w / 255.));\n"
"}\n"
"\n"
"#endif\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/include/ColorSpace.glsl --------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* COLORSPACE_GLSL = 
"#ifndef COLORSPACE_GLSL\n"
"#define COLORSPACE_GLSL\n"
"\n"
"#include <common>\n"
"\n"
"const float SRGB_GAMMA = 1. / 2.2;\n"
"const float SRGB_INV_GAMMA = 2.2;\n"
"\n"
"const mat3 RGB_TO_XYZ = mat3(\n"
"    vec3( 0.4124564,  0.3575761,  0.1804375),\n"
"    vec3( 0.2126729,  0.7151522,  0.0721750),\n"
"    vec3( 0.0193339,  0.1191920,  0.9503041)\n"
");\n"
"\n"
"const mat3 XYZ_TO_RGB = mat3(\n"
"    vec3( 3.2404542, -1.5371385, -0.4985314),\n"
"    vec3(-0.9692660,  1.8760108,  0.0415560),\n"
"    vec3( 0.0556434, -0.2040259,  1.0572252)\n"
");\n"
"\n"
"/* Converts from RGB color space to SRGB color space (approximated). */\n"
"vec3 rgb_to_srgb_approx(vec3 rgb) {\n"
"    return pow(rgb, vec3(SRGB_GAMMA));\n"
"}\n"
"\n"
"/* Converts from SRGB color space to RGB color space (approximated). */\n"
"vec3 srgb_to_rgb_approx(vec3 srgb) {\n"
"    return pow(srgb, vec3(SRGB_INV_GAMMA));\n"
"}\n"
"\n"
"/* Converts from RGB color space to SRGB color space. */\n"
"float rgb_to_srgb(float r) {\n"
"    return r <= 0.0031308 ? r * 12.92 : pow(r, 1. / 2.4) * 1.055 - 0.055;\n"
"}\n"
"\n"
"/* Converts from RGB color space to SRGB color space. */\n"
"vec3 rgb_to_srgb(vec3 rgb) {\n"
"    return vec3(rgb_to_srgb(rgb.r), rgb_to_srgb(rgb.g), rgb_to_srgb(rgb.b));\n"
"}\n"
"\n"
"/* Converts from SRGB color space to RGB color space. */\n"
"float srgb_to_rgb(float r) {\n"
"    return r <= 0.04045 ? r / 12.92 : pow((r + 0.055) / 1.055, 2.4);\n"
"}\n"
"\n"
"/* Converts from SRGB color space to RGB color space. */\n"
"vec3 srgb_to_rgb(vec3 srgb) {\n"
"    return vec3(srgb_to_rgb(srgb.r), srgb_to_rgb(srgb.g), srgb_to_rgb(srgb.b));\n"
"}\n"
"\n"
"/* Converts from RGB color space to XYZ color space. */\n"
"vec3 rgb_to_xyz(vec3 rgb) {\n"
"    return RGB_TO_XYZ * rgb;\n"
"}\n"
"\n"
"/* Converts from XYZ color space to RGB color space. */\n"
"vec3 xyz_to_rgb(vec3 xyz) {\n"
"    return XYZ_TO_RGB * xyz;\n"
"}\n"
"\n"
"/* Converts from hue to RGB color space. */\n"
"vec3 hue_to_rgb(float hue) {\n"
"    float r = abs(hue * 6. - 3.) - 1.;\n"
"    float g = 2. - abs(hue * 6. - 2.);\n"
"    float b = 2. - abs(hue * 6. - 4.);\n"
"    return saturate(vec3(r, g, b));\n"
"}\n"
"\n"
"/* Converts from RGB color space to HCV color space. */\n"
"vec3 rgb_to_hcv(vec3 rgb) {\n"
"    vec4 p = (rgb.g < rgb.b) ? vec4(rgb.bg, -1., 2. / 3.) : vec4(rgb.gb, 0., -1. / 3.);\n"
"    vec4 q = (rgb.r < p.x) ? vec4(p.xyw, rgb.r) : vec4(rgb.r, p.yzx);\n"
"    float chr = q.x - min(q.w, q.y);\n"
"    float hue = abs((q.w - q.y) / (6. * chr + EPS) + q.z);\n"
"    return vec3(hue, chr, q.x);\n"
"}\n"
"\n"
"/* Converts from RGB color space to HSV color space. */\n"
"vec3 rgb_to_hsv(vec3 rgb) {\n"
"    vec3 hcv = rgb_to_hcv(rgb);\n"
"    return vec3(hcv.x, hcv.y / (hcv.z + EPS), hcv.z);\n"
"}\n"
"\n"
"/* Converts from HSV color space to RGB color space. */\n"
"vec3 hsv_to_rgb(vec3 hsv) {\n"
"    return ((hue_to_rgb(hsv.x) - 1.) * hsv.y + 1.) * hsv.z;\n"
"}\n"
"\n"
"/* Converts from RGB color space to HSL color space. */\n"
"vec3 rgb_to_hsl(vec3 rgb) {\n"
"    vec3 hcv = rgb_to_hcv(rgb);\n"
"    float lum = hcv.z - hcv.y * 0.5;\n"
"    float sat = hcv.y / (1. - abs(lum * 2. - 1.) + EPS);\n"
"    return vec3(hcv.x, sat, lum);\n"
"}\n"
"\n"
"/* Converts from HSL color space to RGB color space. */\n"
"vec3 hsl_to_rgb(vec3 hsl) {\n"
"    return (hue_to_rgb(hsl.x) - 0.5) * (1. - abs(2. * hsl.z - 1.)) * hsl.y + hsl.z;\n"
"}\n"
"\n"
"#endif\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/include/Poisson.glsl ------------------------------------ */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* POISSON_GLSL = 
"/**\n"
" * Poisson disks\n"
" * from https://github.com/tobspr/RenderPipeline\n"
" */\n"
"\n"
"#ifndef POISSON_GLSL\n"
"#define POISSON_GLSL\n"
"\n"
"const vec2 POISSON_2D_16[16] = vec2[](\n"
"    vec2(-0.680088, -0.731923),\n"
"    vec2(-0.957909, -0.247622),\n"
"    vec2(0.0948045, -0.992508),\n"
"    vec2(-0.316418, -0.93561),\n"
"    vec2(0.508091, -0.270309),\n"
"    vec2(0.986855, -0.161122),\n"
"    vec2(-0.0783372, -0.377044),\n"
"    vec2(0.678299, -0.730012),\n"
"    vec2(0.264303, 0.150586),\n"
"    vec2(0.375585, 0.926198),\n"
"    vec2(0.869216, 0.485342),\n"
"    vec2(-0.0303609, 0.582547),\n"
"    vec2(-0.456659, 0.886469),\n"
"    vec2(-0.0502123, 0.998242),\n"
"    vec2(-0.522737, 0.0698312),\n"
"    vec2(-0.857891, 0.512805)\n"
");\n"
"\n"
"const vec2 POISSON_2D_32[32] = vec2[](\n"
"    vec2(-0.645095, -0.281509),\n"
"    vec2(-0.491004, -0.535543),\n"
"    vec2(-0.251013, -0.96746),\n"
"    vec2(-0.527482, -0.838249),\n"
"    vec2(-0.940142, -0.338633),\n"
"    vec2(-0.78887, -0.602182),\n"
"    vec2(-0.306068, -0.207344),\n"
"    vec2(0.0236232, -0.763255),\n"
"    vec2(0.326509, -0.930717),\n"
"    vec2(0.0188184, -0.427136),\n"
"    vec2(0.734814, -0.2611),\n"
"    vec2(0.997828, -0.0467772),\n"
"    vec2(0.102272, -0.0652116),\n"
"    vec2(0.800923, -0.596026),\n"
"    vec2(0.532466, -0.718217),\n"
"    vec2(0.45988, -0.0147904),\n"
"    vec2(0.428033, -0.434308),\n"
"    vec2(0.569939, 0.31757),\n"
"    vec2(0.847669, 0.530125),\n"
"    vec2(0.517769, 0.839368),\n"
"    vec2(0.247533, 0.477887),\n"
"    vec2(0.952392, 0.24895),\n"
"    vec2(-0.49404, 0.843787),\n"
"    vec2(0.071269, 0.98377),\n"
"    vec2(-0.901791, 0.425404),\n"
"    vec2(-0.511218, 0.404082),\n"
"    vec2(-0.166727, 0.573622),\n"
"    vec2(-0.719383, 0.656148),\n"
"    vec2(-0.221952, 0.956912),\n"
"    vec2(-0.639361, 0.0178937),\n"
"    vec2(-0.192214, 0.149272),\n"
"    vec2(-0.999393, 0.0286758)\n"
");\n"
"\n"
"const vec2 POISSON_2D_64[64] = vec2[](\n"
"    vec2(-0.086316, -0.968111),\n"
"    vec2(-0.279227, -0.959008),\n"
"    vec2(-0.174659, -0.779219),\n"
"    vec2(-0.994046, -0.100337),\n"
"    vec2(-0.830417, -0.258316),\n"
"    vec2(-0.29098, -0.276363),\n"
"    vec2(-0.0711791, -0.309748),\n"
"    vec2(0.127704, -0.414434),\n"
"    vec2(0.255973, -0.712316),\n"
"    vec2(-0.472927, -0.394059),\n"
"    vec2(0.35572, -0.928901),\n"
"    vec2(-0.658504, -0.535877),\n"
"    vec2(0.105184, -0.99382),\n"
"    vec2(-0.414437, -0.0607066),\n"
"    vec2(-0.366554, -0.691781),\n"
"    vec2(-0.464703, -0.862405),\n"
"    vec2(-0.161989, -0.104327),\n"
"    vec2(-0.174757, -0.515362),\n"
"    vec2(-0.637325, -0.770203),\n"
"    vec2(-0.664806, -0.128825),\n"
"    vec2(-0.879395, -0.465052),\n"
"    vec2(0.0304411, -0.762748),\n"
"    vec2(0.443908, -0.577779),\n"
"    vec2(0.188869, -0.0525923),\n"
"    vec2(0.988219, -0.136184),\n"
"    vec2(0.369901, -0.259416),\n"
"    vec2(0.805865, -0.578815),\n"
"    vec2(0.628829, -0.776143),\n"
"    vec2(0.626141, -0.343504),\n"
"    vec2(0.74722, 0.655944),\n"
"    vec2(0.234085, 0.806087),\n"
"    vec2(0.93415, 0.337947),\n"
"    vec2(0.412135, 0.901209),\n"
"    vec2(0.676801, 0.206996),\n"
"    vec2(0.0943238, 0.661738),\n"
"    vec2(0.118536, 0.984615),\n"
"    vec2(0.589421, 0.79131),\n"
"    vec2(0.731047, 0.429766),\n"
"    vec2(0.478587, 0.439142),\n"
"    vec2(0.22497, 0.445659),\n"
"    vec2(0.374495, 0.660695),\n"
"    vec2(0.906059, 0.0973713),\n"
"    vec2(0.33495, 0.190513),\n"
"    vec2(0.757964, -0.136907),\n"
"    vec2(0.54374, -0.0527806),\n"
"    vec2(0.908866, -0.36894),\n"
"    vec2(-0.0776111, 0.477161),\n"
"    vec2(-0.181392, 0.975743),\n"
"    vec2(-0.139923, 0.74221),\n"
"    vec2(-0.78206, 0.102957),\n"
"    vec2(-0.576849, 0.450708),\n"
"    vec2(-0.00888789, 0.05214),\n"
"    vec2(-0.708787, 0.698047),\n"
"    vec2(-0.84552, 0.531998),\n"
"    vec2(-0.253359, 0.267802),\n"
"    vec2(-0.493709, 0.659425),\n"
"    vec2(-0.992843, 0.11576),\n"
"    vec2(-0.302095, 0.564714),\n"
"    vec2(-0.464918, 0.874781),\n"
"    vec2(-0.711149, 0.302547),\n"
"    vec2(-0.915964, 0.323287),\n"
"    vec2(-0.263747, 0.0616327),\n"
"    vec2(-0.519476, 0.175641),\n"
"    vec2(0.0307961, 0.26891)\n"
");\n"
"\n"
"const vec3 POISSON_3D_16[16] = vec3[](\n"
"    vec3(-0.0601252, -0.090253, 0.242585),\n"
"    vec3(-0.954277, -0.230389, 0.0491814),\n"
"    vec3(-0.440708, -0.751107, 0.48156),\n"
"    vec3(0.157285, -0.98681, -0.0113457),\n"
"    vec3(-0.463958, -0.603355, -0.645049),\n"
"    vec3(0.352804, -0.600314, 0.705076),\n"
"    vec3(0.945725, -0.167066, -0.26733),\n"
"    vec3(0.378653, -0.457091, -0.789674),\n"
"    vec3(0.175363, 0.326807, -0.923022),\n"
"    vec3(0.75046, 0.133097, 0.643648),\n"
"    vec3(0.533623, 0.832552, 0.0769863),\n"
"    vec3(-0.685037, 0.662173, -0.295431),\n"
"    vec3(-0.0454946, 0.134477, 0.985295),\n"
"    vec3(-0.534475, 0.0997254, -0.826768),\n"
"    vec3(-0.307152, 0.748555, 0.585295),\n"
"    vec3(-0.709746, 0.0374277, 0.68205)\n"
");\n"
"\n"
"const vec3 POISSON_3D_32[32] = vec3[](\n"
"    vec3(-0.637433, -0.0795689, 0.749829),\n"
"    vec3(-0.166216, -0.0315564, -0.983756),\n"
"    vec3(-0.528491, -0.143633, 0.195646),\n"
"    vec3(-0.249567, -0.825099, 0.498893),\n"
"    vec3(-0.558757, -0.434904, -0.667499),\n"
"    vec3(-0.858404, -0.473501, -0.145915),\n"
"    vec3(-0.135047, -0.296593, -0.335407),\n"
"    vec3(0.316311, -0.757367, 0.261108),\n"
"    vec3(-0.163747, -0.374054, 0.88854),\n"
"    vec3(0.268538, -0.898278, -0.335799),\n"
"    vec3(-0.381943, -0.906282, -0.16664),\n"
"    vec3(0.837301, -0.420993, 0.347183),\n"
"    vec3(0.36155, -0.12942, 0.913665),\n"
"    vec3(0.722952, 0.655391, 0.209401),\n"
"    vec3(0.374059, 0.504317, -0.764699),\n"
"    vec3(0.466859, 0.811274, -0.270417),\n"
"    vec3(0.962492, 0.082706, 0.102222),\n"
"    vec3(0.726909, 0.197392, 0.592712),\n"
"    vec3(0.726963, -0.519139, -0.26076),\n"
"    vec3(0.863379, 0.0510047, -0.479442),\n"
"    vec3(0.39697, -0.392489, -0.778489),\n"
"    vec3(0.395817, 0.28567, -0.152613),\n"
"    vec3(0.0115477, 0.0392268, 0.262437),\n"
"    vec3(-0.124912, 0.314161, 0.906488),\n"
"    vec3(-0.795609, 0.124298, -0.584104),\n"
"    vec3(-0.58822, 0.533162, 0.596359),\n"
"    vec3(-0.465148, 0.867438, 0.068886),\n"
"    vec3(-0.347131, 0.479682, -0.773511),\n"
"    vec3(-0.0359631, 0.862508, 0.497219),\n"
"    vec3(-0.869958, 0.454845, 0.0919322),\n"
"    vec3(-0.100729, 0.895672, -0.400421),\n"
"    vec3(-0.312793, 0.316496, -0.184027)\n"
");\n"
"\n"
"const vec3 POISSON_3D_64[64] = vec3[](\n"
"    vec3(-0.15773, -0.0451896, -0.9623),\n"
"    vec3(-0.0130272, -0.640955, -0.751804),\n"
"    vec3(-0.214195, -0.789982, 0.571629),\n"
"    vec3(-0.22401, -0.127885, -0.479033),\n"
"    vec3(-0.0381656, -0.400932, 0.389145),\n"
"    vec3(-0.958604, -0.255706, 0.0608546),\n"
"    vec3(-0.491959, -0.0240958, 0.867465),\n"
"    vec3(-0.735088, -0.589627, -0.0952913),\n"
"    vec3(-0.361071, -0.387626, -0.841852),\n"
"    vec3(-0.454339, -0.716331, -0.490685),\n"
"    vec3(-0.464857, -0.00371937, 0.416402),\n"
"    vec3(-0.252955, -0.952897, -0.0173203),\n"
"    vec3(-0.310376, -0.450015, 0.834997),\n"
"    vec3(-0.481379, -0.12181, -0.0713362),\n"
"    vec3(0.17995, -0.273669, -0.910183),\n"
"    vec3(0.100674, -0.18972, 0.96892),\n"
"    vec3(0.229951, -0.770195, -0.427458),\n"
"    vec3(-0.534464, -0.703867, 0.273916),\n"
"    vec3(-0.779887, -0.341913, 0.505745),\n"
"    vec3(-0.793823, -0.290259, -0.406983),\n"
"    vec3(0.512152, -0.541214, -0.661702),\n"
"    vec3(0.616316, -0.597375, 0.505634),\n"
"    vec3(0.798438, -0.464642, -0.225906),\n"
"    vec3(0.546185, -0.833083, -0.0299397),\n"
"    vec3(0.38638, -0.311687, 0.154036),\n"
"    vec3(0.172597, -0.639959, 0.741325),\n"
"    vec3(0.124675, -0.818478, 0.257105),\n"
"    vec3(-0.172153, -0.492998, -0.0557574),\n"
"    vec3(0.318022, -0.24362, -0.38011),\n"
"    vec3(0.531653, -0.165474, 0.749569),\n"
"    vec3(0.529882, 0.24116, -0.257057),\n"
"    vec3(0.466419, 0.215838, 0.25547),\n"
"    vec3(0.0712114, 0.0329548, 0.548568),\n"
"    vec3(0.723801, -0.0377743, -0.668303),\n"
"    vec3(0.814331, 0.205909, 0.504961),\n"
"    vec3(0.93028, 0.357367, 0.0674278),\n"
"    vec3(0.921439, -0.213509, 0.314967),\n"
"    vec3(0.96477, -0.0016533, -0.256952),\n"
"    vec3(-0.0662509, 0.986109, -0.0656959),\n"
"    vec3(0.185578, 0.617772, -0.181412),\n"
"    vec3(0.107275, 0.241256, -0.491317),\n"
"    vec3(0.164571, 0.247343, 0.917695),\n"
"    vec3(0.0651426, 0.739647, -0.652712),\n"
"    vec3(0.456338, 0.504012, 0.706019),\n"
"    vec3(0.293143, 0.169755, -0.932919),\n"
"    vec3(0.032654, 0.0818838, 0.00303189),\n"
"    vec3(0.601786, 0.735747, 0.216412),\n"
"    vec3(0.151178, 0.940888, 0.300587),\n"
"    vec3(0.702927, 0.659986, -0.246973),\n"
"    vec3(0.0395814, 0.691424, 0.686644),\n"
"    vec3(0.451051, 0.515958, -0.689563),\n"
"    vec3(-0.891535, 0.111488, 0.301105),\n"
"    vec3(-0.720135, 0.120472, -0.674127),\n"
"    vec3(-0.747936, 0.51219, 0.177613),\n"
"    vec3(-0.16722, 0.372523, -0.887329),\n"
"    vec3(-0.674427, 0.357484, 0.643218),\n"
"    vec3(-0.249121, 0.379101, 0.871001),\n"
"    vec3(-0.929231, 0.219506, -0.255281),\n"
"    vec3(-0.487606, 0.461665, -0.621111),\n"
"    vec3(-0.451381, 0.890955, 0.0284355),\n"
"    vec3(-0.724563, 0.612287, -0.273063),\n"
"    vec3(-0.228439, 0.555403, 0.34383),\n"
"    vec3(-0.328195, 0.828738, -0.447974),\n"
"    vec3(-0.417829, 0.32502, -0.232616)\n"
");\n"
"\n"
"#endif\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/include/Halton.glsl ------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* HALTON_GLSL = 
"/**\n"
" * Halton sequences\n"
" * from https://github.com/tobspr/RenderPipeline\n"
" */\n"
"\n"
"#ifndef HALTON_GLSL\n"
"#define HALTON_GLSL\n"
"\n"
"const vec2 HALTON_2D_16[16] = vec2[](\n"
"    vec2(0.00000, -0.33333),\n"
"    vec2(-0.50000, 0.33333),\n"
"    vec2(0.50000, -0.77778),\n"
"    vec2(-0.75000, -0.11111),\n"
"    vec2(0.25000, 0.55556),\n"
"    vec2(-0.25000, -0.55556),\n"
"    vec2(0.75000, 0.11111),\n"
"    vec2(-0.87500, 0.77778),\n"
"    vec2(0.12500, -0.92593),\n"
"    vec2(-0.37500, -0.25926),\n"
"    vec2(0.62500, 0.40741),\n"
"    vec2(-0.62500, -0.70370),\n"
"    vec2(0.37500, -0.03704),\n"
"    vec2(-0.12500, 0.62963),\n"
"    vec2(0.87500, -0.48148),\n"
"    vec2(-0.93750, 0.18519)\n"
");\n"
"\n"
"const vec2 HALTON_2D_32[32] = vec2[](\n"
"    vec2(0.00000, -0.33333),\n"
"    vec2(-0.50000, 0.33333),\n"
"    vec2(0.50000, -0.77778),\n"
"    vec2(-0.75000, -0.11111),\n"
"    vec2(0.25000, 0.55556),\n"
"    vec2(-0.25000, -0.55556),\n"
"    vec2(0.75000, 0.11111),\n"
"    vec2(-0.87500, 0.77778),\n"
"    vec2(0.12500, -0.92593),\n"
"    vec2(-0.37500, -0.25926),\n"
"    vec2(0.62500, 0.40741),\n"
"    vec2(-0.62500, -0.70370),\n"
"    vec2(0.37500, -0.03704),\n"
"    vec2(-0.12500, 0.62963),\n"
"    vec2(0.87500, -0.48148),\n"
"    vec2(-0.93750, 0.18519),\n"
"    vec2(0.06250, 0.85185),\n"
"    vec2(-0.43750, -0.85185),\n"
"    vec2(0.56250, -0.18519),\n"
"    vec2(-0.68750, 0.48148),\n"
"    vec2(0.31250, -0.62963),\n"
"    vec2(-0.18750, 0.03704),\n"
"    vec2(0.81250, 0.70370),\n"
"    vec2(-0.81250, -0.40741),\n"
"    vec2(0.18750, 0.25926),\n"
"    vec2(-0.31250, 0.92593),\n"
"    vec2(0.68750, -0.97531),\n"
"    vec2(-0.56250, -0.30864),\n"
"    vec2(0.43750, 0.35802),\n"
"    vec2(-0.06250, -0.75309),\n"
"    vec2(0.93750, -0.08642),\n"
"    vec2(-0.96875, 0.58025)\n"
");\n"
"\n"
"const vec2 HALTON_2D_64[64] = vec2[](\n"
"    vec2(0.00000, -0.33333),\n"
"    vec2(-0.50000, 0.33333),\n"
"    vec2(0.50000, -0.77778),\n"
"    vec2(-0.75000, -0.11111),\n"
"    vec2(0.25000, 0.55556),\n"
"    vec2(-0.25000, -0.55556),\n"
"    vec2(0.75000, 0.11111),\n"
"    vec2(-0.87500, 0.77778),\n"
"    vec2(0.12500, -0.92593),\n"
"    vec2(-0.37500, -0.25926),\n"
"    vec2(0.62500, 0.40741),\n"
"    vec2(-0.62500, -0.70370),\n"
"    vec2(0.37500, -0.03704),\n"
"    vec2(-0.12500, 0.62963),\n"
"    vec2(0.87500, -0.48148),\n"
"    vec2(-0.93750, 0.18519),\n"
"    vec2(0.06250, 0.85185),\n"
"    vec2(-0.43750, -0.85185),\n"
"    vec2(0.56250, -0.18519),\n"
"    vec2(-0.68750, 0.48148),\n"
"    vec2(0.31250, -0.62963),\n"
"    vec2(-0.18750, 0.03704),\n"
"    vec2(0.81250, 0.70370),\n"
"    vec2(-0.81250, -0.40741),\n"
"    vec2(0.18750, 0.25926),\n"
"    vec2(-0.31250, 0.92593),\n"
"    vec2(0.68750, -0.97531),\n"
"    vec2(-0.56250, -0.30864),\n"
"    vec2(0.43750, 0.35802),\n"
"    vec2(-0.06250, -0.75309),\n"
"    vec2(0.93750, -0.08642),\n"
"    vec2(-0.96875, 0.58025),\n"
"    vec2(0.03125, -0.53086),\n"
"    vec2(-0.46875, 0.13580),\n"
"    vec2(0.53125, 0.80247),\n"
"    vec2(-0.71875, -0.90123),\n"
"    vec2(0.28125, -0.23457),\n"
"    vec2(-0.21875, 0.43210),\n"
"    vec2(0.78125, -0.67901),\n"
"    vec2(-0.84375, -0.01235),\n"
"    vec2(0.15625, 0.65432),\n"
"    vec2(-0.34375, -0.45679),\n"
"    vec2(0.65625, 0.20988),\n"
"    vec2(-0.59375, 0.87654),\n"
"    vec2(0.40625, -0.82716),\n"
"    vec2(-0.09375, -0.16049),\n"
"    vec2(0.90625, 0.50617),\n"
"    vec2(-0.90625, -0.60494),\n"
"    vec2(0.09375, 0.06173),\n"
"    vec2(-0.40625, 0.72840),\n"
"    vec2(0.59375, -0.38272),\n"
"    vec2(-0.65625, 0.28395),\n"
"    vec2(0.34375, 0.95062),\n"
"    vec2(-0.15625, -0.95062),\n"
"    vec2(0.84375, -0.28395),\n"
"    vec2(-0.78125, 0.38272),\n"
"    vec2(0.21875, -0.72840),\n"
"    vec2(-0.28125, -0.06173),\n"
"    vec2(0.71875, 0.60494),\n"
"    vec2(-0.53125, -0.50617),\n"
"    vec2(0.46875, 0.16049),\n"
"    vec2(-0.03125, 0.82716),\n"
"    vec2(0.96875, -0.87654),\n"
"    vec2(-0.98438, -0.20988)\n"
");\n"
"\n"
"const vec3 HALTON_3D_16[16] = vec3[](\n"
"    vec3(0.00000, -0.33333, -0.60000),\n"
"    vec3(-0.50000, 0.33333, -0.20000),\n"
"    vec3(0.50000, -0.77778, 0.20000),\n"
"    vec3(-0.75000, -0.11111, 0.60000),\n"
"    vec3(0.25000, 0.55556, -0.92000),\n"
"    vec3(-0.25000, -0.55556, -0.52000),\n"
"    vec3(0.75000, 0.11111, -0.12000),\n"
"    vec3(-0.87500, 0.77778, 0.28000),\n"
"    vec3(0.12500, -0.92593, 0.68000),\n"
"    vec3(-0.37500, -0.25926, -0.84000),\n"
"    vec3(0.62500, 0.40741, -0.44000),\n"
"    vec3(-0.62500, -0.70370, -0.04000),\n"
"    vec3(0.37500, -0.03704, 0.36000),\n"
"    vec3(-0.12500, 0.62963, 0.76000),\n"
"    vec3(0.87500, -0.48148, -0.76000),\n"
"    vec3(-0.93750, 0.18519, -0.36000)\n"
");\n"
"\n"
"const vec3 HALTON_3D_32[32] = vec3[](\n"
"    vec3(0.00000, -0.33333, -0.60000),\n"
"    vec3(-0.50000, 0.33333, -0.20000),\n"
"    vec3(0.50000, -0.77778, 0.20000),\n"
"    vec3(-0.75000, -0.11111, 0.60000),\n"
"    vec3(0.25000, 0.55556, -0.92000),\n"
"    vec3(-0.25000, -0.55556, -0.52000),\n"
"    vec3(0.75000, 0.11111, -0.12000),\n"
"    vec3(-0.87500, 0.77778, 0.28000),\n"
"    vec3(0.12500, -0.92593, 0.68000),\n"
"    vec3(-0.37500, -0.25926, -0.84000),\n"
"    vec3(0.62500, 0.40741, -0.44000),\n"
"    vec3(-0.62500, -0.70370, -0.04000),\n"
"    vec3(0.37500, -0.03704, 0.36000),\n"
"    vec3(-0.12500, 0.62963, 0.76000),\n"
"    vec3(0.87500, -0.48148, -0.76000),\n"
"    vec3(-0.93750, 0.18519, -0.36000),\n"
"    vec3(0.06250, 0.85185, 0.04000),\n"
"    vec3(-0.43750, -0.85185, 0.44000),\n"
"    vec3(0.56250, -0.18519, 0.84000),\n"
"    vec3(-0.68750, 0.48148, -0.68000),\n"
"    vec3(0.31250, -0.62963, -0.28000),\n"
"    vec3(-0.18750, 0.03704, 0.12000),\n"
"    vec3(0.81250, 0.70370, 0.52000),\n"
"    vec3(-0.81250, -0.40741, 0.92000),\n"
"    vec3(0.18750, 0.25926, -0.98400),\n"
"    vec3(-0.31250, 0.92593, -0.58400),\n"
"    vec3(0.68750, -0.97531, -0.18400),\n"
"    vec3(-0.56250, -0.30864, 0.21600),\n"
"    vec3(0.43750, 0.35802, 0.61600),\n"
"    vec3(-0.06250, -0.75309, -0.90400),\n"
"    vec3(0.93750, -0.08642, -0.50400),\n"
"    vec3(-0.96875, 0.58025, -0.10400)\n"
");\n"
"\n"
"const vec3 HALTON_3D_64[64] = vec3[](\n"
"    vec3(0.00000, -0.33333, -0.60000),\n"
"    vec3(-0.50000, 0.33333, -0.20000),\n"
"    vec3(0.50000, -0.77778, 0.20000),\n"
"    vec3(-0.75000, -0.11111, 0.60000),\n"
"    vec3(0.25000, 0.55556, -0.92000),\n"
"    vec3(-0.25000, -0.55556, -0.52000),\n"
"    vec3(0.75000, 0.11111, -0.12000),\n"
"    vec3(-0.87500, 0.77778, 0.28000),\n"
"    vec3(0.12500, -0.92593, 0.68000),\n"
"    vec3(-0.37500, -0.25926, -0.84000),\n"
"    vec3(0.62500, 0.40741, -0.44000),\n"
"    vec3(-0.62500, -0.70370, -0.04000),\n"
"    vec3(0.37500, -0.03704, 0.36000),\n"
"    vec3(-0.12500, 0.62963, 0.76000),\n"
"    vec3(0.87500, -0.48148, -0.76000),\n"
"    vec3(-0.93750, 0.18519, -0.36000),\n"
"    vec3(0.06250, 0.85185, 0.04000),\n"
"    vec3(-0.43750, -0.85185, 0.44000),\n"
"    vec3(0.56250, -0.18519, 0.84000),\n"
"    vec3(-0.68750, 0.48148, -0.68000),\n"
"    vec3(0.31250, -0.62963, -0.28000),\n"
"    vec3(-0.18750, 0.03704, 0.12000),\n"
"    vec3(0.81250, 0.70370, 0.52000),\n"
"    vec3(-0.81250, -0.40741, 0.92000),\n"
"    vec3(0.18750, 0.25926, -0.98400),\n"
"    vec3(-0.31250, 0.92593, -0.58400),\n"
"    vec3(0.68750, -0.97531, -0.18400),\n"
"    vec3(-0.56250, -0.30864, 0.21600),\n"
"    vec3(0.43750, 0.35802, 0.61600),\n"
"    vec3(-0.06250, -0.75309, -0.90400),\n"
"    vec3(0.93750, -0.08642, -0.50400),\n"
"    vec3(-0.96875, 0.58025, -0.10400),\n"
"    vec3(0.03125, -0.53086, 0.29600),\n"
"    vec3(-0.46875, 0.13580, 0.69600),\n"
"    vec3(0.53125, 0.80247, -0.82400),\n"
"    vec3(-0.71875, -0.90123, -0.42400),\n"
"    vec3(0.28125, -0.23457, -0.02400),\n"
"    vec3(-0.21875, 0.43210, 0.37600),\n"
"    vec3(0.78125, -0.67901, 0.77600),\n"
"    vec3(-0.84375, -0.01235, -0.74400),\n"
"    vec3(0.15625, 0.65432, -0.34400),\n"
"    vec3(-0.34375, -0.45679, 0.05600),\n"
"    vec3(0.65625, 0.20988, 0.45600),\n"
"    vec3(-0.59375, 0.87654, 0.85600),\n"
"    vec3(0.40625, -0.82716, -0.66400),\n"
"    vec3(-0.09375, -0.16049, -0.26400),\n"
"    vec3(0.90625, 0.50617, 0.13600),\n"
"    vec3(-0.90625, -0.60494, 0.53600),\n"
"    vec3(0.09375, 0.06173, 0.93600),\n"
"    vec3(-0.40625, 0.72840, -0.96800),\n"
"    vec3(0.59375, -0.38272, -0.56800),\n"
"    vec3(-0.65625, 0.28395, -0.16800),\n"
"    vec3(0.34375, 0.95062, 0.23200),\n"
"    vec3(-0.15625, -0.95062, 0.63200),\n"
"    vec3(0.84375, -0.28395, -0.88800),\n"
"    vec3(-0.78125, 0.38272, -0.48800),\n"
"    vec3(0.21875, -0.72840, -0.08800),\n"
"    vec3(-0.28125, -0.06173, 0.31200),\n"
"    vec3(0.71875, 0.60494, 0.71200),\n"
"    vec3(-0.53125, -0.50617, -0.80800),\n"
"    vec3(0.46875, 0.16049, -0.40800),\n"
"    vec3(-0.03125, 0.82716, -0.00800),\n"
"    vec3(0.96875, -0.87654, 0.39200),\n"
"    vec3(-0.98438, -0.20988, 0.79200)\n"
");\n"
"\n"
"#endif\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/include/BRDF.glsl --------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* BRDF_GLSL = 
"#ifndef BRDF_GLSL\n"
"#define BRDF_GLSL\n"
"\n"
"#include <common>\n"
"\n"
"/* Returns the value of Lambert BRDF. */\n"
"vec3 brdf_lambert(vec3 diffuse) {\n"
"    return diffuse * INV_PI;\n"
"}\n"
"\n"
"/* Calculates Fresnel with Schlick's approximation. */\n"
"vec3 f_schlick(vec3 f0, float voh) {\n"
"    float fresnel = exp2((-5.55473 * voh - 6.98316) * voh);\n"
"    return f0 * (1. - fresnel) + fresnel;\n"
"}\n"
"\n"
"/* Calculates Geometry with GGX-Smith model. */\n"
"float v_ggx(float a, float nol, float nov) {\n"
"    float a2 = a * a;\n"
"    float gv = nol * sqrt(a2 + (1. - a2) * nov * nov);\n"
"    float gl = nov * sqrt(a2 + (1. - a2) * nol * nol);\n"
"    return 0.5 / max(gv + gl, EPS);\n"
"}\n"
"\n"
"/* Calculates NDF with GGX model. */\n"
"float d_ggx(float a, float noh) {\n"
"    float a2 = a * a;\n"
"    float d = noh * noh * (a2 - 1.) + 1.;\n"
"    return INV_PI * a2 / (d * d);\n"
"}\n"
"\n"
"/* Returns the value of GGX BRDF (Schlick Fresnel, GGX-Smith Geometry, GGX NDF). */\n"
"vec3 brdf_ggx(vec3 light_dir, vec3 view_dir, vec3 normal, vec3 f0, float roughness) {\n"
"    float a = roughness * roughness;\n"
"    vec3 half_dir = normalize(light_dir + view_dir);\n"
"    float nol = saturate(dot(normal, light_dir));\n"
"    float nov = saturate(dot(normal, view_dir));\n"
"    float noh = saturate(dot(normal, half_dir));\n"
"    float voh = saturate(dot(view_dir, half_dir));\n"
"    return v_ggx(a, nol, nov) * d_ggx(a, noh) * f_schlick(f0, voh);\n"
"}\n"
"\n"
"/* Returns the approximation of the DFG LUT. */\n"
"vec2 lut_dfg(vec3 normal, vec3 view_dir, float roughness) {\n"
"    const vec4 c0 = vec4(-1., -0.0275, -0.572, 0.022);\n"
"    const vec4 c1 = vec4( 1.,  0.0425,   1.04, -0.04);\n"
"    float nov = saturate(dot(normal, view_dir));\n"
"    vec4 r = roughness * c0 + c1;\n"
"    float a004 = min(r.x * r.x, exp2(-9.28 * nov)) * r.x + r.y;\n"
"    return vec2(-1.04, 1.04) * a004 + r.zw;\n"
"}\n"
"\n"
"/* Returns the value of multiple-scattering microfacet model. */\n"
"void scattering(vec3 normal, vec3 view_dir, vec3 f0, float roughness,\n"
"                inout vec3 single_scatter, inout vec3 multi_scatter) {\n"
"    vec2 f_ab = lut_dfg(normal, view_dir, roughness);\n"
"    vec3 fss_ess = f0 * f_ab.x + f_ab.y;\n"
"    float ess = f_ab.x + f_ab.y;\n"
"    float ems = 1. - ess;\n"
"    vec3 favg = f0 + (1. - f0) * 0.047619;\n"
"    vec3 fms = fss_ess * favg / (1. - ems * favg);\n"
"    single_scatter += fss_ess;\n"
"    multi_scatter += fms * ems;\n"
"}\n"
"\n"
"#endif\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/include/Shadow.glsl ------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* SHADOW_GLSL = 
"#ifndef SHADOW_GLSL\n"
"#define SHADOW_GLSL\n"
"\n"
"#include <poisson>\n"
"\n"
"#define SHADOW_HARD          0\n"
"#define SHADOW_PCF           1\n"
"#define SHADOW_PCSS          2\n"
"\n"
"#if SHADOW_SAMPLES == 16\n"
"#define POISSON_2D POISSON_2D_16\n"
"#elif SHADOW_SAMPLES == 32\n"
"#define POISSON_2D POISSON_2D_32\n"
"#elif SHADOW_SAMPLES == 64\n"
"#define POISSON_2D POISSON_2D_64\n"
"#endif\n"
"\n"
"struct GlobalShadow {\n"
"    sampler2DArray map;\n"
"    vec2 size;\n"
"};\n"
"\n"
"struct Shadow {\n"
"    int type;\n"
"    int map_id;\n"
"    float bias;\n"
"    float normal_bias;\n"
"    float radius;\n"
"    mat4 view_proj;\n"
"};\n"
"\n"
"uniform GlobalShadow global_shadow;\n"
"\n"
"/* Returns true if the coordinate is out of screen. */\n"
"bool out_of_screen(vec3 coord) {\n"
"    return any(bvec2(any(bvec3(coord.x < 0., coord.y < 0., coord.z < 0.)),\n"
"                     any(bvec3(coord.x > 1., coord.y > 1., coord.z > 1.))));\n"
"}\n"
"\n"
"/* Searches for the depth of the neighbouring blockers. */\n"
"float find_blocker(Shadow shadow, vec3 light_pos, vec2 texel_size, float radius) {\n"
"    float blocker_count = 0.;\n"
"    float blocker_sum = 0.;\n"
"    for (int i = 0; i < 16; ++i) {\n"
"        vec2 offset = POISSON_2D_16[i] * radius * texel_size;\n"
"        vec3 coord = vec3(light_pos.xy + offset, shadow.map_id);\n"
"        float shadow_z = textureLod(global_shadow.map, coord, 0).x;\n"
"        float accept = step(shadow_z, light_pos.z);\n"
"        blocker_count += accept;\n"
"        blocker_sum += accept * shadow_z;\n"
"    }\n"
"    return blocker_count == 0. ? 1. : blocker_sum / blocker_count;\n"
"}\n"
"\n"
"/* Calculates the shadow with hard edge. */\n"
"float shadow_hard(Shadow shadow, vec3 light_pos) {\n"
"    vec3 coord = vec3(light_pos.xy, shadow.map_id);\n"
"    float shadow_z = textureLod(global_shadow.map, coord, 0).x;\n"
"    return step(shadow_z, light_pos.z);\n"
"}\n"
"\n"
"/* Calculates the Percentage Closer Filtering shadow (PCF). */\n"
"float shadow_pcf(Shadow shadow, vec3 light_pos, vec2 texel_size, float radius) {\n"
"    float shadow_sum = 0.;\n"
"    for (int i = 0; i < SHADOW_SAMPLES; ++i) {\n"
"        vec2 offset = POISSON_2D[i] * radius * texel_size;\n"
"        vec3 coord = vec3(light_pos.xy + offset, shadow.map_id);\n"
"        float shadow_z = textureLod(global_shadow.map, coord, 0).x;\n"
"        shadow_sum += step(shadow_z, light_pos.z);\n"
"    }\n"
"    return shadow_sum / SHADOW_SAMPLES;\n"
"}\n"
"\n"
"/* Calculates the Percentage Closer Soft Shadow (PCSS). */\n"
"float shadow_pcss(Shadow shadow, vec3 light_pos, vec2 texel_size, float radius) {\n"
"    /* Step 1. Blocker search */\n"
"    float z_blocker = find_blocker(shadow, light_pos, texel_size, radius);\n"
"    float z_receiver = light_pos.z;\n"
"    \n"
"    /* Step 2. Penumbra estimation */\n"
"    float penumbra = (z_receiver - z_blocker) / z_blocker * shadow.radius;\n"
"    float filter_radius = max(penumbra, 1.);\n"
"    \n"
"    /* Step 3. Filtering */\n"
"    return shadow_pcf(shadow, light_pos, texel_size, filter_radius);\n"
"}\n"
"\n"
"/* Calculates the shadow factor by shadow mapping. */\n"
"float get_shadow(Shadow shadow, vec3 world_pos, vec3 normal) {\n"
"    /* calculate light position */\n"
"    world_pos += normal * shadow.normal_bias;\n"
"    vec4 light_ndc = shadow.view_proj * vec4(world_pos, 1.);\n"
"    vec3 light_pos = light_ndc.xyz / light_ndc.w * 0.5 + 0.5;\n"
"    light_pos.z -= shadow.bias;\n"
"    \n"
"    /* ignore the pixels outside shadow map */\n"
"    if (out_of_screen(light_pos)) return 1.;\n"
"    vec2 texel_size = 1. / global_shadow.size;\n"
"    \n"
"    /* calculate shadow factor with edge fade */\n"
"    if (shadow.type == SHADOW_HARD) {\n"
"        return 1. - shadow_hard(shadow, light_pos);\n"
"    }\n"
"    if (shadow.type == SHADOW_PCF) {\n"
"        return 1. - shadow_pcf(shadow, light_pos, texel_size, shadow.radius);\n"
"    }\n"
"    if (shadow.type == SHADOW_PCSS) {\n"
"        float search_radius = global_shadow.size.x * 0.03125;\n"
"        return 1. - shadow_pcss(shadow, light_pos, texel_size, search_radius);\n"
"    }\n"
"}\n"
"\n"
"#endif\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/include/Lights.glsl ------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* LIGHTS_GLSL = 
"#ifndef LIGHTS_GLSL\n"
"#define LIGHTS_GLSL\n"
"\n"
"#include <common>\n"
"#include <brdf>\n"
"#include <shadow>\n"
"\n"
"struct Material {\n"
"    vec3 color;\n"
"    vec3 f0;\n"
"    float roughness;\n"
"};\n"
"\n"
"struct Geometry {\n"
"    vec3 position;\n"
"    vec3 view_dir;\n"
"    vec3 normal;\n"
"};\n"
"\n"
"struct PointLight {\n"
"    bool visible;\n"
"    vec3 position;\n"
"    vec3 color;\n"
"    float distance;\n"
"    float decay;\n"
"};\n"
"\n"
"struct SpotLight {\n"
"    bool visible;\n"
"    vec3 position;\n"
"    vec3 direction;\n"
"    vec3 color;\n"
"    float distance;\n"
"    float decay;\n"
"    float angle;\n"
"    float penumbra;\n"
"    bool cast_shadow;\n"
"    Shadow shadow;\n"
"};\n"
"\n"
"struct DirectionalLight {\n"
"    bool visible;\n"
"    vec3 direction;\n"
"    vec3 color;\n"
"    bool cast_shadow;\n"
"    Shadow shadow;\n"
"};\n"
"\n"
"struct HemisphereLight {\n"
"    bool visible;\n"
"    vec3 direction;\n"
"    vec3 sky_color;\n"
"    vec3 ground_color;\n"
"};\n"
"\n"
"/* Calculates the attenuation with light distance. */\n"
"float attenuate(float light_distance, float max_distance, float decay) {\n"
"    if (max_distance > 0. && decay > 0.) {\n"
"        return pow(saturate(1. - light_distance / max_distance), decay);\n"
"    }\n"
"    return 1.;\n"
"}\n"
"\n"
"/* Calculates the attenuation with spot light angle. */\n"
"float spot_attenuate(float angle_cos, float max_angle_cos, float penumbra_cos) {\n"
"    return smoothstep(max_angle_cos, penumbra_cos, angle_cos);\n"
"}\n"
"\n"
"/* Calculates direct light with Lambert BRDF & GGX BRDF. */\n"
"vec3 lighting_direct(Material mat, vec3 light_dir, vec3 view_dir, vec3 normal) {\n"
"    float nol = saturate(dot(normal, light_dir));\n"
"    vec3 specular = brdf_ggx(light_dir, view_dir, normal, mat.f0, mat.roughness);\n"
"    vec3 diffuse = brdf_lambert(mat.color);\n"
"    return nol * (specular + diffuse);\n"
"}\n"
"\n"
"/* Applies the specified point light to material. */\n"
"vec3 apply_light(PointLight light, Material mat, Geometry geom) {\n"
"    /* check whether the light is visible */\n"
"    if (!light.visible) return vec3(0.);\n"
"    \n"
"    /* calculate the vector from world to light */\n"
"    vec3 light_dir = light.position - geom.position;\n"
"    float light_distance = length(light_dir);\n"
"    light_dir = normalize(light_dir);\n"
"    \n"
"    /* calculate distance attenuation */\n"
"    float attenuation = attenuate(light_distance, light.distance, light.decay);\n"
"    if (attenuation <= 0.) return vec3(0.);\n"
"    \n"
"    /* calculate the color of light */\n"
"    vec3 light_color = light.color * attenuation;\n"
"    \n"
"    /* calculate with lighting model */\n"
"    light_color *= lighting_direct(mat, light_dir, geom.view_dir, geom.normal);\n"
"    \n"
"    /* return the color of light */\n"
"    return light_color;\n"
"}\n"
"\n"
"/* Applies the specified spot light to material. */\n"
"vec3 apply_light(SpotLight light, Material mat, Geometry geom) {\n"
"    /* check whether the light is visible */\n"
"    if (!light.visible) return vec3(0.);\n"
"    \n"
"    /* calculate the vector from world to light */\n"
"    vec3 light_dir = light.position - geom.position;\n"
"    float light_distance = length(light_dir);\n"
"    light_dir = normalize(light_dir);\n"
"    \n"
"    /* calculate spot & distance attenuation */\n"
"    float attenuation = 1.;\n"
"    float angle_cos = dot(light_dir, light.direction);\n"
"    attenuation *= spot_attenuate(angle_cos, light.angle, light.penumbra);\n"
"    attenuation *= attenuate(light_distance, light.distance, light.decay);\n"
"    if (attenuation <= 0.) return vec3(0.);\n"
"    \n"
"    /* calculate the color of light */\n"
"    vec3 light_color = light.color * attenuation;\n"
"    \n"
"    /* apply spot light shadow */\n"
"    light_color *= light.cast_shadow ?\n"
"        get_shadow(light.shadow, geom.position, geom.normal) : 1.;\n"
"    \n"
"    /* calculate with lighting model */\n"
"    light_color *= lighting_direct(mat, light_dir, geom.view_dir, geom.normal);\n"
"    \n"
"    /* return the color of light */\n"
"    return light_color;\n"
"}\n"
"\n"
"/* Applies the specified directional light to material. */\n"
"vec3 apply_light(DirectionalLight light, Material mat, Geometry geom) {\n"
"    /* check whether the light is visible */\n"
"    if (!light.visible) return vec3(0.);\n"
"    \n"
"    /* calculate the color of light */\n"
"    vec3 light_color = light.color;\n"
"    \n"
"    /* apply directional light shadow */\n"
"    light_color *= light.cast_shadow ?\n"
"        get_shadow(light.shadow, geom.position, geom.normal) : 1.;\n"
"    \n"
"    /* calculate with lighting model */\n"
"    light_color *= lighting_direct(mat, light.direction, geom.view_dir, geom.normal);\n"
"    \n"
"    /* return the color of light */\n"
"    return light_color;\n"
"}\n"
"\n"
"/* Applies the specified hemisphere light to material. */\n"
"vec3 apply_light(HemisphereLight light, Material mat, Geometry geom) {\n"
"    /* check whether the light is visible */\n"
"    if (!light.visible) return vec3(0.);\n"
"    \n"
"    /* calculate the color of light */\n"
"    float weight = dot(geom.normal, light.direction) * 0.5 + 0.5;\n"
"    vec3 light_color = mix(light.ground_color, light.sky_color, weight);\n"
"    \n"
"    /* calculate with lighting model */\n"
"    light_color *= brdf_lambert(mat.color);\n"
"    \n"
"    /* return the color of light */\n"
"    return light_color;\n"
"}\n"
"\n"
"#endif\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/include/Fogs.glsl --------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* FOGS_GLSL = 
"#ifndef FOGS_GLSL\n"
"#define FOGS_GLSL\n"
"\n"
"#include <common>\n"
"\n"
"struct LinearFog {\n"
"    bool visible;\n"
"    vec3 color;\n"
"    float near;\n"
"    float far;\n"
"};\n"
"\n"
"struct Exp2Fog {\n"
"    bool visible;\n"
"    vec3 color;\n"
"    float near;\n"
"    float density;\n"
"};\n"
"\n"
"/* Applies the specified linear fog to color. */\n"
"void apply_fog(LinearFog fog, float camera_distance, inout vec3 color) {\n"
"    if (!fog.visible) return;\n"
"    float factor = saturate((fog.far - camera_distance) / (fog.far - fog.near));\n"
"    color = mix(fog.color, color, factor);\n"
"}\n"
"\n"
"/* Applies the specified exp square fog to color. */\n"
"void apply_fog(Exp2Fog fog, float camera_distance, inout vec3 color) {\n"
"    if (!fog.visible) return;\n"
"    float fog_distance = max(0., camera_distance - fog.near);\n"
"    float factor = exp(-square(fog_distance * fog.density));\n"
"    color = mix(fog.color, color, factor);\n"
"}\n"
"\n"
"#endif\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/include/Cubemap.glsl ------------------------------------ */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* CUBEMAP_GLSL = 
"#ifndef CUBEMAP_GLSL\n"
"#define CUBEMAP_GLSL\n"
"\n"
"/* Converts face uv to unnormalized direction. */\n"
"vec3 face_to_cube(vec2 uv, int face) {\n"
"    vec3 dir = vec3(uv, 1.);\n"
"    if (face == 0) return vec3(dir.z, -dir.y, -dir.x);\n"
"    if (face == 1) return vec3(-dir.z, -dir.y, dir.x);\n"
"    if (face == 2) return vec3(dir.x, dir.z, dir.y);\n"
"    if (face == 3) return vec3(dir.x, -dir.z, -dir.y);\n"
"    if (face == 4) return vec3(dir.x, -dir.y, dir.z);\n"
"    if (face == 5) return vec3(-dir.x, -dir.y, -dir.z);\n"
"    return vec3(0., 0., 0.);\n"
"}\n"
"\n"
"/* Converts normalized direction to equirectangular uv. */\n"
"vec2 cube_to_equirect(vec3 dir) {\n"
"    float u = atan(dir.z, dir.x) * INV_PI * 0.5 + 0.5;\n"
"    float v = asin(clamp(dir.y, -1., 1.)) * INV_PI + 0.5;\n"
"    return vec2(u, v);\n"
"}\n"
"\n"
"#endif\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/include/IBLFilter.glsl ---------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* IBLFILTER_GLSL = 
"#ifndef IBLFILTER_GLSL\n"
"#define IBLFILTER_GLSL\n"
"\n"
"/* Sample the environment map with roughness. */\n"
"vec4 texture_env(samplerCube map, float max_lod, vec3 dir, float roughness) {\n"
"    float mip = 0.;\n"
"    if (roughness >= 0.4) {\n"
"        mip = (0.8 - roughness) * 7.5 - 1.;\n"
"    } else if (roughness >= 0.305) {\n"
"        mip = (0.4 - roughness) / 0.095 + 2.;\n"
"    } else if (roughness >= 0.21) {\n"
"        mip = (0.305 - roughness) / 0.095 + 3.;\n"
"    } else {\n"
"        mip = -2. * log2(1.16 * roughness);\n"
"    }\n"
"    mip = clamp(max_lod - mip, 0., max_lod);\n"
"    return textureLod(map, dir, mip);\n"
"}\n"
"\n"
"/* Calculate specular light from the environment map. */\n"
"vec3 ibl_specular(samplerCube map, float max_lod, vec3 view_dir, vec3 normal, float roughness) {\n"
"    vec3 reflect_vec = reflect(-view_dir, normal);\n"
"    reflect_vec = normalize(mix(reflect_vec, normal, roughness * roughness));\n"
"    return texture_env(map, max_lod, reflect_vec, roughness).xyz;\n"
"}\n"
"\n"
"/* Calculate diffuse light from the environment map. */\n"
"vec3 ibl_diffuse(samplerCube map, float max_lod, vec3 normal) {\n"
"    return PI * texture_env(map, max_lod, normal, 1.).xyz;\n"
"}\n"
"\n"
"#endif\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/include/ToneMapping.glsl -------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* TONEMAPPING_GLSL = 
"#ifndef TONEMAPPING_GLSL\n"
"#define TONEMAPPING_GLSL\n"
"\n"
"const mat3 ACES_INPUT = mat3(\n"
"    vec3(0.59719, 0.07600, 0.02840),\n"
"    vec3(0.35458, 0.90834, 0.13383),\n"
"    vec3(0.04823, 0.01566, 0.83777)\n"
");\n"
"\n"
"const mat3 ACES_OUTPUT = mat3(\n"
"    vec3( 1.60475, -0.10208, -0.00327),\n"
"    vec3(-0.53108,  1.10813, -0.07276),\n"
"    vec3(-0.07367, -0.00605,  1.07602)\n"
");\n"
"\n"
"/* Linear tonemapping operator. */\n"
"vec3 linear_tone_map(vec3 color, float exposure) {\n"
"    return color * exposure;\n"
"}\n"
"\n"
"/* Reinhard tonemapping operator. */\n"
"vec3 reinhard_tone_map(vec3 color, float exposure) {\n"
"    color *= exposure;\n"
"    return saturate(color / (vec3(1.) + color));\n"
"}\n"
"\n"
"/* Optimized operator of the Haarm-Peter Duiker’s curve. */\n"
"vec3 optimized_tone_map(vec3 color, float exposure) {\n"
"    color *= exposure;\n"
"    color = max(vec3(0.), color - 0.004);\n"
"    color = (color * (6.2 * color + 0.5)) / (color * (6.2 * color + 1.7) + 0.06);\n"
"    return pow(color, vec3(2.2));\n"
"}\n"
"\n"
"/* RTT and ODT fit operator. */\n"
"vec3 rtt_and_odt_fit(vec3 v) {\n"
"    vec3 a = v * (v + 0.0245786) - 0.000090537;\n"
"    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;\n"
"    return a / b;\n"
"}\n"
"\n"
"/* ACES filmic tonemapping operator. */\n"
"vec3 aces_filmic_tone_map(vec3 color, float exposure) {\n"
"    color = ACES_INPUT * (color * exposure);\n"
"    color = ACES_OUTPUT * rtt_and_odt_fit(color);\n"
"    return saturate(color);\n"
"}\n"
"\n"
"#endif\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/include/ColorGrading.glsl ------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* COLORGRADING_GLSL = 
"#ifndef COLORGRADING_GLSL\n"
"#define COLORGRADING_GLSL\n"
"\n"
"/* Applies color grading to color by adjusting parameters. */\n"
"vec3 color_grading(vec3 color, vec3 saturation, vec3 contrast, vec3 gamma, vec3 gain, vec3 offset) {\n"
"    float luma = relative_luminance(color);\n"
"    color = max(vec3(0.), mix(vec3(luma), color, saturation));\n"
"    color = pow(color * 5.55555555, contrast) * 0.18;\n"
"    color = pow(color, 1. / gamma);\n"
"    return color * gain + offset;\n"
"}\n"
"\n"
"#endif\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/include/Fxaa3_11.glsl ----------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* FXAA3_11_GLSL = 
"/*============================================================================\n"
"\n"
"\n"
"                    NVIDIA FXAA 3.11 by TIMOTHY LOTTES\n"
"\n"
"\n"
"------------------------------------------------------------------------------\n"
"COPYRIGHT (C) 2010, 2011 NVIDIA CORPORATION. ALL RIGHTS RESERVED.\n"
"------------------------------------------------------------------------------\n"
"TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED\n"
"*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS\n"
"OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF\n"
"MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL NVIDIA\n"
"OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR\n"
"CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR\n"
"LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION,\n"
"OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE\n"
"THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH\n"
"DAMAGES.\n"
"\n"
"------------------------------------------------------------------------------\n"
"                           INTEGRATION CHECKLIST\n"
"------------------------------------------------------------------------------\n"
"(1.)\n"
"In the shader source, setup defines for the desired configuration.\n"
"When providing multiple shaders (for different presets),\n"
"simply setup the defines differently in multiple files.\n"
"Example,\n"
"\n"
"  #define FXAA_PC 1\n"
"  #define FXAA_HLSL_5 1\n"
"  #define FXAA_QUALITY__PRESET 12\n"
"\n"
"Or,\n"
"\n"
"  #define FXAA_360 1\n"
"  \n"
"Or,\n"
"\n"
"  #define FXAA_PS3 1\n"
"  \n"
"Etc.\n"
"\n"
"(2.)\n"
"Then include this file,\n"
"\n"
"  #include \"Fxaa3_11.h\"\n"
"\n"
"(3.)\n"
"Then call the FXAA pixel shader from within your desired shader.\n"
"Look at the FXAA Quality FxaaPixelShader() for docs on inputs.\n"
"As for FXAA 3.11 all inputs for all shaders are the same\n"
"to enable easy porting between platforms.\n"
"\n"
"  return FxaaPixelShader(...);\n"
"\n"
"(4.)\n"
"Insure pass prior to FXAA outputs RGBL (see next section).\n"
"Or use,\n"
"\n"
"  #define FXAA_GREEN_AS_LUMA 1\n"
"\n"
"(5.)\n"
"Setup engine to provide the following constants\n"
"which are used in the FxaaPixelShader() inputs,\n"
"\n"
"  FxaaFloat2 fxaaQualityRcpFrame,\n"
"  FxaaFloat4 fxaaConsoleRcpFrameOpt,\n"
"  FxaaFloat4 fxaaConsoleRcpFrameOpt2,\n"
"  FxaaFloat4 fxaaConsole360RcpFrameOpt2,\n"
"  FxaaFloat fxaaQualitySubpix,\n"
"  FxaaFloat fxaaQualityEdgeThreshold,\n"
"  FxaaFloat fxaaQualityEdgeThresholdMin,\n"
"  FxaaFloat fxaaConsoleEdgeSharpness,\n"
"  FxaaFloat fxaaConsoleEdgeThreshold,\n"
"  FxaaFloat fxaaConsoleEdgeThresholdMin,\n"
"  FxaaFloat4 fxaaConsole360ConstDir\n"
"\n"
"Look at the FXAA Quality FxaaPixelShader() for docs on inputs.\n"
"\n"
"(6.)\n"
"Have FXAA vertex shader run as a full screen triangle,\n"
"and output \"pos\" and \"fxaaConsolePosPos\"\n"
"such that inputs in the pixel shader provide,\n"
"\n"
"  // {xy} = center of pixel\n"
"  FxaaFloat2 pos,\n"
"\n"
"  // {xy__} = upper left of pixel\n"
"  // {__zw} = lower right of pixel\n"
"  FxaaFloat4 fxaaConsolePosPos,\n"
"\n"
"(7.)\n"
"Insure the texture sampler(s) used by FXAA are set to bilinear filtering.\n"
"\n"
"\n"
"------------------------------------------------------------------------------\n"
"                    INTEGRATION - RGBL AND COLORSPACE\n"
"------------------------------------------------------------------------------\n"
"FXAA3 requires RGBL as input unless the following is set,\n"
"\n"
"  #define FXAA_GREEN_AS_LUMA 1\n"
"\n"
"In which case the engine uses green in place of luma,\n"
"and requires RGB input is in a non-linear colorspace.\n"
"\n"
"RGB should be LDR (low dynamic range).\n"
"Specifically do FXAA after tonemapping.\n"
"\n"
"RGB data as returned by a texture fetch can be non-linear,\n"
"or linear when FXAA_GREEN_AS_LUMA is not set.\n"
"Note an \"sRGB format\" texture counts as linear,\n"
"because the result of a texture fetch is linear data.\n"
"Regular \"RGBA8\" textures in the sRGB colorspace are non-linear.\n"
"\n"
"If FXAA_GREEN_AS_LUMA is not set,\n"
"luma must be stored in the alpha channel prior to running FXAA.\n"
"This luma should be in a perceptual space (could be gamma 2.0).\n"
"Example pass before FXAA where output is gamma 2.0 encoded,\n"
"\n"
"  color.rgb = ToneMap(color.rgb); // linear color output\n"
"  color.rgb = sqrt(color.rgb);    // gamma 2.0 color output\n"
"  return color;\n"
"\n"
"To use FXAA,\n"
"\n"
"  color.rgb = ToneMap(color.rgb);  // linear color output\n"
"  color.rgb = sqrt(color.rgb);     // gamma 2.0 color output\n"
"  color.a = dot(color.rgb, FxaaFloat3(0.299, 0.587, 0.114)); // compute luma\n"
"  return color;\n"
"\n"
"Another example where output is linear encoded,\n"
"say for instance writing to an sRGB formated render target,\n"
"where the render target does the conversion back to sRGB after blending,\n"
"\n"
"  color.rgb = ToneMap(color.rgb); // linear color output\n"
"  return color;\n"
"\n"
"To use FXAA,\n"
"\n"
"  color.rgb = ToneMap(color.rgb); // linear color output\n"
"  color.a = sqrt(dot(color.rgb, FxaaFloat3(0.299, 0.587, 0.114))); // compute luma\n"
"  return color;\n"
"\n"
"Getting luma correct is required for the algorithm to work correctly.\n"
"\n"
"\n"
"------------------------------------------------------------------------------\n"
"                          BEING LINEARLY CORRECT?\n"
"------------------------------------------------------------------------------\n"
"Applying FXAA to a framebuffer with linear RGB color will look worse.\n"
"This is very counter intuitive, but happends to be true in this case.\n"
"The reason is because dithering artifacts will be more visiable\n"
"in a linear colorspace.\n"
"\n"
"\n"
"------------------------------------------------------------------------------\n"
"                             COMPLEX INTEGRATION\n"
"------------------------------------------------------------------------------\n"
"Q. What if the engine is blending into RGB before wanting to run FXAA?\n"
"\n"
"A. In the last opaque pass prior to FXAA,\n"
"   have the pass write out luma into alpha.\n"
"   Then blend into RGB only.\n"
"   FXAA should be able to run ok\n"
"   assuming the blending pass did not any add aliasing.\n"
"   This should be the common case for particles and common blending passes.\n"
"\n"
"A. Or use FXAA_GREEN_AS_LUMA.\n"
"\n"
"============================================================================*/\n"
"\n"
"/*============================================================================\n"
"\n"
"                             INTEGRATION KNOBS\n"
"\n"
"============================================================================*/\n"
"//\n"
"// FXAA_PS3 and FXAA_360 choose the console algorithm (FXAA3 CONSOLE).\n"
"// FXAA_360_OPT is a prototype for the new optimized 360 version.\n"
"//\n"
"// 1 = Use API.\n"
"// 0 = Don't use API.\n"
"//\n"
"/*--------------------------------------------------------------------------*/\n"
"#ifndef FXAA_PS3\n"
"    #define FXAA_PS3 0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#ifndef FXAA_360\n"
"    #define FXAA_360 0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#ifndef FXAA_360_OPT\n"
"    #define FXAA_360_OPT 0\n"
"#endif\n"
"/*==========================================================================*/\n"
"#ifndef FXAA_PC\n"
"    //\n"
"    // FXAA Quality\n"
"    // The high quality PC algorithm.\n"
"    //\n"
"    #define FXAA_PC 0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#ifndef FXAA_PC_CONSOLE\n"
"    //\n"
"    // The console algorithm for PC is included\n"
"    // for developers targeting really low spec machines.\n"
"    // Likely better to just run FXAA_PC, and use a really low preset.\n"
"    //\n"
"    #define FXAA_PC_CONSOLE 0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#ifndef FXAA_GLSL_120\n"
"    #define FXAA_GLSL_120 0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#ifndef FXAA_GLSL_130\n"
"    #define FXAA_GLSL_130 0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#ifndef FXAA_HLSL_3\n"
"    #define FXAA_HLSL_3 0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#ifndef FXAA_HLSL_4\n"
"    #define FXAA_HLSL_4 0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#ifndef FXAA_HLSL_5\n"
"    #define FXAA_HLSL_5 0\n"
"#endif\n"
"/*==========================================================================*/\n"
"#ifndef FXAA_GREEN_AS_LUMA\n"
"    //\n"
"    // For those using non-linear color,\n"
"    // and either not able to get luma in alpha, or not wanting to,\n"
"    // this enables FXAA to run using green as a proxy for luma.\n"
"    // So with this enabled, no need to pack luma in alpha.\n"
"    //\n"
"    // This will turn off AA on anything which lacks some amount of green.\n"
"    // Pure red and blue or combination of only R and B, will get no AA.\n"
"    //\n"
"    // Might want to lower the settings for both,\n"
"    //    fxaaConsoleEdgeThresholdMin\n"
"    //    fxaaQualityEdgeThresholdMin\n"
"    // In order to insure AA does not get turned off on colors\n"
"    // which contain a minor amount of green.\n"
"    //\n"
"    // 1 = On.\n"
"    // 0 = Off.\n"
"    //\n"
"    #define FXAA_GREEN_AS_LUMA 0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#ifndef FXAA_EARLY_EXIT\n"
"    //\n"
"    // Controls algorithm's early exit path.\n"
"    // On PS3 turning this ON adds 2 cycles to the shader.\n"
"    // On 360 turning this OFF adds 10ths of a millisecond to the shader.\n"
"    // Turning this off on console will result in a more blurry image.\n"
"    // So this defaults to on.\n"
"    //\n"
"    // 1 = On.\n"
"    // 0 = Off.\n"
"    //\n"
"    #define FXAA_EARLY_EXIT 1\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#ifndef FXAA_DISCARD\n"
"    //\n"
"    // Only valid for PC OpenGL currently.\n"
"    // Probably will not work when FXAA_GREEN_AS_LUMA = 1.\n"
"    //\n"
"    // 1 = Use discard on pixels which don't need AA.\n"
"    //     For APIs which enable concurrent TEX+ROP from same surface.\n"
"    // 0 = Return unchanged color on pixels which don't need AA.\n"
"    //\n"
"    #define FXAA_DISCARD 0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#ifndef FXAA_FAST_PIXEL_OFFSET\n"
"    //\n"
"    // Used for GLSL 120 only.\n"
"    //\n"
"    // 1 = GL API supports fast pixel offsets\n"
"    // 0 = do not use fast pixel offsets\n"
"    //\n"
"    #ifdef GL_EXT_gpu_shader4\n"
"        #define FXAA_FAST_PIXEL_OFFSET 1\n"
"    #endif\n"
"    #ifdef GL_NV_gpu_shader5\n"
"        #define FXAA_FAST_PIXEL_OFFSET 1\n"
"    #endif\n"
"    #ifdef GL_ARB_gpu_shader5\n"
"        #define FXAA_FAST_PIXEL_OFFSET 1\n"
"    #endif\n"
"    #ifndef FXAA_FAST_PIXEL_OFFSET\n"
"        #define FXAA_FAST_PIXEL_OFFSET 0\n"
"    #endif\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#ifndef FXAA_GATHER4_ALPHA\n"
"    //\n"
"    // 1 = API supports gather4 on alpha channel.\n"
"    // 0 = API does not support gather4 on alpha channel.\n"
"    //\n"
"    #if (FXAA_HLSL_5 == 1)\n"
"        #define FXAA_GATHER4_ALPHA 1\n"
"    #endif\n"
"    #ifdef GL_ARB_gpu_shader5\n"
"        #define FXAA_GATHER4_ALPHA 1\n"
"    #endif\n"
"    #ifdef GL_NV_gpu_shader5\n"
"        #define FXAA_GATHER4_ALPHA 1\n"
"    #endif\n"
"    #ifndef FXAA_GATHER4_ALPHA\n"
"        #define FXAA_GATHER4_ALPHA 0\n"
"    #endif\n"
"#endif\n"
"\n"
"/*============================================================================\n"
"                      FXAA CONSOLE PS3 - TUNING KNOBS\n"
"============================================================================*/\n"
"#ifndef FXAA_CONSOLE__PS3_EDGE_SHARPNESS\n"
"    //\n"
"    // Consoles the sharpness of edges on PS3 only.\n"
"    // Non-PS3 tuning is done with shader input.\n"
"    //\n"
"    // Due to the PS3 being ALU bound,\n"
"    // there are only two safe values here: 4 and 8.\n"
"    // These options use the shaders ability to a free *|/ by 2|4|8.\n"
"    //\n"
"    // 8.0 is sharper\n"
"    // 4.0 is softer\n"
"    // 2.0 is really soft (good for vector graphics inputs)\n"
"    //\n"
"    #if 1\n"
"        #define FXAA_CONSOLE__PS3_EDGE_SHARPNESS 8.0\n"
"    #endif\n"
"    #if 0\n"
"        #define FXAA_CONSOLE__PS3_EDGE_SHARPNESS 4.0\n"
"    #endif\n"
"    #if 0\n"
"        #define FXAA_CONSOLE__PS3_EDGE_SHARPNESS 2.0\n"
"    #endif\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#ifndef FXAA_CONSOLE__PS3_EDGE_THRESHOLD\n"
"    //\n"
"    // Only effects PS3.\n"
"    // Non-PS3 tuning is done with shader input.\n"
"    //\n"
"    // The minimum amount of local contrast required to apply algorithm.\n"
"    // The console setting has a different mapping than the quality setting.\n"
"    //\n"
"    // This only applies when FXAA_EARLY_EXIT is 1.\n"
"    //\n"
"    // Due to the PS3 being ALU bound,\n"
"    // there are only two safe values here: 0.25 and 0.125.\n"
"    // These options use the shaders ability to a free *|/ by 2|4|8.\n"
"    //\n"
"    // 0.125 leaves less aliasing, but is softer\n"
"    // 0.25 leaves more aliasing, and is sharper\n"
"    //\n"
"    #if 1\n"
"        #define FXAA_CONSOLE__PS3_EDGE_THRESHOLD 0.125\n"
"    #else\n"
"        #define FXAA_CONSOLE__PS3_EDGE_THRESHOLD 0.25\n"
"    #endif\n"
"#endif\n"
"\n"
"/*============================================================================\n"
"                        FXAA QUALITY - TUNING KNOBS\n"
"------------------------------------------------------------------------------\n"
"NOTE the other tuning knobs are now in the shader function inputs!\n"
"============================================================================*/\n"
"#ifndef FXAA_QUALITY__PRESET\n"
"    //\n"
"    // Choose the quality preset.\n"
"    // This needs to be compiled into the shader as it effects code.\n"
"    // Best option to include multiple presets is to\n"
"    // in each shader define the preset, then include this file.\n"
"    //\n"
"    // OPTIONS\n"
"    // -----------------------------------------------------------------------\n"
"    // 10 to 15 - default medium dither (10=fastest, 15=highest quality)\n"
"    // 20 to 29 - less dither, more expensive (20=fastest, 29=highest quality)\n"
"    // 39       - no dither, very expensive\n"
"    //\n"
"    // NOTES\n"
"    // -----------------------------------------------------------------------\n"
"    // 12 = slightly faster then FXAA 3.9 and higher edge quality (default)\n"
"    // 13 = about same speed as FXAA 3.9 and better than 12\n"
"    // 23 = closest to FXAA 3.9 visually and performance wise\n"
"    //  _ = the lowest digit is directly related to performance\n"
"    // _  = the highest digit is directly related to style\n"
"    //\n"
"    #define FXAA_QUALITY__PRESET 12\n"
"#endif\n"
"\n"
"\n"
"/*============================================================================\n"
"\n"
"                           FXAA QUALITY - PRESETS\n"
"\n"
"============================================================================*/\n"
"\n"
"/*============================================================================\n"
"                     FXAA QUALITY - MEDIUM DITHER PRESETS\n"
"============================================================================*/\n"
"#if (FXAA_QUALITY__PRESET == 10)\n"
"    #define FXAA_QUALITY__PS 3\n"
"    #define FXAA_QUALITY__P0 1.5\n"
"    #define FXAA_QUALITY__P1 3.0\n"
"    #define FXAA_QUALITY__P2 12.0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#if (FXAA_QUALITY__PRESET == 11)\n"
"    #define FXAA_QUALITY__PS 4\n"
"    #define FXAA_QUALITY__P0 1.0\n"
"    #define FXAA_QUALITY__P1 1.5\n"
"    #define FXAA_QUALITY__P2 3.0\n"
"    #define FXAA_QUALITY__P3 12.0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#if (FXAA_QUALITY__PRESET == 12)\n"
"    #define FXAA_QUALITY__PS 5\n"
"    #define FXAA_QUALITY__P0 1.0\n"
"    #define FXAA_QUALITY__P1 1.5\n"
"    #define FXAA_QUALITY__P2 2.0\n"
"    #define FXAA_QUALITY__P3 4.0\n"
"    #define FXAA_QUALITY__P4 12.0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#if (FXAA_QUALITY__PRESET == 13)\n"
"    #define FXAA_QUALITY__PS 6\n"
"    #define FXAA_QUALITY__P0 1.0\n"
"    #define FXAA_QUALITY__P1 1.5\n"
"    #define FXAA_QUALITY__P2 2.0\n"
"    #define FXAA_QUALITY__P3 2.0\n"
"    #define FXAA_QUALITY__P4 4.0\n"
"    #define FXAA_QUALITY__P5 12.0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#if (FXAA_QUALITY__PRESET == 14)\n"
"    #define FXAA_QUALITY__PS 7\n"
"    #define FXAA_QUALITY__P0 1.0\n"
"    #define FXAA_QUALITY__P1 1.5\n"
"    #define FXAA_QUALITY__P2 2.0\n"
"    #define FXAA_QUALITY__P3 2.0\n"
"    #define FXAA_QUALITY__P4 2.0\n"
"    #define FXAA_QUALITY__P5 4.0\n"
"    #define FXAA_QUALITY__P6 12.0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#if (FXAA_QUALITY__PRESET == 15)\n"
"    #define FXAA_QUALITY__PS 8\n"
"    #define FXAA_QUALITY__P0 1.0\n"
"    #define FXAA_QUALITY__P1 1.5\n"
"    #define FXAA_QUALITY__P2 2.0\n"
"    #define FXAA_QUALITY__P3 2.0\n"
"    #define FXAA_QUALITY__P4 2.0\n"
"    #define FXAA_QUALITY__P5 2.0\n"
"    #define FXAA_QUALITY__P6 4.0\n"
"    #define FXAA_QUALITY__P7 12.0\n"
"#endif\n"
"\n"
"/*============================================================================\n"
"                     FXAA QUALITY - LOW DITHER PRESETS\n"
"============================================================================*/\n"
"#if (FXAA_QUALITY__PRESET == 20)\n"
"    #define FXAA_QUALITY__PS 3\n"
"    #define FXAA_QUALITY__P0 1.5\n"
"    #define FXAA_QUALITY__P1 2.0\n"
"    #define FXAA_QUALITY__P2 8.0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#if (FXAA_QUALITY__PRESET == 21)\n"
"    #define FXAA_QUALITY__PS 4\n"
"    #define FXAA_QUALITY__P0 1.0\n"
"    #define FXAA_QUALITY__P1 1.5\n"
"    #define FXAA_QUALITY__P2 2.0\n"
"    #define FXAA_QUALITY__P3 8.0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#if (FXAA_QUALITY__PRESET == 22)\n"
"    #define FXAA_QUALITY__PS 5\n"
"    #define FXAA_QUALITY__P0 1.0\n"
"    #define FXAA_QUALITY__P1 1.5\n"
"    #define FXAA_QUALITY__P2 2.0\n"
"    #define FXAA_QUALITY__P3 2.0\n"
"    #define FXAA_QUALITY__P4 8.0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#if (FXAA_QUALITY__PRESET == 23)\n"
"    #define FXAA_QUALITY__PS 6\n"
"    #define FXAA_QUALITY__P0 1.0\n"
"    #define FXAA_QUALITY__P1 1.5\n"
"    #define FXAA_QUALITY__P2 2.0\n"
"    #define FXAA_QUALITY__P3 2.0\n"
"    #define FXAA_QUALITY__P4 2.0\n"
"    #define FXAA_QUALITY__P5 8.0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#if (FXAA_QUALITY__PRESET == 24)\n"
"    #define FXAA_QUALITY__PS 7\n"
"    #define FXAA_QUALITY__P0 1.0\n"
"    #define FXAA_QUALITY__P1 1.5\n"
"    #define FXAA_QUALITY__P2 2.0\n"
"    #define FXAA_QUALITY__P3 2.0\n"
"    #define FXAA_QUALITY__P4 2.0\n"
"    #define FXAA_QUALITY__P5 3.0\n"
"    #define FXAA_QUALITY__P6 8.0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#if (FXAA_QUALITY__PRESET == 25)\n"
"    #define FXAA_QUALITY__PS 8\n"
"    #define FXAA_QUALITY__P0 1.0\n"
"    #define FXAA_QUALITY__P1 1.5\n"
"    #define FXAA_QUALITY__P2 2.0\n"
"    #define FXAA_QUALITY__P3 2.0\n"
"    #define FXAA_QUALITY__P4 2.0\n"
"    #define FXAA_QUALITY__P5 2.0\n"
"    #define FXAA_QUALITY__P6 4.0\n"
"    #define FXAA_QUALITY__P7 8.0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#if (FXAA_QUALITY__PRESET == 26)\n"
"    #define FXAA_QUALITY__PS 9\n"
"    #define FXAA_QUALITY__P0 1.0\n"
"    #define FXAA_QUALITY__P1 1.5\n"
"    #define FXAA_QUALITY__P2 2.0\n"
"    #define FXAA_QUALITY__P3 2.0\n"
"    #define FXAA_QUALITY__P4 2.0\n"
"    #define FXAA_QUALITY__P5 2.0\n"
"    #define FXAA_QUALITY__P6 2.0\n"
"    #define FXAA_QUALITY__P7 4.0\n"
"    #define FXAA_QUALITY__P8 8.0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#if (FXAA_QUALITY__PRESET == 27)\n"
"    #define FXAA_QUALITY__PS 10\n"
"    #define FXAA_QUALITY__P0 1.0\n"
"    #define FXAA_QUALITY__P1 1.5\n"
"    #define FXAA_QUALITY__P2 2.0\n"
"    #define FXAA_QUALITY__P3 2.0\n"
"    #define FXAA_QUALITY__P4 2.0\n"
"    #define FXAA_QUALITY__P5 2.0\n"
"    #define FXAA_QUALITY__P6 2.0\n"
"    #define FXAA_QUALITY__P7 2.0\n"
"    #define FXAA_QUALITY__P8 4.0\n"
"    #define FXAA_QUALITY__P9 8.0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#if (FXAA_QUALITY__PRESET == 28)\n"
"    #define FXAA_QUALITY__PS 11\n"
"    #define FXAA_QUALITY__P0 1.0\n"
"    #define FXAA_QUALITY__P1 1.5\n"
"    #define FXAA_QUALITY__P2 2.0\n"
"    #define FXAA_QUALITY__P3 2.0\n"
"    #define FXAA_QUALITY__P4 2.0\n"
"    #define FXAA_QUALITY__P5 2.0\n"
"    #define FXAA_QUALITY__P6 2.0\n"
"    #define FXAA_QUALITY__P7 2.0\n"
"    #define FXAA_QUALITY__P8 2.0\n"
"    #define FXAA_QUALITY__P9 4.0\n"
"    #define FXAA_QUALITY__P10 8.0\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#if (FXAA_QUALITY__PRESET == 29)\n"
"    #define FXAA_QUALITY__PS 12\n"
"    #define FXAA_QUALITY__P0 1.0\n"
"    #define FXAA_QUALITY__P1 1.5\n"
"    #define FXAA_QUALITY__P2 2.0\n"
"    #define FXAA_QUALITY__P3 2.0\n"
"    #define FXAA_QUALITY__P4 2.0\n"
"    #define FXAA_QUALITY__P5 2.0\n"
"    #define FXAA_QUALITY__P6 2.0\n"
"    #define FXAA_QUALITY__P7 2.0\n"
"    #define FXAA_QUALITY__P8 2.0\n"
"    #define FXAA_QUALITY__P9 2.0\n"
"    #define FXAA_QUALITY__P10 4.0\n"
"    #define FXAA_QUALITY__P11 8.0\n"
"#endif\n"
"\n"
"/*============================================================================\n"
"                     FXAA QUALITY - EXTREME QUALITY\n"
"============================================================================*/\n"
"#if (FXAA_QUALITY__PRESET == 39)\n"
"    #define FXAA_QUALITY__PS 12\n"
"    #define FXAA_QUALITY__P0 1.0\n"
"    #define FXAA_QUALITY__P1 1.0\n"
"    #define FXAA_QUALITY__P2 1.0\n"
"    #define FXAA_QUALITY__P3 1.0\n"
"    #define FXAA_QUALITY__P4 1.0\n"
"    #define FXAA_QUALITY__P5 1.5\n"
"    #define FXAA_QUALITY__P6 2.0\n"
"    #define FXAA_QUALITY__P7 2.0\n"
"    #define FXAA_QUALITY__P8 2.0\n"
"    #define FXAA_QUALITY__P9 2.0\n"
"    #define FXAA_QUALITY__P10 4.0\n"
"    #define FXAA_QUALITY__P11 8.0\n"
"#endif\n"
"\n"
"\n"
"\n"
"/*============================================================================\n"
"\n"
"                                API PORTING\n"
"\n"
"============================================================================*/\n"
"#if (FXAA_GLSL_120 == 1) || (FXAA_GLSL_130 == 1)\n"
"    #define FxaaBool bool\n"
"    #define FxaaDiscard discard\n"
"    #define FxaaFloat float\n"
"    #define FxaaFloat2 vec2\n"
"    #define FxaaFloat3 vec3\n"
"    #define FxaaFloat4 vec4\n"
"    #define FxaaHalf float\n"
"    #define FxaaHalf2 vec2\n"
"    #define FxaaHalf3 vec3\n"
"    #define FxaaHalf4 vec4\n"
"    #define FxaaInt2 ivec2\n"
"    #define FxaaSat(x) clamp(x, 0.0, 1.0)\n"
"    #define FxaaTex sampler2D\n"
"#else\n"
"    #define FxaaBool bool\n"
"    #define FxaaDiscard clip(-1)\n"
"    #define FxaaFloat float\n"
"    #define FxaaFloat2 float2\n"
"    #define FxaaFloat3 float3\n"
"    #define FxaaFloat4 float4\n"
"    #define FxaaHalf half\n"
"    #define FxaaHalf2 half2\n"
"    #define FxaaHalf3 half3\n"
"    #define FxaaHalf4 half4\n"
"    #define FxaaSat(x) saturate(x)\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#if (FXAA_GLSL_120 == 1)\n"
"    // Requires,\n"
"    //  #version 120\n"
"    // And at least,\n"
"    //  #extension GL_EXT_gpu_shader4 : enable\n"
"    //  (or set FXAA_FAST_PIXEL_OFFSET 1 to work like DX9)\n"
"    #define FxaaTexTop(t, p) texture2DLod(t, p, 0.0)\n"
"    #if (FXAA_FAST_PIXEL_OFFSET == 1)\n"
"        #define FxaaTexOff(t, p, o, r) texture2DLodOffset(t, p, 0.0, o)\n"
"    #else\n"
"        #define FxaaTexOff(t, p, o, r) texture2DLod(t, p + (o * r), 0.0)\n"
"    #endif\n"
"    #if (FXAA_GATHER4_ALPHA == 1)\n"
"        // use #extension GL_ARB_gpu_shader5 : enable\n"
"        #define FxaaTexAlpha4(t, p) textureGather(t, p, 3)\n"
"        #define FxaaTexOffAlpha4(t, p, o) textureGatherOffset(t, p, o, 3)\n"
"        #define FxaaTexGreen4(t, p) textureGather(t, p, 1)\n"
"        #define FxaaTexOffGreen4(t, p, o) textureGatherOffset(t, p, o, 1)\n"
"    #endif\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#if (FXAA_GLSL_130 == 1)\n"
"    // Requires \"#version 130\" or better\n"
"    #define FxaaTexTop(t, p) textureLod(t, p, 0.0)\n"
"    #define FxaaTexOff(t, p, o, r) textureLodOffset(t, p, 0.0, o)\n"
"    #if (FXAA_GATHER4_ALPHA == 1)\n"
"        // use #extension GL_ARB_gpu_shader5 : enable\n"
"        #define FxaaTexAlpha4(t, p) textureGather(t, p, 3)\n"
"        #define FxaaTexOffAlpha4(t, p, o) textureGatherOffset(t, p, o, 3)\n"
"        #define FxaaTexGreen4(t, p) textureGather(t, p, 1)\n"
"        #define FxaaTexOffGreen4(t, p, o) textureGatherOffset(t, p, o, 1)\n"
"    #endif\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#if (FXAA_HLSL_3 == 1) || (FXAA_360 == 1) || (FXAA_PS3 == 1)\n"
"    #define FxaaInt2 float2\n"
"    #define FxaaTex sampler2D\n"
"    #define FxaaTexTop(t, p) tex2Dlod(t, float4(p, 0.0, 0.0))\n"
"    #define FxaaTexOff(t, p, o, r) tex2Dlod(t, float4(p + (o * r), 0, 0))\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#if (FXAA_HLSL_4 == 1)\n"
"    #define FxaaInt2 int2\n"
"    struct FxaaTex { SamplerState smpl; Texture2D tex; };\n"
"    #define FxaaTexTop(t, p) t.tex.SampleLevel(t.smpl, p, 0.0)\n"
"    #define FxaaTexOff(t, p, o, r) t.tex.SampleLevel(t.smpl, p, 0.0, o)\n"
"#endif\n"
"/*--------------------------------------------------------------------------*/\n"
"#if (FXAA_HLSL_5 == 1)\n"
"    #define FxaaInt2 int2\n"
"    struct FxaaTex { SamplerState smpl; Texture2D tex; };\n"
"    #define FxaaTexTop(t, p) t.tex.SampleLevel(t.smpl, p, 0.0)\n"
"    #define FxaaTexOff(t, p, o, r) t.tex.SampleLevel(t.smpl, p, 0.0, o)\n"
"    #define FxaaTexAlpha4(t, p) t.tex.GatherAlpha(t.smpl, p)\n"
"    #define FxaaTexOffAlpha4(t, p, o) t.tex.GatherAlpha(t.smpl, p, o)\n"
"    #define FxaaTexGreen4(t, p) t.tex.GatherGreen(t.smpl, p)\n"
"    #define FxaaTexOffGreen4(t, p, o) t.tex.GatherGreen(t.smpl, p, o)\n"
"#endif\n"
"\n"
"\n"
"/*============================================================================\n"
"                   GREEN AS LUMA OPTION SUPPORT FUNCTION\n"
"============================================================================*/\n"
"#if (FXAA_GREEN_AS_LUMA == 0)\n"
"    FxaaFloat FxaaLuma(FxaaFloat4 rgba) { return rgba.w; }\n"
"#else\n"
"    FxaaFloat FxaaLuma(FxaaFloat4 rgba) { return rgba.y; }\n"
"#endif\n"
"\n"
"\n"
"\n"
"\n"
"/*============================================================================\n"
"\n"
"                             FXAA3 QUALITY - PC\n"
"\n"
"============================================================================*/\n"
"#if (FXAA_PC == 1)\n"
"/*--------------------------------------------------------------------------*/\n"
"FxaaFloat4 FxaaPixelShader(\n"
"    //\n"
"    // Use noperspective interpolation here (turn off perspective interpolation).\n"
"    // {xy} = center of pixel\n"
"    FxaaFloat2 pos,\n"
"    //\n"
"    // Used only for FXAA Console, and not used on the 360 version.\n"
"    // Use noperspective interpolation here (turn off perspective interpolation).\n"
"    // {xy__} = upper left of pixel\n"
"    // {__zw} = lower right of pixel\n"
"    FxaaFloat4 fxaaConsolePosPos,\n"
"    //\n"
"    // Input color texture.\n"
"    // {rgb_} = color in linear or perceptual color space\n"
"    // if (FXAA_GREEN_AS_LUMA == 0)\n"
"    //     {___a} = luma in perceptual color space (not linear)\n"
"    FxaaTex tex,\n"
"    //\n"
"    // Only used on the optimized 360 version of FXAA Console.\n"
"    // For everything but 360, just use the same input here as for \"tex\".\n"
"    // For 360, same texture, just alias with a 2nd sampler.\n"
"    // This sampler needs to have an exponent bias of -1.\n"
"    FxaaTex fxaaConsole360TexExpBiasNegOne,\n"
"    //\n"
"    // Only used on the optimized 360 version of FXAA Console.\n"
"    // For everything but 360, just use the same input here as for \"tex\".\n"
"    // For 360, same texture, just alias with a 3nd sampler.\n"
"    // This sampler needs to have an exponent bias of -2.\n"
"    FxaaTex fxaaConsole360TexExpBiasNegTwo,\n"
"    //\n"
"    // Only used on FXAA Quality.\n"
"    // This must be from a constant/uniform.\n"
"    // {x_} = 1.0/screenWidthInPixels\n"
"    // {_y} = 1.0/screenHeightInPixels\n"
"    FxaaFloat2 fxaaQualityRcpFrame,\n"
"    //\n"
"    // Only used on FXAA Console.\n"
"    // This must be from a constant/uniform.\n"
"    // This effects sub-pixel AA quality and inversely sharpness.\n"
"    //   Where N ranges between,\n"
"    //     N = 0.50 (default)\n"
"    //     N = 0.33 (sharper)\n"
"    // {x___} = -N/screenWidthInPixels\n"
"    // {_y__} = -N/screenHeightInPixels\n"
"    // {__z_} =  N/screenWidthInPixels\n"
"    // {___w} =  N/screenHeightInPixels\n"
"    FxaaFloat4 fxaaConsoleRcpFrameOpt,\n"
"    //\n"
"    // Only used on FXAA Console.\n"
"    // Not used on 360, but used on PS3 and PC.\n"
"    // This must be from a constant/uniform.\n"
"    // {x___} = -2.0/screenWidthInPixels\n"
"    // {_y__} = -2.0/screenHeightInPixels\n"
"    // {__z_} =  2.0/screenWidthInPixels\n"
"    // {___w} =  2.0/screenHeightInPixels\n"
"    FxaaFloat4 fxaaConsoleRcpFrameOpt2,\n"
"    //\n"
"    // Only used on FXAA Console.\n"
"    // Only used on 360 in place of fxaaConsoleRcpFrameOpt2.\n"
"    // This must be from a constant/uniform.\n"
"    // {x___} =  8.0/screenWidthInPixels\n"
"    // {_y__} =  8.0/screenHeightInPixels\n"
"    // {__z_} = -4.0/screenWidthInPixels\n"
"    // {___w} = -4.0/screenHeightInPixels\n"
"    FxaaFloat4 fxaaConsole360RcpFrameOpt2,\n"
"    //\n"
"    // Only used on FXAA Quality.\n"
"    // This used to be the FXAA_QUALITY__SUBPIX define.\n"
"    // It is here now to allow easier tuning.\n"
"    // Choose the amount of sub-pixel aliasing removal.\n"
"    // This can effect sharpness.\n"
"    //   1.00 - upper limit (softer)\n"
"    //   0.75 - default amount of filtering\n"
"    //   0.50 - lower limit (sharper, less sub-pixel aliasing removal)\n"
"    //   0.25 - almost off\n"
"    //   0.00 - completely off\n"
"    FxaaFloat fxaaQualitySubpix,\n"
"    //\n"
"    // Only used on FXAA Quality.\n"
"    // This used to be the FXAA_QUALITY__EDGE_THRESHOLD define.\n"
"    // It is here now to allow easier tuning.\n"
"    // The minimum amount of local contrast required to apply algorithm.\n"
"    //   0.333 - too little (faster)\n"
"    //   0.250 - low quality\n"
"    //   0.166 - default\n"
"    //   0.125 - high quality\n"
"    //   0.063 - overkill (slower)\n"
"    FxaaFloat fxaaQualityEdgeThreshold,\n"
"    //\n"
"    // Only used on FXAA Quality.\n"
"    // This used to be the FXAA_QUALITY__EDGE_THRESHOLD_MIN define.\n"
"    // It is here now to allow easier tuning.\n"
"    // Trims the algorithm from processing darks.\n"
"    //   0.0833 - upper limit (default, the start of visible unfiltered edges)\n"
"    //   0.0625 - high quality (faster)\n"
"    //   0.0312 - visible limit (slower)\n"
"    // Special notes when using FXAA_GREEN_AS_LUMA,\n"
"    //   Likely want to set this to zero.\n"
"    //   As colors that are mostly not-green\n"
"    //   will appear very dark in the green channel!\n"
"    //   Tune by looking at mostly non-green content,\n"
"    //   then start at zero and increase until aliasing is a problem.\n"
"    FxaaFloat fxaaQualityEdgeThresholdMin,\n"
"    //\n"
"    // Only used on FXAA Console.\n"
"    // This used to be the FXAA_CONSOLE__EDGE_SHARPNESS define.\n"
"    // It is here now to allow easier tuning.\n"
"    // This does not effect PS3, as this needs to be compiled in.\n"
"    //   Use FXAA_CONSOLE__PS3_EDGE_SHARPNESS for PS3.\n"
"    //   Due to the PS3 being ALU bound,\n"
"    //   there are only three safe values here: 2 and 4 and 8.\n"
"    //   These options use the shaders ability to a free *|/ by 2|4|8.\n"
"    // For all other platforms can be a non-power of two.\n"
"    //   8.0 is sharper (default!!!)\n"
"    //   4.0 is softer\n"
"    //   2.0 is really soft (good only for vector graphics inputs)\n"
"    FxaaFloat fxaaConsoleEdgeSharpness,\n"
"    //\n"
"    // Only used on FXAA Console.\n"
"    // This used to be the FXAA_CONSOLE__EDGE_THRESHOLD define.\n"
"    // It is here now to allow easier tuning.\n"
"    // This does not effect PS3, as this needs to be compiled in.\n"
"    //   Use FXAA_CONSOLE__PS3_EDGE_THRESHOLD for PS3.\n"
"    //   Due to the PS3 being ALU bound,\n"
"    //   there are only two safe values here: 1/4 and 1/8.\n"
"    //   These options use the shaders ability to a free *|/ by 2|4|8.\n"
"    // The console setting has a different mapping than the quality setting.\n"
"    // Other platforms can use other values.\n"
"    //   0.125 leaves less aliasing, but is softer (default!!!)\n"
"    //   0.25 leaves more aliasing, and is sharper\n"
"    FxaaFloat fxaaConsoleEdgeThreshold,\n"
"    //\n"
"    // Only used on FXAA Console.\n"
"    // This used to be the FXAA_CONSOLE__EDGE_THRESHOLD_MIN define.\n"
"    // It is here now to allow easier tuning.\n"
"    // Trims the algorithm from processing darks.\n"
"    // The console setting has a different mapping than the quality setting.\n"
"    // This only applies when FXAA_EARLY_EXIT is 1.\n"
"    // This does not apply to PS3,\n"
"    // PS3 was simplified to avoid more shader instructions.\n"
"    //   0.06 - faster but more aliasing in darks\n"
"    //   0.05 - default\n"
"    //   0.04 - slower and less aliasing in darks\n"
"    // Special notes when using FXAA_GREEN_AS_LUMA,\n"
"    //   Likely want to set this to zero.\n"
"    //   As colors that are mostly not-green\n"
"    //   will appear very dark in the green channel!\n"
"    //   Tune by looking at mostly non-green content,\n"
"    //   then start at zero and increase until aliasing is a problem.\n"
"    FxaaFloat fxaaConsoleEdgeThresholdMin,\n"
"    //\n"
"    // Extra constants for 360 FXAA Console only.\n"
"    // Use zeros or anything else for other platforms.\n"
"    // These must be in physical constant registers and NOT immedates.\n"
"    // Immedates will result in compiler un-optimizing.\n"
"    // {xyzw} = float4(1.0, -1.0, 0.25, -0.25)\n"
"    FxaaFloat4 fxaaConsole360ConstDir\n"
") {\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat2 posM;\n"
"    posM.x = pos.x;\n"
"    posM.y = pos.y;\n"
"    #if (FXAA_GATHER4_ALPHA == 1)\n"
"        #if (FXAA_DISCARD == 0)\n"
"            FxaaFloat4 rgbyM = FxaaTexTop(tex, posM);\n"
"            #if (FXAA_GREEN_AS_LUMA == 0)\n"
"                #define lumaM rgbyM.w\n"
"            #else\n"
"                #define lumaM rgbyM.y\n"
"            #endif\n"
"        #endif\n"
"        #if (FXAA_GREEN_AS_LUMA == 0)\n"
"            FxaaFloat4 luma4A = FxaaTexAlpha4(tex, posM);\n"
"            FxaaFloat4 luma4B = FxaaTexOffAlpha4(tex, posM, FxaaInt2(-1, -1));\n"
"        #else\n"
"            FxaaFloat4 luma4A = FxaaTexGreen4(tex, posM);\n"
"            FxaaFloat4 luma4B = FxaaTexOffGreen4(tex, posM, FxaaInt2(-1, -1));\n"
"        #endif\n"
"        #if (FXAA_DISCARD == 1)\n"
"            #define lumaM luma4A.w\n"
"        #endif\n"
"        #define lumaE luma4A.z\n"
"        #define lumaS luma4A.x\n"
"        #define lumaSE luma4A.y\n"
"        #define lumaNW luma4B.w\n"
"        #define lumaN luma4B.z\n"
"        #define lumaW luma4B.x\n"
"    #else\n"
"        FxaaFloat4 rgbyM = FxaaTexTop(tex, posM);\n"
"        #if (FXAA_GREEN_AS_LUMA == 0)\n"
"            #define lumaM rgbyM.w\n"
"        #else\n"
"            #define lumaM rgbyM.y\n"
"        #endif\n"
"        FxaaFloat lumaS = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 0, 1), fxaaQualityRcpFrame.xy));\n"
"        FxaaFloat lumaE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 1, 0), fxaaQualityRcpFrame.xy));\n"
"        FxaaFloat lumaN = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 0,-1), fxaaQualityRcpFrame.xy));\n"
"        FxaaFloat lumaW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1, 0), fxaaQualityRcpFrame.xy));\n"
"    #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat maxSM = max(lumaS, lumaM);\n"
"    FxaaFloat minSM = min(lumaS, lumaM);\n"
"    FxaaFloat maxESM = max(lumaE, maxSM);\n"
"    FxaaFloat minESM = min(lumaE, minSM);\n"
"    FxaaFloat maxWN = max(lumaN, lumaW);\n"
"    FxaaFloat minWN = min(lumaN, lumaW);\n"
"    FxaaFloat rangeMax = max(maxWN, maxESM);\n"
"    FxaaFloat rangeMin = min(minWN, minESM);\n"
"    FxaaFloat rangeMaxScaled = rangeMax * fxaaQualityEdgeThreshold;\n"
"    FxaaFloat range = rangeMax - rangeMin;\n"
"    FxaaFloat rangeMaxClamped = max(fxaaQualityEdgeThresholdMin, rangeMaxScaled);\n"
"    FxaaBool earlyExit = range < rangeMaxClamped;\n"
"/*--------------------------------------------------------------------------*/\n"
"    if(earlyExit)\n"
"        #if (FXAA_DISCARD == 1)\n"
"            FxaaDiscard;\n"
"        #else\n"
"            return rgbyM;\n"
"        #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"    #if (FXAA_GATHER4_ALPHA == 0)\n"
"        FxaaFloat lumaNW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1,-1), fxaaQualityRcpFrame.xy));\n"
"        FxaaFloat lumaSE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 1, 1), fxaaQualityRcpFrame.xy));\n"
"        FxaaFloat lumaNE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2( 1,-1), fxaaQualityRcpFrame.xy));\n"
"        FxaaFloat lumaSW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1, 1), fxaaQualityRcpFrame.xy));\n"
"    #else\n"
"        FxaaFloat lumaNE = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(1, -1), fxaaQualityRcpFrame.xy));\n"
"        FxaaFloat lumaSW = FxaaLuma(FxaaTexOff(tex, posM, FxaaInt2(-1, 1), fxaaQualityRcpFrame.xy));\n"
"    #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat lumaNS = lumaN + lumaS;\n"
"    FxaaFloat lumaWE = lumaW + lumaE;\n"
"    FxaaFloat subpixRcpRange = 1.0/range;\n"
"    FxaaFloat subpixNSWE = lumaNS + lumaWE;\n"
"    FxaaFloat edgeHorz1 = (-2.0 * lumaM) + lumaNS;\n"
"    FxaaFloat edgeVert1 = (-2.0 * lumaM) + lumaWE;\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat lumaNESE = lumaNE + lumaSE;\n"
"    FxaaFloat lumaNWNE = lumaNW + lumaNE;\n"
"    FxaaFloat edgeHorz2 = (-2.0 * lumaE) + lumaNESE;\n"
"    FxaaFloat edgeVert2 = (-2.0 * lumaN) + lumaNWNE;\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat lumaNWSW = lumaNW + lumaSW;\n"
"    FxaaFloat lumaSWSE = lumaSW + lumaSE;\n"
"    FxaaFloat edgeHorz4 = (abs(edgeHorz1) * 2.0) + abs(edgeHorz2);\n"
"    FxaaFloat edgeVert4 = (abs(edgeVert1) * 2.0) + abs(edgeVert2);\n"
"    FxaaFloat edgeHorz3 = (-2.0 * lumaW) + lumaNWSW;\n"
"    FxaaFloat edgeVert3 = (-2.0 * lumaS) + lumaSWSE;\n"
"    FxaaFloat edgeHorz = abs(edgeHorz3) + edgeHorz4;\n"
"    FxaaFloat edgeVert = abs(edgeVert3) + edgeVert4;\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat subpixNWSWNESE = lumaNWSW + lumaNESE;\n"
"    FxaaFloat lengthSign = fxaaQualityRcpFrame.x;\n"
"    FxaaBool horzSpan = edgeHorz >= edgeVert;\n"
"    FxaaFloat subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;\n"
"/*--------------------------------------------------------------------------*/\n"
"    if(!horzSpan) lumaN = lumaW;\n"
"    if(!horzSpan) lumaS = lumaE;\n"
"    if(horzSpan) lengthSign = fxaaQualityRcpFrame.y;\n"
"    FxaaFloat subpixB = (subpixA * (1.0/12.0)) - lumaM;\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat gradientN = lumaN - lumaM;\n"
"    FxaaFloat gradientS = lumaS - lumaM;\n"
"    FxaaFloat lumaNN = lumaN + lumaM;\n"
"    FxaaFloat lumaSS = lumaS + lumaM;\n"
"    FxaaBool pairN = abs(gradientN) >= abs(gradientS);\n"
"    FxaaFloat gradient = max(abs(gradientN), abs(gradientS));\n"
"    if(pairN) lengthSign = -lengthSign;\n"
"    FxaaFloat subpixC = FxaaSat(abs(subpixB) * subpixRcpRange);\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat2 posB;\n"
"    posB.x = posM.x;\n"
"    posB.y = posM.y;\n"
"    FxaaFloat2 offNP;\n"
"    offNP.x = (!horzSpan) ? 0.0 : fxaaQualityRcpFrame.x;\n"
"    offNP.y = ( horzSpan) ? 0.0 : fxaaQualityRcpFrame.y;\n"
"    if(!horzSpan) posB.x += lengthSign * 0.5;\n"
"    if( horzSpan) posB.y += lengthSign * 0.5;\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat2 posN;\n"
"    posN.x = posB.x - offNP.x * FXAA_QUALITY__P0;\n"
"    posN.y = posB.y - offNP.y * FXAA_QUALITY__P0;\n"
"    FxaaFloat2 posP;\n"
"    posP.x = posB.x + offNP.x * FXAA_QUALITY__P0;\n"
"    posP.y = posB.y + offNP.y * FXAA_QUALITY__P0;\n"
"    FxaaFloat subpixD = ((-2.0)*subpixC) + 3.0;\n"
"    FxaaFloat lumaEndN = FxaaLuma(FxaaTexTop(tex, posN));\n"
"    FxaaFloat subpixE = subpixC * subpixC;\n"
"    FxaaFloat lumaEndP = FxaaLuma(FxaaTexTop(tex, posP));\n"
"/*--------------------------------------------------------------------------*/\n"
"    if(!pairN) lumaNN = lumaSS;\n"
"    FxaaFloat gradientScaled = gradient * 1.0/4.0;\n"
"    FxaaFloat lumaMM = lumaM - lumaNN * 0.5;\n"
"    FxaaFloat subpixF = subpixD * subpixE;\n"
"    FxaaBool lumaMLTZero = lumaMM < 0.0;\n"
"/*--------------------------------------------------------------------------*/\n"
"    lumaEndN -= lumaNN * 0.5;\n"
"    lumaEndP -= lumaNN * 0.5;\n"
"    FxaaBool doneN = abs(lumaEndN) >= gradientScaled;\n"
"    FxaaBool doneP = abs(lumaEndP) >= gradientScaled;\n"
"    if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P1;\n"
"    if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P1;\n"
"    FxaaBool doneNP = (!doneN) || (!doneP);\n"
"    if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P1;\n"
"    if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P1;\n"
"/*--------------------------------------------------------------------------*/\n"
"    if(doneNP) {\n"
"        if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"        if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"        doneN = abs(lumaEndN) >= gradientScaled;\n"
"        doneP = abs(lumaEndP) >= gradientScaled;\n"
"        if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P2;\n"
"        if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P2;\n"
"        doneNP = (!doneN) || (!doneP);\n"
"        if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P2;\n"
"        if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P2;\n"
"/*--------------------------------------------------------------------------*/\n"
"        #if (FXAA_QUALITY__PS > 3)\n"
"        if(doneNP) {\n"
"            if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"            if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"            if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"            if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"            doneN = abs(lumaEndN) >= gradientScaled;\n"
"            doneP = abs(lumaEndP) >= gradientScaled;\n"
"            if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P3;\n"
"            if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P3;\n"
"            doneNP = (!doneN) || (!doneP);\n"
"            if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P3;\n"
"            if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P3;\n"
"/*--------------------------------------------------------------------------*/\n"
"            #if (FXAA_QUALITY__PS > 4)\n"
"            if(doneNP) {\n"
"                if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"                if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"                if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"                if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"                doneN = abs(lumaEndN) >= gradientScaled;\n"
"                doneP = abs(lumaEndP) >= gradientScaled;\n"
"                if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P4;\n"
"                if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P4;\n"
"                doneNP = (!doneN) || (!doneP);\n"
"                if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P4;\n"
"                if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P4;\n"
"/*--------------------------------------------------------------------------*/\n"
"                #if (FXAA_QUALITY__PS > 5)\n"
"                if(doneNP) {\n"
"                    if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"                    if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"                    if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"                    if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"                    doneN = abs(lumaEndN) >= gradientScaled;\n"
"                    doneP = abs(lumaEndP) >= gradientScaled;\n"
"                    if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P5;\n"
"                    if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P5;\n"
"                    doneNP = (!doneN) || (!doneP);\n"
"                    if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P5;\n"
"                    if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P5;\n"
"/*--------------------------------------------------------------------------*/\n"
"                    #if (FXAA_QUALITY__PS > 6)\n"
"                    if(doneNP) {\n"
"                        if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"                        if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"                        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"                        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"                        doneN = abs(lumaEndN) >= gradientScaled;\n"
"                        doneP = abs(lumaEndP) >= gradientScaled;\n"
"                        if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P6;\n"
"                        if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P6;\n"
"                        doneNP = (!doneN) || (!doneP);\n"
"                        if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P6;\n"
"                        if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P6;\n"
"/*--------------------------------------------------------------------------*/\n"
"                        #if (FXAA_QUALITY__PS > 7)\n"
"                        if(doneNP) {\n"
"                            if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"                            if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"                            if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"                            if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"                            doneN = abs(lumaEndN) >= gradientScaled;\n"
"                            doneP = abs(lumaEndP) >= gradientScaled;\n"
"                            if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P7;\n"
"                            if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P7;\n"
"                            doneNP = (!doneN) || (!doneP);\n"
"                            if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P7;\n"
"                            if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P7;\n"
"/*--------------------------------------------------------------------------*/\n"
"    #if (FXAA_QUALITY__PS > 8)\n"
"    if(doneNP) {\n"
"        if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"        if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"        doneN = abs(lumaEndN) >= gradientScaled;\n"
"        doneP = abs(lumaEndP) >= gradientScaled;\n"
"        if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P8;\n"
"        if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P8;\n"
"        doneNP = (!doneN) || (!doneP);\n"
"        if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P8;\n"
"        if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P8;\n"
"/*--------------------------------------------------------------------------*/\n"
"        #if (FXAA_QUALITY__PS > 9)\n"
"        if(doneNP) {\n"
"            if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"            if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"            if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"            if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"            doneN = abs(lumaEndN) >= gradientScaled;\n"
"            doneP = abs(lumaEndP) >= gradientScaled;\n"
"            if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P9;\n"
"            if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P9;\n"
"            doneNP = (!doneN) || (!doneP);\n"
"            if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P9;\n"
"            if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P9;\n"
"/*--------------------------------------------------------------------------*/\n"
"            #if (FXAA_QUALITY__PS > 10)\n"
"            if(doneNP) {\n"
"                if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"                if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"                if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"                if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"                doneN = abs(lumaEndN) >= gradientScaled;\n"
"                doneP = abs(lumaEndP) >= gradientScaled;\n"
"                if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P10;\n"
"                if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P10;\n"
"                doneNP = (!doneN) || (!doneP);\n"
"                if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P10;\n"
"                if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P10;\n"
"/*--------------------------------------------------------------------------*/\n"
"                #if (FXAA_QUALITY__PS > 11)\n"
"                if(doneNP) {\n"
"                    if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"                    if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"                    if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"                    if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"                    doneN = abs(lumaEndN) >= gradientScaled;\n"
"                    doneP = abs(lumaEndP) >= gradientScaled;\n"
"                    if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P11;\n"
"                    if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P11;\n"
"                    doneNP = (!doneN) || (!doneP);\n"
"                    if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P11;\n"
"                    if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P11;\n"
"/*--------------------------------------------------------------------------*/\n"
"                    #if (FXAA_QUALITY__PS > 12)\n"
"                    if(doneNP) {\n"
"                        if(!doneN) lumaEndN = FxaaLuma(FxaaTexTop(tex, posN.xy));\n"
"                        if(!doneP) lumaEndP = FxaaLuma(FxaaTexTop(tex, posP.xy));\n"
"                        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
"                        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
"                        doneN = abs(lumaEndN) >= gradientScaled;\n"
"                        doneP = abs(lumaEndP) >= gradientScaled;\n"
"                        if(!doneN) posN.x -= offNP.x * FXAA_QUALITY__P12;\n"
"                        if(!doneN) posN.y -= offNP.y * FXAA_QUALITY__P12;\n"
"                        doneNP = (!doneN) || (!doneP);\n"
"                        if(!doneP) posP.x += offNP.x * FXAA_QUALITY__P12;\n"
"                        if(!doneP) posP.y += offNP.y * FXAA_QUALITY__P12;\n"
"/*--------------------------------------------------------------------------*/\n"
"                    }\n"
"                    #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"                }\n"
"                #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"            }\n"
"            #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"        }\n"
"        #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"    }\n"
"    #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"                        }\n"
"                        #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"                    }\n"
"                    #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"                }\n"
"                #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"            }\n"
"            #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"        }\n"
"        #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"    }\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat dstN = posM.x - posN.x;\n"
"    FxaaFloat dstP = posP.x - posM.x;\n"
"    if(!horzSpan) dstN = posM.y - posN.y;\n"
"    if(!horzSpan) dstP = posP.y - posM.y;\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaBool goodSpanN = (lumaEndN < 0.0) != lumaMLTZero;\n"
"    FxaaFloat spanLength = (dstP + dstN);\n"
"    FxaaBool goodSpanP = (lumaEndP < 0.0) != lumaMLTZero;\n"
"    FxaaFloat spanLengthRcp = 1.0/spanLength;\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaBool directionN = dstN < dstP;\n"
"    FxaaFloat dst = min(dstN, dstP);\n"
"    FxaaBool goodSpan = directionN ? goodSpanN : goodSpanP;\n"
"    FxaaFloat subpixG = subpixF * subpixF;\n"
"    FxaaFloat pixelOffset = (dst * (-spanLengthRcp)) + 0.5;\n"
"    FxaaFloat subpixH = subpixG * fxaaQualitySubpix;\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat pixelOffsetGood = goodSpan ? pixelOffset : 0.0;\n"
"    FxaaFloat pixelOffsetSubpix = max(pixelOffsetGood, subpixH);\n"
"    if(!horzSpan) posM.x += pixelOffsetSubpix * lengthSign;\n"
"    if( horzSpan) posM.y += pixelOffsetSubpix * lengthSign;\n"
"    #if (FXAA_DISCARD == 1)\n"
"        return FxaaTexTop(tex, posM);\n"
"    #else\n"
"        return FxaaFloat4(FxaaTexTop(tex, posM).xyz, lumaM);\n"
"    #endif\n"
"}\n"
"/*==========================================================================*/\n"
"#endif\n"
"\n"
"\n"
"\n"
"\n"
"/*============================================================================\n"
"\n"
"                         FXAA3 CONSOLE - PC VERSION\n"
"                         \n"
"------------------------------------------------------------------------------\n"
"Instead of using this on PC, I'd suggest just using FXAA Quality with\n"
"    #define FXAA_QUALITY__PRESET 10\n"
"Or\n"
"    #define FXAA_QUALITY__PRESET 20\n"
"Either are higher qualilty and almost as fast as this on modern PC GPUs.\n"
"============================================================================*/\n"
"#if (FXAA_PC_CONSOLE == 1)\n"
"/*--------------------------------------------------------------------------*/\n"
"FxaaFloat4 FxaaPixelShader(\n"
"    // See FXAA Quality FxaaPixelShader() source for docs on Inputs!\n"
"    FxaaFloat2 pos,\n"
"    FxaaFloat4 fxaaConsolePosPos,\n"
"    FxaaTex tex,\n"
"    FxaaTex fxaaConsole360TexExpBiasNegOne,\n"
"    FxaaTex fxaaConsole360TexExpBiasNegTwo,\n"
"    FxaaFloat2 fxaaQualityRcpFrame,\n"
"    FxaaFloat4 fxaaConsoleRcpFrameOpt,\n"
"    FxaaFloat4 fxaaConsoleRcpFrameOpt2,\n"
"    FxaaFloat4 fxaaConsole360RcpFrameOpt2,\n"
"    FxaaFloat fxaaQualitySubpix,\n"
"    FxaaFloat fxaaQualityEdgeThreshold,\n"
"    FxaaFloat fxaaQualityEdgeThresholdMin,\n"
"    FxaaFloat fxaaConsoleEdgeSharpness,\n"
"    FxaaFloat fxaaConsoleEdgeThreshold,\n"
"    FxaaFloat fxaaConsoleEdgeThresholdMin,\n"
"    FxaaFloat4 fxaaConsole360ConstDir\n"
") {\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat lumaNw = FxaaLuma(FxaaTexTop(tex, fxaaConsolePosPos.xy));\n"
"    FxaaFloat lumaSw = FxaaLuma(FxaaTexTop(tex, fxaaConsolePosPos.xw));\n"
"    FxaaFloat lumaNe = FxaaLuma(FxaaTexTop(tex, fxaaConsolePosPos.zy));\n"
"    FxaaFloat lumaSe = FxaaLuma(FxaaTexTop(tex, fxaaConsolePosPos.zw));\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat4 rgbyM = FxaaTexTop(tex, pos.xy);\n"
"    #if (FXAA_GREEN_AS_LUMA == 0)\n"
"        FxaaFloat lumaM = rgbyM.w;\n"
"    #else\n"
"        FxaaFloat lumaM = rgbyM.y;\n"
"    #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat lumaMaxNwSw = max(lumaNw, lumaSw);\n"
"    lumaNe += 1.0/384.0;\n"
"    FxaaFloat lumaMinNwSw = min(lumaNw, lumaSw);\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat lumaMaxNeSe = max(lumaNe, lumaSe);\n"
"    FxaaFloat lumaMinNeSe = min(lumaNe, lumaSe);\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat lumaMax = max(lumaMaxNeSe, lumaMaxNwSw);\n"
"    FxaaFloat lumaMin = min(lumaMinNeSe, lumaMinNwSw);\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat lumaMaxScaled = lumaMax * fxaaConsoleEdgeThreshold;\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat lumaMinM = min(lumaMin, lumaM);\n"
"    FxaaFloat lumaMaxScaledClamped = max(fxaaConsoleEdgeThresholdMin, lumaMaxScaled);\n"
"    FxaaFloat lumaMaxM = max(lumaMax, lumaM);\n"
"    FxaaFloat dirSwMinusNe = lumaSw - lumaNe;\n"
"    FxaaFloat lumaMaxSubMinM = lumaMaxM - lumaMinM;\n"
"    FxaaFloat dirSeMinusNw = lumaSe - lumaNw;\n"
"    if(lumaMaxSubMinM < lumaMaxScaledClamped) return rgbyM;\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat2 dir;\n"
"    dir.x = dirSwMinusNe + dirSeMinusNw;\n"
"    dir.y = dirSwMinusNe - dirSeMinusNw;\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat2 dir1 = normalize(dir.xy);\n"
"    FxaaFloat4 rgbyN1 = FxaaTexTop(tex, pos.xy - dir1 * fxaaConsoleRcpFrameOpt.zw);\n"
"    FxaaFloat4 rgbyP1 = FxaaTexTop(tex, pos.xy + dir1 * fxaaConsoleRcpFrameOpt.zw);\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat dirAbsMinTimesC = min(abs(dir1.x), abs(dir1.y)) * fxaaConsoleEdgeSharpness;\n"
"    FxaaFloat2 dir2 = clamp(dir1.xy / dirAbsMinTimesC, -2.0, 2.0);\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat4 rgbyN2 = FxaaTexTop(tex, pos.xy - dir2 * fxaaConsoleRcpFrameOpt2.zw);\n"
"    FxaaFloat4 rgbyP2 = FxaaTexTop(tex, pos.xy + dir2 * fxaaConsoleRcpFrameOpt2.zw);\n"
"/*--------------------------------------------------------------------------*/\n"
"    FxaaFloat4 rgbyA = rgbyN1 + rgbyP1;\n"
"    FxaaFloat4 rgbyB = ((rgbyN2 + rgbyP2) * 0.25) + (rgbyA * 0.25);\n"
"/*--------------------------------------------------------------------------*/\n"
"    #if (FXAA_GREEN_AS_LUMA == 0)\n"
"        FxaaBool twoTap = (rgbyB.w < lumaMin) || (rgbyB.w > lumaMax);\n"
"    #else\n"
"        FxaaBool twoTap = (rgbyB.y < lumaMin) || (rgbyB.y > lumaMax);\n"
"    #endif\n"
"    if(twoTap) rgbyB.xyz = rgbyA.xyz * 0.5;\n"
"    return rgbyB; }\n"
"/*==========================================================================*/\n"
"#endif\n"
"\n"
"\n"
"\n"
"/*============================================================================\n"
"\n"
"                      FXAA3 CONSOLE - 360 PIXEL SHADER\n"
"\n"
"------------------------------------------------------------------------------\n"
"This optimized version thanks to suggestions from Andy Luedke.\n"
"Should be fully tex bound in all cases.\n"
"As of the FXAA 3.11 release, I have still not tested this code,\n"
"however I fixed a bug which was in both FXAA 3.9 and FXAA 3.10.\n"
"And note this is replacing the old unoptimized version.\n"
"If it does not work, please let me know so I can fix it.\n"
"============================================================================*/\n"
"#if (FXAA_360 == 1)\n"
"/*--------------------------------------------------------------------------*/\n"
"[reduceTempRegUsage(4)]\n"
"float4 FxaaPixelShader(\n"
"    // See FXAA Quality FxaaPixelShader() source for docs on Inputs!\n"
"    FxaaFloat2 pos,\n"
"    FxaaFloat4 fxaaConsolePosPos,\n"
"    FxaaTex tex,\n"
"    FxaaTex fxaaConsole360TexExpBiasNegOne,\n"
"    FxaaTex fxaaConsole360TexExpBiasNegTwo,\n"
"    FxaaFloat2 fxaaQualityRcpFrame,\n"
"    FxaaFloat4 fxaaConsoleRcpFrameOpt,\n"
"    FxaaFloat4 fxaaConsoleRcpFrameOpt2,\n"
"    FxaaFloat4 fxaaConsole360RcpFrameOpt2,\n"
"    FxaaFloat fxaaQualitySubpix,\n"
"    FxaaFloat fxaaQualityEdgeThreshold,\n"
"    FxaaFloat fxaaQualityEdgeThresholdMin,\n"
"    FxaaFloat fxaaConsoleEdgeSharpness,\n"
"    FxaaFloat fxaaConsoleEdgeThreshold,\n"
"    FxaaFloat fxaaConsoleEdgeThresholdMin,\n"
"    FxaaFloat4 fxaaConsole360ConstDir\n"
") {\n"
"/*--------------------------------------------------------------------------*/\n"
"    float4 lumaNwNeSwSe;\n"
"    #if (FXAA_GREEN_AS_LUMA == 0)\n"
"        asm {\n"
"            tfetch2D lumaNwNeSwSe.w___, tex, pos.xy, OffsetX = -0.5, OffsetY = -0.5, UseComputedLOD=false\n"
"            tfetch2D lumaNwNeSwSe._w__, tex, pos.xy, OffsetX =  0.5, OffsetY = -0.5, UseComputedLOD=false\n"
"            tfetch2D lumaNwNeSwSe.__w_, tex, pos.xy, OffsetX = -0.5, OffsetY =  0.5, UseComputedLOD=false\n"
"            tfetch2D lumaNwNeSwSe.___w, tex, pos.xy, OffsetX =  0.5, OffsetY =  0.5, UseComputedLOD=false\n"
"        };\n"
"    #else\n"
"        asm {\n"
"            tfetch2D lumaNwNeSwSe.y___, tex, pos.xy, OffsetX = -0.5, OffsetY = -0.5, UseComputedLOD=false\n"
"            tfetch2D lumaNwNeSwSe._y__, tex, pos.xy, OffsetX =  0.5, OffsetY = -0.5, UseComputedLOD=false\n"
"            tfetch2D lumaNwNeSwSe.__y_, tex, pos.xy, OffsetX = -0.5, OffsetY =  0.5, UseComputedLOD=false\n"
"            tfetch2D lumaNwNeSwSe.___y, tex, pos.xy, OffsetX =  0.5, OffsetY =  0.5, UseComputedLOD=false\n"
"        };\n"
"    #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"    lumaNwNeSwSe.y += 1.0/384.0;\n"
"    float2 lumaMinTemp = min(lumaNwNeSwSe.xy, lumaNwNeSwSe.zw);\n"
"    float2 lumaMaxTemp = max(lumaNwNeSwSe.xy, lumaNwNeSwSe.zw);\n"
"    float lumaMin = min(lumaMinTemp.x, lumaMinTemp.y);\n"
"    float lumaMax = max(lumaMaxTemp.x, lumaMaxTemp.y);\n"
"/*--------------------------------------------------------------------------*/\n"
"    float4 rgbyM = tex2Dlod(tex, float4(pos.xy, 0.0, 0.0));\n"
"    #if (FXAA_GREEN_AS_LUMA == 0)\n"
"        float lumaMinM = min(lumaMin, rgbyM.w);\n"
"        float lumaMaxM = max(lumaMax, rgbyM.w);\n"
"    #else\n"
"        float lumaMinM = min(lumaMin, rgbyM.y);\n"
"        float lumaMaxM = max(lumaMax, rgbyM.y);\n"
"    #endif\n"
"    if((lumaMaxM - lumaMinM) < max(fxaaConsoleEdgeThresholdMin, lumaMax * fxaaConsoleEdgeThreshold)) return rgbyM;\n"
"/*--------------------------------------------------------------------------*/\n"
"    float2 dir;\n"
"    dir.x = dot(lumaNwNeSwSe, fxaaConsole360ConstDir.yyxx);\n"
"    dir.y = dot(lumaNwNeSwSe, fxaaConsole360ConstDir.xyxy);\n"
"    dir = normalize(dir);\n"
"/*--------------------------------------------------------------------------*/\n"
"    float4 dir1 = dir.xyxy * fxaaConsoleRcpFrameOpt.xyzw;\n"
"/*--------------------------------------------------------------------------*/\n"
"    float4 dir2;\n"
"    float dirAbsMinTimesC = min(abs(dir.x), abs(dir.y)) * fxaaConsoleEdgeSharpness;\n"
"    dir2 = saturate(fxaaConsole360ConstDir.zzww * dir.xyxy / dirAbsMinTimesC + 0.5);\n"
"    dir2 = dir2 * fxaaConsole360RcpFrameOpt2.xyxy + fxaaConsole360RcpFrameOpt2.zwzw;\n"
"/*--------------------------------------------------------------------------*/\n"
"    float4 rgbyN1 = tex2Dlod(fxaaConsole360TexExpBiasNegOne, float4(pos.xy + dir1.xy, 0.0, 0.0));\n"
"    float4 rgbyP1 = tex2Dlod(fxaaConsole360TexExpBiasNegOne, float4(pos.xy + dir1.zw, 0.0, 0.0));\n"
"    float4 rgbyN2 = tex2Dlod(fxaaConsole360TexExpBiasNegTwo, float4(pos.xy + dir2.xy, 0.0, 0.0));\n"
"    float4 rgbyP2 = tex2Dlod(fxaaConsole360TexExpBiasNegTwo, float4(pos.xy + dir2.zw, 0.0, 0.0));\n"
"/*--------------------------------------------------------------------------*/\n"
"    float4 rgbyA = rgbyN1 + rgbyP1;\n"
"    float4 rgbyB = rgbyN2 + rgbyP2 + rgbyA * 0.5;\n"
"/*--------------------------------------------------------------------------*/\n"
"    float4 rgbyR = ((FxaaLuma(rgbyB) - lumaMax) > 0.0) ? rgbyA : rgbyB;\n"
"    rgbyR = ((FxaaLuma(rgbyB) - lumaMin) > 0.0) ? rgbyR : rgbyA;\n"
"    return rgbyR; }\n"
"/*==========================================================================*/\n"
"#endif\n"
"\n"
"\n"
"\n"
"/*============================================================================\n"
"\n"
"         FXAA3 CONSOLE - OPTIMIZED PS3 PIXEL SHADER (NO EARLY EXIT)\n"
"\n"
"==============================================================================\n"
"The code below does not exactly match the assembly.\n"
"I have a feeling that 12 cycles is possible, but was not able to get there.\n"
"Might have to increase register count to get full performance.\n"
"Note this shader does not use perspective interpolation.\n"
"\n"
"Use the following cgc options,\n"
"\n"
"  --fenable-bx2 --fastmath --fastprecision --nofloatbindings\n"
"\n"
"------------------------------------------------------------------------------\n"
"                             NVSHADERPERF OUTPUT\n"
"------------------------------------------------------------------------------\n"
"For reference and to aid in debug, output of NVShaderPerf should match this,\n"
"\n"
"Shader to schedule:\n"
"  0: texpkb h0.w(TRUE), v5.zyxx, #0\n"
"  2: addh h2.z(TRUE), h0.w, constant(0.001953, 0.000000, 0.000000, 0.000000).x\n"
"  4: texpkb h0.w(TRUE), v5.xwxx, #0\n"
"  6: addh h0.z(TRUE), -h2, h0.w\n"
"  7: texpkb h1.w(TRUE), v5, #0\n"
"  9: addh h0.x(TRUE), h0.z, -h1.w\n"
" 10: addh h3.w(TRUE), h0.z, h1\n"
" 11: texpkb h2.w(TRUE), v5.zwzz, #0\n"
" 13: addh h0.z(TRUE), h3.w, -h2.w\n"
" 14: addh h0.x(TRUE), h2.w, h0\n"
" 15: nrmh h1.xz(TRUE), h0_n\n"
" 16: minh_m8 h0.x(TRUE), |h1|, |h1.z|\n"
" 17: maxh h4.w(TRUE), h0, h1\n"
" 18: divx h2.xy(TRUE), h1_n.xzzw, h0_n\n"
" 19: movr r1.zw(TRUE), v4.xxxy\n"
" 20: madr r2.xz(TRUE), -h1, constant(cConst5.x, cConst5.y, cConst5.z, cConst5.w).zzww, r1.zzww\n"
" 22: minh h5.w(TRUE), h0, h1\n"
" 23: texpkb h0(TRUE), r2.xzxx, #0\n"
" 25: madr r0.zw(TRUE), h1.xzxz, constant(cConst5.x, cConst5.y, cConst5.z, cConst5.w), r1\n"
" 27: maxh h4.x(TRUE), h2.z, h2.w\n"
" 28: texpkb h1(TRUE), r0.zwzz, #0\n"
" 30: addh_d2 h1(TRUE), h0, h1\n"
" 31: madr r0.xy(TRUE), -h2, constant(cConst5.x, cConst5.y, cConst5.z, cConst5.w).xyxx, r1.zwzz\n"
" 33: texpkb h0(TRUE), r0, #0\n"
" 35: minh h4.z(TRUE), h2, h2.w\n"
" 36: fenct TRUE\n"
" 37: madr r1.xy(TRUE), h2, constant(cConst5.x, cConst5.y, cConst5.z, cConst5.w).xyxx, r1.zwzz\n"
" 39: texpkb h2(TRUE), r1, #0\n"
" 41: addh_d2 h0(TRUE), h0, h2\n"
" 42: maxh h2.w(TRUE), h4, h4.x\n"
" 43: minh h2.x(TRUE), h5.w, h4.z\n"
" 44: addh_d2 h0(TRUE), h0, h1\n"
" 45: slth h2.x(TRUE), h0.w, h2\n"
" 46: sgth h2.w(TRUE), h0, h2\n"
" 47: movh h0(TRUE), h0\n"
" 48: addx.c0 rc(TRUE), h2, h2.w\n"
" 49: movh h0(c0.NE.x), h1\n"
"\n"
"IPU0 ------ Simplified schedule: --------\n"
"Pass |  Unit  |  uOp |  PC:  Op\n"
"-----+--------+------+-------------------------\n"
"   1 | SCT0/1 |  mov |   0:  TXLr h0.w, g[TEX1].zyxx, const.xxxx, TEX0;\n"
"     |    TEX |  txl |   0:  TXLr h0.w, g[TEX1].zyxx, const.xxxx, TEX0;\n"
"     |   SCB1 |  add |   2:  ADDh h2.z, h0.--w-, const.--x-;\n"
"     |        |      |\n"
"   2 | SCT0/1 |  mov |   4:  TXLr h0.w, g[TEX1].xwxx, const.xxxx, TEX0;\n"
"     |    TEX |  txl |   4:  TXLr h0.w, g[TEX1].xwxx, const.xxxx, TEX0;\n"
"     |   SCB1 |  add |   6:  ADDh h0.z,-h2, h0.--w-;\n"
"     |        |      |\n"
"   3 | SCT0/1 |  mov |   7:  TXLr h1.w, g[TEX1], const.xxxx, TEX0;\n"
"     |    TEX |  txl |   7:  TXLr h1.w, g[TEX1], const.xxxx, TEX0;\n"
"     |   SCB0 |  add |   9:  ADDh h0.x, h0.z---,-h1.w---;\n"
"     |   SCB1 |  add |  10:  ADDh h3.w, h0.---z, h1;\n"
"     |        |      |\n"
"   4 | SCT0/1 |  mov |  11:  TXLr h2.w, g[TEX1].zwzz, const.xxxx, TEX0;\n"
"     |    TEX |  txl |  11:  TXLr h2.w, g[TEX1].zwzz, const.xxxx, TEX0;\n"
"     |   SCB0 |  add |  14:  ADDh h0.x, h2.w---, h0;\n"
"     |   SCB1 |  add |  13:  ADDh h0.z, h3.--w-,-h2.--w-;\n"
"     |        |      |\n"
"   5 |   SCT1 |  mov |  15:  NRMh h1.xz, h0;\n"
"     |    SRB |  nrm |  15:  NRMh h1.xz, h0;\n"
"     |   SCB0 |  min |  16:  MINh*8 h0.x, |h1|, |h1.z---|;\n"
"     |   SCB1 |  max |  17:  MAXh h4.w, h0, h1;\n"
"     |        |      |\n"
"   6 |   SCT0 |  div |  18:  DIVx h2.xy, h1.xz--, h0;\n"
"     |   SCT1 |  mov |  19:  MOVr r1.zw, g[TEX0].--xy;\n"
"     |   SCB0 |  mad |  20:  MADr r2.xz,-h1, const.z-w-, r1.z-w-;\n"
"     |   SCB1 |  min |  22:  MINh h5.w, h0, h1;\n"
"     |        |      |\n"
"   7 | SCT0/1 |  mov |  23:  TXLr h0, r2.xzxx, const.xxxx, TEX0;\n"
"     |    TEX |  txl |  23:  TXLr h0, r2.xzxx, const.xxxx, TEX0;\n"
"     |   SCB0 |  max |  27:  MAXh h4.x, h2.z---, h2.w---;\n"
"     |   SCB1 |  mad |  25:  MADr r0.zw, h1.--xz, const, r1;\n"
"     |        |      |\n"
"   8 | SCT0/1 |  mov |  28:  TXLr h1, r0.zwzz, const.xxxx, TEX0;\n"
"     |    TEX |  txl |  28:  TXLr h1, r0.zwzz, const.xxxx, TEX0;\n"
"     | SCB0/1 |  add |  30:  ADDh/2 h1, h0, h1;\n"
"     |        |      |\n"
"   9 |   SCT0 |  mad |  31:  MADr r0.xy,-h2, const.xy--, r1.zw--;\n"
"     |   SCT1 |  mov |  33:  TXLr h0, r0, const.zzzz, TEX0;\n"
"     |    TEX |  txl |  33:  TXLr h0, r0, const.zzzz, TEX0;\n"
"     |   SCB1 |  min |  35:  MINh h4.z, h2, h2.--w-;\n"
"     |        |      |\n"
"  10 |   SCT0 |  mad |  37:  MADr r1.xy, h2, const.xy--, r1.zw--;\n"
"     |   SCT1 |  mov |  39:  TXLr h2, r1, const.zzzz, TEX0;\n"
"     |    TEX |  txl |  39:  TXLr h2, r1, const.zzzz, TEX0;\n"
"     | SCB0/1 |  add |  41:  ADDh/2 h0, h0, h2;\n"
"     |        |      |\n"
"  11 |   SCT0 |  min |  43:  MINh h2.x, h5.w---, h4.z---;\n"
"     |   SCT1 |  max |  42:  MAXh h2.w, h4, h4.---x;\n"
"     | SCB0/1 |  add |  44:  ADDh/2 h0, h0, h1;\n"
"     |        |      |\n"
"  12 |   SCT0 |  set |  45:  SLTh h2.x, h0.w---, h2;\n"
"     |   SCT1 |  set |  46:  SGTh h2.w, h0, h2;\n"
"     | SCB0/1 |  mul |  47:  MOVh h0, h0;\n"
"     |        |      |\n"
"  13 |   SCT0 |  mad |  48:  ADDxc0_s rc, h2, h2.w---;\n"
"     | SCB0/1 |  mul |  49:  MOVh h0(NE0.xxxx), h1;\n"
" \n"
"Pass   SCT  TEX  SCB\n"
"  1:   0% 100%  25%\n"
"  2:   0% 100%  25%\n"
"  3:   0% 100%  50%\n"
"  4:   0% 100%  50%\n"
"  5:   0%   0%  50%\n"
"  6: 100%   0%  75%\n"
"  7:   0% 100%  75%\n"
"  8:   0% 100% 100%\n"
"  9:   0% 100%  25%\n"
" 10:   0% 100% 100%\n"
" 11:  50%   0% 100%\n"
" 12:  50%   0% 100%\n"
" 13:  25%   0% 100%\n"
"\n"
"MEAN:  17%  61%  67%\n"
"\n"
"Pass   SCT0  SCT1   TEX  SCB0  SCB1\n"
"  1:    0%    0%  100%    0%  100%\n"
"  2:    0%    0%  100%    0%  100%\n"
"  3:    0%    0%  100%  100%  100%\n"
"  4:    0%    0%  100%  100%  100%\n"
"  5:    0%    0%    0%  100%  100%\n"
"  6:  100%  100%    0%  100%  100%\n"
"  7:    0%    0%  100%  100%  100%\n"
"  8:    0%    0%  100%  100%  100%\n"
"  9:    0%    0%  100%    0%  100%\n"
" 10:    0%    0%  100%  100%  100%\n"
" 11:  100%  100%    0%  100%  100%\n"
" 12:  100%  100%    0%  100%  100%\n"
" 13:  100%    0%    0%  100%  100%\n"
"\n"
"MEAN:   30%   23%   61%   76%  100%\n"
"Fragment Performance Setup: Driver RSX Compiler, GPU RSX, Flags 0x5\n"
"Results 13 cycles, 3 r regs, 923,076,923 pixels/s\n"
"============================================================================*/\n"
"#if (FXAA_PS3 == 1) && (FXAA_EARLY_EXIT == 0)\n"
"/*--------------------------------------------------------------------------*/\n"
"#pragma regcount 7\n"
"#pragma disablepc all\n"
"#pragma option O3\n"
"#pragma option OutColorPrec=fp16\n"
"#pragma texformat default RGBA8\n"
"/*==========================================================================*/\n"
"half4 FxaaPixelShader(\n"
"    // See FXAA Quality FxaaPixelShader() source for docs on Inputs!\n"
"    FxaaFloat2 pos,\n"
"    FxaaFloat4 fxaaConsolePosPos,\n"
"    FxaaTex tex,\n"
"    FxaaTex fxaaConsole360TexExpBiasNegOne,\n"
"    FxaaTex fxaaConsole360TexExpBiasNegTwo,\n"
"    FxaaFloat2 fxaaQualityRcpFrame,\n"
"    FxaaFloat4 fxaaConsoleRcpFrameOpt,\n"
"    FxaaFloat4 fxaaConsoleRcpFrameOpt2,\n"
"    FxaaFloat4 fxaaConsole360RcpFrameOpt2,\n"
"    FxaaFloat fxaaQualitySubpix,\n"
"    FxaaFloat fxaaQualityEdgeThreshold,\n"
"    FxaaFloat fxaaQualityEdgeThresholdMin,\n"
"    FxaaFloat fxaaConsoleEdgeSharpness,\n"
"    FxaaFloat fxaaConsoleEdgeThreshold,\n"
"    FxaaFloat fxaaConsoleEdgeThresholdMin,\n"
"    FxaaFloat4 fxaaConsole360ConstDir\n"
") {\n"
"/*--------------------------------------------------------------------------*/\n"
"// (1)\n"
"    half4 dir;\n"
"    half4 lumaNe = h4tex2Dlod(tex, half4(fxaaConsolePosPos.zy, 0, 0));\n"
"    #if (FXAA_GREEN_AS_LUMA == 0)\n"
"        lumaNe.w += half(1.0/512.0);\n"
"        dir.x = -lumaNe.w;\n"
"        dir.z = -lumaNe.w;\n"
"    #else\n"
"        lumaNe.y += half(1.0/512.0);\n"
"        dir.x = -lumaNe.y;\n"
"        dir.z = -lumaNe.y;\n"
"    #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"// (2)\n"
"    half4 lumaSw = h4tex2Dlod(tex, half4(fxaaConsolePosPos.xw, 0, 0));\n"
"    #if (FXAA_GREEN_AS_LUMA == 0)\n"
"        dir.x += lumaSw.w;\n"
"        dir.z += lumaSw.w;\n"
"    #else\n"
"        dir.x += lumaSw.y;\n"
"        dir.z += lumaSw.y;\n"
"    #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"// (3)\n"
"    half4 lumaNw = h4tex2Dlod(tex, half4(fxaaConsolePosPos.xy, 0, 0));\n"
"    #if (FXAA_GREEN_AS_LUMA == 0)\n"
"        dir.x -= lumaNw.w;\n"
"        dir.z += lumaNw.w;\n"
"    #else\n"
"        dir.x -= lumaNw.y;\n"
"        dir.z += lumaNw.y;\n"
"    #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"// (4)\n"
"    half4 lumaSe = h4tex2Dlod(tex, half4(fxaaConsolePosPos.zw, 0, 0));\n"
"    #if (FXAA_GREEN_AS_LUMA == 0)\n"
"        dir.x += lumaSe.w;\n"
"        dir.z -= lumaSe.w;\n"
"    #else\n"
"        dir.x += lumaSe.y;\n"
"        dir.z -= lumaSe.y;\n"
"    #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"// (5)\n"
"    half4 dir1_pos;\n"
"    dir1_pos.xy = normalize(dir.xyz).xz;\n"
"    half dirAbsMinTimesC = min(abs(dir1_pos.x), abs(dir1_pos.y)) * half(FXAA_CONSOLE__PS3_EDGE_SHARPNESS);\n"
"/*--------------------------------------------------------------------------*/\n"
"// (6)\n"
"    half4 dir2_pos;\n"
"    dir2_pos.xy = clamp(dir1_pos.xy / dirAbsMinTimesC, half(-2.0), half(2.0));\n"
"    dir1_pos.zw = pos.xy;\n"
"    dir2_pos.zw = pos.xy;\n"
"    half4 temp1N;\n"
"    temp1N.xy = dir1_pos.zw - dir1_pos.xy * fxaaConsoleRcpFrameOpt.zw;\n"
"/*--------------------------------------------------------------------------*/\n"
"// (7)\n"
"    temp1N = h4tex2Dlod(tex, half4(temp1N.xy, 0.0, 0.0));\n"
"    half4 rgby1;\n"
"    rgby1.xy = dir1_pos.zw + dir1_pos.xy * fxaaConsoleRcpFrameOpt.zw;\n"
"/*--------------------------------------------------------------------------*/\n"
"// (8)\n"
"    rgby1 = h4tex2Dlod(tex, half4(rgby1.xy, 0.0, 0.0));\n"
"    rgby1 = (temp1N + rgby1) * 0.5;\n"
"/*--------------------------------------------------------------------------*/\n"
"// (9)\n"
"    half4 temp2N;\n"
"    temp2N.xy = dir2_pos.zw - dir2_pos.xy * fxaaConsoleRcpFrameOpt2.zw;\n"
"    temp2N = h4tex2Dlod(tex, half4(temp2N.xy, 0.0, 0.0));\n"
"/*--------------------------------------------------------------------------*/\n"
"// (10)\n"
"    half4 rgby2;\n"
"    rgby2.xy = dir2_pos.zw + dir2_pos.xy * fxaaConsoleRcpFrameOpt2.zw;\n"
"    rgby2 = h4tex2Dlod(tex, half4(rgby2.xy, 0.0, 0.0));\n"
"    rgby2 = (temp2N + rgby2) * 0.5;\n"
"/*--------------------------------------------------------------------------*/\n"
"// (11)\n"
"    // compilier moves these scalar ops up to other cycles\n"
"    #if (FXAA_GREEN_AS_LUMA == 0)\n"
"        half lumaMin = min(min(lumaNw.w, lumaSw.w), min(lumaNe.w, lumaSe.w));\n"
"        half lumaMax = max(max(lumaNw.w, lumaSw.w), max(lumaNe.w, lumaSe.w));\n"
"    #else\n"
"        half lumaMin = min(min(lumaNw.y, lumaSw.y), min(lumaNe.y, lumaSe.y));\n"
"        half lumaMax = max(max(lumaNw.y, lumaSw.y), max(lumaNe.y, lumaSe.y));\n"
"    #endif\n"
"    rgby2 = (rgby2 + rgby1) * 0.5;\n"
"/*--------------------------------------------------------------------------*/\n"
"// (12)\n"
"    #if (FXAA_GREEN_AS_LUMA == 0)\n"
"        bool twoTapLt = rgby2.w < lumaMin;\n"
"        bool twoTapGt = rgby2.w > lumaMax;\n"
"    #else\n"
"        bool twoTapLt = rgby2.y < lumaMin;\n"
"        bool twoTapGt = rgby2.y > lumaMax;\n"
"    #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"// (13)\n"
"    if(twoTapLt || twoTapGt) rgby2 = rgby1;\n"
"/*--------------------------------------------------------------------------*/\n"
"    return rgby2; }\n"
"/*==========================================================================*/\n"
"#endif\n"
"\n"
"\n"
"\n"
"/*============================================================================\n"
"\n"
"       FXAA3 CONSOLE - OPTIMIZED PS3 PIXEL SHADER (WITH EARLY EXIT)\n"
"\n"
"==============================================================================\n"
"The code mostly matches the assembly.\n"
"I have a feeling that 14 cycles is possible, but was not able to get there.\n"
"Might have to increase register count to get full performance.\n"
"Note this shader does not use perspective interpolation.\n"
"\n"
"Use the following cgc options,\n"
"\n"
" --fenable-bx2 --fastmath --fastprecision --nofloatbindings\n"
"\n"
"Use of FXAA_GREEN_AS_LUMA currently adds a cycle (16 clks).\n"
"Will look at fixing this for FXAA 3.12.\n"
"------------------------------------------------------------------------------\n"
"                             NVSHADERPERF OUTPUT\n"
"------------------------------------------------------------------------------\n"
"For reference and to aid in debug, output of NVShaderPerf should match this,\n"
"\n"
"Shader to schedule:\n"
"  0: texpkb h0.w(TRUE), v5.zyxx, #0\n"
"  2: addh h2.y(TRUE), h0.w, constant(0.001953, 0.000000, 0.000000, 0.000000).x\n"
"  4: texpkb h1.w(TRUE), v5.xwxx, #0\n"
"  6: addh h0.x(TRUE), h1.w, -h2.y\n"
"  7: texpkb h2.w(TRUE), v5.zwzz, #0\n"
"  9: minh h4.w(TRUE), h2.y, h2\n"
" 10: maxh h5.x(TRUE), h2.y, h2.w\n"
" 11: texpkb h0.w(TRUE), v5, #0\n"
" 13: addh h3.w(TRUE), -h0, h0.x\n"
" 14: addh h0.x(TRUE), h0.w, h0\n"
" 15: addh h0.z(TRUE), -h2.w, h0.x\n"
" 16: addh h0.x(TRUE), h2.w, h3.w\n"
" 17: minh h5.y(TRUE), h0.w, h1.w\n"
" 18: nrmh h2.xz(TRUE), h0_n\n"
" 19: minh_m8 h2.w(TRUE), |h2.x|, |h2.z|\n"
" 20: divx h4.xy(TRUE), h2_n.xzzw, h2_n.w\n"
" 21: movr r1.zw(TRUE), v4.xxxy\n"
" 22: maxh h2.w(TRUE), h0, h1\n"
" 23: fenct TRUE\n"
" 24: madr r0.xy(TRUE), -h2.xzzw, constant(cConst5.x, cConst5.y, cConst5.z, cConst5.w).zwzz, r1.zwzz\n"
" 26: texpkb h0(TRUE), r0, #0\n"
" 28: maxh h5.x(TRUE), h2.w, h5\n"
" 29: minh h5.w(TRUE), h5.y, h4\n"
" 30: madr r1.xy(TRUE), h2.xzzw, constant(cConst5.x, cConst5.y, cConst5.z, cConst5.w).zwzz, r1.zwzz\n"
" 32: texpkb h2(TRUE), r1, #0\n"
" 34: addh_d2 h2(TRUE), h0, h2\n"
" 35: texpkb h1(TRUE), v4, #0\n"
" 37: maxh h5.y(TRUE), h5.x, h1.w\n"
" 38: minh h4.w(TRUE), h1, h5\n"
" 39: madr r0.xy(TRUE), -h4, constant(cConst5.x, cConst5.y, cConst5.z, cConst5.w).xyxx, r1.zwzz\n"
" 41: texpkb h0(TRUE), r0, #0\n"
" 43: addh_m8 h5.z(TRUE), h5.y, -h4.w\n"
" 44: madr r2.xy(TRUE), h4, constant(cConst5.x, cConst5.y, cConst5.z, cConst5.w).xyxx, r1.zwzz\n"
" 46: texpkb h3(TRUE), r2, #0\n"
" 48: addh_d2 h0(TRUE), h0, h3\n"
" 49: addh_d2 h3(TRUE), h0, h2\n"
" 50: movh h0(TRUE), h3\n"
" 51: slth h3.x(TRUE), h3.w, h5.w\n"
" 52: sgth h3.w(TRUE), h3, h5.x\n"
" 53: addx.c0 rc(TRUE), h3.x, h3\n"
" 54: slth.c0 rc(TRUE), h5.z, h5\n"
" 55: movh h0(c0.NE.w), h2\n"
" 56: movh h0(c0.NE.x), h1\n"
"\n"
"IPU0 ------ Simplified schedule: --------\n"
"Pass |  Unit  |  uOp |  PC:  Op\n"
"-----+--------+------+-------------------------\n"
"   1 | SCT0/1 |  mov |   0:  TXLr h0.w, g[TEX1].zyxx, const.xxxx, TEX0;\n"
"     |    TEX |  txl |   0:  TXLr h0.w, g[TEX1].zyxx, const.xxxx, TEX0;\n"
"     |   SCB0 |  add |   2:  ADDh h2.y, h0.-w--, const.-x--;\n"
"     |        |      |\n"
"   2 | SCT0/1 |  mov |   4:  TXLr h1.w, g[TEX1].xwxx, const.xxxx, TEX0;\n"
"     |    TEX |  txl |   4:  TXLr h1.w, g[TEX1].xwxx, const.xxxx, TEX0;\n"
"     |   SCB0 |  add |   6:  ADDh h0.x, h1.w---,-h2.y---;\n"
"     |        |      |\n"
"   3 | SCT0/1 |  mov |   7:  TXLr h2.w, g[TEX1].zwzz, const.xxxx, TEX0;\n"
"     |    TEX |  txl |   7:  TXLr h2.w, g[TEX1].zwzz, const.xxxx, TEX0;\n"
"     |   SCB0 |  max |  10:  MAXh h5.x, h2.y---, h2.w---;\n"
"     |   SCB1 |  min |   9:  MINh h4.w, h2.---y, h2;\n"
"     |        |      |\n"
"   4 | SCT0/1 |  mov |  11:  TXLr h0.w, g[TEX1], const.xxxx, TEX0;\n"
"     |    TEX |  txl |  11:  TXLr h0.w, g[TEX1], const.xxxx, TEX0;\n"
"     |   SCB0 |  add |  14:  ADDh h0.x, h0.w---, h0;\n"
"     |   SCB1 |  add |  13:  ADDh h3.w,-h0, h0.---x;\n"
"     |        |      |\n"
"   5 |   SCT0 |  mad |  16:  ADDh h0.x, h2.w---, h3.w---;\n"
"     |   SCT1 |  mad |  15:  ADDh h0.z,-h2.--w-, h0.--x-;\n"
"     |   SCB0 |  min |  17:  MINh h5.y, h0.-w--, h1.-w--;\n"
"     |        |      |\n"
"   6 |   SCT1 |  mov |  18:  NRMh h2.xz, h0;\n"
"     |    SRB |  nrm |  18:  NRMh h2.xz, h0;\n"
"     |   SCB1 |  min |  19:  MINh*8 h2.w, |h2.---x|, |h2.---z|;\n"
"     |        |      |\n"
"   7 |   SCT0 |  div |  20:  DIVx h4.xy, h2.xz--, h2.ww--;\n"
"     |   SCT1 |  mov |  21:  MOVr r1.zw, g[TEX0].--xy;\n"
"     |   SCB1 |  max |  22:  MAXh h2.w, h0, h1;\n"
"     |        |      |\n"
"   8 |   SCT0 |  mad |  24:  MADr r0.xy,-h2.xz--, const.zw--, r1.zw--;\n"
"     |   SCT1 |  mov |  26:  TXLr h0, r0, const.xxxx, TEX0;\n"
"     |    TEX |  txl |  26:  TXLr h0, r0, const.xxxx, TEX0;\n"
"     |   SCB0 |  max |  28:  MAXh h5.x, h2.w---, h5;\n"
"     |   SCB1 |  min |  29:  MINh h5.w, h5.---y, h4;\n"
"     |        |      |\n"
"   9 |   SCT0 |  mad |  30:  MADr r1.xy, h2.xz--, const.zw--, r1.zw--;\n"
"     |   SCT1 |  mov |  32:  TXLr h2, r1, const.xxxx, TEX0;\n"
"     |    TEX |  txl |  32:  TXLr h2, r1, const.xxxx, TEX0;\n"
"     | SCB0/1 |  add |  34:  ADDh/2 h2, h0, h2;\n"
"     |        |      |\n"
"  10 | SCT0/1 |  mov |  35:  TXLr h1, g[TEX0], const.xxxx, TEX0;\n"
"     |    TEX |  txl |  35:  TXLr h1, g[TEX0], const.xxxx, TEX0;\n"
"     |   SCB0 |  max |  37:  MAXh h5.y, h5.-x--, h1.-w--;\n"
"     |   SCB1 |  min |  38:  MINh h4.w, h1, h5;\n"
"     |        |      |\n"
"  11 |   SCT0 |  mad |  39:  MADr r0.xy,-h4, const.xy--, r1.zw--;\n"
"     |   SCT1 |  mov |  41:  TXLr h0, r0, const.zzzz, TEX0;\n"
"     |    TEX |  txl |  41:  TXLr h0, r0, const.zzzz, TEX0;\n"
"     |   SCB0 |  mad |  44:  MADr r2.xy, h4, const.xy--, r1.zw--;\n"
"     |   SCB1 |  add |  43:  ADDh*8 h5.z, h5.--y-,-h4.--w-;\n"
"     |        |      |\n"
"  12 | SCT0/1 |  mov |  46:  TXLr h3, r2, const.xxxx, TEX0;\n"
"     |    TEX |  txl |  46:  TXLr h3, r2, const.xxxx, TEX0;\n"
"     | SCB0/1 |  add |  48:  ADDh/2 h0, h0, h3;\n"
"     |        |      |\n"
"  13 | SCT0/1 |  mad |  49:  ADDh/2 h3, h0, h2;\n"
"     | SCB0/1 |  mul |  50:  MOVh h0, h3;\n"
"     |        |      |\n"
"  14 |   SCT0 |  set |  51:  SLTh h3.x, h3.w---, h5.w---;\n"
"     |   SCT1 |  set |  52:  SGTh h3.w, h3, h5.---x;\n"
"     |   SCB0 |  set |  54:  SLThc0 rc, h5.z---, h5;\n"
"     |   SCB1 |  add |  53:  ADDxc0_s rc, h3.---x, h3;\n"
"     |        |      |\n"
"  15 | SCT0/1 |  mul |  55:  MOVh h0(NE0.wwww), h2;\n"
"     | SCB0/1 |  mul |  56:  MOVh h0(NE0.xxxx), h1;\n"
" \n"
"Pass   SCT  TEX  SCB\n"
"  1:   0% 100%  25%\n"
"  2:   0% 100%  25%\n"
"  3:   0% 100%  50%\n"
"  4:   0% 100%  50%\n"
"  5:  50%   0%  25%\n"
"  6:   0%   0%  25%\n"
"  7: 100%   0%  25%\n"
"  8:   0% 100%  50%\n"
"  9:   0% 100% 100%\n"
" 10:   0% 100%  50%\n"
" 11:   0% 100%  75%\n"
" 12:   0% 100% 100%\n"
" 13: 100%   0% 100%\n"
" 14:  50%   0%  50%\n"
" 15: 100%   0% 100%\n"
"\n"
"MEAN:  26%  60%  56%\n"
"\n"
"Pass   SCT0  SCT1   TEX  SCB0  SCB1\n"
"  1:    0%    0%  100%  100%    0%\n"
"  2:    0%    0%  100%  100%    0%\n"
"  3:    0%    0%  100%  100%  100%\n"
"  4:    0%    0%  100%  100%  100%\n"
"  5:  100%  100%    0%  100%    0%\n"
"  6:    0%    0%    0%    0%  100%\n"
"  7:  100%  100%    0%    0%  100%\n"
"  8:    0%    0%  100%  100%  100%\n"
"  9:    0%    0%  100%  100%  100%\n"
" 10:    0%    0%  100%  100%  100%\n"
" 11:    0%    0%  100%  100%  100%\n"
" 12:    0%    0%  100%  100%  100%\n"
" 13:  100%  100%    0%  100%  100%\n"
" 14:  100%  100%    0%  100%  100%\n"
" 15:  100%  100%    0%  100%  100%\n"
"\n"
"MEAN:   33%   33%   60%   86%   80%\n"
"Fragment Performance Setup: Driver RSX Compiler, GPU RSX, Flags 0x5\n"
"Results 15 cycles, 3 r regs, 800,000,000 pixels/s\n"
"============================================================================*/\n"
"#if (FXAA_PS3 == 1) && (FXAA_EARLY_EXIT == 1)\n"
"/*--------------------------------------------------------------------------*/\n"
"#pragma regcount 7\n"
"#pragma disablepc all\n"
"#pragma option O2\n"
"#pragma option OutColorPrec=fp16\n"
"#pragma texformat default RGBA8\n"
"/*==========================================================================*/\n"
"half4 FxaaPixelShader(\n"
"    // See FXAA Quality FxaaPixelShader() source for docs on Inputs!\n"
"    FxaaFloat2 pos,\n"
"    FxaaFloat4 fxaaConsolePosPos,\n"
"    FxaaTex tex,\n"
"    FxaaTex fxaaConsole360TexExpBiasNegOne,\n"
"    FxaaTex fxaaConsole360TexExpBiasNegTwo,\n"
"    FxaaFloat2 fxaaQualityRcpFrame,\n"
"    FxaaFloat4 fxaaConsoleRcpFrameOpt,\n"
"    FxaaFloat4 fxaaConsoleRcpFrameOpt2,\n"
"    FxaaFloat4 fxaaConsole360RcpFrameOpt2,\n"
"    FxaaFloat fxaaQualitySubpix,\n"
"    FxaaFloat fxaaQualityEdgeThreshold,\n"
"    FxaaFloat fxaaQualityEdgeThresholdMin,\n"
"    FxaaFloat fxaaConsoleEdgeSharpness,\n"
"    FxaaFloat fxaaConsoleEdgeThreshold,\n"
"    FxaaFloat fxaaConsoleEdgeThresholdMin,\n"
"    FxaaFloat4 fxaaConsole360ConstDir\n"
") {\n"
"/*--------------------------------------------------------------------------*/\n"
"// (1)\n"
"    half4 rgbyNe = h4tex2Dlod(tex, half4(fxaaConsolePosPos.zy, 0, 0));\n"
"    #if (FXAA_GREEN_AS_LUMA == 0)\n"
"        half lumaNe = rgbyNe.w + half(1.0/512.0);\n"
"    #else\n"
"        half lumaNe = rgbyNe.y + half(1.0/512.0);\n"
"    #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"// (2)\n"
"    half4 lumaSw = h4tex2Dlod(tex, half4(fxaaConsolePosPos.xw, 0, 0));\n"
"    #if (FXAA_GREEN_AS_LUMA == 0)\n"
"        half lumaSwNegNe = lumaSw.w - lumaNe;\n"
"    #else\n"
"        half lumaSwNegNe = lumaSw.y - lumaNe;\n"
"    #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"// (3)\n"
"    half4 lumaNw = h4tex2Dlod(tex, half4(fxaaConsolePosPos.xy, 0, 0));\n"
"    #if (FXAA_GREEN_AS_LUMA == 0)\n"
"        half lumaMaxNwSw = max(lumaNw.w, lumaSw.w);\n"
"        half lumaMinNwSw = min(lumaNw.w, lumaSw.w);\n"
"    #else\n"
"        half lumaMaxNwSw = max(lumaNw.y, lumaSw.y);\n"
"        half lumaMinNwSw = min(lumaNw.y, lumaSw.y);\n"
"    #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"// (4)\n"
"    half4 lumaSe = h4tex2Dlod(tex, half4(fxaaConsolePosPos.zw, 0, 0));\n"
"    #if (FXAA_GREEN_AS_LUMA == 0)\n"
"        half dirZ =  lumaNw.w + lumaSwNegNe;\n"
"        half dirX = -lumaNw.w + lumaSwNegNe;\n"
"    #else\n"
"        half dirZ =  lumaNw.y + lumaSwNegNe;\n"
"        half dirX = -lumaNw.y + lumaSwNegNe;\n"
"    #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"// (5)\n"
"    half3 dir;\n"
"    dir.y = 0.0;\n"
"    #if (FXAA_GREEN_AS_LUMA == 0)\n"
"        dir.x =  lumaSe.w + dirX;\n"
"        dir.z = -lumaSe.w + dirZ;\n"
"        half lumaMinNeSe = min(lumaNe, lumaSe.w);\n"
"    #else\n"
"        dir.x =  lumaSe.y + dirX;\n"
"        dir.z = -lumaSe.y + dirZ;\n"
"        half lumaMinNeSe = min(lumaNe, lumaSe.y);\n"
"    #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"// (6)\n"
"    half4 dir1_pos;\n"
"    dir1_pos.xy = normalize(dir).xz;\n"
"    half dirAbsMinTimes8 = min(abs(dir1_pos.x), abs(dir1_pos.y)) * half(FXAA_CONSOLE__PS3_EDGE_SHARPNESS);\n"
"/*--------------------------------------------------------------------------*/\n"
"// (7)\n"
"    half4 dir2_pos;\n"
"    dir2_pos.xy = clamp(dir1_pos.xy / dirAbsMinTimes8, half(-2.0), half(2.0));\n"
"    dir1_pos.zw = pos.xy;\n"
"    dir2_pos.zw = pos.xy;\n"
"    #if (FXAA_GREEN_AS_LUMA == 0)\n"
"        half lumaMaxNeSe = max(lumaNe, lumaSe.w);\n"
"    #else\n"
"        half lumaMaxNeSe = max(lumaNe, lumaSe.y);\n"
"    #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"// (8)\n"
"    half4 temp1N;\n"
"    temp1N.xy = dir1_pos.zw - dir1_pos.xy * fxaaConsoleRcpFrameOpt.zw;\n"
"    temp1N = h4tex2Dlod(tex, half4(temp1N.xy, 0.0, 0.0));\n"
"    half lumaMax = max(lumaMaxNwSw, lumaMaxNeSe);\n"
"    half lumaMin = min(lumaMinNwSw, lumaMinNeSe);\n"
"/*--------------------------------------------------------------------------*/\n"
"// (9)\n"
"    half4 rgby1;\n"
"    rgby1.xy = dir1_pos.zw + dir1_pos.xy * fxaaConsoleRcpFrameOpt.zw;\n"
"    rgby1 = h4tex2Dlod(tex, half4(rgby1.xy, 0.0, 0.0));\n"
"    rgby1 = (temp1N + rgby1) * 0.5;\n"
"/*--------------------------------------------------------------------------*/\n"
"// (10)\n"
"    half4 rgbyM = h4tex2Dlod(tex, half4(pos.xy, 0.0, 0.0));\n"
"    #if (FXAA_GREEN_AS_LUMA == 0)\n"
"        half lumaMaxM = max(lumaMax, rgbyM.w);\n"
"        half lumaMinM = min(lumaMin, rgbyM.w);\n"
"    #else\n"
"        half lumaMaxM = max(lumaMax, rgbyM.y);\n"
"        half lumaMinM = min(lumaMin, rgbyM.y);\n"
"    #endif\n"
"/*--------------------------------------------------------------------------*/\n"
"// (11)\n"
"    half4 temp2N;\n"
"    temp2N.xy = dir2_pos.zw - dir2_pos.xy * fxaaConsoleRcpFrameOpt2.zw;\n"
"    temp2N = h4tex2Dlod(tex, half4(temp2N.xy, 0.0, 0.0));\n"
"    half4 rgby2;\n"
"    rgby2.xy = dir2_pos.zw + dir2_pos.xy * fxaaConsoleRcpFrameOpt2.zw;\n"
"    half lumaRangeM = (lumaMaxM - lumaMinM) / FXAA_CONSOLE__PS3_EDGE_THRESHOLD;\n"
"/*--------------------------------------------------------------------------*/\n"
"// (12)\n"
"    rgby2 = h4tex2Dlod(tex, half4(rgby2.xy, 0.0, 0.0));\n"
"    rgby2 = (temp2N + rgby2) * 0.5;\n"
"/*--------------------------------------------------------------------------*/\n"
"// (13)\n"
"    rgby2 = (rgby2 + rgby1) * 0.5;\n"
"/*--------------------------------------------------------------------------*/\n"
"// (14)\n"
"    #if (FXAA_GREEN_AS_LUMA == 0)\n"
"        bool twoTapLt = rgby2.w < lumaMin;\n"
"        bool twoTapGt = rgby2.w > lumaMax;\n"
"    #else\n"
"        bool twoTapLt = rgby2.y < lumaMin;\n"
"        bool twoTapGt = rgby2.y > lumaMax;\n"
"    #endif\n"
"    bool earlyExit = lumaRangeM < lumaMax;\n"
"    bool twoTap = twoTapLt || twoTapGt;\n"
"/*--------------------------------------------------------------------------*/\n"
"// (15)\n"
"    if(twoTap) rgby2 = rgby1;\n"
"    if(earlyExit) rgby2 = rgbyM;\n"
"/*--------------------------------------------------------------------------*/\n"
"    return rgby2; }\n"
"/*==========================================================================*/\n"
"#endif\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/include/LightProcess.glsl ------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* LIGHTPROCESS_GLSL = 
"#ifndef LIGHTPROCESS_GLSL\n"
"#define LIGHTPROCESS_GLSL\n"
"\n"
"#include <lights>\n"
"#include <fogs>\n"
"\n"
"#if NUM_POINT_LIGHT > 0\n"
"uniform PointLight point_lights[NUM_POINT_LIGHT];\n"
"#endif\n"
"\n"
"#if NUM_SPOT_LIGHT > 0\n"
"uniform SpotLight spot_lights[NUM_SPOT_LIGHT];\n"
"#endif\n"
"\n"
"#if NUM_DIRECTIONAL_LIGHT > 0\n"
"uniform DirectionalLight directional_lights[NUM_DIRECTIONAL_LIGHT];\n"
"#endif\n"
"\n"
"#if NUM_HEMISPHERE_LIGHT > 0\n"
"uniform HemisphereLight hemisphere_lights[NUM_HEMISPHERE_LIGHT];\n"
"#endif\n"
"\n"
"#ifdef USE_LINEAR_FOG\n"
"uniform LinearFog fog;\n"
"#endif\n"
"\n"
"#ifdef USE_EXP2_FOG\n"
"uniform Exp2Fog fog;\n"
"#endif\n"
"\n"
"vec3 light_process(Material mat, Geometry geom, vec3 light, float occlusion) {\n"
"    /* apply point / spot / directional / hemisphere lights */\n"
"    vec3 direct_light = vec3(0.);\n"
"    vec3 indirect_light = light;\n"
"    #if NUM_POINT_LIGHT > 0\n"
"        for (int i = 0; i < NUM_POINT_LIGHT; ++i) {\n"
"            direct_light += apply_light(point_lights[i], mat, geom);\n"
"        }\n"
"    #endif\n"
"    #if NUM_SPOT_LIGHT > 0\n"
"        for (int i = 0; i < NUM_SPOT_LIGHT; ++i) {\n"
"            direct_light += apply_light(spot_lights[i], mat, geom);\n"
"        }\n"
"    #endif\n"
"    #if NUM_DIRECTIONAL_LIGHT > 0\n"
"        for (int i = 0; i < NUM_DIRECTIONAL_LIGHT; ++i) {\n"
"            direct_light += apply_light(directional_lights[i], mat, geom);\n"
"        }\n"
"    #endif\n"
"    #if NUM_HEMISPHERE_LIGHT > 0\n"
"        for (int i = 0; i < NUM_HEMISPHERE_LIGHT; ++i) {\n"
"            indirect_light += apply_light(hemisphere_lights[i], mat, geom);\n"
"        }\n"
"    #endif\n"
"    vec3 total_light = direct_light + indirect_light * occlusion;\n"
"    \n"
"    /* apply fog and tone mapping on total light */\n"
"    #if defined(USE_LINEAR_FOG) || defined(USE_EXP2_FOG)\n"
"        apply_fog(fog, distance(camera_pos, geom.position), total_light);\n"
"    #endif\n"
"    return total_light;\n"
"}\n"
"\n"
"#endif\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/Copy.vert.glsl ---------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* COPY_VERT_GLSL = 
"#include <common>\n"
"\n"
"in vec3 vertex;\n"
"in vec2 uv;\n"
"\n"
"out vec2 v_uv;\n"
"\n"
"void main() {\n"
"    v_uv = uv;\n"
"    gl_Position = vec4(vertex, 1.);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/Copy.frag.glsl ---------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* COPY_FRAG_GLSL = 
"#include <common>\n"
"\n"
"uniform sampler2D map;\n"
"\n"
"uniform float lod;\n"
"\n"
"in vec2 v_uv;\n"
"\n"
"layout(location = 0) out vec4 out_color;\n"
"\n"
"void main() {\n"
"    out_color = textureLod(map, v_uv, lod);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/Blend.vert.glsl --------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* BLEND_VERT_GLSL = 
"#include <common>\n"
"\n"
"in vec3 vertex;\n"
"in vec2 uv;\n"
"\n"
"out vec2 v_uv;\n"
"\n"
"void main() {\n"
"    v_uv = uv;\n"
"    gl_Position = vec4(vertex, 1.);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/Blend.frag.glsl --------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* BLEND_FRAG_GLSL = 
"#include <common>\n"
"\n"
"uniform sampler2D map_a;\n"
"uniform sampler2D map_b;\n"
"\n"
"in vec2 v_uv;\n"
"\n"
"layout(location = 0) out vec4 out_color;\n"
"\n"
"void main() {\n"
"    out_color = BLEND_OP(textureLod(map_a, v_uv, 0) A_SWIZZLE,\n"
"                         textureLod(map_b, v_uv, 0) B_SWIZZLE);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/BoxBlur.vert.glsl ------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* BOXBLUR_VERT_GLSL = 
"#include <common>\n"
"\n"
"in vec3 vertex;\n"
"in vec2 uv;\n"
"\n"
"out vec2 v_uv;\n"
"\n"
"void main() {\n"
"    v_uv = uv;\n"
"    gl_Position = vec4(vertex, 1.);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/BoxBlur.frag.glsl ------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* BOXBLUR_FRAG_GLSL = 
"#include <common>\n"
"\n"
"uniform sampler2D map;\n"
"\n"
"uniform float lod;\n"
"uniform vec2 direction;\n"
"uniform int radius;\n"
"\n"
"in vec2 v_uv;\n"
"\n"
"layout(location = 0) out TYPE out_color;\n"
"\n"
"void main() {\n"
"    TYPE blur_sum = TYPE(0.);\n"
"    \n"
"    /* sampling color along the direction */\n"
"    for (float i = -radius + 1.; i < radius; ++i) {\n"
"        vec2 offset = direction * i;\n"
"        blur_sum += textureLod(map, v_uv + offset, lod) SWIZZLE;\n"
"    }\n"
"    \n"
"    /* output filtered color */\n"
"    out_color = blur_sum / (float(radius) * 2. - 1.);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/GaussianBlur.vert.glsl -------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* GAUSSIANBLUR_VERT_GLSL = 
"#include <common>\n"
"\n"
"in vec3 vertex;\n"
"in vec2 uv;\n"
"\n"
"out vec2 v_uv;\n"
"\n"
"void main() {\n"
"    v_uv = uv;\n"
"    gl_Position = vec4(vertex, 1.);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/GaussianBlur.frag.glsl -------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* GAUSSIANBLUR_FRAG_GLSL = 
"#include <common>\n"
"\n"
"uniform sampler2D map;\n"
"\n"
"uniform float lod;\n"
"uniform vec2 direction;\n"
"uniform int radius;\n"
"uniform float sigma_s;\n"
"\n"
"in vec2 v_uv;\n"
"\n"
"layout(location = 0) out TYPE out_color;\n"
"\n"
"void main() {\n"
"    TYPE blur_sum = TYPE(0.);\n"
"    float weight_sum = 0.;\n"
"    \n"
"    /* prepare for Gaussian filtering */\n"
"    float factor_s = 1. / (sigma_s * sigma_s);\n"
"    \n"
"    /* sample color along the direction */\n"
"    for (float i = -radius + 1.; i < radius; ++i) {\n"
"        vec2 offset = direction * i;\n"
"        TYPE color = textureLod(map, v_uv + offset, lod) SWIZZLE;\n"
"        float weight = exp(i * i * factor_s * -0.5);\n"
"        blur_sum += color * weight;\n"
"        weight_sum += weight;\n"
"    }\n"
"    \n"
"    /* output filtered color */\n"
"    out_color = blur_sum / weight_sum;\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/BilateralBlur.vert.glsl ------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* BILATERALBLUR_VERT_GLSL = 
"#include <common>\n"
"\n"
"in vec3 vertex;\n"
"in vec2 uv;\n"
"\n"
"out vec2 v_uv;\n"
"\n"
"void main() {\n"
"    v_uv = uv;\n"
"    gl_Position = vec4(vertex, 1.);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/BilateralBlur.frag.glsl ------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* BILATERALBLUR_FRAG_GLSL = 
"#include <common>\n"
"\n"
"uniform sampler2D map;\n"
"\n"
"uniform float lod;\n"
"uniform vec2 direction;\n"
"uniform int radius;\n"
"uniform float sigma_s;\n"
"uniform float sigma_r;\n"
"\n"
"in vec2 v_uv;\n"
"\n"
"layout(location = 0) out TYPE out_color;\n"
"\n"
"void main() {\n"
"    TYPE blur_sum = TYPE(0.);\n"
"    float weight_sum = 0.;\n"
"    \n"
"    /* prepare for bilateral filtering */\n"
"    TYPE center_color = textureLod(map, v_uv, lod) SWIZZLE;\n"
"    float factor_s = 1. / (sigma_s * sigma_s);\n"
"    float factor_r = 1. / (sigma_r * sigma_r);\n"
"    \n"
"    /* sample color along the direction */\n"
"    for (float i = -radius + 1.; i < radius; ++i) {\n"
"        vec2 offset = direction * i;\n"
"        TYPE color = textureLod(map, v_uv + offset, lod) SWIZZLE;\n"
"        TYPE delta_color = center_color - color;\n"
"        float weight_r = dot(delta_color, delta_color);\n"
"        float weight = exp((i * i * factor_s + weight_r * factor_r) * -0.5);\n"
"        blur_sum += color * weight;\n"
"        weight_sum += weight;\n"
"    }\n"
"    \n"
"    /* output filtered color */\n"
"    out_color = blur_sum / weight_sum;\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/SphericalBlur.vert.glsl ------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* SPHERICALBLUR_VERT_GLSL = 
"#include <common>\n"
"#include <cubemap>\n"
"\n"
"uniform int face;\n"
"\n"
"in vec3 vertex;\n"
"in vec2 uv;\n"
"\n"
"out vec3 v_dir;\n"
"\n"
"void main() {\n"
"    v_dir = face_to_cube(uv * 2. - 1., face);\n"
"    gl_Position = vec4(vertex, 1.);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/SphericalBlur.frag.glsl ------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* SPHERICALBLUR_FRAG_GLSL = 
"#include <common>\n"
"#include <cubemap>\n"
"\n"
"#define N    20\n"
"\n"
"uniform samplerCube map;\n"
"\n"
"uniform float lod;\n"
"uniform int samples;\n"
"uniform float weights[N];\n"
"uniform bool latitudinal;\n"
"uniform float d_theta;\n"
"uniform vec3 pole_axis;\n"
"\n"
"in vec3 v_dir;\n"
"\n"
"layout(location = 0) out vec3 out_color;\n"
"\n"
"vec3 get_sample(float theta, vec3 axis) {\n"
"    float cos_theta = cos(theta);\n"
"    vec3 dir = v_dir * cos_theta + cross(axis, v_dir) * sin(theta)\n"
"        + axis * dot(axis, v_dir) * (1. - cos_theta);\n"
"    return textureLod(map, dir, lod).xyz;\n"
"}\n"
"\n"
"void main() {\n"
"    vec3 axis = latitudinal ? pole_axis : cross(pole_axis, v_dir);\n"
"    if (all(equal(axis, vec3(0.)))) {\n"
"        axis = vec3(v_dir.z, 0., -v_dir.x);\n"
"    }\n"
"    axis = normalize(axis);\n"
"    vec3 blur_sum = weights[0] * get_sample(0., axis);\n"
"    for (int i = 1; i < N; ++i) {\n"
"        if (i >= samples) break;\n"
"        float theta = d_theta * i;\n"
"        blur_sum += weights[i] * get_sample(-theta, axis);\n"
"        blur_sum += weights[i] * get_sample(theta, axis);\n"
"    }\n"
"    out_color = blur_sum;\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/Cubemap.vert.glsl ------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* CUBEMAP_VERT_GLSL = 
"#include <common>\n"
"#include <cubemap>\n"
"\n"
"uniform int face;\n"
"\n"
"in vec3 vertex;\n"
"in vec2 uv;\n"
"\n"
"out vec3 v_dir;\n"
"\n"
"void main() {\n"
"    v_dir = face_to_cube(uv * 2. - 1., face);\n"
"    gl_Position = vec4(vertex, 1.);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/Cubemap.frag.glsl ------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* CUBEMAP_FRAG_GLSL = 
"#include <common>\n"
"#include <cubemap>\n"
"\n"
"#ifdef USE_CUBEMAP\n"
"uniform samplerCube map;\n"
"#endif\n"
"\n"
"#ifdef USE_EQUIRECT\n"
"uniform sampler2D map;\n"
"#endif\n"
"\n"
"in vec3 v_dir;\n"
"\n"
"layout(location = 0) out vec4 out_color;\n"
"\n"
"void main() {\n"
"    #ifdef USE_CUBEMAP\n"
"        out_color = textureLod(map, v_dir, 0);\n"
"    #endif\n"
"    #ifdef USE_EQUIRECT\n"
"        vec2 uv = cube_to_equirect(normalize(v_dir));\n"
"        out_color = textureLod(map, uv, 0);\n"
"    #endif\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/Cube.vert.glsl ---------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* CUBE_VERT_GLSL = 
"#include <common>\n"
"\n"
"uniform mat4 view_proj;\n"
"\n"
"in vec3 vertex;\n"
"\n"
"out vec3 v_dir;\n"
"\n"
"void main() {\n"
"    v_dir = vertex;\n"
"    gl_Position = (view_proj * vec4(vertex, 1.)).xyww;\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/Cube.frag.glsl ---------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* CUBE_FRAG_GLSL = 
"#include <common>\n"
"#include <cubemap>\n"
"\n"
"#ifdef USE_CUBEMAP\n"
"uniform samplerCube map;\n"
"#endif\n"
"\n"
"#ifdef USE_EQUIRECT\n"
"uniform sampler2D map;\n"
"#endif\n"
"\n"
"uniform float intensity;\n"
"\n"
"in vec3 v_dir;\n"
"\n"
"layout(location = 0) out vec4 out_color;\n"
"\n"
"void main() {\n"
"    #ifdef USE_CUBEMAP\n"
"        out_color = textureLod(map, v_dir, 0) * intensity;\n"
"    #endif\n"
"    #ifdef USE_EQUIRECT\n"
"        vec2 uv = cube_to_equirect(normalize(v_dir));\n"
"        out_color = textureLod(map, uv, 0) * intensity;\n"
"    #endif\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/Standard.vert.glsl ------------------------------ */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* STANDARD_VERT_GLSL = 
"#include <common>\n"
"\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 proj;\n"
"uniform mat4 model_view;\n"
"uniform mat4 model_view_proj;\n"
"uniform mat3 normal_mat;\n"
"uniform vec3 camera_pos;\n"
"\n"
"#ifdef USE_DISPLACEMENT_MAP\n"
"uniform sampler2D displacement_map;\n"
"uniform float displacement_scale;\n"
"#endif\n"
"\n"
"in vec3 vertex;\n"
"in vec3 normal;\n"
"in vec2 uv;\n"
"\n"
"out vec3 v_normal;\n"
"out vec2 v_uv;\n"
"out vec3 v_world_pos;\n"
"\n"
"#ifdef USE_TANGENT_SPACE\n"
"in vec4 tangent;\n"
"out vec3 v_tangent;\n"
"out vec3 v_bitangent;\n"
"#endif\n"
"\n"
"#ifdef USE_VERTEX_COLOR\n"
"in vec3 color;\n"
"out vec3 v_color;\n"
"#endif\n"
"\n"
"void main() {\n"
"    vec3 t_vertex = vertex;\n"
"    vec3 t_normal = normal;\n"
"    vec2 t_uv = uv;\n"
"    #ifdef USE_TANGENT_SPACE\n"
"        vec3 t_tangent = tangent.xyz;\n"
"        vec3 t_bitangent;\n"
"    #endif\n"
"    #ifdef USE_VERTEX_COLOR\n"
"        vec3 t_color = color;\n"
"    #endif\n"
"    \n"
"    #ifdef USE_DISPLACEMENT_MAP\n"
"        /* reposition the vertex by displacement map */\n"
"        t_vertex += t_normal * texture(displacement_map, t_uv).x * displacement_scale;\n"
"    #endif\n"
"    \n"
"    /* transform normal from object space to world space */\n"
"    t_normal = normalize(normal_mat * t_normal);\n"
"    \n"
"    #ifdef USE_TANGENT_SPACE\n"
"        /* transform tangent from object space to world space */\n"
"        t_tangent = normalize((model * vec4(t_tangent, 0.)).xyz);\n"
"        \n"
"        /* calculate bitangent with normal and tangent */\n"
"        t_bitangent = normalize(cross(t_normal, t_tangent) * tangent.w);\n"
"    #endif\n"
"    \n"
"    /* pass parameters to fragment shader */\n"
"    v_normal = t_normal;\n"
"    v_uv = t_uv;\n"
"    v_world_pos = (model * vec4(t_vertex, 1.)).xyz;\n"
"    #ifdef USE_TANGENT_SPACE\n"
"        v_tangent = t_tangent;\n"
"        v_bitangent = t_bitangent;\n"
"    #endif\n"
"    #ifdef USE_VERTEX_COLOR\n"
"        v_color = t_color;\n"
"    #endif\n"
"    gl_Position = model_view_proj * vec4(t_vertex, 1.);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/Standard.frag.glsl ------------------------------ */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* STANDARD_FRAG_GLSL = 
"#include <common>\n"
"#include <packing>\n"
"#include <brdf>\n"
"#include <iblfilter>\n"
"\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 proj;\n"
"uniform mat4 model_view;\n"
"uniform mat4 model_view_proj;\n"
"uniform mat3 normal_mat;\n"
"uniform vec3 camera_pos;\n"
"\n"
"#ifdef FORWARD_RENDERING\n"
"#include <lightprocess>\n"
"#endif\n"
"\n"
"uniform float alpha_test;\n"
"\n"
"#ifdef USE_NORMAL_MAP\n"
"uniform sampler2D normal_map;\n"
"uniform float normal_scale;\n"
"#endif\n"
"\n"
"uniform vec3 color;\n"
"\n"
"#if defined(USE_COLOR_MAP) || defined(USE_COLOR_ALPHA_MAP)\n"
"uniform sampler2D color_map;\n"
"#endif\n"
"\n"
"uniform float alpha;\n"
"\n"
"#ifdef USE_ALPHA_MAP\n"
"uniform sampler2D alpha_map;\n"
"#endif\n"
"\n"
"uniform vec3 emissive;\n"
"\n"
"#ifdef USE_EMISSIVE_MAP\n"
"uniform sampler2D emissive_map;\n"
"#endif\n"
"\n"
"#ifdef USE_AO_MAP\n"
"uniform sampler2D ao_map;\n"
"uniform float ao_intensity;\n"
"#endif\n"
"\n"
"uniform float metalness;\n"
"\n"
"#ifdef USE_METALNESS_MAP\n"
"uniform sampler2D metalness_map;\n"
"#endif\n"
"\n"
"uniform float roughness;\n"
"\n"
"#ifdef USE_ROUGHNESS_MAP\n"
"uniform sampler2D roughness_map;\n"
"#endif\n"
"\n"
"uniform float specular;\n"
"\n"
"#ifdef USE_SPECULAR_MAP\n"
"uniform sampler2D specular_map;\n"
"#endif\n"
"\n"
"#ifdef USE_REFLECTION_PROBE\n"
"uniform samplerCube ref_map;\n"
"uniform float ref_lod;\n"
"uniform float ref_intensity;\n"
"#endif\n"
"\n"
"in vec3 v_normal;\n"
"in vec2 v_uv;\n"
"in vec3 v_world_pos;\n"
"\n"
"#ifdef USE_TANGENT_SPACE\n"
"in vec3 v_tangent;\n"
"in vec3 v_bitangent;\n"
"#endif\n"
"\n"
"#ifdef USE_VERTEX_COLOR\n"
"in vec3 v_color;\n"
"#endif\n"
"\n"
"#ifdef FORWARD_RENDERING\n"
"layout(location = 0) out vec4 out_color;\n"
"#endif\n"
"\n"
"#ifdef DEFERRED_RENDERING\n"
"layout(location = 0) out vec4 g_color;       /* G-Buffer base color */\n"
"layout(location = 1) out vec4 g_normal;      /* G-Buffer world normal */\n"
"layout(location = 2) out vec4 g_material;    /* G-Buffer material data */\n"
"layout(location = 3) out vec4 g_light;       /* G-Buffer indirect light */\n"
"#endif\n"
"\n"
"void main() {\n"
"    /* calculate color and alpha */\n"
"    vec4 t_color = vec4(color, alpha);\n"
"    #ifdef USE_VERTEX_COLOR\n"
"        t_color.xyz *= v_color;\n"
"    #endif\n"
"    #ifdef USE_COLOR_MAP\n"
"        t_color.xyz *= texture(color_map, v_uv).xyz;\n"
"    #endif\n"
"    #ifdef USE_ALPHA_MAP\n"
"        t_color.w *= texture(alpha_map, v_uv).x;\n"
"    #endif\n"
"    #ifdef USE_COLOR_ALPHA_MAP\n"
"        t_color.xyzw *= texture(color_map, v_uv).xyzw;\n"
"    #endif\n"
"    \n"
"    /* discard if failing alpha test */\n"
"    if (t_color.w < alpha_test) discard;\n"
"    \n"
"    /* calculate normal in world space */\n"
"    float face_dir = gl_FrontFacing ? 1. : -1.;\n"
"    vec3 t_normal = normalize(v_normal) * face_dir;\n"
"    #ifdef USE_NORMAL_MAP\n"
"        vec3 normal = texture(normal_map, v_uv).xyz;\n"
"        normal = normalize(unpack_normal(normal));\n"
"        normal.xy *= normal_scale;\n"
"        #ifdef USE_TANGENT_SPACE\n"
"            vec3 tangent = normalize(v_tangent) * face_dir;\n"
"            vec3 bitangent = normalize(v_bitangent) * face_dir;\n"
"            mat3 tbn_mat = mat3(tangent, bitangent, t_normal);\n"
"            t_normal = normalize(tbn_mat * normal);\n"
"        #endif\n"
"        #ifdef USE_OBJECT_SPACE\n"
"            t_normal = normalize(normal_mat * normal);\n"
"        #endif\n"
"    #endif\n"
"    \n"
"    /* calculate metalness */\n"
"    float t_metalness = metalness;\n"
"    #ifdef USE_METALNESS_MAP\n"
"        t_metalness *= texture(metalness_map, v_uv).x;\n"
"    #endif\n"
"    \n"
"    /* calculate roughness */\n"
"    float t_roughness = roughness;\n"
"    #ifdef USE_ROUGHNESS_MAP\n"
"        t_roughness *= texture(roughness_map, v_uv).x;\n"
"    #endif\n"
"    \n"
"    /* calculate specular IOR */\n"
"    float t_specular = specular;\n"
"    #ifdef USE_SPECULAR_MAP\n"
"        t_specular *= texture(specular_map, v_uv).x;\n"
"    #endif\n"
"    \n"
"    /* calculate emissive color */\n"
"    vec3 t_emissive = emissive;\n"
"    #ifdef USE_EMISSIVE_MAP\n"
"        t_emissive *= texture(emissive_map, v_uv).xyz;\n"
"    #endif\n"
"    \n"
"    /* calculate ambient occlusion */\n"
"    float t_occlusion = 1.;\n"
"    #ifdef USE_AO_MAP\n"
"        t_occlusion *= (texture(ao_map, v_uv).x - 1.) * ao_intensity + 1.;\n"
"    #endif\n"
"    \n"
"    /* calculate diffuse color */\n"
"    vec3 diffuse = t_color.xyz * (1. - t_metalness);\n"
"    \n"
"    /* calculate specular F0 */\n"
"    vec3 specular_f0 = mix(vec3(t_specular * 0.08), t_color.xyz, t_metalness);\n"
"    \n"
"    /* view from position to camera position */\n"
"    vec3 view_dir = normalize(camera_pos - v_world_pos);\n"
"    \n"
"    /* calculate indirect light */\n"
"    vec3 indirect_light = t_emissive;\n"
"    \n"
"    #ifdef USE_REFLECTION_PROBE\n"
"        /* calculate single-scatter and multi-scatter */\n"
"        vec3 single_scatter = vec3(0.);\n"
"        vec3 multi_scatter = vec3(0.);\n"
"        scattering(t_normal, view_dir, specular_f0, t_roughness, single_scatter, multi_scatter);\n"
"        \n"
"        /* calculate irradiance and radiance from reflection map */\n"
"        vec3 irradiance = ibl_diffuse(ref_map, ref_lod, t_normal) * INV_PI * ref_intensity;\n"
"        vec3 radiance = ibl_specular(ref_map, ref_lod, view_dir, t_normal, t_roughness) * ref_intensity;\n"
"        \n"
"        /* calculate indirect specular light */\n"
"        indirect_light += single_scatter * radiance;\n"
"        indirect_light += multi_scatter * irradiance;\n"
"        \n"
"        /* calculate indirect diffuse light */\n"
"        indirect_light += diffuse * (1. - single_scatter - multi_scatter) * irradiance * t_occlusion;\n"
"    #endif\n"
"    \n"
"    #ifdef DEFERRED_RENDERING\n"
"        /* output G-Buffers in deferred rendering */\n"
"        g_color = vec4(diffuse, t_occlusion);\n"
"        g_normal = vec4(pack_normal(t_normal), 0.);\n"
"        g_material = vec4(specular_f0, t_roughness);\n"
"        g_light = vec4(indirect_light, 0.);\n"
"    #endif\n"
"    \n"
"    #ifdef FORWARD_RENDERING\n"
"        /* create new material object */\n"
"        Material material;\n"
"        material.color = diffuse;\n"
"        material.f0 = specular_f0;\n"
"        material.roughness = t_roughness;\n"
"        \n"
"        /* create new geometry object */\n"
"        Geometry geometry;\n"
"        geometry.position = v_world_pos;\n"
"        geometry.view_dir = view_dir;\n"
"        geometry.normal = t_normal;\n"
"        \n"
"        /* output color in forward rendering */\n"
"        out_color = vec4(light_process(material, geometry, indirect_light, t_occlusion), t_color.w);\n"
"    #endif\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/Shadow.vert.glsl -------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* SHADOW_VERT_GLSL = 
"#include <common>\n"
"\n"
"uniform mat4 model_view_proj;\n"
"\n"
"in vec3 vertex;\n"
"in vec2 uv;\n"
"\n"
"out vec2 v_uv;\n"
"\n"
"void main() {\n"
"    v_uv = uv;\n"
"    gl_Position = model_view_proj * vec4(vertex, 1.);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/Shadow.frag.glsl -------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* SHADOW_FRAG_GLSL = 
"#include <common>\n"
"\n"
"uniform float alpha;\n"
"uniform float alpha_test;\n"
"\n"
"#ifdef USE_COLOR_MAP\n"
"uniform sampler2D color_map;\n"
"#endif\n"
"\n"
"#ifdef USE_ALPHA_MAP\n"
"uniform sampler2D alpha_map;\n"
"#endif\n"
"\n"
"in vec2 v_uv;\n"
"\n"
"layout(location = 0) out vec4 out_color;\n"
"\n"
"void main() {\n"
"    float t_alpha = alpha;\n"
"    #ifdef USE_COLOR_MAP\n"
"        t_alpha *= texture(color_map, v_uv).w;\n"
"    #endif\n"
"    #ifdef USE_ALPHA_MAP\n"
"        t_alpha *= texture(alpha_map, v_uv).x;\n"
"    #endif\n"
"    if (t_alpha < alpha_test) discard;\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/Lighting.vert.glsl ------------------------------ */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* LIGHTING_VERT_GLSL = 
"#include <common>\n"
"\n"
"in vec3 vertex;\n"
"in vec2 uv;\n"
"\n"
"out vec2 v_uv;\n"
"\n"
"void main() {\n"
"    v_uv = uv;\n"
"    gl_Position = vec4(vertex, 1.);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/Lighting.frag.glsl ------------------------------ */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* LIGHTING_FRAG_GLSL = 
"#include <common>\n"
"#include <packing>\n"
"\n"
"uniform sampler2D g_color;       /* G-Buffer base color */\n"
"uniform sampler2D g_normal;      /* G-Buffer world normal */\n"
"uniform sampler2D g_material;    /* G-Buffer material data */\n"
"uniform sampler2D g_light;       /* G-Buffer indirect light */\n"
"uniform sampler2D z_buffer;      /* Z-Buffer */\n"
"\n"
"uniform vec3 camera_pos;\n"
"uniform mat4 inv_view_proj;\n"
"\n"
"#include <lightprocess>\n"
"\n"
"in vec2 v_uv;\n"
"\n"
"layout(location = 0) out vec4 out_color;\n"
"\n"
"void main() {\n"
"    /* sample base color and ambient from G-Buffer color */\n"
"    vec4 diffuse_occlusion = textureLod(g_color, v_uv, 0);\n"
"    vec3 diffuse = diffuse_occlusion.xyz;\n"
"    float occlusion = diffuse_occlusion.w;\n"
"    \n"
"    /* sample depth from Z-Buffer */\n"
"    float depth = textureLod(z_buffer, v_uv, 0).x;\n"
"    \n"
"    if (depth == 1.) {\n"
"        out_color = vec4(diffuse, 0.);\n"
"        return; /* ignore the pixels on skybox */\n"
"    }\n"
"    \n"
"    /* sample world normal from G-Buffer normal */\n"
"    vec3 normal = textureLod(g_normal, v_uv, 0).xyz;\n"
"    \n"
"    /* normalize normal to avoid artifacts */\n"
"    normal = normalize(unpack_normal(normal));\n"
"    \n"
"    /* sample specular and roughness from G-Buffer material */\n"
"    vec4 specular_roughness = textureLod(g_material, v_uv, 0);\n"
"    vec3 specular = specular_roughness.xyz;\n"
"    float roughness = specular_roughness.w;\n"
"    \n"
"    /* sample indirect light color from G-Buffer light */\n"
"    vec3 indirect_light = textureLod(g_light, v_uv, 0).xyz;\n"
"    \n"
"    /* create new material object */\n"
"    Material material;\n"
"    material.color = diffuse;\n"
"    material.f0 = specular;\n"
"    material.roughness = roughness;\n"
"    \n"
"    /* transform from screen space to world space */\n"
"    vec4 ndc = vec4(v_uv.xy, depth, 1.) * 2. - 1.;\n"
"    vec4 world_pos = inv_view_proj * ndc;\n"
"    world_pos /= world_pos.w;\n"
"    \n"
"    /* view from position to camera position */\n"
"    vec3 view_dir = normalize(camera_pos - world_pos.xyz);\n"
"    \n"
"    /* create new geometry object */\n"
"    Geometry geometry;\n"
"    geometry.position = world_pos.xyz;\n"
"    geometry.view_dir = view_dir;\n"
"    geometry.normal = normal;\n"
"    \n"
"    /* calculate color with light pipeline */\n"
"    out_color = vec4(light_process(material, geometry, indirect_light, occlusion), 1.);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/SSAO.vert.glsl ---------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* SSAO_VERT_GLSL = 
"#include <common>\n"
"\n"
"in vec3 vertex;\n"
"in vec2 uv;\n"
"\n"
"out vec2 v_uv;\n"
"\n"
"void main() {\n"
"    v_uv = uv;\n"
"    gl_Position = vec4(vertex, 1.);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/SSAO.frag.glsl ---------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* SSAO_FRAG_GLSL = 
"#include <common>\n"
"#include <packing>\n"
"#include <transform>\n"
"#include <poisson>\n"
"\n"
"#if SAMPLES == 16\n"
"#define POISSON_3D POISSON_3D_16\n"
"#elif SAMPLES == 32\n"
"#define POISSON_3D POISSON_3D_32\n"
"#elif SAMPLES == 64\n"
"#define POISSON_3D POISSON_3D_64\n"
"#endif\n"
"\n"
"uniform sampler2D g_normal;\n"
"uniform sampler2D z_buffer;\n"
"\n"
"uniform float intensity;\n"
"uniform float radius;\n"
"uniform float max_radius;\n"
"uniform float max_z;\n"
"uniform float near;\n"
"uniform float far;\n"
"uniform mat4 view;\n"
"uniform mat4 proj;\n"
"uniform mat4 inv_proj;\n"
"\n"
"in vec2 v_uv;\n"
"\n"
"layout(location = 0) out vec4 out_color;\n"
"\n"
"bool out_of_screen(vec2 coord) {\n"
"    return any(bvec4(coord.x < 0., coord.x > 1., coord.y < 0., coord.y > 1.));\n"
"}\n"
"\n"
"void main() {\n"
"    /* sample depth from Z-Buffer */\n"
"    float depth = textureLod(z_buffer, v_uv, 0).x;\n"
"    float z = depth_to_z_persp(depth, near, far);\n"
"    \n"
"    /* ignore the pixels on skybox */\n"
"    out_color = vec4(1., 1., 1., 1.);\n"
"    if (z > max_z) return;\n"
"    \n"
"    /* sample world normal from G-Buffer normal */\n"
"    vec3 normal = textureLod(g_normal, v_uv, 0).xyz;\n"
"    normal = normalize(unpack_normal(normal));\n"
"    normal = mat3(view) * normal;\n"
"    \n"
"    /* transform from screen space to world space */\n"
"    vec4 ndc = vec4(vec3(v_uv.xy, depth) * 2. - 1., 1.);\n"
"    vec4 view_pos = inv_proj * ndc;\n"
"    view_pos /= view_pos.w;\n"
"    \n"
"    /* generate noise from -1 to 1 */\n"
"    vec3 noise;\n"
"    noise.x = rand(vec3(v_uv, 0.));\n"
"    noise.y = rand(vec3(v_uv, 1.));\n"
"    noise.z = rand(vec3(v_uv, 2.));\n"
"    noise = noise * 2. - 1.;\n"
"    \n"
"    /* calculate ambient occlusion */\n"
"    float occlusion = 0.;\n"
"    for (int i = 0; i < SAMPLES; ++i) {\n"
"        \n"
"        /* calculate offset along with the normal */\n"
"        vec3 offset = mix(POISSON_3D[i], noise, 0.2) * radius;\n"
"        offset *= step(0., dot(offset, normal)) * 2. - 1.;\n"
"        vec3 sample_pos = view_pos.xyz + offset;\n"
"        \n"
"        /* transform view space to screen space */\n"
"        vec4 sample_ndc = proj * vec4(sample_pos, 1.);\n"
"        vec2 sample_uv = sample_ndc.xy / sample_ndc.w * 0.5 + 0.5;\n"
"        \n"
"        /* discard when sample UV is out of screen */\n"
"        if (out_of_screen(sample_uv)) continue;\n"
"        \n"
"        /* calculate difference between depths */\n"
"        float sample_depth = textureLod(z_buffer, sample_uv, 0).x;\n"
"        float sample_z = depth_to_z_persp(sample_depth, near, far);\n"
"        float delta = sample_z - sample_pos.z;\n"
"        \n"
"        /* compare depths and accumulate occlusion */\n"
"        float range_fade = 1. - smoothstep(max_radius * 0.75, max_radius, delta);\n"
"        occlusion += step(0., delta) * range_fade;\n"
"    }\n"
"    \n"
"    /* output ambient occlusion */\n"
"    occlusion *= 1. - smoothstep(max_z * 0.75, max_z, z);\n"
"    out_color = vec4(1. - intensity * occlusion / SAMPLES);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/SSR.vert.glsl ----------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* SSR_VERT_GLSL = 
"#include <common>\n"
"\n"
"in vec3 vertex;\n"
"in vec2 uv;\n"
"\n"
"out vec2 v_uv;\n"
"\n"
"void main() {\n"
"    v_uv = uv;\n"
"    gl_Position = vec4(vertex, 1.);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/SSR.frag.glsl ----------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* SSR_FRAG_GLSL = 
"#include <common>\n"
"#include <packing>\n"
"#include <transform>\n"
"\n"
"uniform sampler2D map;\n"
"uniform sampler2D g_normal;\n"
"uniform sampler2D g_material;\n"
"uniform sampler2D z_buffer;\n"
"\n"
"uniform int max_steps;\n"
"uniform float intensity;\n"
"uniform float thickness;\n"
"uniform float max_roughness;\n"
"uniform float near;\n"
"uniform float far;\n"
"uniform vec2 screen_size;\n"
"uniform mat4 view;\n"
"uniform mat4 proj;\n"
"uniform mat4 inv_proj;\n"
"\n"
"in vec2 v_uv;\n"
"\n"
"layout(location = 0) out vec4 out_color;\n"
"\n"
"bool out_of_screen(vec2 coord) {\n"
"    return any(bvec4(coord.x < 0., coord.x > 1., coord.y < 0., coord.y > 1.));\n"
"}\n"
"\n"
"void main() {\n"
"    /* sample color from texture */\n"
"    vec3 color = textureLod(map, v_uv, 0).xyz;\n"
"    \n"
"    /* sample depth from Z-Buffer */\n"
"    float depth = textureLod(z_buffer, v_uv, 0).x;\n"
"    \n"
"    /* ignore the pixels on skybox */\n"
"    out_color = vec4(color, 1.);\n"
"    if (depth == 1.) return;\n"
"    \n"
"    /* sample roughness from G-Buffer material */\n"
"    float roughness = textureLod(g_material, v_uv, 0).w;\n"
"    if (roughness >= max_roughness) return;\n"
"    \n"
"    /* sample world normal from G-Buffer normal */\n"
"    vec3 normal = textureLod(g_normal, v_uv, 0).xyz;\n"
"    normal = normalize(unpack_normal(normal));\n"
"    normal = mat3(view) * normal;\n"
"    \n"
"    /* transform from screen space to view space */\n"
"    vec4 ndc = vec4(vec3(v_uv.xy, depth) * 2. - 1., 1.);\n"
"    vec4 view_pos = inv_proj * ndc;\n"
"    view_pos /= view_pos.w;\n"
"    vec3 view_dir = normalize(view_pos.xyz);\n"
"    \n"
"    /* calculate reflect direction & position */\n"
"    vec3 reflect_dir = normalize(reflect(view_dir, normal));\n"
"    vec3 reflect_pos = view_pos.xyz + reflect_dir;\n"
"    \n"
"    /* calculate UV and depth of reflected point */\n"
"    vec4 reflect_ndc = proj * vec4(reflect_pos, 1.);\n"
"    reflect_ndc /= reflect_ndc.w;\n"
"    vec2 reflect_uv = reflect_ndc.xy * 0.5 + 0.5;\n"
"    float reflect_depth = reflect_ndc.z * 0.5 + 0.5;\n"
"    \n"
"    /* if ray is traced towards camera or out of the far plane */\n"
"    if (any(bvec2(reflect_depth < depth, reflect_depth > 1.))) return;\n"
"    \n"
"    /* calculate the direction of UV and Z */\n"
"    vec2 direction_uv = reflect_uv - v_uv;\n"
"    float direction_depth = reflect_depth - depth;\n"
"    \n"
"    /* calculate the minimum ray length to step over one texel */\n"
"    vec2 texel_size = 1. / screen_size;\n"
"    float ray_step = min(texel_size.x, texel_size.y) / length(direction_uv);\n"
"    \n"
"    /* initialize ray length to prevent intersecting at the beginning */\n"
"    float ray_length = 2.5 * ray_step;\n"
"    \n"
"    /* initialize total steps */\n"
"    int total_steps = 0;\n"
"    \n"
"    /* ray matching until total steps exceeds the limit */\n"
"    while (total_steps < max_steps) {\n"
"        \n"
"        /* get the UV of the current ray */\n"
"        vec2 ray_uv = v_uv + direction_uv * ray_length;\n"
"        if (out_of_screen(ray_uv)) break;\n"
"        \n"
"        /* get the Z of the current ray and cell */\n"
"        float ray_depth = depth + direction_depth * ray_length;\n"
"        float cell_depth = textureLod(z_buffer, ray_uv, 0).x;\n"
"        \n"
"        /* if ray has intersected with cell */\n"
"        if (ray_depth > cell_depth) {\n"
"            \n"
"            /* linearize the depth of the current ray and cell */\n"
"            if (ray_depth > 1.) return;\n"
"            float ray_z = depth_to_z_persp(ray_depth, near, far);\n"
"            float cell_z = depth_to_z_persp(cell_depth, near, far);\n"
"            \n"
"            /* ignore the pixels if the depth of ray is too large */\n"
"            if (-ray_z > -cell_z + thickness) return;\n"
"            \n"
"            /* calculate intensity attenuation */\n"
"            float attenuation = intensity;\n"
"            \n"
"            /* calculate distance attenuation */\n"
"            float max_f = float(max_steps);\n"
"            attenuation *= 1. - smoothstep(max_f * 0.5, max_f, float(total_steps));\n"
"            \n"
"            /* calculate screen edge attenuation */\n"
"            vec2 coords = smoothstep(0.2, 0.6, abs(vec2(0.5) - ray_uv.xy));\n"
"            float screen_edge_fade = saturate(1. - (coords.x + coords.y));\n"
"            attenuation *= screen_edge_fade;\n"
"            \n"
"            /* calculate reflected color with Fresnel */\n"
"            float cos_theta = max(dot(normal, -view_dir), 0.);\n"
"            vec3 f0 = textureLod(g_material, v_uv, 0).xyz;\n"
"            vec3 fresnel = f0 + (1. - f0) * pow(1. - cos_theta, 5.);\n"
"            vec3 reflect_color = textureLod(map, ray_uv, 0).xyz * fresnel;\n"
"            \n"
"            /* calculate final color with reflected color */\n"
"            out_color = vec4(mix(color, reflect_color, attenuation), 1.);\n"
"            return;\n"
"        }\n"
"        \n"
"        /* step to the next cell */\n"
"        ray_length += ray_step;\n"
"        \n"
"        /* accumulate to total steps */\n"
"        ++total_steps;\n"
"    }\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/BrightPass.vert.glsl ---------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* BRIGHTPASS_VERT_GLSL = 
"#include <common>\n"
"\n"
"in vec3 vertex;\n"
"in vec2 uv;\n"
"\n"
"out vec2 v_uv;\n"
"\n"
"void main() {\n"
"    v_uv = uv;\n"
"    gl_Position = vec4(vertex, 1.);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/BrightPass.frag.glsl ---------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* BRIGHTPASS_FRAG_GLSL = 
"#include <common>\n"
"\n"
"uniform sampler2D map;\n"
"\n"
"uniform float threshold;\n"
"\n"
"in vec2 v_uv;\n"
"\n"
"layout(location = 0) out vec4 out_color;\n"
"\n"
"void main() {\n"
"    out_color = textureLod(map, v_uv, 0);\n"
"    float luminance = relative_luminance(out_color.xyz);\n"
"    float alpha = smoothstep(threshold, threshold + 1., luminance);\n"
"    out_color = mix(vec4(0.), out_color, alpha);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/Bloom.vert.glsl --------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* BLOOM_VERT_GLSL = 
"#include <common>\n"
"\n"
"in vec3 vertex;\n"
"in vec2 uv;\n"
"\n"
"out vec2 v_uv;\n"
"\n"
"void main() {\n"
"    v_uv = uv;\n"
"    gl_Position = vec4(vertex, 1.);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/Bloom.frag.glsl --------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* BLOOM_FRAG_GLSL = 
"#include <common>\n"
"\n"
"uniform sampler2D map;\n"
"uniform sampler2D bloom_map;\n"
"\n"
"uniform vec3 tint;\n"
"uniform float intensity;\n"
"uniform float radius;\n"
"\n"
"in vec2 v_uv;\n"
"\n"
"layout(location = 0) out vec4 out_color;\n"
"\n"
"void main() {\n"
"    /* calculate bloom color */\n"
"    vec3 bloom_color = vec3(0.);\n"
"    bloom_color += textureLod(bloom_map, v_uv, 0).xyz * mix(1.0, 0.2, radius);\n"
"    bloom_color += textureLod(bloom_map, v_uv, 1).xyz * mix(0.8, 0.4, radius);\n"
"    bloom_color += textureLod(bloom_map, v_uv, 2).xyz * 0.6;\n"
"    bloom_color += textureLod(bloom_map, v_uv, 3).xyz * mix(0.4, 0.8, radius);\n"
"    bloom_color += textureLod(bloom_map, v_uv, 4).xyz * mix(0.2, 1.0, radius);\n"
"    \n"
"    /* add bloom color to output */\n"
"    out_color = textureLod(map, v_uv, 0);\n"
"    out_color.xyz += bloom_color * intensity * tint;\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/FXAA.vert.glsl ---------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* FXAA_VERT_GLSL = 
"#include <common>\n"
"\n"
"in vec3 vertex;\n"
"in vec2 uv;\n"
"\n"
"out vec2 v_uv;\n"
"\n"
"void main() {\n"
"    v_uv = uv;\n"
"    gl_Position = vec4(vertex, 1.);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/FXAA.frag.glsl ---------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* FXAA_FRAG_GLSL = 
"#include <common>\n"
"\n"
"#define FXAA_PC                1\n"
"#define FXAA_GLSL_130        1\n"
"#define FXAA_GREEN_AS_LUMA    1\n"
"#include <fxaa3_11>\n"
"\n"
"uniform sampler2D map;\n"
"\n"
"uniform vec2 screen_size;\n"
"\n"
"in vec2 v_uv;\n"
"\n"
"layout(location = 0) out vec4 out_color;\n"
"\n"
"void main() {\n"
"    vec2 texel_size = vec2(1.) / screen_size;\n"
"    out_color = FxaaPixelShader(\n"
"        v_uv,                                /* pos */\n"
"        vec4(0.),                            /* unused */\n"
"        map,                                 /* tex */\n"
"        map,                                 /* unused */\n"
"        map,                                 /* unused */\n"
"        texel_size,                          /* fxaaQualityRcpFrame */\n"
"        vec4(0.),                            /* unused */\n"
"        vec4(0.),                            /* unused */\n"
"        vec4(0.),                            /* unused */\n"
"        0.75,                                /* fxaaQualitySubpix */\n"
"        0.166,                               /* fxaaQualityEdgeThreshold */\n"
"        0.0833,                              /* fxaaQualityEdgeThresholdMin */\n"
"        0.,                                  /* unused */\n"
"        0.,                                  /* unused */\n"
"        0.,                                  /* unused */\n"
"        vec4(0.)                             /* unused */\n"
"    );\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/Grain.vert.glsl --------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* GRAIN_VERT_GLSL = 
"#include <common>\n"
"\n"
"in vec3 vertex;\n"
"in vec2 uv;\n"
"\n"
"out vec2 v_uv;\n"
"\n"
"void main() {\n"
"    v_uv = uv;\n"
"    gl_Position = vec4(vertex, 1.);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/Grain.frag.glsl --------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* GRAIN_FRAG_GLSL = 
"#include <common>\n"
"\n"
"uniform sampler2D map;\n"
"\n"
"uniform float intensity;\n"
"uniform float seed;\n"
"\n"
"in vec2 v_uv;\n"
"\n"
"layout(location = 0) out vec4 out_color;\n"
"\n"
"void main() {\n"
"    float grain = rand(v_uv * seed);\n"
"    out_color = textureLod(map, v_uv, 0);\n"
"    out_color.xyz *= 1. + grain * intensity;\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/ToneMapping.vert.glsl --------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* TONEMAPPING_VERT_GLSL = 
"#include <common>\n"
"\n"
"in vec3 vertex;\n"
"in vec2 uv;\n"
"\n"
"out vec2 v_uv;\n"
"\n"
"void main() {\n"
"    v_uv = uv;\n"
"    gl_Position = vec4(vertex, 1.);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/ToneMapping.frag.glsl --------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* TONEMAPPING_FRAG_GLSL = 
"#include <common>\n"
"#include <tonemapping>\n"
"\n"
"uniform sampler2D map;\n"
"\n"
"uniform float exposure;\n"
"\n"
"in vec2 v_uv;\n"
"\n"
"layout(location = 0) out vec4 out_color;\n"
"\n"
"void main() {\n"
"    out_color = textureLod(map, v_uv, 0);\n"
"    out_color.xyz = TONE_MAP(out_color.xyz, exposure);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/ColorGrading.vert.glsl -------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* COLORGRADING_VERT_GLSL = 
"#include <common>\n"
"\n"
"in vec3 vertex;\n"
"in vec2 uv;\n"
"\n"
"out vec2 v_uv;\n"
"\n"
"void main() {\n"
"    v_uv = uv;\n"
"    gl_Position = vec4(vertex, 1.);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/shaders/library/ColorGrading.frag.glsl -------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

constexpr const char* COLORGRADING_FRAG_GLSL = 
"#include <common>\n"
"#include <colorgrading>\n"
"\n"
"uniform sampler2D map;\n"
"\n"
"uniform vec3 saturation;\n"
"uniform vec3 contrast;\n"
"uniform vec3 gamma;\n"
"uniform vec3 gain;\n"
"uniform vec3 offset;\n"
"\n"
"in vec2 v_uv;\n"
"\n"
"layout(location = 0) out vec4 out_color;\n"
"\n"
"void main() {\n"
"    out_color = textureLod(map, v_uv, 0);\n"
"    out_color.xyz = color_grading(out_color.xyz, saturation, contrast, gamma, gain, offset);\n"
"}\n";

}

/* -------------------------------------------------------------------------- */
/* ---- ink/LoadShaders.cpp ------------------------------------------------- */
/* -------------------------------------------------------------------------- */

namespace ink {

void load_shaders() {
    ShaderCache::load_include("Common", COMMON_GLSL);
    ShaderCache::load_include("Transform", TRANSFORM_GLSL);
    ShaderCache::load_include("Packing", PACKING_GLSL);
    ShaderCache::load_include("ColorSpace", COLORSPACE_GLSL);
    ShaderCache::load_include("Poisson", POISSON_GLSL);
    ShaderCache::load_include("Halton", HALTON_GLSL);
    ShaderCache::load_include("BRDF", BRDF_GLSL);
    ShaderCache::load_include("Shadow", SHADOW_GLSL);
    ShaderCache::load_include("Lights", LIGHTS_GLSL);
    ShaderCache::load_include("Fogs", FOGS_GLSL);
    ShaderCache::load_include("Cubemap", CUBEMAP_GLSL);
    ShaderCache::load_include("IBLFilter", IBLFILTER_GLSL);
    ShaderCache::load_include("ToneMapping", TONEMAPPING_GLSL);
    ShaderCache::load_include("ColorGrading", COLORGRADING_GLSL);
    ShaderCache::load_include("Fxaa3_11", FXAA3_11_GLSL);
    ShaderCache::load_include("LightProcess", LIGHTPROCESS_GLSL);
    ShaderCache::load_vert("Copy", COPY_VERT_GLSL);
    ShaderCache::load_frag("Copy", COPY_FRAG_GLSL);
    ShaderCache::load_vert("Blend", BLEND_VERT_GLSL);
    ShaderCache::load_frag("Blend", BLEND_FRAG_GLSL);
    ShaderCache::load_vert("BoxBlur", BOXBLUR_VERT_GLSL);
    ShaderCache::load_frag("BoxBlur", BOXBLUR_FRAG_GLSL);
    ShaderCache::load_vert("GaussianBlur", GAUSSIANBLUR_VERT_GLSL);
    ShaderCache::load_frag("GaussianBlur", GAUSSIANBLUR_FRAG_GLSL);
    ShaderCache::load_vert("BilateralBlur", BILATERALBLUR_VERT_GLSL);
    ShaderCache::load_frag("BilateralBlur", BILATERALBLUR_FRAG_GLSL);
    ShaderCache::load_vert("SphericalBlur", SPHERICALBLUR_VERT_GLSL);
    ShaderCache::load_frag("SphericalBlur", SPHERICALBLUR_FRAG_GLSL);
    ShaderCache::load_vert("Cubemap", CUBEMAP_VERT_GLSL);
    ShaderCache::load_frag("Cubemap", CUBEMAP_FRAG_GLSL);
    ShaderCache::load_vert("Cube", CUBE_VERT_GLSL);
    ShaderCache::load_frag("Cube", CUBE_FRAG_GLSL);
    ShaderCache::load_vert("Standard", STANDARD_VERT_GLSL);
    ShaderCache::load_frag("Standard", STANDARD_FRAG_GLSL);
    ShaderCache::load_vert("Shadow", SHADOW_VERT_GLSL);
    ShaderCache::load_frag("Shadow", SHADOW_FRAG_GLSL);
    ShaderCache::load_vert("Lighting", LIGHTING_VERT_GLSL);
    ShaderCache::load_frag("Lighting", LIGHTING_FRAG_GLSL);
    ShaderCache::load_vert("SSAO", SSAO_VERT_GLSL);
    ShaderCache::load_frag("SSAO", SSAO_FRAG_GLSL);
    ShaderCache::load_vert("SSR", SSR_VERT_GLSL);
    ShaderCache::load_frag("SSR", SSR_FRAG_GLSL);
    ShaderCache::load_vert("BrightPass", BRIGHTPASS_VERT_GLSL);
    ShaderCache::load_frag("BrightPass", BRIGHTPASS_FRAG_GLSL);
    ShaderCache::load_vert("Bloom", BLOOM_VERT_GLSL);
    ShaderCache::load_frag("Bloom", BLOOM_FRAG_GLSL);
    ShaderCache::load_vert("FXAA", FXAA_VERT_GLSL);
    ShaderCache::load_frag("FXAA", FXAA_FRAG_GLSL);
    ShaderCache::load_vert("Grain", GRAIN_VERT_GLSL);
    ShaderCache::load_frag("Grain", GRAIN_FRAG_GLSL);
    ShaderCache::load_vert("ToneMapping", TONEMAPPING_VERT_GLSL);
    ShaderCache::load_frag("ToneMapping", TONEMAPPING_FRAG_GLSL);
    ShaderCache::load_vert("ColorGrading", COLORGRADING_VERT_GLSL);
    ShaderCache::load_frag("ColorGrading", COLORGRADING_FRAG_GLSL);
}

}
