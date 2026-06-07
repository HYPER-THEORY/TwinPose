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

inline void correct_shelf(MultiPose3D& multi_pose)
{
	for (auto& [pose_id, pose] : multi_pose)
	{
		if (pose.has_joint(0))
		{
			pose.get_joint(0).z += 0.05;
		}
	}
}

inline void correct_4da(MultiPose3D& multi_pose)
{
	for (auto& [pose_id, pose] : multi_pose)
	{
		if (pose.has_joint(0) && pose.has_joint(1) && pose.has_joint(8))
		{
			ink::Vec3 norm = (pose.get_joint(1) - pose.get_joint(8)).normalize();
			float dist = (pose.get_joint(0) - pose.get_joint(1)).dot(norm);
			pose.get_joint(0) = pose.get_joint(1) + norm * dist;
		}
	}
}

}
