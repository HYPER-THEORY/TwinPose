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

#include "PackedIntArray.h"

#include <vector>

namespace v3
{

class DisjointSet
{
public:
	DisjointSet() = default;

	DisjointSet(uint32_t size) : person_id_per_view(size, 0), weights(size), fathers(size)
	{
		for (int i = 0; i < size; ++i)
		{
			fathers[i] = i;
		}
	}

	uint32_t get_size() const
	{
		return fathers.size();
	}

	PackedIntArray& get_person_id_per_view(uint32_t index)
	{
		return person_id_per_view[index];
	}

	const PackedIntArray& get_person_id_per_view(uint32_t index) const
	{
		return person_id_per_view[index];
	}

	float& get_weight(uint32_t index)
	{
		return weights[index];
	}

	const float& get_weight(uint32_t index) const
	{
		return weights[index];
	}

	uint32_t find(uint32_t index) const
	{
		return fathers[index] = fathers[index] == index ? index : find(fathers[index]);
	}

	bool conflicts(uint32_t a, uint32_t b, int n) const
	{
		uint32_t anc_a = find(a);
		uint32_t anc_b = find(b);
		if (anc_a == anc_b)
		{
			return true;
		}
		return v3::conflicts(person_id_per_view[anc_a], person_id_per_view[anc_b], n);
	}

	void merge(uint32_t a, uint32_t b, float weight)
	{
		uint32_t anc_a = find(a);
		uint32_t anc_b = find(b);
		fathers[anc_b] = anc_a;
		weights[anc_a] += weights[anc_b] + weight;
		person_id_per_view[anc_a] = v3::make_union(person_id_per_view[anc_a], person_id_per_view[anc_b]);
	}

private:
	std::vector<PackedIntArray> person_id_per_view;
	std::vector<float> weights;
	mutable std::vector<uint32_t> fathers;
};

}
