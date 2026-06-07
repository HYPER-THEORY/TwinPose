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

#include "algorithm/AnomalyDetector.h"
#include "algorithm/Common.h"
#include "algorithm/Evaluation.h"
#include "algorithm/OpenPoseBody25.h"
#include "algorithm/PoseBuilder.h"
#include "algorithm/Timer.h"

#include "application/Application.h"
#include "application/Room.h"
#include "application/Visualization.h"

#include "dataset/Conversion.h"
#include "dataset/Correction.h"
#include "dataset/Resizing.h"
#include "dataset/Synchronization.h"
#include "dataset/Undistortion.h"

#include "loaders/4DALoader.h"
#include "loaders/CampusLoader.h"
#include "loaders/MMPoseLoader.h"
#include "loaders/OpenPoseLoader.h"
#include "loaders/OSDLoader.h"
#include "loaders/PanopticLoader.h"

#include "imgui.h"

#include "nlohmann/json.hpp"

#include "SDL2/SDL.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstring>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace v3;

constexpr int BONE_LENGTH_HISTORY = 30;

constexpr float MIN_BONE_LENGTHS[] = {
//  0-1   1-8   2-1   3-2   4-3   5-1   6-5   7-6   8-    9-8   10-9  11-10 12-8  13-12 14-13
	0.00, 0.40, 0.05, 0.10, 0.05, 0.05, 0.10, 0.05, 0.00, 0.00, 0.20, 0.20, 0.00, 0.20, 0.20,
};

constexpr float MAX_BONE_LENGTHS[] = {
//  0-1   1-8   2-1   3-2   4-3   5-1   6-5   7-6   8-    9-8   10-9  11-10 12-8  13-12 14-13
	0.50, 0.75, 0.30, 0.50, 0.50, 0.30, 0.50, 0.50, 0.00, 0.25, 0.55, 0.60, 0.25, 0.55, 0.60,
};

const std::vector<int> EVAL_JOINT_TYPES = {
	0, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12, 13, 14,
};

const std::vector<std::pair<int, int>> EVAL_BONE_TYPES = {
	{1, 8}, {2, 3}, {3, 4}, {5, 6}, {6, 7}, {9, 10}, {10, 11}, {12, 13}, {13, 14}, {0, 1},
};

enum CameraLoader : int
{
	CAMERA_LOADER_4DA,
	CAMERA_LOADER_CAMPUS,
	CAMERA_LOADER_OSD,
};

enum DetectionLoader : int
{
	DETECTION_LOADER_4DA,
	DETECTION_LOADER_OPEN_POSE,
	DETECTION_LOADER_MMPOSE,
	DETECTION_LOADER_OSD,
};

enum GroundTruthLoader : int
{
	GROUND_TRUTH_LOADER_NONE,
	GROUND_TRUTH_LOADER_4DA,
	GROUND_TRUTH_LOADER_4DA_SHELF,
	GROUND_TRUTH_LOADER_CAMPUS,
	GROUND_TRUTH_LOADER_PANOPTIC,
};

enum SkeletonType : int
{
	SKELETON_TYPE_BODY_25,
	SKELETON_TYPE_OPTITRACK_21,
	SKELETON_TYPE_SKEL_19,
	SKELETON_TYPE_SHELF_14,
	SKELETON_TYPE_COCO_17,
	SKELETON_TYPE_COCO_19,
};

enum ResultCorrection : int
{
	RESULT_CORRECTION_NONE,
	RESULT_CORRECTION_SHELF,
	RESULT_CORRECTION_4DA,
};

struct FrameScore
{
	std::vector<float> pcp_score;
	std::vector<float> pcp_completeness;
	std::vector<float> pck_score;
	std::vector<float> pck_completeness;
	std::vector<float> mpjpe_score;
	std::vector<float> mpjpe_completeness;
	float execution_time = 0.0f;
};

struct FrameResult
{
	std::vector<std::vector<PoseBuilder::Joint>> joint_list_per_type;
	std::vector<std::vector<PoseBuilder::JointCluster>> cluster_list_per_type;
	std::vector<EdgeSet> global_pose_graph;
	std::vector<std::vector<EdgeSet>> pose_graph_per_person;
};

struct
{
	char dataset_config_path[512] = "dataset/shelf/dataset.json";
	char algorithm_config_path[512] = "dataset/shelf/algorithm.json";

	int camera_loader = CAMERA_LOADER_4DA;
	char camera_load_path[512];

	int detection_loader = DETECTION_LOADER_4DA;
	int detection_skel_type = SKELETON_TYPE_BODY_25;
	char detection_load_path[512];

	int detection_joint_type_num = 0;
	int detection_view_num = 0;
	int detection_max_person_num = 0;
	int detection_frame_begin = 0;
	int detection_frame_end = 0;
	float detection_joint_2d_scale = 1.0f;

	int ground_truth_loader = GROUND_TRUTH_LOADER_NONE;
	int ground_truth_skel_type = SKELETON_TYPE_BODY_25;
	char ground_truth_load_path[512];

	int ground_truth_joint_type_num = 0;
	int ground_truth_max_person_num = 0;

	char sync_points_path[512];

	bool use_temporal_info = false;

	std::vector<int> joint_types = {0, 4, 3, 2, 7, 6, 5, 1, 11, 10, 9, 14, 13, 12, 8};
	int max_person_num = 0;

