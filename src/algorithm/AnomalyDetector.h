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

#include <algorithm>
#include <cmath>
#include <vector>

namespace v3
{

class AnomalyDetector
{
public:
	AnomalyDetector() = default;

	void init_cutoff_hard_thresholds(float cutoff_min, float cutoff_max)
	{
		this->cutoff_min = cutoff_min;
		this->cutoff_max = cutoff_max;
	}

	void init_normal_distribution(const std::vector<float>& values)
	{
		double value_num = values.size();
		double sum = 0;
		double sum_square = 0;
		for (auto& value : values)
		{
			sum += value;
			sum_square += value * value;
		}
		mean = sum / value_num;
		standard_deviation = std::sqrt(sum_square / value_num - mean * mean);
	}

	void init_evaluation_params(float min_z, float max_z, float min_score)
	{
		score_scale = 1.0f / (min_z - max_z);
		score_offset = max_z / (max_z - min_z);
		this->min_score = min_score;
	}

	float evaluate(float value) const
	{
		if (value < cutoff_min || value > cutoff_max)
		{
			return 0;
		}
		float z_score = standard_deviation == 0 ? 0 : std::abs(value - mean) / standard_deviation;
		return std::clamp(z_score * score_scale + score_offset, min_score, 1.0f);
	}

private:
	float cutoff_min = 0.0f;
	float cutoff_max = 0.0f;

	float score_scale = -0.5f; // min_z = 1, max_z = 3
	float score_offset = 1.5f;
	float min_score = 0.5f;

	float mean = 0.0f;
	float standard_deviation = 0.0f;
};

}
