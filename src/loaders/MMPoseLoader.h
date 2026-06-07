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

#include "nlohmann/json.hpp"

#include <format>
#include <fstream>

namespace v3
{

class MMPoseLoader
{
public:
	int joint_type_num = 17;
	int view_num = 5;
	int max_person_num = 4;
	std::pair<int, int> frame_range = {300, 600};

	bool load_multi_views(const std::string& path, std::vector<MultiView>& multi_views) const
	{
		unsigned int next_joint_uid = 0;

		for (int view_index = 0; view_index < view_num; ++view_index)
		{
			std::string file_path = path + std::format("/{}.json", view_index);
			std::ifstream stream(file_path, std::fstream::in);

			if (stream.fail())
			{
				return false;
			}

			nlohmann::json view_json;
			stream >> view_json;

			int frame_index = 0;

			for (auto& instance_info : view_json.at("instance_info"))
			{
				int frame_id = instance_info.at("frame_id");
				if (frame_id < frame_range.first || frame_id > frame_range.second)
				{
					continue;
				}

				if (multi_views.size() == frame_index)
				{
					multi_views.emplace_back();
				}
				auto& multi_view = multi_views[frame_index];

				int person_num = std::min<int>(instance_info.at("instances").size(), max_person_num);

				View new_view;

				for (int person_index = 0; person_index < person_num; ++person_index)
				{
					auto& instance = instance_info.at("instances")[person_index];

					for (int joint_type = 0; joint_type < joint_type_num; ++joint_type)
					{
						Joint2D new_joint;
						new_joint.point.x = instance.at("keypoints")[joint_type][0];
						new_joint.point.y = instance.at("keypoints")[joint_type][1];
						new_joint.conf = instance.at("keypoint_scores")[joint_type];
						new_joint.uid = next_joint_uid++;

						new_view.add_joint(joint_type, std::move(new_joint));
					}
				}

				multi_view.add_view(std::move(new_view));

				++frame_index;
			}

			stream.close();
		}

		return true;
	}
};

}
