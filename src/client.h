
enum e_sprite
{
	e_sprite_player,
	e_sprite_grass,
	e_sprite_dirt,
	e_sprite_stone,
	e_sprite_clay,
	e_sprite_emerald,
	e_sprite_ruby,
	e_sprite_unbreakable,
	e_sprite_damage_0,
	e_sprite_damage_1,
	e_sprite_damage_2,
	e_sprite_rect,
	e_sprite_count,
};

enum e_layer
{
	e_layer_background,
	e_layer_tiles,
	e_layer_tiles_decal,
	e_layer_portal,
	e_layer_player,
	e_layer_particles,
	e_layer_kill_area,
	e_layer_exp_bar,
};

enum e_sound
{
	e_sound_break_tile,
	e_sound_break_gem,
	e_sound_dig,
	e_sound_count,
};

enum e_upgrade
{
	e_upgrade_dig_speed,
	e_upgrade_dig_range,
	e_upgrade_movement_speed,
	e_upgrade_health,
	e_upgrade_extra_jump,
	e_upgrade_count,
};

enum e_state
{
	e_state_main_menu,
	e_state_game,
	e_state_victory,
	e_state_count,
};

enum e_font
{
	e_font_small,
	e_font_medium,
	e_font_big,
	e_font_huge,
	e_font_count,
};

struct s_glyph
{
	int advance_width;
	int width;
	int height;
	int x0;
	int y0;
	int x1;
	int y1;
	s_v2 uv_min;
	s_v2 uv_max;
};

struct s_font
{
	float size;
	float scale;
	int ascent;
	int descent;
	int line_gap;
	s_texture texture;
	s_glyph glyph_arr[1024];
};

enum e_shader
{
	e_shader_default,
	e_shader_count,
};

struct s_shader_paths
{
	#ifdef m_debug
	FILETIME last_write_time;
	#endif // m_debug
	char* vertex_path;
	char* fragment_path;
};

struct s_ui_state
{
	b8 clicked;
};

struct s_ui_foo
{
	u32 id;
	int index;
};

struct s_ui
{
	s_ui_foo selected;
	s_ui_foo hovered;
	s_ui_foo pressed;
};

struct s_label_group
{
	int default_selected;
	float spacing;
	s_v2 pos;
	e_font font_type;
	s_sarray<u32, 8> ids;
};

struct s_entity
{
	union
	{
		struct
		{
			float x;
			float y;
		};
		s_v2 pos;
	};
	union
	{
		struct
		{
			float prev_x;
			float prev_y;
		};
		s_v2 prev_pos;
	};
};

struct s_particle_spawn_data
{
	float speed;
	float speed_rand;
	float radius;
	float radius_rand;
	float duration;
	float duration_rand;
	float angle;
	float angle_rand;
	s_v2 pos;
	s_v4 color;
	float color_rand;
	b8 shrink = true;
	b8 fade = true;
};

struct s_particle
{
	float time;
	float duration;
	float speed;
	s_v2 pos;
	s_v2 dir;
	float radius;
	s_v4 color;
	b8 shrink;
	b8 fade;
};

struct s_delayed_sound
{
	float time;
	float delay;
	s_sound sound;
};

struct s_player : s_entity
{
	b8 jumping;
	int jumps_done;
	int level;
	int exp;
	int damage_taken;
	float dig_timer;
	s_v2 vel;
};

struct s_portal : s_entity
{
	s_v2 target_pos;
};

struct s_sprite_data
{
	s_v2i pos;
	s_v2i size;
};

enum e_tile
{
	e_tile_grass,
	e_tile_dirt,
	e_tile_stone,
	e_tile_clay,
	e_tile_emerald,
	e_tile_ruby,
	e_tile_unbreakable,
	e_tile_count,
};

struct s_tile_data
{
	s64 weight;
	float weight_add;
	int exp;
	int sprite;
	int health;
	e_sound break_sound;
	s_v4 particle_color;
};

global constexpr s_tile_data g_tile_data[] = {
	{
		.weight = 1000,
		.weight_add = -2,
		.exp = 1,
		.sprite = e_sprite_grass,
		.health = 3,
		.break_sound = e_sound_break_tile,
		.particle_color = rgb(0x2CA65F),
	},
	{
		.weight = 900,
		.weight_add = -1,
		.exp = 2,
		.sprite = e_sprite_dirt,
		.health = 4,
		.break_sound = e_sound_break_tile,
		.particle_color = rgb(0x9E492D),
	},
	{
		.weight = 5,
		.weight_add = 20,
		.exp = 3,
		.sprite = e_sprite_stone,
		.health = 5,
		.break_sound = e_sound_break_tile,
		.particle_color = rgb(0x222222),
	},
	{
		.weight = 4,
		.weight_add = 20,
		.exp = 4,
		.sprite = e_sprite_clay,
		.health = 6,
		.break_sound = e_sound_break_tile,
		.particle_color = rgb(0xA5B6C0),
	},

	// @Note(tkap, 30/09/2023): emerald
	{
		.weight = 3,
		.weight_add = 0.2f,
		.exp = 20,
		.sprite = e_sprite_emerald,
		.health = 1,
		.break_sound = e_sound_break_gem,
		.particle_color = rgb(0x56C4A5),
	},

	// @Note(tkap, 30/09/2023): ruby
	{
		.weight = 1,
		.weight_add = 0.1f,
		.exp = 40,
		.sprite = e_sprite_ruby,
		.health = 1,
		.break_sound = e_sound_break_gem,
		.particle_color = rgb(0xF43F3A),
	},

	// @Note(tkap, 30/09/2023): unbreakable
	{
		.weight = 10,
		.weight_add = 1.5f,
		.exp = 0,
		.sprite = e_sprite_unbreakable,
		.health = -1,
	},
};