	PoseBuilderParams params;
	PoseTrackingParams t_params;

	int result_correction = RESULT_CORRECTION_NONE;
} info;

struct
{
	bool started = false;
	bool playing = false;

	int frame_index = 0;
	int max_computed_frame_index = 0;

	int debug_mode = 0;
	int debug_joint_type = 8;
	int debug_person_id = 0;

	bool show_ground_truth = true;
	bool show_result = true;
	bool show_camera_rays = false;

	float pcp_alpha = 0.5f;
	float pck_distance = 0.2f;

	std::vector<MultiView> multi_views;
	std::vector<MultiPose3D> multi_pose_ground_truth;
	std::vector<MultiPose3D> result_per_frame;

	std::unordered_map<int, FrameResult> frame_result;
	std::unordered_map<int, FrameScore> frame_scores;

	std::deque<std::vector<std::vector<float>>> bone_length_history;

	double total_pcp_score = 0;
	double total_pcp_completeness = 0;
	double total_pck_score = 0;
	double total_pck_completeness = 0;
	double total_mpjpe_score = 0;
	double total_mpjpe_completeness = 0;
	double total_execution_time = 0;
	int computed_count = 0;
} state;

static RemapPosition remap_position;

static std::vector<std::pair<int, int>> compute_ordered_bone_types(const std::vector<int>& joint_types)
{
	std::vector<std::pair<int, int>> ordered_bone_types;

	std::unordered_set<int> joint_type_set;

	for (auto& joint_type : joint_types)
	{
		joint_type_set.emplace(joint_type);
	}

	for (auto& joint_type_1 : joint_types)
	{
		for (auto& joint_type_2 : OpenPoseBody25[joint_type_1])
		{
			if (joint_type_set.contains(joint_type_2))
			{
				ordered_bone_types.emplace_back(joint_type_1, joint_type_2);
			}
		}
	}

	return ordered_bone_types;
}

static bool load_sync_points(const std::string& path, std::vector<int>& sync_points_0, std::vector<int>& sync_points_1)
{
	std::ifstream stream(path, std::fstream::in);
	if (stream.fail())
	{
		return false;
	}

	int sync_point_num = 0;
	stream >> sync_point_num;

	sync_points_0.resize(sync_point_num);
	sync_points_1.resize(sync_point_num);

	for (int index = 0; index < sync_point_num; ++index)
	{
		stream >> sync_points_0[index] >> sync_points_1[index];
	}

	stream.close();
	return true;
}

