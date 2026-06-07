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

#include <algorithm>
#include <vector>

namespace v3
{

class EdgeSet
{
public:
	struct Edge { uint32_t node_a = 0, node_b = 0; float weight = 0; };

	EdgeSet() = default;

	void add_edge(uint32_t node_a, uint32_t node_b, float weight)
	{
		edges.emplace_back(node_a, node_b, weight);
	}

	void clear_edges()
	{
		edges.clear();
	}

	uint32_t get_edge_num() const
	{
		return edges.size();
	}

	Edge& get_edge(uint32_t index)
	{
		return edges[index];
	}

	const Edge& get_edge(uint32_t index) const
	{
		return edges[index];
	}

	void sort_by_weight(bool reverse = false)
	{
		auto sort_edges = [](const Edge& edge_a, const Edge& edge_b) -> bool
		{
			return edge_a.weight < edge_b.weight;
		};
		if (!reverse)
		{
			std::sort(edges.begin(), edges.end(), sort_edges);
		}
		else
		{
			std::sort(edges.rbegin(), edges.rend(), sort_edges);
		}
	}

private:
	std::vector<Edge> edges;
};

}
