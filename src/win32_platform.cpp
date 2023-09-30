#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "Ole32.lib")

#include "pch_platform.h"

#include "resource.h"
#include "memory.h"
#include "config.h"
#include "platform_shared.h"
#include "win32_platform.h"

#ifdef m_debug
#include "win32_reload_dll.h"
#endif // m_debug

global s_window g_window;
global s_input g_input;
global s_voice voice_arr[c_max_concurrent_sounds];
global u64 g_cycle_frequency;
global u64 g_start_cycles;
global s_platform_data g_platform_data = zero;

static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

#include "memory.cpp"
#include "platform_shared.cpp"

#ifdef m_debug
int main(int argc, char** argv)
#else // m_debug
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
#endif
{
	#ifdef m_debug
	unreferenced(argc);
	unreferenced(argv);
	#else // m_debug
	unreferenced(hInst);
	unreferenced(hInstPrev);
	unreferenced(cmdline);
	unreferenced(cmdshow);
	#endif

	create_window(c_resolutions[c_base_resolution_index].x, c_resolutions[c_base_resolution_index].y);
	if(!init_audio())
	{
		printf("failed to init audio Aware\n");
	}
	init_performance();

	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)load_gl_func("wglSwapIntervalEXT");

	s_platform_funcs platform_funcs = zero;
	platform_funcs.play_sound = play_sound;
	platform_funcs.load_gl_func = (t_load_gl_func)load_gl_func;
	platform_funcs.set_swap_interval = set_swap_interval;
	platform_funcs.show_cursor = ShowCursor;
	platform_funcs.cycle_between_available_resolutions = cycle_between_available_resolutions;

	#ifdef m_debug
	t_update_game* update_game = null;
	HMODULE dll = null;
	#endif // m_debug
	void* game_memory = null;
	s_lin_arena frame_arena = zero;

	{
		s_lin_arena all = zero;
		all.capacity = 20 * c_mb;

		// @Note(tkap, 26/06/2023): We expect this memory to be zero'd
		all.memory = VirtualAlloc(null, all.capacity, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		game_memory = la_get(&all, c_game_memory);
		frame_arena = make_lin_arena_from_memory(5 * c_mb, la_get(&all, 5 * c_mb));
		g_platform_data.frame_arena = &frame_arena;

	}

	b8 running = true;
	f64 time_passed = 0;
	while(running)
	{
		f64 start_of_frame_seconds = get_seconds();

		MSG msg = zero;
		while(PeekMessage(&msg, null, 0, 0, PM_REMOVE) > 0)
		{
			if(msg.message == WM_QUIT)
			{
				running = false;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		g_platform_data.input = &g_input;
		g_platform_data.quit_after_this_frame = !running;
		g_platform_data.window_width = g_window.width;
		g_platform_data.window_height = g_window.height;
		g_platform_data.time_passed = time_passed;

		#ifdef m_debug
		if(need_to_reload_dll("build/GNOP.dll"))
		{
			if(dll) { unload_dll(dll); }

			for(int i = 0; i < 100; i++)
			{
				if(CopyFile("build/GNOP.dll", "GNOP.dll", false)) { break; }
				Sleep(10);
			}
			dll = load_dll("GNOP.dll");
			update_game = (t_update_game*)GetProcAddress(dll, "update_game");
			assert(update_game);
			printf("Reloaded DLL!\n");
			g_platform_data.recompiled = true;
		}
		#endif // m_debug


		POINT p;
		GetCursorPos(&p);
		ScreenToClient(g_window.handle, &p);
		g_platform_data.mouse.x = (float)p.x;
		g_platform_data.mouse.y = (float)p.y;
		g_platform_data.is_window_active = GetActiveWindow() == g_window.handle;

		update_game(&g_platform_data, platform_funcs, game_memory);
		g_platform_data.recompiled = false;


		SwapBuffers(g_window.dc);

		time_passed = get_seconds() - start_of_frame_seconds;
	}

}

LRESULT window_proc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;
	int key = 0;
	b8 is_down = false;

	switch(msg)
	{

		case WM_CLOSE:
		case WM_DESTROY:
		{
			PostQuitMessage(0);
		} break;

		case WM_SIZE:
		{
			g_window.width = LOWORD(lparam);
			g_window.height = HIWORD(lparam);
		} break;

		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			if(msg == WM_KEYDOWN)
			{
				g_platform_data.any_key_pressed = true;
			}

			key = (int)remap_key_if_necessary(wparam, lparam);
			is_down = !(b32)((HIWORD(lparam) >> 15) & 1);
			int is_echo = is_down && ((lparam >> 30) & 1);
			if(key < c_max_keys && !is_echo)
			{
				s_stored_input si = zero;
				si.key = key;
				si.is_down = is_down;
				apply_event_to_input(&g_input, si);
			}
		} break;

		case WM_LBUTTONDOWN:
			key = c_left_mouse;
			is_down = true;
			goto deez;
		case WM_RBUTTONDOWN:
			key = c_right_mouse;
			is_down = true;
			goto deez;
		case WM_LBUTTONUP:
			key = c_left_mouse;
			is_down = false;
			goto deez;
		case WM_RBUTTONUP:
			key = c_right_mouse;
			is_down = false;
			goto deez;
		{
			deez:
			s_stored_input si = zero;
			si.key = key;
			si.is_down = is_down;
			apply_event_to_input(&g_input, si);
			g_platform_data.any_key_pressed = true;
		} break;

		default:
		{
			result = DefWindowProc(window, msg, wparam, lparam);
		}
	}

	return result;
}