static bool load_dataset()
{
	state = {};

	bool loaded = false;

	if (info.detection_loader == DETECTION_LOADER_4DA)
	{
		_4DALoader loader;
		loader.joint_type_num = info.detection_joint_type_num;
		loader.view_num = info.detection_view_num;
		loaded = loader.load_multi_view(info.detection_load_path, state.multi_views);
	}
	else if (info.detection_loader == DETECTION_LOADER_OPEN_POSE)
	{
		OpenPoseLoader loader;
		loader.joint_type_num = info.detection_joint_type_num;
		loader.view_num = info.detection_view_num;
		loader.max_person_num = info.detection_max_person_num;
		loader.frame_range = {info.detection_frame_begin, info.detection_frame_end};
		loaded = loader.load_multi_views(info.detection_load_path, state.multi_views);
	}
	else if (info.detection_loader == DETECTION_LOADER_MMPOSE)
	{
		MMPoseLoader loader;
		loader.joint_type_num = info.detection_joint_type_num;
		loader.view_num = info.detection_view_num;
		loader.max_person_num = info.detection_max_person_num;
		loader.frame_range = {info.detection_frame_begin, info.detection_frame_end};
		loaded = loader.load_multi_views(info.detection_load_path, state.multi_views);
	}
	else if (info.detection_loader == DETECTION_LOADER_OSD)
	{
		OSDLoader loader;
		loader.joint_type_num = info.detection_joint_type_num;
		loader.view_num = info.detection_view_num;
		loaded = loader.load_multi_view(info.detection_load_path, state.multi_views);
	}

	if (!loaded)
	{
		std::cerr << "Failed to load 2D detections from " << info.detection_load_path << "\n";
		return false;
	}

	if (info.detection_joint_2d_scale != 1.0f)
	{
		dataset::resize_2d(state.multi_views, info.detection_joint_type_num, ink::Vec2(1 / info.detection_joint_2d_scale));
	}

	if (info.detection_skel_type == SKELETON_TYPE_COCO_17)
	{
		state.multi_views = dataset::convert_coco17_to_body25(state.multi_views);
	}

	loaded = false;

	if (info.camera_loader == CAMERA_LOADER_4DA)
	{
		_4DALoader camera_loader;
		loaded = camera_loader.load_cameras(info.camera_load_path, state.multi_views);
	}
	else if (info.camera_loader == CAMERA_LOADER_CAMPUS)
	{
		CampusLoader camera_loader;
		camera_loader.view_num = info.detection_view_num;
		loaded = camera_loader.load_cameras(info.camera_load_path, state.multi_views);
	}
	else if (info.camera_loader == CAMERA_LOADER_OSD)
	{
		OSDLoader camera_loader;
		camera_loader.view_num = info.detection_view_num;
		loaded = camera_loader.load_cameras(info.camera_load_path, state.multi_views);
	}

	if (!loaded)
	{
		std::cerr << "Failed to load cameras from " << info.camera_load_path << "\n";
		return false;
	}

	if (info.ground_truth_loader != GROUND_TRUTH_LOADER_NONE)
	{
		bool loaded = false;

		if (info.ground_truth_loader == GROUND_TRUTH_LOADER_4DA)
		{
			_4DALoader loader;
			loaded = loader.load_multi_poses(info.ground_truth_load_path, state.multi_pose_ground_truth);
		}
		else if (info.ground_truth_loader == GROUND_TRUTH_LOADER_4DA_SHELF)
		{
			_4DALoader loader;
			loaded = loader.load_multi_poses_2(info.ground_truth_load_path, state.multi_pose_ground_truth);
		}
		else if (info.ground_truth_loader == GROUND_TRUTH_LOADER_CAMPUS)
		{
			CampusLoader loader;
			loader.joint_type_num = info.ground_truth_joint_type_num;
			loaded = loader.load_multi_poses(info.ground_truth_load_path, state.multi_pose_ground_truth);
		}
		else if (info.ground_truth_loader == GROUND_TRUTH_LOADER_PANOPTIC)
		{
			PanopticLoader loader;
			loader.frame_range = {info.detection_frame_begin, info.detection_frame_end};
			loaded = loader.load_multi_poses(info.ground_truth_load_path, state.multi_pose_ground_truth);
		}

		if (!loaded)
		{
			std::cerr << "Failed to load ground truth from " << info.ground_truth_load_path << "\n";
			return false;
		}

		if (info.ground_truth_skel_type == SKELETON_TYPE_OPTITRACK_21)
		{
			state.multi_pose_ground_truth = dataset::convert_optitrack21_to_body25(state.multi_pose_ground_truth);
		}
		else if (info.ground_truth_skel_type == SKELETON_TYPE_SKEL_19)
		{
			state.multi_pose_ground_truth = dataset::convert_skel19_to_body25(state.multi_pose_ground_truth);
		}
		else if (info.ground_truth_skel_type == SKELETON_TYPE_SHELF_14)
		{
			state.multi_pose_ground_truth = dataset::convert_shelf14_to_body25(state.multi_pose_ground_truth);
		}
		else if (info.ground_truth_skel_type == SKELETON_TYPE_COCO_17)
		{
			state.multi_pose_ground_truth = dataset::convert_coco17_to_body25(state.multi_pose_ground_truth);
		}
		else if (info.ground_truth_skel_type == SKELETON_TYPE_COCO_19)
		{
			state.multi_pose_ground_truth = dataset::convert_coco19_to_body25(state.multi_pose_ground_truth);
		}

		if (info.sync_points_path[0] != '\0')
		{
			std::vector<int> sync_points_0;
			std::vector<int> sync_points_1;
			if (!load_sync_points(info.sync_points_path, sync_points_0, sync_points_1))
			{
				std::cerr << "Failed to load sync points from " << info.sync_points_path << "\n";
				return false;
			}
			state.multi_pose_ground_truth = dataset::synchronize(state.multi_pose_ground_truth, sync_points_0, sync_points_1);
		}

		if (info.result_correction == RESULT_CORRECTION_4DA)
		{
			remap_position = [](const ink::Vec3& position) -> ink::Vec3
			{
				return ink::Vec3(position.z, position.y, position.x) + ink::Vec3(2, 0.1, 0);
			};
		}
		else if (info.result_correction == RESULT_CORRECTION_SHELF)
		{
			remap_position = [](const ink::Vec3& position) -> ink::Vec3
			{
				return ink::Vec3(position.x, position.z, -position.y) + ink::Vec3(1, 0.1, 0);
			};
		}
	}

	int frame_num = state.multi_views.size();
	state.multi_pose_ground_truth.resize(frame_num);
	state.result_per_frame.assign(frame_num, MultiPose3D());

	state.frame_result.clear();
	state.frame_scores.clear();
	state.bone_length_history.clear();

	state.total_pcp_score = 0;
	state.total_pcp_completeness = 0;
	state.total_pck_score = 0;
	state.total_pck_completeness = 0;
	state.total_mpjpe_score = 0;
	state.total_mpjpe_completeness = 0;
	state.total_execution_time = 0;
	state.computed_count = 0;

	state.frame_index = 0;
	state.max_computed_frame_index = -1;
	state.playing = false;
	state.started = true;

	if (info.max_person_num == 0)
	{
		info.max_person_num = info.detection_max_person_num;
	}

	return true;
}

static void record_bone_lengths(const MultiPose3D& result, const std::vector<std::pair<int, int>>& ordered_bone_types)
{
	std::vector<std::vector<float>> samples(OpenPoseBody25.size());

	for (auto& [pose_id, pose] : result)
	{
		for (auto& [joint_type_1, joint_type_2] : ordered_bone_types)
		{
			if (!pose.has_joint(joint_type_1) || !pose.has_joint(joint_type_2))
			{
				continue;
			}
			float length = pose.get_joint(joint_type_1).distance(pose.get_joint(joint_type_2));
			samples[joint_type_2].emplace_back(length);
		}
	}

	state.bone_length_history.emplace_back(std::move(samples));
	if (state.bone_length_history.size() > BONE_LENGTH_HISTORY)
	{
		state.bone_length_history.pop_front();
	}
}

