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

#include <cmath>
#include <string>
#include <unordered_map>

namespace v3
{

class Room
{
public:
	static constexpr ink::Vec3 DEFAULT_COLOR = ink::Vec3(0x16, 0x19, 0x3B) / 255.0f;

	Room() = default;

	void set_realtime_reflection(bool enable)
	{
		enable_realtime_reflection = enable;
	}

	void prepare_resources()
	{
		const std::string path_s = "Assets/standard/";
		const std::string path_g = "Assets/vr-exhibition-gallery-baked/";

		images["Env"] = ink::Loader::load_image("Assets/Studio_Soft.hdr");
		images["Env"].flip_vertical();

		probes["Env"] = ink::ReflectionProbe(1, 2048);
		probes["Env"].load_equirect(images["Env"]);

		probes["Ground"] = ink::ReflectionProbe(1.5, 1024);

		meshes["Sphere"] = ink::Loader::load_obj(path_s + "Sphere.obj").mesh[0];
		meshes["Sphere"].create_tangents();

		meshes["Cylinder"] = ink::Loader::load_obj(path_s + "Cylinder.obj").mesh[0];
		meshes["Cylinder"].create_tangents();

		meshes["Gallery"] = ink::Loader::load_obj(path_g + "source/2.obj").mesh[0];
		meshes["Gallery"].create_tangents();

		images["WallL2"] = ink::Loader::load_image(path_g + "textures/wall_l2.png");
		images["WallL2"].flip_vertical();

		images["WallR2"] = ink::Loader::load_image(path_g + "textures/wall_r2.png");
		images["WallR2"].flip_vertical();

		images["WallBack2"] = ink::Loader::load_image(path_g + "textures/wall_back2.png");
		images["WallBack2"].flip_vertical();

		materials["Material.001"] = ink::Material();
		materials["Material.001"].color = {0.460, 0.470, 0.480};
		materials["Material.001"].roughness = 0.10;
		materials["Material.001"].metalness = 0.25;
		materials["Material.001"].reflection_probe = &probes["Ground"];

		materials["Material.002"] = ink::Material();
		materials["Material.002"].visible = false;

		materials["Material.003"] = ink::Material();
		materials["Material.003"].emissive_intensity = 1.1;
		materials["Material.003"].emissive = {1, 1, 1};
		materials["Material.003"].emissive_map = &images["WallR2"];

		materials["Material.004"] = ink::Material();
		materials["Material.004"].emissive_intensity = 1.1;
		materials["Material.004"].emissive = {1, 1, 1};
		materials["Material.004"].emissive_map = &images["WallL2"];

		materials["Material.006"] = ink::Material();
		materials["Material.006"].emissive_intensity = 1.1;
		materials["Material.006"].emissive = {1, 1, 1};
		materials["Material.006"].emissive_map = &images["WallBack2"];

		materials["Material.005"] = ink::Material();
		materials["Material.005"].side = ink::DOUBLE_SIDE;
		materials["Material.005"].emissive_intensity = 0.9;
		materials["Material.005"].emissive = {1, 1, 1};

		materials["Light.001"] = ink::Material();
		materials["Light.001"].side = ink::DOUBLE_SIDE;
		materials["Light.001"].emissive_intensity = 1.5;
		materials["Light.001"].emissive = {1, 1, 1};

		instances["Gallery"] = ink::Instance();
		instances["Gallery"].mesh = &meshes["Gallery"];
		instances["Gallery"].scale = {0.2, 0.2, 0.2};

		if (!enable_realtime_reflection)
		{
			materials["Material.001"].emissive = {0.560, 0.570, 0.580};
			materials["Material.001"].reflection_probe = nullptr;
		}

		scene.add(&instances["Gallery"]);

		scene.set_material("Material.001", &materials["Material.001"]);
		scene.set_material("Material.002", &materials["Material.002"]);
		scene.set_material("Material.003", &materials["Material.003"]);
		scene.set_material("Material.004", &materials["Material.004"]);
		scene.set_material("Material.006", &materials["Material.006"]);
		scene.set_material("Material.005", &materials["Material.005"]);
		scene.set_material("light.001", &materials["Light.001"]);

		renderer.load_scene(scene);
		renderer.load_mesh(meshes["Sphere"]);
		renderer.load_mesh(meshes["Cylinder"]);
	}

