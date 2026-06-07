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

#include "algorithm/Common.h"

namespace v3::dataset
{

inline unsigned int get_max_joint_uid(const MultiView& multi_view, int type_num)
{
	unsigned int max_joint_uid = 0;
	int view_num = multi_view.get_view_num();
	for (int view_index = 0; view_index < view_num; ++view_index)
	{
		const View& view = multi_view.get_view(view_index);
		for (int type = 0; type < type_num; ++type)
		{
			int joint_num = view.get_joint_num(type);
			for (int index = 1; index <= joint_num; ++index)
			{
				max_joint_uid = std::max(max_joint_uid, view.get_joint(type, index).uid);
			}
		}
	}
	return max_joint_uid;
}

// source[type] -> destination[type_map[type]]
inline MultiView convert(const MultiView& source, int type_num, const int* type_map,
	int bone_num = 0, const std::pair<int, int>* bones = nullptr)
{
	MultiView destination;

	int view_num = source.get_view_num();
	for (int view_index = 0; view_index < view_num; ++view_index)
	{
		const View& source_view = source.get_view(view_index);
		View destination_view(source_view.get_camera());

		for (int type = 0; type < type_num; ++type)
		{
			if (type_map[type] < 0) continue;
			int joint_num = source_view.get_joint_num(type);
			for (int index = 1; index <= joint_num; ++index)
			{
				destination_view.add_joint(type_map[type], source_view.get_joint(type, index));
			}
		}

		for (int bone_index = 0; bone_index < bone_num; ++bone_index)
		{
			auto& [type_a, type_b] = bones[bone_index];
			if (type_map[type_a] < 0 || type_map[type_b] < 0) continue;
			int joint_num_a = source_view.get_joint_num(type_a);
			int joint_num_b = source_view.get_joint_num(type_b);
			for (int candidate_a = 1; candidate_a <= joint_num_a; ++candidate_a)
			{
				for (int candidate_b = 1; candidate_b <= joint_num_b; ++candidate_b)
				{
					const Joint2D& joint_a = source_view.get_joint(type_a, candidate_a);
					const Joint2D& joint_b = source_view.get_joint(type_b, candidate_b);
					destination_view.set_bone_weight(joint_a, joint_b,
						source_view.get_bone_weight(joint_a, joint_b));
				}
			}
		}

		destination.add_view(std::move(destination_view));
	}

	return destination;
}

// Default output of CID (bone data is not included)
// https://github.com/kennethwdk/CID
inline MultiView convert_coco17_to_body25(const MultiView& source)
{
	constexpr int TYPE_NUM = 17;
	constexpr int TYPE_MAP[] = {
		0, 16, 15, 18, 17, 5, 2, 6, 3, 7, 4, 12, 9, 13, 10, 14, 11,
	};

	MultiView destination = convert(source, TYPE_NUM, TYPE_MAP);

	unsigned int next_joint_uid = get_max_joint_uid(source, TYPE_NUM) + 1;

	int view_num = destination.get_view_num();
	for (int view_index = 0; view_index < view_num; ++view_index)
	{
		View& destination_view = destination.get_view(view_index);
		int joint_num = destination_view.get_joint_num(0);

		for (int index = 1; index <= joint_num; ++index)
		{
			const Joint2D& right_shoulder = destination_view.get_joint(2, index);
			const Joint2D& left_shoulder = destination_view.get_joint(5, index);
			Joint2D neck;
			neck.uid = next_joint_uid++;
			neck.point = (right_shoulder.point + left_shoulder.point) * 0.5;
			neck.conf = (right_shoulder.conf + left_shoulder.conf) * 0.5;
			destination_view.add_joint(1, std::move(neck));
		}

		for (int index = 1; index <= joint_num; ++index)
		{
			const Joint2D& right_hip = destination_view.get_joint(9, index);
			const Joint2D& left_hip = destination_view.get_joint(12, index);
			Joint2D mid_hip;
			mid_hip.uid = next_joint_uid++;
			mid_hip.point = (right_hip.point + left_hip.point) * 0.5;
			mid_hip.conf = (right_hip.conf + left_hip.conf) * 0.5;
			destination_view.add_joint(8, std::move(mid_hip));
		}
	}

	return destination;
}

inline std::vector<MultiView> convert_coco17_to_body25(const std::vector<MultiView>& source)
{
	std::vector<MultiView> destination;
	for (const MultiView& multi_view : source)
	{
		destination.emplace_back(convert_coco17_to_body25(multi_view));
	}
	return destination;
}

// source[type] -> destination[type_map[type]]
inline MultiPose3D convert(const MultiPose3D& source, int type_num, const int* type_map)
{
	MultiPose3D destination;

	for (const auto& [pose_id, source_pose] : source)
	{
		Pose3D destination_pose;
		for (int type = 0; type < type_num; ++type)
		{
			if (type_map[type] < 0 || !source_pose.has_joint(type)) continue;
			destination_pose.add_joint(type_map[type], source_pose.get_joint(type));
		}
		destination.add_pose(pose_id, std::move(destination_pose));
	}

	return destination;
}

inline MultiPose3D convert_coco17_to_body25(const MultiPose3D& source)
{
	constexpr int TYPE_NUM = 17;
	constexpr int TYPE_MAP[] = {
		0, 16, 15, 18, 17, 5, 2, 6, 3, 7, 4, 12, 9, 13, 10, 14, 11,
	};

	MultiPose3D destination = convert(source, TYPE_NUM, TYPE_MAP);

	for (auto& [pose_id, pose] : destination)
	{
		if (pose.has_joint(2) && pose.has_joint(5))
		{
			pose.add_joint(1, (pose.get_joint(2) + pose.get_joint(5)) * 0.5);
		}
		if (pose.has_joint(9) && pose.has_joint(12))
		{
			pose.add_joint(8, (pose.get_joint(9) + pose.get_joint(12)) * 0.5);
		}
	}

	return destination;
}

inline std::vector<MultiPose3D> convert_coco17_to_body25(const std::vector<MultiPose3D>& source)
{
	std::vector<MultiPose3D> destination;
	for (const MultiPose3D& multi_pose : source)
	{
		destination.emplace_back(convert_coco17_to_body25(multi_pose));
	}
	return destination;
}

// Openpose COCO 18
// https://github.com/CMU-Perceptual-Computing-Lab/openpose/blob/master/doc/02_output.md
inline MultiPose3D convert_coco18_to_body25(const MultiPose3D& source)
{
	constexpr int TYPE_NUM = 18;
	constexpr int TYPE_MAP[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
	};

	MultiPose3D destination = convert(source, TYPE_NUM, TYPE_MAP);

	for (auto& [pose_id, pose] : destination)
	{
		if (pose.has_joint(9) && pose.has_joint(12))
		{
			pose.add_joint(8, (pose.get_joint(9) + pose.get_joint(12)) * 0.5);
		}
	}

	return destination;
}

inline std::vector<MultiPose3D> convert_coco18_to_body25(const std::vector<MultiPose3D>& source)
{
	std::vector<MultiPose3D> destination;
	for (const MultiPose3D& multi_pose : source)
	{
		destination.emplace_back(convert_coco18_to_body25(multi_pose));
	}
	return destination;
}

// CMU Panoptic dataset
// http://domedb.perception.cs.cmu.edu/
inline MultiPose3D convert_coco19_to_body25(const MultiPose3D& source)
{
	// https://github.com/CMU-Perceptual-Computing-Lab/panoptic-toolbox/issues/16
	constexpr int TYPE_NUM = 19;
	constexpr int TYPE_MAP[] = {
		1, 0, 8, 5, 6, 7, 12, 13, 14, 2, 3, 4, 9, 10, 11, 15, 16, 17, 18,
	};

	return convert(source, TYPE_NUM, TYPE_MAP);
}

inline std::vector<MultiPose3D> convert_coco19_to_body25(const std::vector<MultiPose3D>& source)
{
	std::vector<MultiPose3D> destination;
	for (const MultiPose3D& multi_pose : source)
	{
		destination.emplace_back(convert_coco19_to_body25(multi_pose));
	}
	return destination;
}

// Shelf / Campus dataset
// https://campar.in.tum.de/Chair/MultiHumanPose
inline MultiPose3D convert_shelf14_to_body25(const MultiPose3D& source)
{
	constexpr int TYPE_NUM = 14;
	constexpr int TYPE_MAP[] = {
		11, 10, 9, 12, 13, 14, 4, 3, 2, 5, 6, 7, 1, 0,
	};

	MultiPose3D destination = convert(source, TYPE_NUM, TYPE_MAP);

	for (auto& [pose_id, pose] : destination)
	{
		if (pose.has_joint(2) && pose.has_joint(5))
		{
			pose.add_joint(1, (pose.get_joint(2) + pose.get_joint(5)) * 0.5);
		}
		if (pose.has_joint(9) && pose.has_joint(12))
		{
			pose.add_joint(8, (pose.get_joint(9) + pose.get_joint(12)) * 0.5);
		}
	}

	return destination;
}

inline std::vector<MultiPose3D> convert_shelf14_to_body25(const std::vector<MultiPose3D>& source)
{
	std::vector<MultiPose3D> destination;
	for (const MultiPose3D& multi_pose : source)
	{
		destination.emplace_back(convert_shelf14_to_body25(multi_pose));
	}
	return destination;
}

inline MultiPose3D convert_optitrack21_to_body25(const MultiPose3D& source)
{
	constexpr int TYPE_NUM = 21;
	constexpr int TYPE_MAP[] = {
		8, -1, -1, -1, 0, -1, 5, 6, 7, -1, 2, 3, 4, 12, 13, 14, 9, 10, 11, 19, 22,
	};

	MultiPose3D destination = convert(source, TYPE_NUM, TYPE_MAP);

	for (auto& [pose_id, pose] : destination)
	{
		if (pose.has_joint(2) && pose.has_joint(5))
		{
			pose.add_joint(1, (pose.get_joint(2) + pose.get_joint(5)) * 0.5);
		}
	}

	return destination;
}

inline std::vector<MultiPose3D> convert_optitrack21_to_body25(const std::vector<MultiPose3D>& source)
{
	std::vector<MultiPose3D> destination;
	for (const MultiPose3D& multi_pose : source)
	{
		destination.emplace_back(convert_optitrack21_to_body25(multi_pose));
	}
	return destination;
}

// Default output of 4D Association
// https://github.com/zhangyux15/4d_association
inline MultiPose3D convert_skel19_to_body25(const MultiPose3D& source)
{
	constexpr int TYPE_NUM = 19;
	constexpr int TYPE_MAP[] = {
		8, 1, 9, 12, 0, 2, 5, 10, 13, 17, 18, 3, 6, 11, 14, 4, 7, 19, 22,
	};

	MultiPose3D destination = convert(source, TYPE_NUM, TYPE_MAP);

	for (auto& [pose_id, pose] : destination)
	{
		if (pose.has_joint(2) && pose.has_joint(5))
		{
			pose.add_joint(1, (pose.get_joint(2) + pose.get_joint(5)) * 0.5);
		}
	}

	return destination;
}

inline std::vector<MultiPose3D> convert_skel19_to_body25(const std::vector<MultiPose3D>& source)
{
	std::vector<MultiPose3D> destination;
	for (const MultiPose3D& multi_pose : source)
	{
		destination.emplace_back(convert_skel19_to_body25(multi_pose));
	}
	return destination;
}

}