static void execute_algorithm(int frame_index)
{
	std::vector<std::pair<int, int>> ordered_bone_types = compute_ordered_bone_types(info.joint_types);

	int detector_num = info.joint_types.size();
	std::vector<AnomalyDetector> bone_length_detectors(detector_num);

	for (auto& [joint_type_1, joint_type_2] : ordered_bone_types)
	{
		bone_length_detectors[joint_type_2].init_cutoff_hard_thresholds(
			MIN_BONE_LENGTHS[joint_type_2], MAX_BONE_LENGTHS[joint_type_2]);

		bone_length_detectors[joint_type_2].init_evaluation_params(
			2.0f, 3.0f, 0.5f);

		std::vector<float> values;
		for (auto& frame_samples : state.bone_length_history)
		{
			for (float length : frame_samples[joint_type_2])
			{
				values.emplace_back(length);
			}
		}
		if (!values.empty())
		{
			bone_length_detectors[joint_type_2].init_normal_distribution(values);
		}
	}

	PoseBuilderParams params = info.params;
	params.max_person_num = info.max_person_num;
	params.skeleton_tree = OpenPoseBody25;
	params.ordered_joint_types = info.joint_types;

	std::vector<const AnomalyDetector*> bone_length_pointers(detector_num);
	for (int index = 0; index < detector_num; ++index)
	{
		bone_length_pointers[index] = &bone_length_detectors[index];
	}
	params.bone_lengths = bone_length_pointers;

	PoseTrackingParams t_params = info.t_params;

	if (info.use_temporal_info)
	{
		const MultiPose3D* history_multi_pose = nullptr;
		if (frame_index > 0)
		{
			history_multi_pose = &state.result_per_frame[frame_index - 1];
		}
		t_params.history_multi_pose = history_multi_pose;
	}

	PoseBuilder builder(&state.multi_views[frame_index], params, t_params);

	auto start_time = std::chrono::steady_clock::now();
	builder.build();
	auto end_time = std::chrono::steady_clock::now();
	float execution_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count() * 0.001f;

	state.result_per_frame[frame_index] = std::move(builder.result);

	FrameResult result;
	result.joint_list_per_type = std::move(builder.joint_list_per_type);
	result.cluster_list_per_type = std::move(builder.cluster_list_per_type);
	result.pose_graph_per_person = std::move(builder.pose_graph_per_person);
	state.frame_result[frame_index] = std::move(result);

	state.frame_scores[frame_index].execution_time = execution_time;

	record_bone_lengths(state.result_per_frame[frame_index], ordered_bone_types);

	if (info.result_correction == RESULT_CORRECTION_SHELF)
	{
		dataset::correct_shelf(state.result_per_frame[frame_index]);
	}
	else if (info.result_correction == RESULT_CORRECTION_4DA)
	{
		dataset::correct_4da(state.result_per_frame[frame_index]);
	}
}

static void print_scores(const std::vector<float>& values, const std::vector<float>& completeness, const char* method,
	int frame_index)
{
	float total = 0, total_completeness = 0;
	for (int index = 0; index < values.size(); ++index)
	{
		total += values[index];
		total_completeness += completeness[index];
	}

	if (frame_index < 0)
	{
		std::cout << "Total:     ";
	}
	else
	{
		std::cout << "Frame: " << std::setw(4) << frame_index;
	}
	std::cout << " | " << method;

	for (int index = 0; index < values.size(); ++index)
	{
		std::cout << " | A" << index + 1 << ": ";
		if (completeness[index] <= 0.0f)
		{
			std::cout << "N/A   ";
		}
		else
		{
			std::cout << std::fixed << std::setprecision(4) << values[index] / completeness[index];
		}
	}

	std::cout << " | Avg: ";
	if (total_completeness <= 0.0f)
	{
		std::cout << "N/A   \n";
	}
	else
	{
		std::cout << std::fixed << std::setprecision(4) << total / total_completeness << "\n";
	}
}

static void evaluate_results(int frame_index)
{
	Evaluation evaluation(EVAL_JOINT_TYPES, EVAL_BONE_TYPES);
	evaluation.pcp_alpha = state.pcp_alpha;
	evaluation.pck_distance = state.pck_distance;

	const MultiPose3D& ground_truth = state.multi_pose_ground_truth[frame_index];
	const MultiPose3D& result = state.result_per_frame[frame_index];

	FrameScore& score = state.frame_scores[frame_index];
	score.pcp_score.assign(info.max_person_num, 0.0f);
	score.pcp_completeness.assign(info.max_person_num, 0.0f);
	score.pck_score.assign(info.max_person_num, 0.0f);
	score.pck_completeness.assign(info.max_person_num, 0.0f);
	score.mpjpe_score.assign(info.max_person_num, 0.0f);
	score.mpjpe_completeness.assign(info.max_person_num, 0.0f);

	std::vector<bool> bone_results;
	std::vector<bool> joint_results;
	std::vector<float> distance_results;

	for (int person_id = 0; person_id < info.max_person_num; ++person_id)
	{
		if (!ground_truth.has_pose(person_id))
		{
			continue;
		}
		const Pose3D& ground_truth_pose = ground_truth.get_pose(person_id);

		score.pcp_score[person_id] = evaluation.pcp(
			ground_truth_pose, result, bone_results, score.pcp_completeness[person_id]);
		score.pck_score[person_id] = evaluation.pck(
			ground_truth_pose, result, joint_results, score.pck_completeness[person_id]);
		score.mpjpe_score[person_id] = evaluation.mpjpe(
			ground_truth_pose, result, distance_results, score.mpjpe_completeness[person_id]);

		state.total_pcp_score += score.pcp_score[person_id];
		state.total_pcp_completeness += score.pcp_completeness[person_id];
		state.total_pck_score += score.pck_score[person_id];
		state.total_pck_completeness += score.pck_completeness[person_id];
		state.total_mpjpe_score += score.mpjpe_score[person_id];
		state.total_mpjpe_completeness += score.mpjpe_completeness[person_id];
	}

	state.total_execution_time += score.execution_time;
	++state.computed_count;

	print_scores(score.pcp_score, score.pcp_completeness, "PCP", frame_index);
}

