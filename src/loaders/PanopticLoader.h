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

class PanopticLoader
{
public:
	std::pair<int, int> frame_range = {8500, 10500};

	bool load_multi_poses(const std::string& path, std::vector<MultiPose3D>& multi_poses) const
	{
		multi_poses.resize(frame_range.second - frame_range.first + 1);

		for (int frame_index = frame_range.first; frame_index <= frame_range.second; ++frame_index)
		{
			std::string file_path = path + std::format("/body3DScene_{:08d}.json", frame_index);

			std::ifstream stream(file_path, std::fstream::in);
			if (!stream.is_open())
			{
				continue;
			}

			nlohmann::json multi_pose_json;
			stream >> multi_pose_json;

			for (auto& body : multi_pose_json.at("bodies"))
			{
				int person_id = body.at("id");
				auto& joints19 = body.at("joints19");

				Pose3D new_pose;
				for (int index = 0; index < joints19.size(); index += 4)
				{
					new_pose.add_joint(index / 4, {joints19[index], joints19[index + 1], joints19[index + 2]});
				}

				multi_poses[frame_index - frame_range.first].add_pose(person_id, std::move(new_pose));
			}

			stream.close();
		}

		return true;
	}
};

}