func void create_window(int width, int height)
{
	char* class_name = "GNOP_CLASS";
	HINSTANCE instance = GetModuleHandle(null);

	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = null;
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = null;

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		dummy start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	{
		WNDCLASSEX window_class = zero;
		window_class.cbSize = sizeof(window_class);
		window_class.style = CS_OWNDC;
		window_class.lpfnWndProc = DefWindowProc;
		window_class.lpszClassName = class_name;
		window_class.hInstance = instance;
		check(RegisterClassEx(&window_class));

		HWND dummy_window = CreateWindowEx(
			0,
			class_name,
			"dummy",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			null,
			null,
			instance,
			null
		);
		assert(dummy_window != INVALID_HANDLE_VALUE);

		HDC dc = GetDC(dummy_window);

		PIXELFORMATDESCRIPTOR pfd = zero;
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW;
		pfd.cColorBits = 24;
		pfd.cDepthBits = 24;
		pfd.iLayerType = PFD_MAIN_PLANE;
		int format = ChoosePixelFormat(dc, &pfd);
		check(DescribePixelFormat(dc, format, sizeof(pfd), &pfd));
		check(SetPixelFormat(dc, format, &pfd));

		HGLRC glrc = wglCreateContext(dc);
		check(wglMakeCurrent(dc, glrc));

		wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)load_gl_func("wglCreateContextAttribsARB");
		wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)load_gl_func("wglChoosePixelFormatARB");

		check(wglMakeCurrent(null, null));
		check(wglDeleteContext(glrc));
		check(ReleaseDC(dummy_window, dc));
		check(DestroyWindow(dummy_window));
		check(UnregisterClass(class_name, instance));

	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		dummy end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		window start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	{
		WNDCLASSEX window_class = zero;
		window_class.cbSize = sizeof(window_class);
		window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		window_class.lpfnWndProc = window_proc;
		window_class.lpszClassName = class_name;
		window_class.hInstance = instance;
		window_class.hCursor = LoadCursor(null, IDC_ARROW);
		window_class.hIcon = LoadIcon(instance, MAKEINTRESOURCE(MY_ICON));
		check(RegisterClassEx(&window_class));

		DWORD style = (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX;
		// DWORD style = WS_POPUP | WS_VISIBLE;
		RECT rect = zero;
		rect.right = width;
		rect.bottom = height;
		AdjustWindowRect(&rect, style, false);

		g_window.handle = CreateWindowEx(
			0,
			class_name,
			"GNOP",
			style,
			CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
			null,
			null,
			instance,
			null
		);
		assert(g_window.handle != INVALID_HANDLE_VALUE);

		center_window();

		g_window.dc = GetDC(g_window.handle);

		int pixel_attribs[] = {
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_SWAP_METHOD_ARB, WGL_SWAP_COPY_ARB,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			0
		};

		PIXELFORMATDESCRIPTOR pfd = zero;
		pfd.nSize = sizeof(pfd);
		int format;
		u32 num_formats;
		check(wglChoosePixelFormatARB(g_window.dc, pixel_attribs, null, 1, &format, &num_formats));
		check(DescribePixelFormat(g_window.dc, format, sizeof(pfd), &pfd));
		SetPixelFormat(g_window.dc, format, &pfd);

		int gl_attribs[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 3,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
			0
		};
		HGLRC glrc = wglCreateContextAttribsARB(g_window.dc, null, gl_attribs);
		check(wglMakeCurrent(g_window.dc, glrc));
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		window end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}

func PROC load_gl_func(char* name)
{
	PROC result = wglGetProcAddress(name);
	if(!result)
	{
		printf("Failed to load %s\n", name);
		assert(false);
	}
	return result;
}

// @Note(tkap, 16/05/2023): https://stackoverflow.com/a/15977613
func WPARAM remap_key_if_necessary(WPARAM vk, LPARAM lparam)
{
	WPARAM new_vk = vk;
	UINT scancode = (lparam & 0x00ff0000) >> 16;
	int extended  = (lparam & 0x01000000) != 0;

	switch(vk)
	{
		case VK_SHIFT:
		{
			new_vk = MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX);
		} break;

		case VK_CONTROL:
		{
			new_vk = extended ? VK_RCONTROL : VK_LCONTROL;
		} break;

		case VK_MENU:
		{
			new_vk = extended ? VK_RMENU : VK_LMENU;
		} break;

		default:
		{
			new_vk = vk;
		} break;
	}

	return new_vk;
}

