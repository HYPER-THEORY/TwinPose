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
#include "opencv2/opencv.hpp"

#include <format>
#include <fstream>

namespace v3
{

inline const std::vector<std::pair<int, int>> _4da_default_bone_types = {
	{ 1,  8},
	{ 9, 10},
	{10, 11},
	{ 8,  9},
	{ 8, 12},
	{12, 13},
	{13, 14},
	{ 1,  2},
	{ 2,  3},
	{ 3,  4},
	{ 2, 17},
	{ 1,  5},
	{ 5,  6},
	{ 6,  7},
	{ 5, 18},
	{ 1,  0},
	{ 0, 15},
	{ 0, 16},
	{15, 17},
	{16, 18},
	{14, 19},
	{19, 20},
	{14, 21},
	{11, 22},
	{22, 23},
	{11, 24},
};

class _4DALoader
{
public:
	ink::Vec2 screen_size = {1032, 776};
	int joint_type_num = 25;
	int view_num = 5;
	float conf_exponent = 1.0f;
	std::vector<std::pair<int, int>> bone_types = _4da_default_bone_types;

	bool load_multi_view(const std::string& path, std::vector<MultiView>& multi_views) const
	{
		for (int view_index = 0; view_index < view_num; ++view_index)
		{
			std::string file_path = std::format("{}/{}.txt", path, view_index);
			std::ifstream stream(file_path, std::fstream::in);

			if (stream.fail())
			{
				return false;
			}

			int frame_num = 0;
			stream >> frame_num;
			while (frame_num < 10)
			{
				stream >> frame_num; // ignore skeleton type
			}

			if (multi_views.size() != frame_num)
			{
				multi_views.resize(frame_num);
			}

			unsigned int next_joint_uid = 0;

			for (int frame_index = 0; frame_index < frame_num; ++frame_index)
			{
				multi_views[frame_index].add_view(View());
				View& current_view = multi_views[frame_index].get_view(view_index);

				for (int joint_type = 0; joint_type < joint_type_num; ++joint_type)
				{
					int candidate_num = 0;
					stream >> candidate_num;

					std::vector<Joint2D> candidates(candidate_num);

					// https://github.com/zhangyux15/4d_association/blob/windows/src/main.cpp#L32
					for (int swizzle = 0; swizzle < 3; ++swizzle)
					{
						for (int joint_index = 0; joint_index < candidate_num; ++joint_index)
						{
							float value = 0;
							stream >> value;
							if (swizzle == 0)
							{
								candidates[joint_index].point.x = value * (screen_size.x - 1.f);
							}
							else if (swizzle == 1)
							{
								candidates[joint_index].point.y = value * (screen_size.y - 1.f);
							}
							else
							{
								candidates[joint_index].conf = value;
								candidates[joint_index].uid = next_joint_uid++;
							}
						}
					}

					for (auto& candidate : candidates)
					{
						current_view.add_joint(joint_type, std::move(candidate));
					}
				}

				for (auto& [type_a, type_b] : bone_types)
				{
					int num_a = current_view.get_joint_num(type_a);
					int num_b = current_view.get_joint_num(type_b);

					for (int index_a = 0; index_a < num_a; ++index_a)
					{
						for (int index_b = 0; index_b < num_b; ++index_b)
						{
							const Joint2D& joint_a = current_view.get_joint(type_a, index_a);
							const Joint2D& joint_b = current_view.get_joint(type_b, index_b);
							float paf = 0;
							stream >> paf;
							current_view.set_bone_weight(joint_a, joint_b, std::pow(paf, conf_exponent));
						}
					}
				}
			}

			stream.close();
		}

		return true;
	}

