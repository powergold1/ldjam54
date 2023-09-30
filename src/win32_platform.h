


struct s_window
{
	HDC dc;
	HWND handle;
	int width;
	int height;
};

struct s_voice : IXAudio2VoiceCallback
{
	IXAudio2SourceVoice* voice;

	volatile int playing;

	void OnStreamEnd()
	{
		voice->Stop();
		InterlockedExchange((LONG*)&playing, false);
	}

	#pragma warning(push, 0)
	void OnBufferStart(void * pBufferContext) {}
	void OnVoiceProcessingPassEnd() { }
	void OnVoiceProcessingPassStart(UINT32 SamplesRequired) { unreferenced(SamplesRequired); }
	void OnBufferEnd(void * pBufferContext) { unreferenced(pBufferContext); }
	void OnLoopEnd(void * pBufferContext) { unreferenced(pBufferContext); }
	void OnVoiceError(void * pBufferContext, HRESULT Error) { unreferenced(pBufferContext); unreferenced(Error);}
	#pragma warning(pop)
};



func void create_window(int width, int height);
func WPARAM remap_key_if_necessary(WPARAM vk, LPARAM lparam);
func PROC load_gl_func(char* name);
func b8 init_audio();
func b8 thread_safe_set_bool_to_true(volatile int* var);
func b8 play_sound(s_sound sound);
func void init_performance();
func f64 get_seconds();
func void set_swap_interval(int interval);
func int cycle_between_available_resolutions(int current);
func void center_window();
func s_v2i set_actual_window_size(int width, int height);