static void goto_next_frame()
{
	if (state.frame_index + 1 < state.multi_views.size())
	{
		++state.frame_index;
	}
	else
	{
		state.playing = false;
	}
}

static void goto_previous_frame()
{
	if (state.frame_index > 0)
	{
		--state.frame_index;
	}
}

static void handle_input(Application& application, Room& room, float delta_time)
{
	if (!application.want_capture_keyboard())
	{
		if (application.is_key_pressed(SDLK_RIGHT))
		{
			goto_next_frame();
		}
		if (application.is_key_pressed(SDLK_LEFT))
		{
			goto_previous_frame();
		}
		if (application.is_key_pressed(SDLK_SPACE))
		{
			state.playing = !state.playing;
		}
		for (int digit = 0; digit <= 6; ++digit)
		{
			if (application.is_key_pressed(SDLK_0 + digit))
			{
				state.debug_mode = digit;
			}
		}
	}

	if (state.playing)
	{
		goto_next_frame();
	}

	if (!application.want_capture_mouse() && application.is_mouse_down(Application::RIGHT_MOUSE_BUTTON))
	{
		ink::Vec3 move = {0, 0, 0};
		if (application.is_key_down(SDLK_w)) move.z += 1;
		if (application.is_key_down(SDLK_s)) move.z -= 1;
		if (application.is_key_down(SDLK_d)) move.x += 1;
		if (application.is_key_down(SDLK_a)) move.x -= 1;
		if (move.magnitude() != 0) move = move.normalize() * (delta_time * 2.0f);

		ink::Vec2 motion = application.get_mouse_motion();
		room.control_camera(move, {-motion.x, -motion.y}, 0.003f);
	}
	else
	{
		room.control_camera({0, 0, 0}, {0, 0}, 0);
	}
}

static void visualize_frame(Room& room)
{
	room.clean_up_joints_and_bones();

	const MultiPose3D& ground_truth = state.multi_pose_ground_truth[state.frame_index];
	const MultiPose3D& result = state.result_per_frame[state.frame_index];

	if (state.show_ground_truth)
	{
		viz_multi_pose_3d(room, ground_truth, info.max_person_num, remap_position);
	}
	if (state.show_result)
	{
		viz_multi_pose_3d(room, result, info.max_person_num, remap_position, ink::Vec3(1.5));
	}

	const FrameResult& frame_result = state.frame_result[state.frame_index];
	std::vector<std::pair<int, int>> ordered_bone_types = compute_ordered_bone_types(info.joint_types);

	float joint_radius = info.params.max_joint_radius * 0.25f;
	float cluster_radius = info.params.max_joint_cluster_radius * 0.25f;

	switch (state.debug_mode)
	{
	case 1:
		viz_joints(room, frame_result.joint_list_per_type,
			info.joint_types, state.debug_joint_type, joint_radius, remap_position);
		break;
	case 2:
		if (state.debug_joint_type < frame_result.joint_list_per_type.size())
		{
			viz_joints(room, frame_result.joint_list_per_type[state.debug_joint_type],
				joint_radius, remap_position);
		}
		break;
	case 3:
		viz_clusters(room, frame_result.cluster_list_per_type,
			info.joint_types, state.debug_joint_type, cluster_radius, remap_position);
		break;
	case 4:
		viz_clusters(room, frame_result.cluster_list_per_type,
			{state.debug_joint_type}, state.debug_joint_type, cluster_radius, remap_position);
		break;
	case 5:
		viz_pose_graphs(room, frame_result.cluster_list_per_type,
			frame_result.pose_graph_per_person, ordered_bone_types, remap_position);
		break;
	case 6:
		if (state.debug_person_id < frame_result.pose_graph_per_person.size())
		{
			viz_pose_graph(room, frame_result.cluster_list_per_type,
				frame_result.pose_graph_per_person[state.debug_person_id], ordered_bone_types, remap_position);
		}
		break;
	}

	if (state.show_camera_rays)
	{
		viz_rays(room, state.multi_views[state.frame_index], info.detection_view_num, state.debug_joint_type, remap_position);
	}
}

static bool parse_json_config(const std::string& path, nlohmann::json& data)
{
	std::ifstream stream(path, std::fstream::in);
	if (stream.fail())
	{
		return false;
	}

	data = nlohmann::json::parse(stream, nullptr, false);
	return data.is_object();
}

