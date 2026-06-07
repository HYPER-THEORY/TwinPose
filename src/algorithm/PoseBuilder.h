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

#include "AnomalyDetector.h"
#include "Common.h"
#include "DisjointSet.h"
#include "EdgeSet.h"
#include "Math.h"
#include "PackedIntArray.h"
#include "Timer.h"
#include "Triangulation.h"

#include <cassert>

namespace v3
{

struct PoseBuilderParams
{
	std::vector<std::vector<int>> skeleton_tree;         ///< the skeleton tree in pose estimation
	std::vector<int> ordered_joint_types;                ///< the joint types in topological order
	int max_person_num = 0;                              ///< the maximun output number of persons

	bool use_bone_length_score = true;                   ///< whether to use bone length score
	bool use_edge_num_in_association = false;            ///< whether to use edge num when computing association scores

	float conf_discard_threshold = 0.0f;                 ///< discards the joint if the confidence is below the threshold
	float conf_exponent = 1.0f;                          ///< the exponent of confidence

	float max_joint_radius = 0.05f;                      ///< the maximum joint radius after pairwise triangulation
	float max_joint_cluster_radius = 0.10f;              ///< the maximum joint cluster radius

	float association_score_threshold = 5.0f;            ///< the minimum score to make an association edge
	float pose_discard_threshold = 10.0f;                ///< discards the pose output if the score is below the threshold

	std::vector<const AnomalyDetector*> bone_lengths;    ///< the bone length anomaly detectors
};

struct PoseTrackingParams
{
	float tracking_discard_threshold = 10.0f;            ///< the minimum score to keep a pose track
	float tracking_distance = 0.1f;                      ///< the maximum distance for bone tracking
	float lost_track_penalty = 0.5f;                     ///< the penalty when the bone loses tracking
	const MultiPose3D* history_multi_pose = nullptr;     ///< the multi pose from the previous frame
};

class PoseBuilder
{
public:
	struct Joint
	{
		ink::Vec3 position = {0, 0, 0};
		float radius = 0;
		float epipolar_score = 0;
		float conf_score = 0;
		float score = 0;
		uint16_t view_id_1 = 0;
		uint16_t view_id_2 = 0;
		PackedIntArray person_id_per_view;
	};

	struct JointCluster
	{
		ink::Vec3 position = {0, 0, 0};
		float score = 0;
		PackedIntArray person_id_per_view;
		std::vector<Joint> members;
	};

	struct Branch
	{
		Branch* last_branch = nullptr;
		float last_score = 0.0f;
		float score = 0.0f;
		std::vector<JointCluster*> clusters;
		std::vector<float> bone_scores;
	};

	PoseBuilder(const MultiView* multi_view, PoseBuilderParams params, PoseTrackingParams t_params) :
		multi_view(multi_view), params(std::move(params)), t_params(std::move(t_params)) {}

	void build()
	{
		{
			Timer timer("PoseBuilder_Init");
			init();
		}
		{
			Timer timer("PoseBuilder_BuildJointLists");
			build_joint_lists();
		}
		{
			Timer timer("PoseBuilder_ClusterJoints");
			cluster_joints();
		}
		{
			Timer timer("PoseBuilder_AssociateViews");
			associate_views();
		}
		{
			Timer timer("PoseBuilder_BuildPoseGraphs");
			build_pose_graphs();
		}
		if (t_params.history_multi_pose)
		{
			Timer timer("PoseBuilder_TrackPoses");
			track_poses();
		}
		{
			Timer timer("PoseBuilder_ResolvePoseGraphs");
			resolve_pose_graphs();
		}
	}

	std::vector<std::vector<Joint>> joint_list_per_type;
	std::vector<std::vector<JointCluster>> cluster_list_per_type;

	std::vector<std::pair<PackedIntArray, float>> association_infos;
	std::vector<std::vector<EdgeSet>> pose_graph_per_person;

	MultiPose3D result;

protected:
	struct PackedIntArrayHash
	{
		size_t operator()(const PackedIntArray& value) const
		{
			const uint64_t* array = as_array<uint64_t>(value);
			size_t lo = std::hash<uint64_t>()(array[0]);
			size_t hi = std::hash<uint64_t>()(array[1]);
			return lo ^ (hi << 4);
		}
	};

