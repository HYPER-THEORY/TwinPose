/// 
/// Copyright(C) 2026 HYPERTHEORY
/// 
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
/// 

#pragma once

#include "Common.h"
#include "Math.h"

#include <cassert>

namespace v3
{

inline void triangulate(const ink::Ray* rays, int ray_num, ink::Vec3& center)
{
	assert(ray_num >= 2);
	ink::Mat3 A;
	ink::Vec3 b;
	for (int i = 0; i < ray_num; ++i)
	{
		const ink::Ray& ray = rays[i];
		float dx = ray.direction.x;
		float dy = ray.direction.y;
		float dz = ray.direction.z;
		ink::Mat3 N = {
			-1 + dx * dx,      dx * dy,      dx * dz,
			     dy * dx, -1 + dy * dy,      dy * dz,
			     dz * dx,      dz * dy, -1 + dz * dz,
		};
		A += N;
		b += N * ray.origin;
	}
	center = inverse_3x3(A) * b;
}

inline void triangulate(const ink::Ray* rays, int ray_num, ink::Vec3& center, float& radius)
{
	assert(ray_num >= 2);
	triangulate(rays, ray_num, center);
	radius = 0;
	for (int i = 0; i < ray_num; ++i)
	{
		radius = std::max(radius, distance(rays[i], center));
	}
}

inline void triangulate(const ink::Ray* rays, const float* weights, int ray_num, ink::Vec3& center)
{
	assert(ray_num >= 2);
	ink::Mat3 A;
	ink::Vec3 b;
	for (int i = 0; i < ray_num; ++i)
	{
		const ink::Ray& ray = rays[i];
		float weight = weights[i];
		float dx = ray.direction.x;
		float dy = ray.direction.y;
		float dz = ray.direction.z;
		ink::Mat3 N = {
			-1 + dx * dx,      dx * dy,      dx * dz,
			     dy * dx, -1 + dy * dy,      dy * dz,
			     dz * dx,      dz * dy, -1 + dz * dz,
		};
		A += N * weight;
		b += N * ray.origin * weight;
	}
	center = inverse_3x3(A) * b;
}

inline void triangulate(const ink::Ray* rays, const float* weights, int ray_num, ink::Vec3& center, float& radius)
{
	assert(ray_num >= 2);
	triangulate(rays, weights, ray_num, center);
	radius = 0;
	for (int i = 0; i < ray_num; ++i)
	{
		radius = std::max(radius, distance(rays[i], center));
	}
}

}
