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

inline std::vector<MultiPose3D> synchronize(const std::vector<MultiPose3D>& source,
	const std::vector<int>& sync_points_0, const std::vector<int>& sync_points_1)
{
	std::vector<MultiPose3D> destination(sync_points_0.back());

	size_t sync_index = 0;
	for (int frame_index = sync_points_0.front(); frame_index < sync_points_0.back(); ++frame_index)
	{
		while (frame_index >= sync_points_0[sync_index]) sync_index++;
		float delta_time_0 = sync_points_0[sync_index] - sync_points_0[sync_index - 1];
		float delta_time_1 = sync_points_1[sync_index] - sync_points_1[sync_index - 1];
		float index = sync_points_1[sync_index - 1] +
			(frame_index - sync_points_0[sync_index - 1]) * delta_time_1 / delta_time_0;
		destination[frame_index] = source[int(index)];
	}

	return destination;
}

}