	void init()
	{
		assert(params.ordered_joint_types.size() == params.bone_lengths.size());

		std::unordered_set<int> joint_type_set;

		for (auto& joint_type : params.ordered_joint_types)
		{
			joint_type_set.emplace(joint_type);
		}

		for (auto& joint_type_1 : params.ordered_joint_types)
		{
			for (auto& joint_type_2 : params.skeleton_tree[joint_type_1])
			{
				if (joint_type_set.contains(joint_type_2))
				{
					ordered_bone_types.emplace_back(joint_type_1, joint_type_2);
				}
			}
		}

		view_num = multi_view->get_view_num();
	}

	void build_joint_lists()
	{
		joint_list_per_type.resize(params.ordered_joint_types.size());

		for (auto& joint_type : params.ordered_joint_types)
		{
			std::vector<std::pair<int, int>> view_id_person_id_pairs;

			for (int view_id = 0; view_id < view_num; ++view_id)
			{
				int joint_num = multi_view->get_view(view_id).get_joint_num(joint_type);

				for (int person_id = 1; person_id <= joint_num; ++person_id)
				{
					view_id_person_id_pairs.emplace_back(view_id, person_id);
				}
			}

			auto& joint_list = joint_list_per_type[joint_type];

			for (auto& [view_id_1, person_id_1] : view_id_person_id_pairs)
			{
				auto& view_1 = multi_view->get_view(view_id_1);

				for (auto& [view_id_2, person_id_2] : view_id_person_id_pairs)
				{
					if (view_id_1 <= view_id_2)
					{
						continue;
					}

					auto& view_2 = multi_view->get_view(view_id_2);

					auto& joint_1 = view_1.get_joint(joint_type, person_id_1);
					auto& joint_2 = view_2.get_joint(joint_type, person_id_2);

					auto& ray_1 = view_1.get_ray(joint_1);
					auto& ray_2 = view_2.get_ray(joint_2);

					float radius = distance(ray_1, ray_2) * 0.5f;

					if (radius >= params.max_joint_radius)
					{
						continue;
					}

					float conf = std::sqrt(joint_1.conf * joint_2.conf);

					if (conf <= params.conf_discard_threshold)
					{
						continue;
					}

					conf = (conf - params.conf_discard_threshold) / (1 - params.conf_discard_threshold);

					Joint& new_joint = joint_list.emplace_back();
					new_joint.view_id_1 = view_id_1;
					new_joint.view_id_2 = view_id_2;

					uint8_t* int_array = as_array(new_joint.person_id_per_view);
					int_array[view_id_1] = person_id_1;
					int_array[view_id_2] = person_id_2;

					ink::Ray rays[2] = {ray_1, ray_2};
					triangulate(rays, 2, new_joint.position);
					new_joint.radius = radius;

					new_joint.epipolar_score = (1.0f - radius / params.max_joint_radius);
					new_joint.conf_score = std::pow(conf, params.conf_exponent);

					new_joint.score = std::sqrt(new_joint.epipolar_score * new_joint.conf_score);
				}
			}

			auto sort_joint_list = [](const Joint& a, const Joint& b) -> bool
			{
				return a.score > b.score;
			};
			std::sort(joint_list.begin(), joint_list.end(), sort_joint_list);
		}
	}

