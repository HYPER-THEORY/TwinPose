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

#include "opengl/glad.h"

#include "ink/ink.h"

#include "imgui.h"
#include "imgui_spectrum.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include <SDL.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <utility>

namespace v3
{

class Application
{
public:
	static constexpr int LEFT_MOUSE_BUTTON = 1;
	static constexpr int RIGHT_MOUSE_BUTTON = 2;

	Application() = default;

	~Application()
	{
		shutdown();
	}

	bool init(const std::string& title, int width, int height, int fps = 60, bool high_dpi = false)
	{
		this->window_width = width;
		this->window_height = height;
		this->interval = fps > 0 ? 1000 / fps : 0;

		std::fill_n(key_down, KEY_NUM, false);
		std::fill_n(key_pressed, KEY_NUM, false);

		if (high_dpi)
		{
			SDL_SetHint(SDL_HINT_WINDOWS_DPI_SCALING, "1");
		}

		if (SDL_Init(SDL_INIT_VIDEO) != 0)
		{
			std::cerr << "Failed to initialize SDL: " << SDL_GetError() << '\n';
			return false;
		}

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

		uint32_t flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
		if (high_dpi)
		{
			flags |= SDL_WINDOW_ALLOW_HIGHDPI;
		}
		sdl_window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, width, height, flags);
		if (sdl_window == nullptr)
		{
			std::cerr << "Failed to create window: " << SDL_GetError() << '\n';
			return false;
		}

		gl_context = SDL_GL_CreateContext(sdl_window);
		if (gl_context == nullptr)
		{
			std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << '\n';
			return false;
		}
		SDL_GL_MakeCurrent(sdl_window, gl_context);
		SDL_GL_SetSwapInterval(0);

		SDL_GL_GetDrawableSize(sdl_window, &drawable_width, &drawable_height);
		dpi_scale = window_width > 0 ? float(drawable_width) / float(window_width) : 1.0f;

		if (gladLoadGL() == 0)
		{
			std::cerr << "Failed to load OpenGL\n";
			return false;
		}

		ink::RenderPass::set_viewport(ink::gpu::Rect(drawable_width, drawable_height));
		ink::load_shaders();

		ink::Error::set_callback([](const std::string& message) -> void
		{
			std::cerr << message << '\n';
		});

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGui::Spectrum::LoadFont(16.0f * dpi_scale);
		ImGui::Spectrum::StyleColorsSpectrum();

		if (dpi_scale != 1.0f)
		{
			ImGui::GetIO().FontGlobalScale = 1.0f / dpi_scale;
		}

		ImGui_ImplSDL2_InitForOpenGL(sdl_window, gl_context);
		ImGui_ImplOpenGL3_Init("#version 150");

		open = true;
		start_time = SDL_GetTicks();
		last_time = start_time;

		return true;
	}

	void run(const std::function<void(float)>& update)
	{
		while (open)
		{
			poll_events();

			uint32_t now = SDL_GetTicks();
			float delta_time = (now - last_time) * 0.001f;
			last_time = now;

			ink::gpu::RenderTarget::activate(nullptr);
			ink::gpu::State::set_clear_color({0.1, 0.1, 0.1}, 1);
			ink::gpu::State::clear();

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplSDL2_NewFrame();
			ImGui::NewFrame();

			if (update)
			{
				update(delta_time);
			}

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			SDL_GL_SwapWindow(sdl_window);

			std::string error = ink::gpu::State::get_error();
			if (!error.empty()) std::cerr << error;

			if (interval > 0)
			{
				uint32_t frame_time = SDL_GetTicks() - now;
				if (frame_time < interval) SDL_Delay(interval - frame_time);
			}
		}
	}

	void shutdown()
	{
		if (!initialized_imgui())
		{
			return;
		}

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		if (gl_context != nullptr)
		{
			SDL_GL_DeleteContext(gl_context);
			gl_context = nullptr;
		}
		if (sdl_window != nullptr)
		{
			SDL_DestroyWindow(sdl_window);
			sdl_window = nullptr;
		}
		SDL_Quit();
	}

	bool is_open() const
	{
		return open;
	}

	void close()
	{
		open = false;
	}

	bool is_key_down(int key_code) const
	{
		return key_down[remap_key(key_code)];
	}

	bool is_key_pressed(int key_code) const
	{
		return key_pressed[remap_key(key_code)];
	}

	bool is_mouse_down(int button) const
	{
		return key_down[button];
	}

	ink::Vec2 get_mouse_motion() const
	{
		return mouse_motion;
	}

	bool want_capture_mouse() const
	{
		return ImGui::GetIO().WantCaptureMouse;
	}

	bool want_capture_keyboard() const
	{
		return ImGui::GetIO().WantCaptureKeyboard;
	}

	std::pair<int, int> get_size() const
	{
		return {window_width, window_height};
	}

	std::pair<int, int> get_framebuffer_size() const
	{
		return {drawable_width, drawable_height};
	}

	float get_dpi_scale() const
	{
		return dpi_scale;
	}

	void set_title(const std::string& title)
	{
		SDL_SetWindowTitle(sdl_window, title.c_str());
	}

private:
	static constexpr int KEY_NUM = 512;

	static int remap_key(int key_code)
	{
		if (key_code > 127) key_code -= 0x3FFFFF80;
		if (key_code < 0 || key_code >= KEY_NUM) return 0;
		return key_code;
	}

	bool initialized_imgui() const
	{
		return ImGui::GetCurrentContext() != nullptr;
	}

	void poll_events()
	{
		std::fill_n(key_pressed, KEY_NUM, false);
		mouse_motion = {0, 0};

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);

			if (event.type == SDL_QUIT)
			{
				open = false;
				return;
			}
			else if (event.type == SDL_KEYDOWN)
			{
				int key = remap_key(event.key.keysym.sym);
				key_pressed[key] = !key_down[key];
				key_down[key] = true;
			}
			else if (event.type == SDL_KEYUP)
			{
				int key = remap_key(event.key.keysym.sym);
				key_down[key] = false;
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				if (event.button.button == SDL_BUTTON_LEFT) key_down[LEFT_MOUSE_BUTTON] = true;
				else if (event.button.button == SDL_BUTTON_RIGHT) key_down[RIGHT_MOUSE_BUTTON] = true;
			}
			else if (event.type == SDL_MOUSEBUTTONUP)
			{
				if (event.button.button == SDL_BUTTON_LEFT) key_down[LEFT_MOUSE_BUTTON] = false;
				else if (event.button.button == SDL_BUTTON_RIGHT) key_down[RIGHT_MOUSE_BUTTON] = false;
			}
			else if (event.type == SDL_MOUSEMOTION)
			{
				mouse_motion.x += event.motion.xrel;
				mouse_motion.y += event.motion.yrel;
			}
		}
	}

	bool open = false;

	int window_width = 1280;
	int window_height = 720;
	int drawable_width = 1280;
	int drawable_height = 720;
	float dpi_scale = 1.0f;
	uint32_t interval = 0;

	uint32_t start_time = 0;
	uint32_t last_time = 0;

	SDL_Window* sdl_window = nullptr;
	SDL_GLContext gl_context = nullptr;

	bool key_down[KEY_NUM] = {};
	bool key_pressed[KEY_NUM] = {};

	ink::Vec2 mouse_motion = {0, 0};
};

}