	bool load_cameras(const std::string& path, std::vector<MultiView>& multi_views) const
	{
		std::ifstream stream(path, std::fstream::in);

		if (stream.fail())
		{
			return false;
		}

		nlohmann::json cameras_json;
		stream >> cameras_json;

		int view_index = 0;
		for (auto& [name, camera_json] : cameras_json.items())
		{
			ink::Mat3 matrix_k;
			ink::Mat3 matrix_r;
			ink::Vec3 vector_t;

			if (camera_json.contains("K"))
			{
				auto& matrix = camera_json.at("K");
				for (int row = 0; row < 3; ++row)
				{
					for (int col = 0; col < 3; ++col)
					{
						matrix_k[row][col] = matrix[row * 3 + col];
					}
				}
			}

			if (camera_json.contains("R"))
			{
				auto& matrix = camera_json.at("R");
				if (matrix.size() == 3)
				{
					cv::Vec3f vector_r;
					cv::Mat cv_matrix_r;
					for (int index = 0; index < 3; ++index)
					{
						vector_r[index] = matrix[index];
					}
					cv::Rodrigues(vector_r, cv_matrix_r);
					std::copy(cv_matrix_r.begin<float>(), cv_matrix_r.end<float>(), matrix_r[0]);
				}
				else if (matrix.size() == 9)
				{
					for (int row = 0; row < 3; ++row)
					{
						for (int col = 0; col < 3; ++col)
						{
							matrix_r[row][col] = matrix[row * 3 + col];
						}
					}
				}
			}

			if (camera_json.contains("T"))
			{
				auto& vector = camera_json.at("T");
				vector_t = {vector[0], vector[1], vector[2]};
			}

			if (camera_json.contains("RT"))
			{
				auto& matrix = camera_json.at("RT");
				for (int row = 0; row < 3; ++row)
				{
					for (int col = 0; col < 3; ++col)
					{
						matrix_r[row][col] = matrix[row * 4 + col];
					}
				}
				vector_t = {matrix[3], matrix[7], matrix[11]};
			}

			cv::Size_<float> img_size;
			if (camera_json.contains("imgSize"))
			{
				auto& vector = camera_json.at("imgSize");
				img_size = cv::Size_<float>(vector[0], vector[1]);
			}

			cv::Mat_<float> dist_coeffs = cv::Mat_<float>::zeros(5, 1);
			if (camera_json.contains("distCoeff"))
			{
				auto& vector = camera_json.at("distCoeff");
				for (int index = 0; index < vector.size(); ++index)
				{
					dist_coeffs(index) = vector[index];
				}
			}

			float rectify_alpha = 0;
			if (camera_json.contains("rectifyAlpha"))
			{
				rectify_alpha = camera_json.at("rectifyAlpha");
			}

			cv::Mat cv_matrix_k = cv::Mat(3, 3, CV_32F, matrix_k[0]);
			cv_matrix_k = cv::getOptimalNewCameraMatrix(cv_matrix_k, dist_coeffs, img_size, rectify_alpha);
			std::copy(cv_matrix_k.begin<float>(), cv_matrix_k.end<float>(), matrix_k[0]);

			Camera* new_camera = new Camera(matrix_k, matrix_r, vector_t);

			for (auto& multi_view : multi_views)
			{
				multi_view.get_view(view_index).set_camera(new_camera);
				multi_view.get_view(view_index).precompute_rays();
			}

			++view_index;
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

		int joint_type_num = 21;
		int frame_num = 0;
		stream >> frame_num;

		multi_poses.resize(frame_num);

		for (int frame_index = 0; frame_index < frame_num; ++frame_index)
		{
			int person_num = 0;
			stream >> person_num;

			for (int person_index = 0; person_index < person_num; ++person_index)
			{
				Pose3D new_pose;

				for (int joint_type = 0; joint_type < joint_type_num; ++joint_type)
				{
					new_pose.add_joint(joint_type, {});
				}

				for (int swizzle = 0; swizzle < 4; ++swizzle)
				{
					for (int joint_type = 0; joint_type < joint_type_num; ++joint_type)
					{
						if (swizzle < 3)
						{
							Joint3D& position = new_pose.get_joint(joint_type);
							stream >> (&position.x)[swizzle];
						}
						else
						{
							float valid = 0;
							stream >> valid;
							if (valid < 1E-4)
							{
								new_pose.remove_joint(joint_type);
							}
						}
					}
				}

				multi_poses[frame_index].add_pose(person_index, std::move(new_pose));
			}
		}

		stream.close();

		return true;
	}

	bool load_multi_poses_2(const std::string& path, std::vector<MultiPose3D>& multi_poses) const
	{
		std::ifstream stream(path, std::fstream::in);

		if (stream.fail())
		{
			return false;
		}

		int joint_type_num = 0;
		int frame_num = 0;
		stream >> joint_type_num >> frame_num;

		multi_poses.resize(frame_num);

		for (int frame_index = 0; frame_index < frame_num; ++frame_index)
		{
			int person_num = 0;
			stream >> person_num;

			for (int person_index = 0; person_index < person_num; ++person_index)
			{
				int person_id = 0;
				stream >> person_id;

				Pose3D new_pose;

				for (int joint_type = 0; joint_type < joint_type_num; ++joint_type)
				{
					new_pose.add_joint(joint_type, {});
				}

				for (int swizzle = 0; swizzle < 4; ++swizzle)
				{
					for (int joint_type = 0; joint_type < joint_type_num; ++joint_type)
					{
						if (swizzle < 3)
						{
							Joint3D& position = new_pose.get_joint(joint_type);
							stream >> (&position.x)[swizzle];
						}
						else
						{
							float valid = 0;
							stream >> valid;
							if (valid < 1E-4)
							{
								new_pose.remove_joint(joint_type);
							}
						}
					}
				}

				multi_poses[frame_index].add_pose(person_id, std::move(new_pose));
			}
		}

		stream.close();

		return true;
	}
};

}