static void apply_int(const nlohmann::json& data, const char* key, int& target)
{
	if (data.contains(key) && data[key].is_number())
	{
		target = data[key].get<int>();
	}
}

static void apply_float(const nlohmann::json& data, const char* key, float& target)
{
	if (data.contains(key) && data[key].is_number())
	{
		target = data[key].get<float>();
	}
}

static void apply_bool(const nlohmann::json& data, const char* key, bool& target)
{
	if (data.contains(key) && data[key].is_boolean())
	{
		target = data[key].get<bool>();
	}
}

static void apply_name(const nlohmann::json& data, const char* key, const std::vector<std::string>& names, int& target)
{
	if (data.contains(key) && data[key].is_string())
	{
		target = std::distance(names.begin(), std::find(names.begin(), names.end(), data[key].get<std::string>()));
		if (target == names.size())
		{
			target = 0;
		}
	}
}

static void apply_path(const nlohmann::json& data, const char* key, const std::string& path, char* buffer, size_t size)
{
	std::filesystem::path base_dir = std::filesystem::path(path).parent_path();

	if (data.contains(key) && data[key].is_string())
	{
		std::string value = data[key].get<std::string>();
		if (!value.empty())
		{
			value = (base_dir / value).generic_string();
		}
		size_t length = std::min(value.size(), size - 1);
		std::memcpy(buffer, value.data(), length);
		buffer[length] = '\0';
	}
};

static bool import_dataset_config(const std::string& path)
{
	nlohmann::json data;
	if (!parse_json_config(path, data))
	{
		return false;
	}

	apply_name(data, "camera_loader", {"4DA", "Campus", "OSD"}, info.camera_loader);
	apply_path(data, "camera_load_path", path, info.camera_load_path, sizeof(info.camera_load_path));
	apply_name(data, "detection_loader", {"4DA", "OpenPose", "MMPose", "OSD"}, info.detection_loader);
	apply_name(data, "detection_skel_type", {"Body25", "OptiTrack21", "Skel19", "Shelf14", "Coco17", "Coco19"}, info.detection_skel_type);
	apply_path(data, "detection_load_path", path, info.detection_load_path, sizeof(info.detection_load_path));
	apply_int(data, "detection_joint_type_num", info.detection_joint_type_num);
	apply_int(data, "detection_view_num", info.detection_view_num);
	apply_int(data, "detection_max_person_num", info.detection_max_person_num);
	apply_float(data, "detection_joint_2d_scale", info.detection_joint_2d_scale);

	if (data.contains("detection_frame_range") && data["detection_frame_range"].is_array())
	{
		const nlohmann::json& range = data["detection_frame_range"];
		if (range.size() >= 1 && range[0].is_number())
		{
			info.detection_frame_begin = range[0].get<int>();
		}
		if (range.size() >= 2 && range[1].is_number())
		{
			info.detection_frame_end = range[1].get<int>();
		}
	}

	apply_name(data, "ground_truth_loader", {"None", "4DA", "4DA Shelf", "Campus", "Panoptic"}, info.ground_truth_loader);
	apply_name(data, "ground_truth_skel_type", {"Body25", "OptiTrack21", "Skel19", "Shelf14", "Coco17", "Coco19"}, info.ground_truth_skel_type);
	apply_path(data, "ground_truth_load_path", path, info.ground_truth_load_path, sizeof(info.ground_truth_load_path));
	apply_int(data, "ground_truth_joint_type_num", info.ground_truth_joint_type_num);
	apply_int(data, "ground_truth_max_person_num", info.ground_truth_max_person_num);

	apply_path(data, "sync_points_path", path, info.sync_points_path, sizeof(info.sync_points_path));
	
	apply_name(data, "result_correction", {"None", "Shelf", "4DA"}, info.result_correction);

	return true;
}

static bool import_algorithm_config(const std::string& path)
{
	nlohmann::json data;
	if (!parse_json_config(path, data))
	{
		return false;
	}

	if (data.contains("joint_types") && data["joint_types"].is_array())
	{
		std::vector<int> joint_types;
		for (const nlohmann::json& element : data["joint_types"])
		{
			if (element.is_number())
			{
				joint_types.emplace_back(element.get<int>());
			}
		}
		if (!joint_types.empty())
		{
			info.joint_types = std::move(joint_types);
		}
	}

	apply_int(data, "max_person_num", info.max_person_num);

	apply_bool(data, "use_bone_length_score", info.params.use_bone_length_score);
	apply_bool(data, "use_edge_num_in_association", info.params.use_edge_num_in_association);

	apply_float(data, "conf_discard_threshold", info.params.conf_discard_threshold);
	apply_float(data, "conf_exponent", info.params.conf_exponent);

	apply_float(data, "max_joint_radius", info.params.max_joint_radius);
	apply_float(data, "max_joint_cluster_radius", info.params.max_joint_cluster_radius);

	apply_float(data, "association_score_threshold", info.params.association_score_threshold);
	apply_float(data, "pose_discard_threshold", info.params.pose_discard_threshold);

	apply_float(data, "tracking_discard_threshold", info.t_params.tracking_discard_threshold);
	apply_float(data, "tracking_distance", info.t_params.tracking_distance);
	apply_float(data, "lost_track_penalty", info.t_params.lost_track_penalty);

	return true;
}

