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

#include <vector>

namespace v3
{

class UndirectedGraph
{
public:
	struct Edge { uint32_t next_node = 0; float weight = 0; };

	UndirectedGraph() = default;

	UndirectedGraph(uint32_t size) : nodes(size) {}

	uint32_t add_node()
	{
		nodes.emplace_back();
		return nodes.size() - 1;
	}

	void clear_nodes()
	{
		nodes.clear();
	}

	uint32_t get_node_num() const
	{
		return nodes.size();
	}

	void add_edge(uint32_t node_a, uint32_t node_b, float weight)
	{
		nodes[node_a].emplace_back(node_b, weight);
		nodes[node_b].emplace_back(node_a, weight);
	}

	void clear_edges(uint32_t node)
	{
		nodes[node].clear();
	}

	uint32_t get_edge_num(uint32_t node) const
	{
		return nodes[node].size();
	}

	Edge& get_edge(uint32_t node, int edge_index)
	{
		return nodes[node][edge_index];
	}

	const Edge& get_edge(uint32_t node, int edge_index) const
	{
		return nodes[node][edge_index];
	}

private:
	std::vector<std::vector<Edge>> nodes;
};

}