	void setup_pipeline(int width, int height)
	{
		viewport = ink::gpu::Rect(width, height);

		int width_2 = viewport.width * 2;
		int height_2 = viewport.height * 2;

		maps["DepthMap"].init_2d(width_2, height_2, ink::TEXTURE_D24_UNORM);
		maps["DepthMap"].set_filters(ink::TEXTURE_LINEAR, ink::TEXTURE_LINEAR);

		maps["PostMap0"].init_2d(width_2, height_2, ink::TEXTURE_R16G16B16_SFLOAT);
		maps["PostMap0"].set_filters(ink::TEXTURE_LINEAR, ink::TEXTURE_LINEAR);

		maps["PostMap1"].init_2d(width_2, height_2, ink::TEXTURE_R16G16B16_SFLOAT);
		maps["PostMap1"].set_filters(ink::TEXTURE_LINEAR, ink::TEXTURE_LINEAR);

		targets["BaseTarget"].set_texture(maps["PostMap0"], 0);
		targets["BaseTarget"].set_depth_texture(maps["DepthMap"]);

		targets["PostTarget0"].set_texture(maps["PostMap0"], 0);

		targets["PostTarget1"].set_texture(maps["PostMap1"], 0);

		renderer.set_rendering_mode(ink::FORWARD_RENDERING);
		renderer.set_viewport(ink::gpu::Rect(width_2, height_2));

		camera = ink::PerspCamera(75 * ink::DEG_TO_RAD, float(width) / float(height), 0.05, 500);
		set_camera({4, 1, 0}, {1, 0, 0});

		bloom_pass = ink::BloomPass(width_2, height_2);
		bloom_pass.init();
		bloom_pass.threshold = 1.0;
		bloom_pass.radius = 0.2;
		bloom_pass.intensity = 0.5;

		tone_map_pass = ink::ToneMapPass();
		tone_map_pass.init();
		tone_map_pass.exposure = 1.0;
		tone_map_pass.mode = ink::ACES_FILMIC_TONE_MAP;

		copy_pass = ink::CopyPass();
		copy_pass.init();

		fxaa_pass = ink::FXAAPass();
		fxaa_pass.init();

		renderer.set_target(&targets["BaseTarget"]);

		bloom_pass.set_texture(&maps["PostMap0"]);
		bloom_pass.set_target(&targets["PostTarget1"]);

		tone_map_pass.set_texture(&maps["PostMap1"]);
		tone_map_pass.set_target(&targets["PostTarget0"]);

		fxaa_pass.set_texture(&maps["PostMap0"]);
		fxaa_pass.set_target(&targets["PostTarget1"]);

		copy_pass.set_texture(&maps["PostMap1"]);
	}

	void control_camera(const ink::Vec3& move, const ink::Vec2& look_delta, float sensitivity)
	{
		axis_y += look_delta.x * sensitivity;
		axis_z += look_delta.y * sensitivity;
		if (axis_z > ink::PI_2) axis_z = ink::PI_2;
		if (axis_z < -ink::PI_2) axis_z = -ink::PI_2;

		camera.direction.x = std::sin(axis_y) * std::cos(axis_z);
		camera.direction.y = std::sin(axis_z);
		camera.direction.z = std::cos(axis_y) * std::cos(axis_z);

		camera.up.x = -std::sin(axis_y) * std::sin(axis_z);
		camera.up.y = std::cos(axis_z);
		camera.up.z = -std::cos(axis_y) * std::sin(axis_z);

		camera.position += move.z * camera.direction;
		camera.position += move.x * (camera.direction.cross(camera.up));

		camera.lookat(camera.position, -camera.direction, camera.up);
	}

	void update(float delta_time)
	{
		probes["Ground"].position = camera.position;
		probes["Ground"].position.y = -probes["Ground"].position.y;

		ink::Renderer::update_scene(scene);
	}

	void render(const ink::Camera* override_camera = nullptr)
	{
		ink::gpu::Rect viewport_2;
		viewport_2.width = viewport.width * 2;
		viewport_2.height = viewport.height * 2;

		renderer.set_clear_color({0.1, 0.1, 0.1, 1});
		if (enable_realtime_reflection)
		{
			materials["Material.001"].visible = false;
			renderer.update_probe(scene, probes["Ground"]);
			materials["Material.001"].visible = true;
		}

		ink::RenderPass::set_viewport(viewport_2);

		const ink::Camera* available_camera = override_camera;
		if (available_camera == nullptr)
		{
			available_camera = &camera;
		}

		renderer.set_clear_color({1, 1, 1, 1});
		renderer.clear();
		renderer.render(scene, *available_camera);
		renderer.render_transparent(scene, *available_camera);
		bloom_pass.render();
		tone_map_pass.render();
		fxaa_pass.render();

		ink::RenderPass::set_viewport(viewport);

		copy_pass.render();
	}

	void set_joint(const ink::Vec3& position, float radius = 0.015f,
		const ink::Vec3& color = DEFAULT_COLOR, float alpha = 1.0f)
	{
		std::string color_name = color.to_string(3) + std::to_string(alpha);
		materials[color_name].color = color;
		materials[color_name].roughness = 0.2;
		materials[color_name].reflection_probe = &probes["Env"];
		materials[color_name].alpha = alpha;
		materials[color_name].blending = alpha < 1.0f;

		std::string instance_name = "Joint" + std::to_string(joint_number++);

		if (instances.count(instance_name) == 0)
		{
			instances[instance_name] = ink::Instance();
			instances[instance_name].mesh = &meshes["Sphere"];
			scene.add(&instances[instance_name]);
		}
		instances[instance_name].visible = true;
		instances[instance_name].position = position;
		instances[instance_name].scale = {radius * 2, radius * 2, radius * 2};

		scene.set_material("default", instances[instance_name], &materials[color_name]);
	}

