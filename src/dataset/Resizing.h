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

// scale = target resolution / original resolution
inline void resize_2d(std::vector<MultiView>& multi_view_vec, int type_num, const ink::Vec2& scale)
{
	for (MultiView& multi_view : multi_view_vec)
	{
		int view_num = multi_view.get_view_num();
		for (int view_index = 0; view_index < view_num; ++view_index)
		{
			View& view = multi_view.get_view(view_index);
			for (int type = 0; type < type_num; ++type)
			{
				int joint_num = view.get_joint_num(type);
				for (int index = 1; index <= joint_num; ++index)
				{
					Joint2D& joint = view.get_joint(type, index);
					joint.point.x *= scale.x;
					joint.point.y *= scale.y;
				}
			}
		}
	}
}

inline void resize_3d(std::vector<MultiView>& multi_view_vec, float scale)
{
	for (int frame_index = 0; frame_index < int(multi_view_vec.size()); ++frame_index)
	{
		MultiView& multi_view = multi_view_vec[frame_index];
		int view_num = multi_view.get_view_num();
		for (int view_index = 0; view_index < view_num; ++view_index)
		{
			View& view = multi_view.get_view(view_index);
			if (frame_index == 0)
			{
				Camera& camera = *view.get_camera();
				camera = Camera(camera.K, camera.R * scale, camera.t);
			}
			view.precompute_rays();
		}
	}
}

inline void resize_3d(std::vector<MultiPose3D>& multi_pose_vec, float scale)
{
	for (MultiPose3D& multi_pose : multi_pose_vec)
	{
		for (auto& [pose_id, pose] : multi_pose)
		{
			for (auto& [joint_type, joint] : pose)
			{
				joint *= scale;
			}
		}
	}
}

}