	void cluster_joints()
	{
		const float inv_grid_size = 1.0f / (params.max_joint_cluster_radius * 2.0f);

		cluster_list_per_type.resize(params.ordered_joint_types.size());

		for (auto& joint_type : params.ordered_joint_types)
		{
			struct Int3
			{
				int32_t x = 0;
				int32_t y = 0;
				int32_t z = 0;
			};

			struct Int3Hash
			{
				size_t operator()(const Int3& value) const
				{
					size_t x = std::hash<int32_t>()(value.x);
					size_t y = std::hash<int32_t>()(value.y);
					size_t z = std::hash<int32_t>()(value.z);
					return x ^ (y << 1) ^ (z << 2);
				}
			};

			struct Int3Equal
			{
				bool operator()(const Int3& a, const Int3& b) const
				{
					return a.x == b.x && a.y == b.y && a.z == b.z;
				}
			};

			std::unordered_map<Int3, std::vector<Joint>, Int3Hash, Int3Equal> grids;

			auto& joint_list = joint_list_per_type[joint_type];

			for (auto& joint : joint_list)
			{
				int32_t coord_x = std::round(joint.position.x * inv_grid_size);
				int32_t coord_y = std::round(joint.position.y * inv_grid_size);
				int32_t coord_z = std::round(joint.position.z * inv_grid_size);

				Int3 grid_coords[] = {
					{     coord_x,      coord_y,      coord_z},
					{     coord_x,      coord_y, -1 + coord_z},
					{     coord_x, -1 + coord_y,      coord_z},
					{     coord_x, -1 + coord_y, -1 + coord_z},
					{-1 + coord_x,      coord_y,      coord_z},
					{-1 + coord_x,      coord_y, -1 + coord_z},
					{-1 + coord_x, -1 + coord_y,      coord_z},
					{-1 + coord_x, -1 + coord_y, -1 + coord_z},
				};

				for (auto& grid_coord : grid_coords)
				{
					grids[grid_coord].emplace_back(joint);
				}
			}

			std::unordered_map<PackedIntArray, JointCluster, PackedIntArrayHash> cluster_map;

			for (auto& joint : joint_list)
			{
				JointCluster new_cluster;
				new_cluster.person_id_per_view = joint.person_id_per_view;

				int32_t coord_x = std::floor(joint.position.x * inv_grid_size);
				int32_t coord_y = std::floor(joint.position.y * inv_grid_size);
				int32_t coord_z = std::floor(joint.position.z * inv_grid_size);

				Int3 grid_coord = {coord_x, coord_y, coord_z};

				for (auto& nearby_joint : grids[grid_coord])
				{
					if (conflicts(new_cluster.person_id_per_view, nearby_joint.person_id_per_view, n))
					{
						continue;
					}

					float distance = joint.position.distance(nearby_joint.position);

					if (distance >= params.max_joint_cluster_radius - nearby_joint.radius)
					{
						continue;
					}

					float decay = (1.0f - distance / params.max_joint_cluster_radius);
					new_cluster.score += nearby_joint.score * decay;
					new_cluster.members.emplace_back(nearby_joint);

					apply_union(new_cluster.person_id_per_view, nearby_joint.person_id_per_view);
				}

				auto& current_cluster = cluster_map[new_cluster.person_id_per_view];

				if (new_cluster.score < current_cluster.score)
				{
					continue;
				}

				static ink::Ray rays[16];
				static float weights[16];
				int ray_num = 0;

				for (int view_id = 0; view_id < view_num; ++view_id)
				{
					uint8_t person_id = as_array(new_cluster.person_id_per_view)[view_id];
					if (!person_id)
					{
						continue;
					}

					auto& view = multi_view->get_view(view_id);
					auto& joint = view.get_joint(joint_type, person_id);

					rays[ray_num] = view.get_ray(joint);
					weights[ray_num] = joint.conf;

					ray_num += 1;
				}

				triangulate(rays, weights, ray_num, new_cluster.position /*, new_cluster.radius */ );

				current_cluster = std::move(new_cluster);
			}
			
			auto& cluster_list = cluster_list_per_type[joint_type];
			cluster_list.reserve(cluster_map.size());

			for (auto& [person_id_per_view, cluster] : cluster_map)
			{
				cluster_list.emplace_back(std::move(cluster));
			}
		}
	}

