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

#include <cstdint>

#ifdef _MSC_VER
#include <__msvc_int128.hpp>
#endif

namespace v3
{

#if defined(MAX_CAMERA_NUM) && MAX_CAMERA_NUM <= 8
using PackedIntArray = uint64_t;
#else
#ifdef _MSC_VER
using PackedIntArray = std::_Unsigned128;
#else
using PackedIntArray = unsigned __int128;
#endif
#endif

template <typename T = uint8_t>
inline const T* as_array(const PackedIntArray& packed_int_array)
{
	return reinterpret_cast<const T*>(&packed_int_array);
}

template <typename T = uint8_t>
inline T* as_array(PackedIntArray& packed_int_array)
{
	return reinterpret_cast<T*>(&packed_int_array);
}

inline bool conflicts(const PackedIntArray& a, const PackedIntArray& b, int n)
{
	const uint8_t* a_array = as_array(a);
	const uint8_t* b_array = as_array(b);
	for (int i = 0; i < n; ++i)
	{
		if (a_array[i] && b_array[i] && a_array[i] != b_array[i])
		{
			return true;
		}
	}
	return false;
}

inline bool contains(const PackedIntArray& a, const PackedIntArray& b, int n)
{
	const uint8_t* a_array = as_array(a);
	const uint8_t* b_array = as_array(b);
	for (int i = 0; i < n; ++i)
	{
		if (b_array[i] && a_array[i] != b_array[i])
		{
			return false;
		}
	}
	return true;
}

inline PackedIntArray make_union(const PackedIntArray& a, const PackedIntArray& b)
{
	return a | b;
}

inline PackedIntArray& apply_union(PackedIntArray& a, const PackedIntArray& b)
{
	return a |= b;
}

inline PackedIntArray make_intersection(const PackedIntArray& a, const PackedIntArray& b)
{
	return a & b;
}

inline PackedIntArray& apply_intersection(PackedIntArray& a, const PackedIntArray& b)
{
	return a &= b;
}

}
