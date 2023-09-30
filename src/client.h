
enum e_sprite
{
	e_sprite_player,
	e_sprite_dirt,
	e_sprite_grass,
	e_sprite_damage_0,
	e_sprite_damage_1,
	e_sprite_damage_2,
	e_sprite_rect,
	e_sprite_count,
};

enum e_layer
{
	e_layer_tiles,
	e_layer_tiles_decal,
	e_layer_player,
	e_layer_kill_area,
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

struct s_entity
{
	int id;
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
	s8 render_type;
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
};

struct s_particle
{
	s8 render_type;
	float time;
	float duration;
	float speed;
	s_v2 pos;
	s_v2 dir;
	float radius;
	s_v4 color;
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
	int damage_taken;
	float dig_timer;
	s_v2 vel;
};

struct s_sprite_data
{
	s_v2i pos;
	s_v2i size;
};

struct s_tile
{
	int sprite_index;
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
	int current_depth_index;
	int upgrade_index;
	s_sarray<int, 3> upgrade_choices;
	int upgrades_chosen[e_upgrade_count];
};

struct s_game
{
	b8 initialized;
	b8 in_debug_menu;
	b8 reset_game;
	b8 high_speed;
	b8 player_bounds;
	b8 super_dig;
	b8 camera_bounds;

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
	float kill_area_timer;
	s_v2 title_pos;
	s_v3 title_color;
	f64 update_timer;
	s_rng rng;
	e_state state;
	s_font font_arr[e_font_count];
	s_player player;
	s_camera camera;

	s_sarray<s_particle, 16384> particles;
	s_sarray<s_delayed_sound, 64> delayed_sounds;

	u32 default_vao;
	u32 default_ssbo;

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
func s_v2 tile_index_to_tile_center(s_v2i index);
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

#ifdef m_debug
func void hot_reload_shaders(void);
#endif // m_debug