	void set_bone(const ink::Vec3& position_a, const ink::Vec3& position_b, float radius = 0.01f,
		const ink::Vec3& color = DEFAULT_COLOR)
	{
		std::string color_name = color.to_string(3);
		materials[color_name].color = color;
		materials[color_name].roughness = 0.2;
		materials[color_name].reflection_probe = &probes["Env"];

		std::string instance_name = "Bone" + std::to_string(bone_number++);
		if (instances.count(instance_name) == 0)
		{
			instances[instance_name] = ink::Instance();
			instances[instance_name].mesh = &meshes["Sphere"];
			scene.add(&instances[instance_name]);
		}
		ink::Vec3 direction = position_a - position_b;
		ink::Euler rotation = make_euler({0, 1, 0}, direction.normalize());
		instances[instance_name].visible = true;
		instances[instance_name].position = (position_a + position_b) * 0.5;
		instances[instance_name].rotation = rotation;
		instances[instance_name].scale = {radius * 2, direction.magnitude() / 2, radius * 2};

		scene.set_material("default", instances[instance_name], &materials[color_name]);
	}

	void clean_up_joints_and_bones()
	{
		for (int index = 0; index < joint_number; ++index)
		{
			instances["Joint" + std::to_string(index)].visible = false;
		}
		joint_number = 0;
		for (int index = 0; index < bone_number; ++index)
		{
			instances["Bone" + std::to_string(index)].visible = false;
		}
		bone_number = 0;
	}

	void set_camera(const ink::Vec3& position, const ink::Vec3& direction)
	{
		camera.position = position;

		ink::Vec3 normalized = -direction.normalize();
		axis_z = std::asin(normalized.y);
		axis_y = std::asin(normalized.x / std::cos(axis_z));
		if (std::isnan(axis_y)) axis_y = 0;
		if (std::cos(axis_y) * normalized.z < 0) axis_y = -axis_y + ink::PI;

		control_camera({0, 0, 0}, {0, 0}, 0);
	}

	const ink::Camera* get_camera() const
	{
		return &camera;
	}

private:
	static ink::Euler make_euler(const ink::Vec3& from, const ink::Vec3& to)
	{
		ink::Vec4 quat;
		float r = from.dot(to) + 1;
		if (r < 0.0001)
		{
			r = 0;
			if (std::fabs(from.x) > std::fabs(from.z))
			{
				quat = {-from.y, from.x, 0, r};
			}
			else
			{
				quat = {0, -from.z, from.y, r};
			}
		}
		else
		{
			quat = {
				from.y * to.z - from.z * to.y,
				from.z * to.x - from.x * to.z,
				from.x * to.y - from.y * to.x,
				r,
			};
		}
		quat = quat.normalize();

		float x = quat.x, y = quat.y, z = quat.z, w = quat.w;
		float x2 = x + x, y2 = y + y, z2 = z + z;
		float xx = x * x2, xy = x * y2, xz = x * z2;
		float yy = y * y2, yz = y * z2, zz = z * z2;
		float wx = w * x2, wy = w * y2, wz = w * z2;
		float m11 = 1 - (yy + zz);
		float m12 = xy - wz;
		float m22 = 1 - (xx + zz);
		float m32 = yz + wx;
		float m13 = xz + wy;
		float m23 = yz - wx;
		float m33 = 1 - (xx + yy);

		ink::Euler euler;
		euler.y = std::asin(m13 < -1 ? -1 : m13 > 1 ? 1 : m13);
		if (std::fabs(m13) < 0.9999)
		{
			euler.x = std::atan2(-m23, m33);
			euler.z = std::atan2(-m12, m11);
		}
		else
		{
			euler.x = std::atan2(m32, m22);
			euler.z = 0;
		}
		return euler;
	}

	bool enable_realtime_reflection = true;

	int joint_number = 0;
	int bone_number = 0;

	float axis_y = 0;
	float axis_z = 0;

	ink::Camera camera;

	ink::gpu::Rect viewport;

	ink::Scene scene;

	ink::Renderer renderer;

	ink::BloomPass bloom_pass;
	ink::ToneMapPass tone_map_pass;
	ink::FXAAPass fxaa_pass;
	ink::CopyPass copy_pass;

	std::unordered_map<std::string, ink::Mesh> meshes;
	std::unordered_map<std::string, ink::Image> images;
	std::unordered_map<std::string, ink::Material> materials;
	std::unordered_map<std::string, ink::ReflectionProbe> probes;
	std::unordered_map<std::string, ink::Instance> instances;

	std::unordered_map<std::string, ink::gpu::Texture> maps;
	std::unordered_map<std::string, ink::gpu::RenderTarget> targets;
};

}