struct s_tile
{
	int type;
	int damage_taken;
};

struct s_bounds
{
	union
	{
		struct
		{
			float left;
			float top;
		};
		s_v2 min_p;
	};

	union
	{
		struct
		{
			float right;
			float bottom;
		};
		s_v2 max_p;
	};
};

struct s_camera
{
	s_v2 prev_center;
	s_v2 center;
};

struct s_tile_collision
{
	b8 collided;
	s_v2 tile_center;
};

struct s_game_transient
{
	b8 in_upgrade_menu;
	b8 winning;
	b8 won;
	int upgrade_index;
	s_sarray<int, 3> upgrade_choices;
	int upgrades_chosen[e_upgrade_count];
	s_player player;
	s_sarray<s_portal, 128> portals;
	float kill_area_timer;
	float kill_area_speed;
	float winning_timer;
	int upgrades_queued;
	f64 beat_time;
};

struct s_game
{
	b8 initialized;
	b8 in_debug_menu;
	b8 reset_game;
	b8 high_speed;
	b8 no_kill_area;
	b8 player_bounds;
	b8 high_gravity;
	b8 super_dig;
	b8 camera_bounds;
	f64 best_time;

	float sound_times[e_sound_count];

	s_game_transient transient;

	#ifdef m_debug
	int debug_menu_index;
	#endif // m_debug

	int current_resolution_index;
	int current_level;
	int score;
	int level_count;
	int update_count;
	int frame_count;
	float total_time;
	float kill_area_bottom;
	s_v2 title_pos;
	s_v3 title_color;
	f64 update_timer;
	s_rng rng;
	e_state state;
	s_font font_arr[e_font_count];
	s_camera camera;

	s_sarray<s_particle, 16384> particles;
	s_sarray<s_delayed_sound, 64> delayed_sounds;

	s_sound sounds[e_sound_count];

	u32 default_vao;
	u32 default_ssbo;

	u32 particle_fbo;
	u32 particle_texture;

	s_tile tiles[c_tiles_down][c_tiles_right];
	b8 tiles_active[c_tiles_down][c_tiles_right];

	s_sprite_data sprite_data[e_sprite_count];

	u32 programs[e_shader_count];

	s_texture noise;
	s_texture atlas;
};

func void update();
func void render(float dt);
func b8 check_for_shader_errors(u32 id, char* out_error);
func void input_system(int start, int count);
func void draw_system(int start, int count, float dt);
func void revive_every_player(void);
func void draw_circle_system(int start, int count, float dt);
func void collision_system(int start, int count);
func s_font load_font(const char* path, float font_size, s_lin_arena* arena);
func s_texture load_texture_from_data(void* data, int width, int height, u32 filtering);
func s_v2 get_text_size(const char* text, e_font font_id);
func s_v2 get_text_size_with_count(const char* text, e_font font_id, int count);
func u32 load_shader(const char* vertex_path, const char* fragment_path);
func void handle_instant_movement_(int entity);
func void handle_instant_resize_(int entity);
func b8 is_key_down(s_input* input, int key);
func b8 is_key_up(s_input* input, int key);
func b8 is_key_pressed(s_input* input, int key);
func b8 is_key_released(s_input* input, int key);
void gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
func void sine_alpha_system(int start, int count);
func char* handle_plural(int num);
func s_texture load_texture_from_file(char* path, u32 filtering);
func void spawn_particles(int count, s_particle_spawn_data data);
func void play_delayed_sound(s_sound sound, float delay);
func void reset_level();
func s_bounds get_camera_bounds(s_camera camera);
func s_tile_collision get_tile_collision(s_v2 pos, s_v2 size);
func b8 is_valid_tile_index(int x, int y);
func s_v2 get_world_mouse(s_camera camera);
func s_v2i point_to_tile(s_v2 pos);
func s_v2i get_hovered_tile(s_camera camera);
func b8 is_valid_tile_index(s_v2i p);
func float get_dig_delay();
func float get_dig_range();
func b8** get_debug_vars();
func int get_max_jumps();
func void trigger_upgrade_menu();
func b8 mouse_collides_rect_topleft(s_v2 mouse, s_v2 pos, s_v2 size);
func void apply_upgrade(int index);
func char* get_upgrade_description(int id, int level);
func float get_upgrade_value(int id, int level);
func float get_movement_speed();
func int get_max_health();
func s64* get_tile_weights_for_y(int y, int count);
func int pick_from_weights(s64* weights, int count);
func void add_exp(int exp);
func int get_required_exp_to_level_up(int level);
func void add_upgrade_to_queue();
func float get_max_y_vel();
func void begin_winning();
func b8 is_tile_active(int x, int y);
func b8 is_tile_active(s_v2i index);
func s_v2i get_closest_tile_to_mouse(s_camera camera);
func s_v2 get_tile_center(int x, int y);
func s_v2 get_tile_center(s_v2i index);
func s_v2 world_to_screen(s_v2 pos, s_camera cam);
func s_v2 get_tile_pos(s_v2i index);
func void do_normal_render(u32 texture, int render_type);
func void do_tile_particles(s_v2 pos, int tile_type, int type);
func void recreate_particle_framebuffer(int width, int height);
func s_label_group begin_label_group(s_v2 pos, e_font font_type, int selected, float spacing);
func s_ui_state add_label(s_label_group* group, char* text);
func int end_label_group(s_label_group* group);
func u32 hash(const char* text);
func void ui_request_hovered(u32 id, int index);
func void ui_request_pressed(u32 id, int index);
func void ui_request_active(u32 id);
func void ui_request_selected(u32 id, int index);
func s_v2 get_camera_wanted_center(s_player player);

#ifdef m_debug
func void hot_reload_shaders(void);
#endif // m_debug
