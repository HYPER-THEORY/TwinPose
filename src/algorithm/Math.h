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

#include "ink/ink.h"

namespace v3
{

constexpr float EPSILON = 1e-6;

inline ink::Vec3 closest_point(const ink::Ray& ray, const ink::Vec3& point)
{
	return ray.origin + ray.direction * (point - ray.origin).dot(ray.direction);
}

inline float distance(const ink::Ray& ray, const ink::Vec3& point)
{
	return closest_point(ray, point).distance(point);
}

inline float distance(const ink::Ray& ray_a, const ink::Ray& ray_b)
{
	ink::Vec3 b_to_a = ray_a.origin - ray_b.origin;
	ink::Vec3 c = ray_a.direction.cross(ray_b.direction);

	return c.magnitude() < EPSILON ? b_to_a.cross(ray_a.direction).magnitude() :
		std::abs(b_to_a.dot(c.normalize()));
}

}
