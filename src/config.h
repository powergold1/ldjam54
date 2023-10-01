

global constexpr s_v2 c_base_res = {1366, 768};
global constexpr s_v2 c_half_res = {1366 / 2.0f, 768 / 2.0f};

global constexpr int c_num_channels = 2;
global constexpr int c_sample_rate = 44100;
global constexpr int c_max_concurrent_sounds = 16;

global constexpr int c_updates_per_second = 60;
global constexpr f64 c_update_delay = 1.0 / c_updates_per_second;

#define c_origin_topleft {1.0f, -1.0f}
#define c_origin_bottomleft {1.0f, 1.0f}
#define c_origin_center {0, 0}

global constexpr int c_max_entities = 4096;

global constexpr float c_delta = 1.0f / c_updates_per_second;

global constexpr int c_base_resolution_index = 5;
global constexpr s_v2i c_resolutions[] = {
	v2i(640, 360),
	v2i(854, 480),
	v2i(960, 540),
	v2i(1024, 576),
	v2i(1280, 720),
	v2i(1366, 768),
	v2i(1600, 900),
	v2i(1920, 1080),
	v2i(2560, 1440),
	v2i(3200, 1800),
	v2i(3840, 2160),
	v2i(5120, 2880),
	v2i(7680, 4320),
};

global constexpr float c_gravity = 1500;
global constexpr s_v2 c_player_size = v2(32 * 0.8f, 64 * 0.8f);
global constexpr int c_tile_size = 64;
global constexpr int c_atlas_size = 256;
global constexpr int c_tiles_right = 64;
global constexpr int c_tiles_down = 512;
global constexpr float c_player_speed = 300;
global constexpr float c_dig_delay = 0.15f;
global constexpr float c_dig_range = 150;
global constexpr float c_kill_area_delay = 1;
global constexpr int c_player_health = 3;

global constexpr char* debug_text[] = {
	"High speed", "Super dig", "Player bounds", "Camera bounds", "High gravity",
};

global constexpr float c_depth_goals[] = {
	1000, 2000, 3000, 4000, 6000, 80000, 10000, 15000, 20000
};