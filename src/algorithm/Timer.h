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

#include <chrono>
#include <iomanip>
#include <iostream>
#include <unordered_map>

namespace v3
{

class Timer
{
public:
	Timer(std::string name, bool log = false) :
		name(std::move(name)), log(log), start(std::chrono::steady_clock::now()) {}

	~Timer()
	{
		auto end = std::chrono::steady_clock::now();
		auto delta = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		total_time[name] += delta;
		if (!log)
		{
			return;
		}
		std::cout << "[" << name << "] Time: "
			<< std::fixed << std::setprecision(3) << delta.count() * 0.001 << " ms\n";
	}

	static double total(const std::string& name)
	{
		return total_time[name].count() * 0.001;
	}

private:
	std::string name;
	bool log = false;
	std::chrono::steady_clock::time_point start;
	static inline std::unordered_map<std::string, std::chrono::microseconds> total_time;
};

}
