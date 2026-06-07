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

#include <vector>

namespace v3
{

inline const std::vector<std::vector<int>> OpenPoseBody25 = {
	{15, 16},      ///< #0  Nose         +---------------------------------+
	{0, 2, 5},     ///< #1  Neck         |                                 |
	{3},           ///< #2  RShoulder    |             16   15             |
	{4},           ///< #3  RElbow       |            /  \ /  \            |
	{},            ///< #4  RWrist       |           18   0   17           |
	{6},           ///< #5  LShoulder    |                |                O
	{7},           ///< #6  LElbow       |           5----1----2           P
	{},            ///< #7  LWrist       |          /     |     \          E
	{1, 9, 12},    ///< #8  MidHip       |         /      |      \         N
	{10},          ///< #9  RHip         |        6       |       3        P
	{11},          ///< #10 RKnee        |        |       |       |        O
	{22, 24},      ///< #11 RAnkle       |        |       |       |        S
	{13},          ///< #12 LHip         |        7   12--8---9   4        E
	{14},          ///< #13 LKnee        |            |       |            |
	{19, 21},      ///< #14 LAnkle       |            |       |            B
	{17},          ///< #15 REye         |            |       |            O
	{18},          ///< #16 LEye         |            13      10           D
	{},            ///< #17 REar         |            |       |            Y
	{},            ///< #18 LEar         |            |       |            2
	{20},          ///< #19 LBigToe      |            |       |            5
	{},            ///< #20 LSmallToe    |            14      11           |
	{},            ///< #21 LHeel        |           /  \    /  \          |
	{23},          ///< #22 RBigToe      |       20-19  21  24  22-23      |
	{},            ///< #23 RSmallToe    |                                 |
	{},            ///< #24 RHeel        +---------------------------------+
};

}
