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

namespace v3
{

class Evaluation
{
public:
	float pcp_alpha = 0.5f;
	float pck_distance = 0.2f;
	float min_distance = 0.5f;

	Evaluation(std::vector<int> joint_types, std::vector<std::pair<int, int>> bone_types) :
		joint_types(joint_types), bone_types(bone_types) {}

	float pcp(const Pose3D& ground_truth, const MultiPose3D& multi_pose, std::vector<bool>& results,
		float& ground_truth_completeness) const
	{
		const int bone_type_num = bone_types.size();
		results.assign(bone_type_num, false);

		const Pose3D* closest_pose = find_closest_pose(ground_truth, multi_pose);
		if (closest_pose == nullptr)
		{
			return 0.0f;
		}

		float score = 0.0f;
		int missing_ground_truth = 0;

		for (int bone_type_index = 0; bone_type_index < bone_type_num; ++bone_type_index)
		{
			auto& [joint_type_1, joint_type_2] = bone_types[bone_type_index];

			if (!closest_pose->has_joint(joint_type_1) || !closest_pose->has_joint(joint_type_2))
			{
				continue;
			}

			if (!ground_truth.has_joint(joint_type_1) || !ground_truth.has_joint(joint_type_2))
			{
				++missing_ground_truth;
				continue;
			}

			auto& gt_joint_1 = ground_truth.get_joint(joint_type_1);
			auto& gt_joint_2 = ground_truth.get_joint(joint_type_2);

			auto& joint_1 = closest_pose->get_joint(joint_type_1);
			auto& joint_2 = closest_pose->get_joint(joint_type_2);

			results[bone_type_index] = gt_joint_1.distance(gt_joint_2) * pcp_alpha >
				(joint_1.distance(gt_joint_1) + joint_2.distance(gt_joint_2)) * 0.5f;

			score += results[bone_type_index];
		}

		ground_truth_completeness = float(bone_type_num - missing_ground_truth) / bone_type_num;
		return score / bone_type_num;
	}
	
	float pck(const Pose3D& ground_truth, const MultiPose3D& multi_pose, std::vector<bool>& results,
		float& ground_truth_completeness) const
	{
		const int joint_type_num = joint_types.size();
		results.assign(joint_types.size(), false);

		const Pose3D* closest_pose = find_closest_pose(ground_truth, multi_pose);
		if (closest_pose == nullptr)
		{
			return 0.0f;
		}

		float score = 0.0f;
		int missing_ground_truth = 0;

		for (int joint_type_index = 0; joint_type_index < joint_type_num; ++joint_type_index)
		{
			int joint_type = joint_types[joint_type_index];

			if (!closest_pose->has_joint(joint_type))
			{
				continue;
			}

			if (!ground_truth.has_joint(joint_type))
			{
				++missing_ground_truth;
				continue;
			}

			results[joint_type_index] = pck_distance >
				closest_pose->get_joint(joint_type).distance(ground_truth.get_joint(joint_type));

			score += results[joint_type_index];
		}

		ground_truth_completeness = float(joint_type_num - missing_ground_truth) / joint_type_num;
		return score / joint_type_num;
	}

	float mpjpe(const Pose3D& ground_truth, const MultiPose3D& multi_pose, std::vector<float>& results,
		float& ground_truth_completeness) const
	{
		const int joint_type_num = joint_types.size();
		results.assign(joint_type_num, 0.0f);

		const Pose3D* closest_pose = find_closest_pose(ground_truth, multi_pose);
		if (closest_pose == nullptr)
		{
			return 0.0f;
		}

		float score = 0.0f;
		int missing_ground_truth = 0;

		for (int joint_type_index = 0; joint_type_index < joint_type_num; ++joint_type_index)
		{
			int joint_type = joint_types[joint_type_index];

			if (!closest_pose->has_joint(joint_type))
			{
				continue;
			}

			if (!ground_truth.has_joint(joint_type))
			{
				++missing_ground_truth;
				continue;
			}

			results[joint_type_index] = closest_pose->get_joint(joint_type).distance(ground_truth.get_joint(joint_type));

			score += results[joint_type_index];
		}
	
		ground_truth_completeness = float(joint_type_num - missing_ground_truth) / joint_type_num;
		return score / joint_type_num * 1000.0f;
	}

	const Pose3D* find_closest_pose(const Pose3D& target, const MultiPose3D& multi_pose) const
	{
		const Pose3D* closest_pose = nullptr;
		float best_total_distance = FLT_MAX;

		for (auto& [pose_id, pose] : multi_pose)
		{
			float total_distance = 0.0f;

			for (auto& joint_type : joint_types)
			{
				if (!target.has_joint(joint_type) || !pose.has_joint(joint_type))
				{
					total_distance += min_distance;
					continue;
				}

				float distance = target.get_joint(joint_type).distance(pose.get_joint(joint_type));
				total_distance += std::min(distance, min_distance);
			}

			if (total_distance < best_total_distance)
			{
				closest_pose = &pose;
				best_total_distance = total_distance;
			}
		}

		return closest_pose;
	}

private:
	std::vector<int> joint_types;
	std::vector<std::pair<int, int>> bone_types;
};

}
