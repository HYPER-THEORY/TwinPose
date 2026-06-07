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

class OSDLoader
{
public:
	int joint_type_num = 17;
	int view_num = 5;
	float score_threshold = 0.01f;

	bool load_multi_view(const std::string& path, std::vector<MultiView>& multi_views) const
	{
		std::ifstream stream(path, std::fstream::in);

		if (stream.fail())
		{
			return false;
		}

		MultiView* current_multi_view = nullptr;
		View* current_view = nullptr;

		unsigned int next_joint_uid = 0;

		while (!stream.eof())
		{
			std::string keyword;
			stream >> keyword;

			if (keyword == "frame")
			{
				int frame_index = 0;
				stream >> frame_index;
				current_multi_view = &multi_views.emplace_back();
				current_view = nullptr;
			}
			else if (keyword == "view")
			{
				int view_index = 0;
				stream >> view_index;

				if (current_multi_view == nullptr)
				{
					continue;
				}

				current_multi_view->add_view(View());
				current_view = &current_multi_view->get_view(current_multi_view->get_view_num() - 1);
			}
			else if (keyword == "p")
			{
				float score = 0;
				stream >> score;

				if (score < score_threshold || current_view == nullptr)
				{
					stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
					continue;
				}

				for (int joint_type = 0; joint_type < joint_type_num; ++joint_type)
				{
					Joint2D new_joint;
					new_joint.uid = next_joint_uid++;
					stream >> new_joint.point.x >> new_joint.point.y >> new_joint.conf;
					current_view->add_joint(joint_type, std::move(new_joint));
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
			for (int index = 0; index < 9; ++index)
			{
				stream >> matrix_k[0][index];
			}

			ink::Mat3 matrix_r;
			for (int index = 0; index < 9; ++index)
			{
				stream >> matrix_r[0][index];
			}

			ink::Vec3 vector_t;
			for (int index = 0; index < 3; ++index)
			{
				stream >> (&vector_t.x)[index];
			}

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
};

}
