#include <SDL2/SDL.h>
#include <GL/gl.h>

#include "pch_platform.h"

#include "resource.h"
#include "memory.h"
#include "config.h"
#include "platform_shared.h"

#include "memory.cpp"
#include "platform_shared.cpp"

func b8 play_sound(s_sound sound)
{
	//TODO
	return true;
}

func int cycle_between_available_resolutions(int current)
{
	int new_index = current;
	return new_index;
}

func void set_swap_interval(int interval)
{
	SDL_GL_SetSwapInterval(interval);
}


func f64 get_seconds(u64 start_cycles, f64 cycle_frequency)
{
	u64 now = SDL_GetPerformanceCounter();
	return (now - start_cycles) / cycle_frequency;
}

extern "C" void update_game(s_platform_data *platform_data, s_platform_funcs platform_funcs, void* game_memory);

int platform_show_cursor(BOOL b)
{
	int v=-1;
	if(b==0)
		v=SDL_DISABLE;
	else if(b==1)
		v=SDL_ENABLE;
	SDL_ShowCursor(v);
	return b;
}


func s_v2i set_actual_window_size(int width, int height)
{
	STUB(__func__);
	return {width, height};
}

func void center_window()
{
	STUB(__func__);
}



int internal_key(SDL_Keycode k)
{
	static const SDL_Keycode sdl_keycodes[] = {
		SDLK_ESCAPE,
		SDLK_F1,
		SDLK_F2,
		SDLK_F3,
		SDLK_F4,
		SDLK_F8,
		SDLK_F9,
		SDLK_UP,
		SDLK_DOWN,
		SDLK_RETURN,
		SDLK_a,
		SDLK_d,
		SDLK_f,
		SDLK_q,
		SDLK_r,
		SDLK_SPACE,
	};
	static const int internal_keycodes[] = {
		c_key_escape,
		c_key_f1,
		c_key_f2,
		c_key_f3,
		c_key_f4,
		c_key_f8,
		c_key_f9,
		c_key_up,
		c_key_down,
		c_key_enter,
		c_key_a,
		c_key_d,
		c_key_f,
		c_key_q,
		c_key_r,
		c_key_space,
	};
	for (int i = 0; i < (int)array_count(sdl_keycodes); ++i)
		if (k == sdl_keycodes[i])
			return internal_keycodes[i];
	return -1;
}


int main(void)
{
	s_input logic_input;
	s_input input;
	int window_width;
	int window_height;
	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetSwapInterval(1);
	SDL_Window *window = SDL_CreateWindow("Gnop", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, c_base_res.x, c_base_res.y, SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL);
	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, gl_context);
	SDL_ShowWindow(window);
	SDL_GetWindowSize(window, &window_width, &window_height);
	f64 cycle_frequency = (f64)SDL_GetPerformanceFrequency();
	u64 start_cycles = SDL_GetPerformanceCounter();

	// TODO: don't like passing platform funcs
	s_platform_funcs platform_funcs = zero;
	platform_funcs.play_sound = play_sound;
	platform_funcs.load_gl_func = (t_load_gl_func)SDL_GL_GetProcAddress;
	platform_funcs.set_swap_interval = set_swap_interval;
	platform_funcs.show_cursor = platform_show_cursor;
	platform_funcs.cycle_between_available_resolutions = cycle_between_available_resolutions;

	void* game_memory = null;
	s_lin_arena frame_arena = zero;
	s_lin_arena all = zero;
	all.capacity = 20 * c_mb;
	// @Note(tkap, 26/06/2023): We expect this memory to be zero'd
	all.memory = calloc(all.capacity, 1);
	game_memory = la_get(&all, c_game_memory);
	frame_arena = make_lin_arena_from_memory(5 * c_mb, la_get(&all, 5 * c_mb));
	s_platform_data platform_data = zero;
	platform_data.frame_arena = &frame_arena;
	b8 running = true;
	f64 time_passed = 0;
	s32 mousex;
	s32 mousey;
	b8 is_window_active = false;
	while(running)
	{
		f64 start_of_frame_seconds = get_seconds(start_cycles, cycle_frequency);
		SDL_Event ev;
		b8 need_size_restore = 0;
		// TODO: what did i have this variable for?
		(void)need_size_restore;
		int key = -1;
		while(SDL_PollEvent(&ev)) {
			switch(ev.type) {
			case SDL_QUIT:
				running = false;
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				if(ev.type == SDL_KEYDOWN)
					platform_data.any_key_pressed = true;
				key = internal_key(ev.key.keysym.sym);
				if(key >= 0) {
					if(key == c_key_q)
						if (ev.key.keysym.mod & KMOD_CTRL)
							running = 0;
					if(key == c_key_f4)
						if (ev.key.keysym.mod & KMOD_ALT)
							running = 0;
					input.keys[key].count += 1;
					input.keys[key].is_down = ev.type == SDL_KEYDOWN;
					logic_input.keys[key].count += 1;
					logic_input.keys[key].is_down = ev.type == SDL_KEYDOWN;
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				key = -1;
				if(ev.button.button == SDL_BUTTON_LEFT)
					key = c_left_mouse;
				else if(ev.button.button == SDL_BUTTON_RIGHT)
					key = c_right_mouse;
				if(key>=0){
					input.keys[key].count=1;
					input.keys[key].is_down=ev.type==SDL_MOUSEBUTTONDOWN;
					logic_input.keys[key].count=1;
					logic_input.keys[key].is_down=ev.type==SDL_MOUSEBUTTONDOWN;
				}
				platform_data.any_key_pressed = true;
				break;
			case SDL_WINDOWEVENT:
				switch(ev.window.event) {
					case SDL_WINDOWEVENT_ENTER:
					case SDL_WINDOWEVENT_FOCUS_GAINED:
					case SDL_WINDOWEVENT_RESTORED:
						if (ev.window.event== SDL_WINDOWEVENT_RESTORED)
							need_size_restore = true;
						is_window_active = true;
						break;
					case SDL_WINDOWEVENT_LEAVE:
					case SDL_WINDOWEVENT_FOCUS_LOST:
					case SDL_WINDOWEVENT_MINIMIZED:
					case SDL_WINDOWEVENT_HIDDEN:
					case SDL_WINDOWEVENT_CLOSE:
						if (ev.window.event== SDL_WINDOWEVENT_MINIMIZED)
							need_size_restore = true;
						is_window_active = false;
						if (ev.window.event == SDL_WINDOWEVENT_CLOSE)
							running = false;
						break;
					case SDL_WINDOWEVENT_MAXIMIZED:
						need_size_restore = true;
						break;
					case SDL_WINDOWEVENT_RESIZED:
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						window_width = ev.window.data1;
						window_height = ev.window.data2;
						break;
				}
				break;
			case SDL_MOUSEMOTION:
				mousex = ev.motion.x;
				mousey = ev.motion.y;
				break;
			}
		}

		platform_data.input = &input;
		platform_data.logic_input = &logic_input;
		platform_data.quit_after_this_frame = !running;
		platform_data.window_width = window_width;
		platform_data.window_height = window_height;
		platform_data.time_passed = time_passed;

		platform_data.mouse.x = (float)mousex,
		platform_data.mouse.y = (float)mousey;
		platform_data.is_window_active = is_window_active;

		update_game(&platform_data, platform_funcs, game_memory);

		SDL_GL_SwapWindow(window);

		time_passed = get_seconds(start_cycles, cycle_frequency) - start_of_frame_seconds;
	}

}