func b8 init_audio()
{
	HRESULT hr = CoInitializeEx(null, COINIT_MULTITHREADED);
	if(FAILED(hr)) { return false; }

	IXAudio2* xaudio2 = null;
	hr = XAudio2Create(&xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if(FAILED(hr)) { return false; }

	IXAudio2MasteringVoice* master_voice = null;
	hr = xaudio2->CreateMasteringVoice(&master_voice);
	if(FAILED(hr)) { return false; }

	WAVEFORMATEX wave = zero;
	wave.wFormatTag = WAVE_FORMAT_PCM;
	wave.nChannels = c_num_channels;
	wave.nSamplesPerSec = c_sample_rate;
	wave.wBitsPerSample = 16;
	wave.nBlockAlign = (c_num_channels * wave.wBitsPerSample) / 8;
	wave.nAvgBytesPerSec = c_sample_rate * wave.nBlockAlign;

	for(int voice_i = 0; voice_i < c_max_concurrent_sounds; voice_i++)
	{
		s_voice* voice = &voice_arr[voice_i];
		hr = xaudio2->CreateSourceVoice(&voice->voice, &wave, 0, XAUDIO2_DEFAULT_FREQ_RATIO, voice, null, null);
		voice->voice->SetVolume(0.25f);
		if(FAILED(hr)) { return false; }
	}

	return true;

}

func b8 play_sound(s_sound sound)
{
	if(!g_platform_data.is_window_active) { return false; }
	assert(sound.sample_count > 0);
	assert(sound.samples);

	XAUDIO2_BUFFER buffer = zero;
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.AudioBytes = sound.sample_count * c_num_channels * sizeof(s16);
	buffer.pAudioData = (BYTE*)sound.samples;

	s_voice* curr_voice = null;
	for(int voice_i = 0; voice_i < c_max_concurrent_sounds; voice_i++)
	{
		s_voice* voice = &voice_arr[voice_i];
		if(!voice->playing)
		{
			if(thread_safe_set_bool_to_true(&voice->playing))
			{
				curr_voice = voice;
				break;
			}
		}
	}

	if(curr_voice == null) { return false; }

	HRESULT hr = curr_voice->voice->SubmitSourceBuffer(&buffer);
	if(FAILED(hr)) { return false; }

	curr_voice->voice->Start();
	// curr_voice->sound = sound;

	return true;
}

func b8 thread_safe_set_bool_to_true(volatile int* var)
{
	return InterlockedCompareExchange((LONG*)var, true, false) == false;
}

func void init_performance()
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&g_cycle_frequency);
	QueryPerformanceCounter((LARGE_INTEGER*)&g_start_cycles);
}

func f64 get_seconds()
{
	u64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	return (now - g_start_cycles) / (f64)g_cycle_frequency;
}


func void set_swap_interval(int interval)
{
	if(wglSwapIntervalEXT)
	{
		wglSwapIntervalEXT(interval);
	}
}

func int cycle_between_available_resolutions(int current)
{
	HMONITOR monitor = MonitorFromWindow(g_window.handle, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO info = zero;
	info.cbSize = sizeof(info);
	BOOL result = GetMonitorInfoA(monitor, &info);
	if(!result) { return current; }
	int monitor_width = info.rcMonitor.right - info.rcMonitor.left;
	int monitor_height = info.rcMonitor.bottom - info.rcMonitor.top;
	if(monitor_width <= 0 || monitor_height <= 0) { return current; }

	int new_index = current;
	while(true)
	{
		new_index = (new_index + 1) % array_count(c_resolutions);
		s_v2i res = c_resolutions[new_index];
		if(res.x > monitor_width || res.y > monitor_height) { continue; }

		if(abs(res.x - monitor_width) < 50 || abs(res.y - monitor_height) < 50)
		{
			SetWindowLongA(g_window.handle, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		}
		else
		{
			SetWindowLongA(g_window.handle, GWL_STYLE, (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
		}

		set_actual_window_size(res.x, res.y);
		center_window();
		break;
	}
	return new_index;

}

func s_v2i set_actual_window_size(int width, int height)
{
	LONG style = GetWindowLongA(g_window.handle, GWL_STYLE);
	RECT rect = zero;
	rect.right = width;
	rect.bottom = height;
	AdjustWindowRect(&rect, style, false);
	int true_width = rect.right - rect.left;
	int true_height = rect.bottom - rect.top;
	SetWindowPos(g_window.handle, null, 0, 0, true_width, true_height, SWP_NOMOVE | SWP_NOZORDER);
	return v2i(true_width, true_height);
}

func void center_window()
{
	HMONITOR monitor = MonitorFromWindow(g_window.handle, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO info = zero;
	info.cbSize = sizeof(info);
	BOOL result = GetMonitorInfoA(monitor, &info);
	if(!result) { return; }

	RECT rect;
	GetWindowRect(g_window.handle, &rect);
	int window_width = rect.right - rect.left;
	int window_height = rect.bottom - rect.top;

	int center_x = (info.rcMonitor.left + info.rcMonitor.right) / 2 - window_width / 2;
	int center_y = (info.rcMonitor.top + info.rcMonitor.bottom) / 2 - window_height / 2;
	SetWindowPos(g_window.handle, null, center_x, center_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}
