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

#include "ink/ink.h"

namespace v3
{

class Camera
{
public:
	ink::Mat3 K;      ///< the camera intrinsic matrix
	ink::Mat3 R;      ///< the rotation matrix
	ink::Vec3 t;      ///< the translation vector
	ink::Mat3 KRi;    ///< (K * R) ^ (-1)
	ink::Vec3 pos;    ///< the camera position
	ink::Mat3x4 P;    ///< the projection matrix

	Camera() = default;

	Camera(const ink::Mat3& K, const ink::Mat3& R, const ink::Vec3& t) : K(K), R(R), t(t)
	{
		KRi = R.transpose() * inverse_3x3(K);
		pos = -R.transpose() * t;
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				P[i][j] = j < 3 ? R[i][j] : (&t.x)[i];
			}
		}
		P = K * P;
	}

	const ink::Vec3& get_position() const
	{
		return pos;
	}

	const ink::Mat3x4& get_projection() const
	{
		return P;
	}

	ink::Ray cast_ray(const ink::Vec2& point) const
	{
		return ink::Ray(pos, ink::Vec3(-KRi * ink::Vec3(point.x, point.y, 1)).normalize());
	}
};

struct Joint2D
{
	ink::Vec2 point;         ///< the point (UV) on the screen
	float conf = 0;          ///< the confidence
	unsigned int uid = 0;    ///< the unique ID
};

class View
{
public:
	View() = default;

	View(Camera* camera) : camera(camera) {}

	Camera* get_camera() const
	{
		return camera;
	}

	void set_camera(Camera* camera)
	{
		this->camera = camera;
	}

	const ink::Ray& get_ray(unsigned int joint_uid) const
	{
		return precomputed_ray_lut.at(joint_uid);
	}

	const ink::Ray& get_ray(const Joint2D& joint) const
	{
		return precomputed_ray_lut.at(joint.uid);
	}

	void precompute_rays()
	{
		for (auto& [joint_key, joint] : joints)
		{
			precomputed_ray_lut.emplace(joint.uid, camera->cast_ray(joint.point));
		}
	}

	int get_joint_num(int joint_type) const
	{
		return joint_num_per_type.contains(joint_type) ? joint_num_per_type.at(joint_type) : 0;
	}

	const Joint2D& get_joint(int joint_type, int index) const
	{
		int joint_key = (joint_type << 16) | index;
		return joints.at(joint_key);
	}

	Joint2D& get_joint(int joint_type, int index)
	{
		int joint_key = (joint_type << 16) | index;
		return joints.at(joint_key);
	}

	void add_joint(int joint_type, const Joint2D& joint)
	{
		int joint_index = ++joint_num_per_type[joint_type];
		int joint_key = (joint_type << 16) | joint_index;
		joints.emplace(joint_key, joint);
	}

	void add_joint(int joint_type, Joint2D&& joint)
	{
		int joint_index = ++joint_num_per_type[joint_type];
		int joint_key = (joint_type << 16) | joint_index;
		joints.emplace(joint_key, std::move(joint));
	}

	void clear_joints()
	{
		joint_num_per_type.clear();
		joints.clear();
		precomputed_ray_lut.clear();
		bone_weights.clear();
	}

	float get_bone_weight(const Joint2D& joint_a, const Joint2D& joint_b) const
	{
		uint64_t a_uid = joint_a.uid;
		uint64_t b_uid = joint_b.uid;
		if (a_uid > b_uid) std::swap(a_uid, b_uid);
		uint64_t bone_key = (a_uid << 32) | b_uid;
		return bone_weights.at(bone_key);
	}

	void set_bone_weight(const Joint2D& joint_a, const Joint2D& joint_b, float weight)
	{
		uint64_t a_uid = joint_a.uid;
		uint64_t b_uid = joint_b.uid;
		if (a_uid > b_uid) std::swap(a_uid, b_uid);
		uint64_t bone_key = (a_uid << 32) | b_uid;
		bone_weights.insert_or_assign(bone_key, weight);
	}

private:
	Camera* camera = nullptr;

	std::unordered_map<int, int> joint_num_per_type;
	std::unordered_map<int, Joint2D> joints;
	std::unordered_map<unsigned int, ink::Ray> precomputed_ray_lut;
	std::unordered_map<unsigned long long, float> bone_weights;
};

class MultiView
{
public:
	MultiView() = default;

	void add_view(const View& view)
	{
		views.emplace_back(view);
	}

	void add_view(View&& view)
	{
		views.emplace_back(std::move(view));
	}

	void clear_views()
	{
		views.clear();
	}

	int get_view_num() const
	{
		return views.size();
	}

	View& get_view(int index)
	{
		return views[index];
	}

	const View& get_view(int index) const
	{
		return views[index];
	}

private:
	std::vector<View> views;
};

using Joint3D = ink::Vec3;

class Pose3D
{
public:
	Pose3D() = default;

	auto begin()
	{
		return joints.begin();
	}

	auto end()
	{
		return joints.end();
	}

	auto begin() const
	{
		return joints.begin();
	}

	auto end() const
	{
		return joints.end();
	}

	void add_joint(int joint_type, const Joint3D& joint)
	{
		joints.insert_or_assign(joint_type, joint);
	}

	void add_joint(int joint_type, Joint3D&& joint)
	{
		joints.insert_or_assign(joint_type, std::move(joint));
	}

	void remove_joint(int joint_type)
	{
		joints.erase(joint_type);
	}

	bool has_joint(int joint_type) const
	{
		return joints.contains(joint_type);
	}

	Joint3D& get_joint(int joint_type)
	{
		return joints.at(joint_type);
	}

	const Joint3D& get_joint(int joint_type) const
	{
		return joints.at(joint_type);
	}

private:
	std::unordered_map<int, Joint3D> joints;
};

class MultiPose3D
{
public:
	MultiPose3D() = default;

	auto begin()
	{
		return poses.begin();
	}

	auto end()
	{
		return poses.end();
	}

	auto begin() const
	{
		return poses.begin();
	}

	auto end() const
	{
		return poses.end();
	}

	void add_pose(int pose_id, const Pose3D& pose)
	{
		poses.insert_or_assign(pose_id, pose);
	}

	void add_pose(int pose_id, Pose3D&& pose)
	{
		poses.insert_or_assign(pose_id, std::move(pose));
	}

	void remove_pose(int pose_id)
	{
		poses.erase(pose_id);
	}

	bool has_pose(int pose_id) const
	{
		return poses.contains(pose_id);
	}

	Pose3D& get_pose(int pose_id)
	{
		return poses.at(pose_id);
	}

	const Pose3D& get_pose(int pose_id) const
	{
		return poses.at(pose_id);
	}

private:
	std::unordered_map<int, Pose3D> poses;
};

}