	void associate_views()
	{
		const int association_graph_node_num = view_num * params.max_person_num;

		EdgeSet association_graph;

		for (int node_1 = 0; node_1 < association_graph_node_num; ++node_1)
		{
			for (int node_2 = 0; node_2 < association_graph_node_num; ++node_2)
			{
				association_graph.add_edge(node_1, node_2, 0);
			}
		}

		for (auto& [joint_type_1, joint_type_2] : ordered_bone_types)
		{
			auto& cluster_list_1 = cluster_list_per_type[joint_type_1];
			auto& cluster_list_2 = cluster_list_per_type[joint_type_2];

			for (int cluster_index_1 = 0; cluster_index_1 < cluster_list_1.size(); ++cluster_index_1)
			{
				for (int cluster_index_2 = 0; cluster_index_2 < cluster_list_2.size(); ++cluster_index_2)
				{
					auto& cluster_1 = cluster_list_1[cluster_index_1];
					auto& cluster_2 = cluster_list_2[cluster_index_2];

					float precomputed_score = 1.0f;

					float bone_length = cluster_1.position.distance(cluster_2.position);
					float bone_length_score = params.bone_lengths[joint_type_2]->evaluate(bone_length);
					if (params.use_bone_length_score)
					{
						precomputed_score *= bone_length_score;
					}

					if (bone_length_score <= 0.0f)
					{
						continue;
					}

					for (auto& joint_1 : cluster_1.members)
					{
						for (auto& joint_2 : cluster_2.members)
						{
							const PackedIntArray& person_id_per_view = joint_1.person_id_per_view;

							if (person_id_per_view != joint_2.person_id_per_view)
							{
								continue;
							}

							uint16_t view_id_1 = joint_1.view_id_1;
							uint16_t view_id_2 = joint_1.view_id_2;

							uint8_t person_id_1 = as_array(person_id_per_view)[view_id_1];
							uint8_t person_id_2 = as_array(person_id_per_view)[view_id_2];

							int node_1 = view_id_1 * params.max_person_num + person_id_1 - 1;
							int node_2 = view_id_2 * params.max_person_num + person_id_2 - 1;
							if (node_1 > node_2) std::swap(node_1, node_2);
							int edge_index = node_1 * association_graph_node_num + node_2;

							float score = params.use_edge_num_in_association ? 
								1.0f : std::sqrt(joint_1.score * joint_2.score) * precomputed_score;

							association_graph.get_edge(edge_index).weight += score;
						}
					}
				}
			}
		}

		association_graph.sort_by_weight(true);

		DisjointSet disjoint_set = DisjointSet(association_graph_node_num);

		for (int node = 0; node < association_graph_node_num; ++node)
		{
			int view_id = node / params.max_person_num;
			int person_id = node % params.max_person_num + 1;
			as_array(disjoint_set.get_person_id_per_view(node))[view_id] = person_id;
		}

		for (int index = 0; index < association_graph.get_edge_num(); ++index)
		{
			auto& edge = association_graph.get_edge(index);
			if (edge.weight < params.association_score_threshold)
			{
				break;
			}

			if (!disjoint_set.conflicts(edge.node_a, edge.node_b, n))
			{
				disjoint_set.merge(edge.node_a, edge.node_b, edge.weight);
			}
		}

		std::unordered_set<PackedIntArray, PackedIntArrayHash> connected_graph_map = {0};

		for (int index = 0; index < disjoint_set.get_size(); ++index)
		{
			int found_index = disjoint_set.find(index);
			auto& person_id_per_view = disjoint_set.get_person_id_per_view(found_index);

			if (connected_graph_map.contains(person_id_per_view))
			{
				continue;
			}
			connected_graph_map.emplace(person_id_per_view);

			float score = disjoint_set.get_weight(found_index);
			association_infos.emplace_back(person_id_per_view, score);
		}

		auto sort_association_infos = [](auto&& a, auto&& b) -> bool
		{
			return a.second > b.second;
		};
		std::sort(association_infos.begin(), association_infos.end(), sort_association_infos);
	}

	void build_pose_graphs()
	{
		for (int index = 0; index < association_infos.size() && index < params.max_person_num; ++index)
		{
			auto& person_id_per_view = association_infos[index].first;

			auto& pose_graph_per_bone = pose_graph_per_person.emplace_back();

			for (auto& [joint_type_1, joint_type_2] : ordered_bone_types)
			{
				auto& pose_graph = pose_graph_per_bone.emplace_back();

				auto& cluster_list_1 = cluster_list_per_type[joint_type_1];
				auto& cluster_list_2 = cluster_list_per_type[joint_type_2];

				for (int cluster_index_1 = 0; cluster_index_1 < cluster_list_1.size(); ++cluster_index_1)
				{
					for (int cluster_index_2 = 0; cluster_index_2 < cluster_list_2.size(); ++cluster_index_2)
					{
						auto& cluster_1 = cluster_list_1[cluster_index_1];
						auto& cluster_2 = cluster_list_2[cluster_index_2];

						float precomputed_score = 1.0f;

						float bone_length = cluster_1.position.distance(cluster_2.position);
						float bone_length_score = params.bone_lengths[joint_type_2]->evaluate(bone_length);
						if (params.use_bone_length_score)
						{
							precomputed_score *= bone_length_score;
						}

						if (bone_length_score <= 0.0f)
						{
							continue;
						}

						float score = 0.0f;

						for (auto& joint_1 : cluster_1.members)
						{
							if (contains(person_id_per_view, joint_1.person_id_per_view, n))
							{
								for (auto& joint_2 : cluster_2.members)
								{
									if (joint_1.person_id_per_view == joint_2.person_id_per_view)
									{
										score += std::sqrt(joint_1.score * joint_2.score);
									}
								}
							}
						}

						if (score > 0.0f)
						{
							pose_graph.add_edge(cluster_index_1, cluster_index_2, score * precomputed_score);
						}
					}
				}
			}
		}
	}