static void draw_loading_window()
{
	ImGui::SetNextWindowPos(ImVec2(40, 40), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(640, 640), ImGuiCond_FirstUseEver);
	ImGui::Begin("Load Dataset");

	if (ImGui::CollapsingHeader("Import JSON Files", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::InputText("##Dataset", info.dataset_config_path, sizeof(info.dataset_config_path));
		ImGui::SameLine();
		static std::string import_dataset_btn_text = "Import Dataset JSON";
		if (ImGui::Button(import_dataset_btn_text.c_str(), ImVec2(160, 0)))
		{
			bool imported = import_dataset_config(info.dataset_config_path);
			import_dataset_btn_text = imported ? "Import Successful!##Dataset" : "Import Failed!##Dataset";
		}
		
		static std::string import_algorithm_btn_text = "Import Algorithm JSON";
		ImGui::InputText("##Algorithm", info.algorithm_config_path, sizeof(info.algorithm_config_path));
		ImGui::SameLine();
		if (ImGui::Button(import_algorithm_btn_text.c_str(), ImVec2(160, 0)))
		{
			bool imported = import_algorithm_config(info.algorithm_config_path);
			import_algorithm_btn_text = imported ? "Import Successful!##Algorithm" : "Import Failed!##Algorithm";
		}
	}

	const char* skeleton_type_names[] = {
		"Body 25", "OptiTrack 21", "Skel 19", "Shelf 14", "Coco 17", "Coco 19",
	};

	if (ImGui::CollapsingHeader("Cameras", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const char* loader_names[] = {"4DA", "Campus", "OSD"};
		ImGui::Combo("Camera Loader", &info.camera_loader, loader_names, IM_ARRAYSIZE(loader_names));
		ImGui::InputText("Camera Load Path", info.camera_load_path, sizeof(info.camera_load_path));
	}

	if (ImGui::CollapsingHeader("Detections", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const char* loader_names[] = {"4DA", "OpenPose", "MMPose", "OSD"};
		ImGui::Combo("Detection Loader", &info.detection_loader, loader_names, IM_ARRAYSIZE(loader_names));
		ImGui::Combo("Detection Skel Type", &info.detection_skel_type, skeleton_type_names, IM_ARRAYSIZE(skeleton_type_names));
		ImGui::InputText("Detection Load Path", info.detection_load_path, sizeof(info.detection_load_path));
		ImGui::InputInt("Joint Type Num", &info.detection_joint_type_num);
		ImGui::InputInt("Loader View Num", &info.detection_view_num);
		ImGui::InputInt("Loader Max Person Num", &info.detection_max_person_num);
		ImGui::InputInt("Frame Begin", &info.detection_frame_begin);
		ImGui::InputInt("Frame End", &info.detection_frame_end);
		ImGui::InputFloat("Detection Joint 2D Scale", &info.detection_joint_2d_scale, 0.0f, 0.0f, "%.7f");
	}

	if (ImGui::CollapsingHeader("Ground Truth", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const char* loader_names[] = {"None", "4DA", "4DA Shelf", "Campus", "Panoptic"};
		ImGui::Combo("Ground Truth Loader", &info.ground_truth_loader, loader_names, IM_ARRAYSIZE(loader_names));
		ImGui::Combo("Ground Truth Skel Type", &info.ground_truth_skel_type, skeleton_type_names, IM_ARRAYSIZE(skeleton_type_names));
		ImGui::InputText("Ground Truth Load Path", info.ground_truth_load_path, sizeof(info.ground_truth_load_path));
		ImGui::InputInt("Ground Truth Joint Type Num", &info.ground_truth_joint_type_num);
		ImGui::InputInt("Ground Truth Max Person Num", &info.ground_truth_max_person_num);
		ImGui::InputText("Sync Points Path", info.sync_points_path, sizeof(info.sync_points_path));
	}

	if (ImGui::CollapsingHeader("Result Correction", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const char* result_correction_names[] = {"None", "Shelf", "4DA"};
		ImGui::Combo("Result Correction##Btn", &info.result_correction, result_correction_names, IM_ARRAYSIZE(result_correction_names));
	}

	if (ImGui::CollapsingHeader("Algorithm Parameters", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Use Bone Length Score", &info.params.use_bone_length_score);
		ImGui::Checkbox("Use Edge Num In Association", &info.params.use_edge_num_in_association);
		ImGui::InputInt("Max Person Num", &info.max_person_num);
		ImGui::SliderFloat("Max Joint Radius", &info.params.max_joint_radius, 0.0f, 0.2f);
		ImGui::SliderFloat("Max Joint Cluster Radius", &info.params.max_joint_cluster_radius, 0.0f, 0.4f);
		ImGui::SliderFloat("Conf Discard Threshold", &info.params.conf_discard_threshold, 0.0f, 1.0f);
		ImGui::SliderFloat("Conf Exponent", &info.params.conf_exponent, 0.0f, 1.0f);
		ImGui::SliderFloat("Association Score Threshold", &info.params.association_score_threshold, 0.0f, 100.0f);
		ImGui::SliderFloat("Pose Discard Threshold", &info.params.pose_discard_threshold, 0.0f, 100.0f);
		ImGui::Separator();
		ImGui::Checkbox("Use Temporal Information", &info.use_temporal_info);
		ImGui::SliderFloat("Tracking Distance", &info.t_params.tracking_distance, 0.0f, 1.0f);
		ImGui::SliderFloat("Tracking Discard Threshold", &info.t_params.tracking_discard_threshold, 0.0f, 100.0f);
		ImGui::SliderFloat("Lost Track Penalty", &info.t_params.lost_track_penalty, 0.0f, 1.0f);
	}

	if (ImGui::Button("Load Dataset", ImVec2(160, 0)))
	{
		load_dataset();
	}

	ImGui::End();
}

static void draw_control_window()
{
	ImGui::SetNextWindowPos(ImVec2(40, 40), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(400, 425), ImGuiCond_FirstUseEver);
	ImGui::Begin("Control");

	ImGui::Text("Frame: %d / %d", state.frame_index, state.multi_views.size() - 1);

	int frame_index = state.frame_index;
	if (ImGui::SliderInt("Timeline", &frame_index, 0, state.max_computed_frame_index))
	{
		state.frame_index = std::clamp(frame_index, 0, state.max_computed_frame_index);
		state.playing = false;
	}

	if (ImGui::Button(state.playing ? "Pause" : "Play"))
	{
		state.playing = !state.playing;
	}
	ImGui::SameLine();
	if (ImGui::Button("Prev"))
	{
		goto_previous_frame();
	}
	ImGui::SameLine();
	if (ImGui::Button("Next"))
	{
		goto_next_frame();
	}

	ImGui::Separator();

	ImGui::SliderInt("Debug Mode", &state.debug_mode, 0, 6);
	ImGui::SliderInt("Debug Joint Type", &state.debug_joint_type, 0, info.joint_types.size() - 1);
	ImGui::SliderInt("Debug Person Id", &state.debug_person_id, 0, info.max_person_num - 1);
	ImGui::Checkbox("Show Ground Truth", &state.show_ground_truth);
	ImGui::SameLine();
	ImGui::Checkbox("Show Result", &state.show_result);
	ImGui::SameLine();
	ImGui::Checkbox("Show Camera Rays", &state.show_camera_rays);

	ImGui::Separator();

	auto average_value = [](double score, double completeness) -> float
	{
		return completeness <= 0.0 ? 0.0f : float(score / completeness);
	};

	const FrameScore& score = state.frame_scores[state.frame_index];
	float pcp = 0, pcp_completeness = 0;
	float pck = 0, pck_completeness = 0;
	float mpjpe = 0, mpjpe_completeness = 0;
	for (int index = 0; index < score.pcp_score.size(); ++index)
	{
		pcp += score.pcp_score[index];
		pcp_completeness += score.pcp_completeness[index];
		pck += score.pck_score[index];
		pck_completeness += score.pck_completeness[index];
		mpjpe += score.mpjpe_score[index];
		mpjpe_completeness += score.mpjpe_completeness[index];
	}
	ImGui::Text("Current Frame:");
	ImGui::Text("\tPCP:\t%.4f", average_value(pcp, pcp_completeness));
	ImGui::Text("\tPCK:\t%.4f", average_value(pck, pck_completeness));
	ImGui::Text("\tMPJPE:\t%.3f mm", average_value(mpjpe, mpjpe_completeness));
	ImGui::Text("\tTime:\t%.3f ms", score.execution_time);

	ImGui::Separator();

	ImGui::Text("All Computed Frames:");
	ImGui::Text("\tPCP:\t%.4f", average_value(state.total_pcp_score, state.total_pcp_completeness));
	ImGui::Text("\tPCK:\t%.4f", average_value(state.total_pck_score, state.total_pck_completeness));
	ImGui::Text("\tMPJPE:\t%.3f mm", average_value(state.total_mpjpe_score, state.total_mpjpe_completeness));
	ImGui::Text("\tTime:\t%.3f ms", state.total_execution_time / state.computed_count);

	ImGui::End();
}

static void set_window_title(Application& application, float delta_time)
{
	static float duration_time = 0.0f;
	static int frame_num = 0;

	duration_time += delta_time;
	frame_num += 1;

	if (duration_time > 1.0f)
	{
		int fps = std::round(frame_num / duration_time);
		duration_time = 0.0f;
		frame_num = 0;
		application.set_title("TwinPose V3 [ FPS: " + std::to_string(fps) + " ]");
	}
}

static void update(Application& application, Room& room, float delta_time)
{
	if (!state.started)
	{
		draw_loading_window();
		return;
	}

	handle_input(application, room, delta_time);

	if (state.frame_index > state.max_computed_frame_index && state.frame_index < state.multi_views.size())
	{
		execute_algorithm(state.frame_index);
		evaluate_results(state.frame_index);

		state.max_computed_frame_index = state.frame_index;
	}

	visualize_frame(room);

	room.update(delta_time);
	room.render();

	draw_control_window();
	set_window_title(application, delta_time);
}

int main(int argc, char* argv[])
{
	Application application;
	if (!application.init("TwinPose V3", 1280, 720, 60, true))
	{
		return -1;
	}

	auto [framebuffer_width, framebuffer_height] = application.get_framebuffer_size();

	Room room;
	room.set_realtime_reflection(false);
	room.prepare_resources();
	room.setup_pipeline(framebuffer_width, framebuffer_height);

	application.run([&](float delta_time) -> void
	{
		update(application, room, delta_time);
	});

	return 0;
}
