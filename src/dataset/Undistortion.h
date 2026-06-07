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

#include "opencv2/opencv.hpp"

namespace v3::dataset
{

inline void undistort(std::vector<MultiView>& multi_view_vec, int type_num,
	std::vector<std::vector<float>> dist_coeffs)
{
	int view_num = multi_view_vec.empty() ? 0 : multi_view_vec.front().get_view_num();

	std::vector<cv::Mat_<float>> dist_coeffs_cv(view_num);
	for (int view_index = 0; view_index < view_num; ++view_index)
	{
		dist_coeffs_cv[view_index] = cv::Mat_<float>(5, 1, dist_coeffs[view_index].data());
	}

	for (MultiView& multi_view : multi_view_vec)
	{
		int current_view_num = multi_view.get_view_num();
		for (int view_index = 0; view_index < current_view_num; ++view_index)
		{
			View& view = multi_view.get_view(view_index);

			std::vector<cv::Point2f> distorted_points;
			std::vector<cv::Point2f> undistorted_points;

			for (int type = 0; type < type_num; ++type)
			{
				int joint_num = view.get_joint_num(type);
				for (int index = 1; index <= joint_num; ++index)
				{
					const Joint2D& joint = view.get_joint(type, index);
					distorted_points.emplace_back(joint.point.x, joint.point.y);
				}
			}

			if (distorted_points.empty()) continue;

			ink::Mat3& matrix_k = view.get_camera()->K;
			cv::Mat_<float> matrix_k_cv = cv::Mat_<float>(3, 3, &matrix_k[0][0]);

			cv::undistortPoints(distorted_points, undistorted_points, matrix_k_cv,
				dist_coeffs_cv[view_index], cv::noArray(), matrix_k_cv);

			int point_index = 0;
			for (int type = 0; type < type_num; ++type)
			{
				int joint_num = view.get_joint_num(type);
				for (int index = 1; index <= joint_num; ++index)
				{
					Joint2D& joint = view.get_joint(type, index);
					joint.point.x = undistorted_points[point_index].x;
					joint.point.y = undistorted_points[point_index].y;
					++point_index;
				}
			}
		}
	}
}

}
