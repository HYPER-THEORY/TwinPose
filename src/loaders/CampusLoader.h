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

#include <fstream>
#include <limits>

namespace v3
{

class CampusLoader
{
public:
	int joint_type_num = 14;
	int view_num = 3;
	int max_person_num = 3;

	bool load_cameras(const std::string& path, std::vector<MultiView>& multi_views) const
	{
		std::ifstream stream(path, std::fstream::in);

		if (stream.fail())
		{
			return false;
		}

		for (int view_index = 0; view_index < view_num; ++view_index)
		{
			ink::Mat3 matrix_k;
			ink::Mat3 matrix_r = ink::Mat3::identity();
			ink::Vec3 vector_t;

			for (int row = 0; row < 3; ++row)
			{
				for (int col = 0; col < 4; ++col)
				{
					if (col == 3)
					{
						stream >> (&vector_t.x)[row];
					}
					else
					{
						stream >> matrix_k[row][col];
					}
				}
			}

			vector_t = ink::inverse_3x3(matrix_k) * vector_t;

			Camera* new_camera = new Camera(matrix_k, matrix_r, vector_t);

			for (auto& multi_view : multi_views)
			{
				multi_view.get_view(view_index).set_camera(new_camera);
				multi_view.get_view(view_index).precompute_rays();
			}
		}

		stream.close();

		return true;
	}

	bool load_multi_poses(const std::string& path, std::vector<MultiPose3D>& multi_poses) const
	{
		std::ifstream stream(path, std::fstream::in);

		if (stream.fail())
		{
			return false;
		}

		MultiPose3D* current_multi_pose = nullptr;

		multi_poses.clear();

		while (!stream.eof())
		{
			std::string keyword;
			stream >> keyword;

			if (keyword == "frame")
			{
				int frame_index = 0;
				stream >> frame_index;
				current_multi_pose = &multi_poses.emplace_back();
			}
			else if (keyword == "p")
			{
				int pose_id = 0;
				stream >> pose_id;

				if (current_multi_pose == nullptr)
				{
					stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
					continue;
				}

				Pose3D new_pose;

				for (int joint_type = 0; joint_type < joint_type_num; ++joint_type)
				{
					Joint3D position;
					stream >> position.x >> position.y >> position.z;
					new_pose.add_joint(joint_type, std::move(position));
				}

				if (pose_id - 1 < max_person_num)
				{
					current_multi_pose->add_pose(pose_id - 1, std::move(new_pose));
				}
			}
			else
			{
				stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			}
		}

		stream.close();

		return true;
	}
};

}