	void track_poses()
	{
		const float tracking_distance_2 = t_params.tracking_distance * 2.0f;

		for (int person_index = 0; person_index < pose_graph_per_person.size(); ++person_index)
		{
			auto& pose_graph_per_bone = pose_graph_per_person[person_index];

			std::vector<std::vector<std::tuple<ink::Vec3, ink::Vec3, float*>>> bone_lists;
			bone_lists.reserve(ordered_bone_types.size());

			for (int bone_index = 0; bone_index < ordered_bone_types.size(); ++bone_index)
			{
				auto& [joint_type_1, joint_type_2] = ordered_bone_types[bone_index];
				auto& bone_list = bone_lists.emplace_back();
				auto& pose_graph = pose_graph_per_bone[bone_index];

				auto& cluster_list_1 = cluster_list_per_type[joint_type_1];
				auto& cluster_list_2 = cluster_list_per_type[joint_type_2];

				for (int edge_index = 0; edge_index < pose_graph.get_edge_num(); ++edge_index)
				{
					auto& edge = pose_graph.get_edge(edge_index);

					auto& cluster_1 = cluster_list_1[edge.node_a];
					auto& cluster_2 = cluster_list_2[edge.node_b];

					bone_list.emplace_back(cluster_1.position, cluster_2.position, &edge.weight);
				}
			}

			const Pose3D* tracked_pose = nullptr;
			float tracked_pose_score = t_params.tracking_discard_threshold;

			for (auto& [pose_id, pose] : *t_params.history_multi_pose)
			{
				float tracking_score = 0.0f;

				for (int bone_index = 0; bone_index < ordered_bone_types.size(); ++bone_index)
				{
					auto& [joint_type_1, joint_type_2] = ordered_bone_types[bone_index];
					auto& bone_list = bone_lists[bone_index];

					if (!pose.has_joint(joint_type_1) || !pose.has_joint(joint_type_2))
					{
						continue;
					}

					const Joint3D& joint_1 = pose.get_joint(joint_type_1);
					const Joint3D& joint_2 = pose.get_joint(joint_type_2);

					for (auto& [position_1, position_2, bone_weight] : bone_list)
					{
						float distance = (joint_1.distance(position_1) + joint_2.distance(position_2));
						tracking_score += distance > tracking_distance_2 ? 0.0f : *bone_weight;
					}
				}

				if (tracking_score > tracked_pose_score)
				{
					tracked_pose = &pose;
					tracked_pose_score = tracking_score;
				}
			}

			if (tracked_pose != nullptr)
			{
				for (int bone_index = 0; bone_index < ordered_bone_types.size(); ++bone_index)
				{
					auto& [joint_type_1, joint_type_2] = ordered_bone_types[bone_index];
					auto& bone_list = bone_lists[bone_index];
					
					if (!tracked_pose->has_joint(joint_type_1) || !tracked_pose->has_joint(joint_type_2))
					{
						continue;
					}

					const Joint3D& joint_1 = tracked_pose->get_joint(joint_type_1);
					const Joint3D& joint_2 = tracked_pose->get_joint(joint_type_2);

					for (auto& [position_1, position_2, bone_weight] : bone_list)
					{
						float distance = (joint_1.distance(position_1) + joint_2.distance(position_2));
						*bone_weight *= distance > tracking_distance_2 ? t_params.lost_track_penalty : 1.0f;
					}
				}
			}
		}
	}

