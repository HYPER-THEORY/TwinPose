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

#include "Room.h"

#include "algorithm/Common.h"
#include "algorithm/EdgeSet.h"
#include "algorithm/PackedIntArray.h"
#include "algorithm/PoseBuilder.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <utility>
#include <vector>

namespace v3
{

constexpr ink::Vec3 WHITE_COLOR = {1.0f, 1.0f, 1.0f};

constexpr int VIZ_JOINT_NUM = 15;

constexpr float VIZ_POSE_GRAPH_BONE_RADIUS = 0.005f;
constexpr float VIZ_RAY_RADIUS = 0.0025f;
constexpr float VIZ_RAY_LENGTH = 10.0f;

using RemapPosition = std::function<ink::Vec3(const ink::Vec3&)>;

inline const std::vector<std::pair<int, int>> VIZ_BONES = {
	{ 5,  6}, { 2,  3}, { 6,  7}, { 3,  4}, {12, 13}, { 9, 10}, {13, 14},
	{10, 11}, { 1,  0}, { 1,  8}, { 1,  2}, { 1,  5}, { 8,  9}, { 8, 12},
};

inline ink::Vec3 get_viz_color(float value)
{
	return ink::Color::hsl_to_rgb({value * 0.33f, 1, 0.5f});
}

inline void viz_pose_3d(Room& room, const Pose3D& pose, const RemapPosition& remap,
	const ink::Vec3& color = Room::DEFAULT_COLOR)
{
	for (int joint_type = 0; joint_type < VIZ_JOINT_NUM; ++joint_type)
	{
		if (!pose.has_joint(joint_type)) continue;
		room.set_joint(remap(pose.get_joint(joint_type)), 0.02f, color);
	}
	for (auto& [joint_type_1, joint_type_2] : VIZ_BONES)
	{
		if (!pose.has_joint(joint_type_1) || !pose.has_joint(joint_type_2)) continue;
		ink::Vec3 position_1 = remap(pose.get_joint(joint_type_1));
		ink::Vec3 position_2 = remap(pose.get_joint(joint_type_2));
		room.set_bone(position_1, position_2, 0.01f, color);
	}
}

inline void viz_multi_pose_3d(Room& room, const MultiPose3D& multi_pose, const RemapPosition& remap,
	const ink::Vec3& color = Room::DEFAULT_COLOR)
{
	for (auto& [pose_id, pose] : multi_pose)
	{
		viz_pose_3d(room, pose, remap, color);
	}
}

inline ink::Vec3 get_palette_color(int index)
{
	return ink::Color::hsl_to_rgb({std::fmodf(index * ink::PI, 1.0f), 0.7f, 0.7f});
}

inline void viz_joints(Room& room,
	const std::vector<std::vector<PoseBuilder::Joint>>& joint_list_per_type,
	const std::vector<int>& joint_types,
	int debug_joint_type,
	float radius,
	const RemapPosition& remap)
{
	for (int joint_type : joint_types)
	{
		if (joint_type >= joint_list_per_type.size())
		{
			continue;
		}
		ink::Vec3 color = get_palette_color(joint_type);
		if (joint_type == debug_joint_type)
		{
			color = WHITE_COLOR;
		}
		for (auto& joint : joint_list_per_type[joint_type])
		{
			room.set_joint(remap(joint.position), radius, color);
		}
	}
}

inline void viz_joints(Room& room,
	const std::vector<PoseBuilder::Joint>& joints,
	float radius,
	const RemapPosition& remap)
{
	float min_score = std::numeric_limits<float>::max();
	float max_score = 0.0f;
	for (auto& joint : joints)
	{
		min_score = std::min(min_score, joint.score);
		max_score = std::max(max_score, joint.score);
	}
	max_score = std::max(min_score + 0.01f, max_score);

	for (auto& joint : joints)
	{
		float score = (joint.score - min_score) / (max_score - min_score);
		room.set_joint(remap(joint.position), radius, get_viz_color(score));
	}
}

inline void viz_clusters(Room& room,
	const std::vector<std::vector<PoseBuilder::JointCluster>>& cluster_list_per_type,
	const std::vector<int>& joint_types,
	int debug_joint_type,
	float radius,
	const RemapPosition& remap)
{
	for (int joint_type : joint_types)
	{
		if (joint_type >= cluster_list_per_type.size())
		{
			continue;
		}
		ink::Vec3 color = get_palette_color(joint_type);
		if (joint_type == debug_joint_type && joint_types.size() > 1)
		{
			color = WHITE_COLOR;
		}
		for (auto& cluster : cluster_list_per_type[joint_type])
		{
			float alpha = std::clamp(1.0f - std::exp(-cluster.score), 0.0f, 1.0f);
			room.set_joint(remap(cluster.position), radius, color, alpha);
		}
	}
}

inline void viz_pose_graphs(Room& room,
	const std::vector<std::vector<PoseBuilder::JointCluster>>& cluster_list_per_type,
	const std::vector<std::vector<EdgeSet>>& pose_graph_per_person,
	const std::vector<std::pair<int, int>>& ordered_bone_types,
	const RemapPosition& remap)
{
	for (int person_index = 0; person_index < pose_graph_per_person.size(); ++person_index)
	{
		auto& pose_graph = pose_graph_per_person[person_index];
		ink::Vec3 color = get_palette_color(person_index);

		int bone_num = std::min(pose_graph.size(), ordered_bone_types.size());
		for (int bone_index = 0; bone_index < bone_num; ++bone_index)
		{
			auto& [joint_type_1, joint_type_2] = ordered_bone_types[bone_index];
			auto& cluster_list_1 = cluster_list_per_type[joint_type_1];
			auto& cluster_list_2 = cluster_list_per_type[joint_type_2];

			auto& edge_set = pose_graph[bone_index];
			for (uint32_t edge_index = 0; edge_index < edge_set.get_edge_num(); ++edge_index)
			{
				auto& edge = edge_set.get_edge(edge_index);
				room.set_bone(remap(cluster_list_1[edge.node_a].position),
					remap(cluster_list_2[edge.node_b].position), VIZ_POSE_GRAPH_BONE_RADIUS, color);
			}
		}
	}
}

inline void viz_pose_graph(Room& room,
	const std::vector<std::vector<PoseBuilder::JointCluster>>& cluster_list_per_type,
	const std::vector<EdgeSet>& pose_graph,
	const std::vector<std::pair<int, int>>& ordered_bone_types,
	const RemapPosition& remap)
{
	int bone_num = std::min(pose_graph.size(), ordered_bone_types.size());
	for (int bone_index = 0; bone_index < bone_num; ++bone_index)
	{
		auto& [joint_type_1, joint_type_2] = ordered_bone_types[bone_index];
		auto& cluster_list_1 = cluster_list_per_type[joint_type_1];
		auto& cluster_list_2 = cluster_list_per_type[joint_type_2];

		auto& edge_set = pose_graph[bone_index];

		float max_bone_score = 0.0f;
		for (uint32_t edge_index = 0; edge_index < edge_set.get_edge_num(); ++edge_index)
		{
			max_bone_score = std::max(max_bone_score, edge_set.get_edge(edge_index).weight);
		}
		max_bone_score = std::max(max_bone_score, 0.01f);

		for (uint32_t edge_index = 0; edge_index < edge_set.get_edge_num(); ++edge_index)
		{
			auto& edge = edge_set.get_edge(edge_index);
			float score = edge.weight / max_bone_score;
			room.set_bone(remap(cluster_list_1[edge.node_a].position),
				remap(cluster_list_2[edge.node_b].position), VIZ_POSE_GRAPH_BONE_RADIUS, get_viz_color(score));
		}
	}
}

inline void viz_rays(Room& room, const MultiView& multi_view, int view_num,
	int joint_type, const RemapPosition& remap)
{
	for (int view_id = 0; view_id < view_num; ++view_id)
	{
		const View& view = multi_view.get_view(view_id);
		ink::Vec3 color = get_palette_color(view_id) * 1.77f;

		int joint_num = view.get_joint_num(joint_type);
		for (int person_id = 1; person_id <= joint_num; ++person_id)
		{
			const Joint2D& joint = view.get_joint(joint_type, person_id);
			const ink::Ray& ray = view.get_ray(joint);

			ink::Vec3 origin = ray.origin;
			ink::Vec3 target = ray.origin - ray.direction * VIZ_RAY_LENGTH;
			room.set_bone(remap(origin), remap(target), VIZ_RAY_RADIUS, color);
		}
	}
}

}