	void resolve_pose_graphs()
	{
		for (int person_index = 0; person_index < pose_graph_per_person.size(); ++person_index)
		{
			auto& pose_graph_per_bone = pose_graph_per_person[person_index];

			std::vector<std::vector<Branch>> branches(params.ordered_joint_types.size());

			for (auto& joint_type : params.ordered_joint_types)
			{
				auto& cluster_list = cluster_list_per_type[joint_type];

				branches[joint_type].resize(cluster_list.size());

				for (int cluster_index = 0; cluster_index < cluster_list.size(); ++cluster_index)
				{
					auto& branch = branches[joint_type][cluster_index];
					branch.clusters.resize(params.ordered_joint_types.size());
					branch.clusters[joint_type] = &cluster_list_per_type[joint_type][cluster_index];
					branch.bone_scores.resize(ordered_bone_types.size());
				}
			}
		
			for (int bone_index = 0; bone_index < ordered_bone_types.size(); ++bone_index)
			{
				auto& [joint_type_1, joint_type_2] = ordered_bone_types[bone_index];
				auto& pose_graph = pose_graph_per_bone[bone_index];

				for (int edge_index = 0; edge_index < pose_graph.get_edge_num(); ++edge_index)
				{
					auto& edge = pose_graph.get_edge(edge_index);

					auto& branch_1 = branches[joint_type_1][edge.node_a];
					auto& branch_2 = branches[joint_type_2][edge.node_b];

					float score = branch_1.last_score + branch_2.score + edge.weight;
					if (score > branch_1.score)
					{
						branch_1.score = score;
						branch_1.last_branch = &branch_2;
					}
				}

				auto& cluster_list_1 = cluster_list_per_type[joint_type_1];
				auto& cluster_list_2 = cluster_list_per_type[joint_type_2];

				for (int cluster_index = 0; cluster_index < cluster_list_1.size(); ++cluster_index)
				{
					auto& branch = branches[joint_type_1][cluster_index];

					if (branch.last_branch == nullptr)
					{
						auto& cluster = cluster_list_1[cluster_index];

						for (int cluster_index_2 = 0; cluster_index_2 < cluster_list_2.size(); ++cluster_index_2)
						{
							auto& fallback_cluster = cluster_list_2[cluster_index_2];

							float bone_length = cluster.position.distance(fallback_cluster.position);
							float bone_length_score = params.bone_lengths[joint_type_2]->evaluate(bone_length);
							if (bone_length_score <= 0.0f)
							{
								continue;
							}

							auto& fallback_branch = branches[joint_type_2][cluster_index_2];
							if (branch.last_branch == nullptr || fallback_branch.score > branch.last_branch->score)
							{
								branch.last_branch = &fallback_branch;
							}
						}

						if (branch.last_branch == nullptr)
						{
							continue;
						}
					}

					branch.bone_scores[bone_index] = branch.score - branch.last_score - branch.last_branch->score;
					branch.last_score = branch.score;

					for (auto& joint_type : params.ordered_joint_types)
					{
						if (branch.clusters[joint_type] == nullptr)
						{
							branch.clusters[joint_type] = branch.last_branch->clusters[joint_type];
						}
					}

					for (int bone_index = 0; bone_index < ordered_bone_types.size(); ++bone_index)
					{
						if (branch.bone_scores[bone_index] == 0.0f)
						{
							branch.bone_scores[bone_index] = branch.last_branch->bone_scores[bone_index];
						}
					}
				}
			}

			int final_joint_type = params.ordered_joint_types.back();

			auto final_branch = branches[final_joint_type].data();
			for (auto& branch : branches[final_joint_type])
			{
				if (branch.score > final_branch->score)
				{
					final_branch = &branch;
				}
			}

			if (final_branch == nullptr || final_branch->score < params.pose_discard_threshold)
			{
				continue;
			}

			Pose3D new_pose;

			for (auto& joint_type : params.ordered_joint_types)
			{
				if (final_branch->clusters[joint_type] != nullptr)
				{
					new_pose.add_joint(joint_type, final_branch->clusters[joint_type]->position);
				}
			}

			result.add_pose(person_index, std::move(new_pose));
		}
	}

protected:
	const MultiView* multi_view = nullptr;
	union { int view_num = 0; int n; };

	PoseBuilderParams params;
	PoseTrackingParams t_params;

	std::vector<std::pair<int, int>> ordered_bone_types;
};

}
