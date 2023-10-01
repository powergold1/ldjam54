
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "opengl32.lib")

#include "pch_client.h"

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_assert assert
#include "external/stb_truetype.h"

#pragma warning(push, 0)
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT assert
#include "external/stb_image.h"
#pragma warning(pop)



#include "config.h"
#include "platform_shared.h"


struct s_texture
{
	u32 id;
	s_v2 size;
	s_v2 sub_size;
};


#include "client.h"
#include "shader_shared.h"
#include "audio.h"

global s_sarray<s_transform, 16384> transforms;
global s_sarray<s_transform, 16384> particles;
global s_sarray<s_transform, c_max_entities> text_arr[e_font_count];

global s_lin_arena* frame_arena;

global s_game_window g_window;
global s_input* g_logic_input;
global s_input* g_input;

global s_platform_data* g_platform_data;
global s_platform_funcs g_platform_funcs;

global s_game* game;

global s_v2 previous_mouse;
global s_ui g_ui;

global s_shader_paths shader_paths[e_shader_count] = {
	{
		.vertex_path = "shaders/vertex.vertex",
		.fragment_path = "shaders/fragment.fragment",
	},
};


#define X(type, name) static type name = null;
m_gl_funcs
#undef X

#include "draw.cpp"
#include "memory.cpp"
#include "file.cpp"
#include "str_builder.cpp"
#include "audio.cpp"

#ifdef m_debug
extern "C" {
__declspec(dllexport)
#endif // m_debug
m_update_game(update_game)
{
	static_assert(c_game_memory >= sizeof(s_game));
	game = (s_game*)game_memory;
	frame_arena = platform_data->frame_arena;
	g_platform_funcs = platform_funcs;
	g_platform_data = platform_data;
	g_input = platform_data->input;
	g_logic_input = platform_data->logic_input;

	if(!game->initialized)
	{
		game->initialized = true;
		#define X(type, name) name = (type)platform_funcs.load_gl_func(#name);
		m_gl_funcs
		#undef X

		game->sounds[e_sound_break_tile] = load_wav("assets/break.wav", g_platform_data->frame_arena);
		game->sounds[e_sound_break_gem] = load_wav("assets/break_gem.wav", g_platform_data->frame_arena);
		game->sounds[e_sound_dig] = load_wav("assets/dig.wav", g_platform_data->frame_arena);
		game->sounds[e_sound_player_hurt] = load_wav("assets/player_hurt.wav", g_platform_data->frame_arena);
		game->sounds[e_sound_portal] = load_wav("assets/portal.wav", g_platform_data->frame_arena);
		game->sounds[e_sound_win] = load_wav("assets/win.wav", g_platform_data->frame_arena);
		game->sounds[e_sound_jump] = load_wav("assets/jump.wav", g_platform_data->frame_arena);

		game->best_time = 999999.0f;

		recreate_particle_framebuffer(platform_data->window_width, platform_data->window_height);

		game->player_bounds = true;
		game->camera_bounds = true;

		game->current_resolution_index = c_base_resolution_index;

		glDebugMessageCallback(gl_debug_callback, null);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

		game->rng.seed = (u32)__rdtsc();
		game->reset_game = true;

		game->state = e_state_main_menu;

		platform_funcs.set_swap_interval(1);

		game->noise = load_texture_from_file("assets/noise.png", GL_LINEAR);
		game->atlas = load_texture_from_file("assets/atlas.png", GL_NEAREST);

		game->font_arr[e_font_small] = load_font("assets/consola.ttf", 24, frame_arena);
		game->font_arr[e_font_medium] = load_font("assets/consola.ttf", 36, frame_arena);
		game->font_arr[e_font_big] = load_font("assets/consola.ttf", 72, frame_arena);
		game->font_arr[e_font_huge] = load_font("assets/consola.ttf", 256, frame_arena);

		for(int shader_i = 0; shader_i < e_shader_count; shader_i++)
		{
			game->programs[shader_i] = load_shader(shader_paths[shader_i].vertex_path, shader_paths[shader_i].fragment_path);
		}

		glGenVertexArrays(1, &game->default_vao);
		glBindVertexArray(game->default_vao);

		glGenBuffers(1, &game->default_ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, game->default_ssbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, game->default_ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(transforms.elements), null, GL_DYNAMIC_DRAW);

	}

	if(platform_data->recompiled)
	{
		#define X(type, name) name = (type)platform_funcs.load_gl_func(#name);
		m_gl_funcs
		#undef X

		game->sprite_data[e_sprite_dirt].pos = v2i(0, 0);
		game->sprite_data[e_sprite_dirt].size = v2i(16, 16);

		game->sprite_data[e_sprite_grass].pos = v2i(0, 16);
		game->sprite_data[e_sprite_grass].size = v2i(16, 16);

		game->sprite_data[e_sprite_stone].pos = v2i(16, 32);
		game->sprite_data[e_sprite_stone].size = v2i(16, 16);

		game->sprite_data[e_sprite_clay].pos = v2i(16, 48);
		game->sprite_data[e_sprite_clay].size = v2i(16, 16);

		game->sprite_data[e_sprite_unbreakable].pos = v2i(16, 64);
		game->sprite_data[e_sprite_unbreakable].size = v2i(16, 16);

		game->sprite_data[e_sprite_emerald].pos = v2i(16, 80);
		game->sprite_data[e_sprite_emerald].size = v2i(16, 16);

		game->sprite_data[e_sprite_ruby].pos = v2i(16, 96);
		game->sprite_data[e_sprite_ruby].size = v2i(16, 16);

		game->sprite_data[e_sprite_damage_0].pos = v2i(0, 32);
		game->sprite_data[e_sprite_damage_0].size = v2i(16, 16);

		game->sprite_data[e_sprite_damage_1].pos = v2i(0, 48);
		game->sprite_data[e_sprite_damage_1].size = v2i(16, 16);

		game->sprite_data[e_sprite_damage_2].pos = v2i(0, 64);
		game->sprite_data[e_sprite_damage_2].size = v2i(16, 16);

		game->sprite_data[e_sprite_rect].pos = v2i(32, 0);
		game->sprite_data[e_sprite_rect].size = v2i(16, 16);

	}

	if(platform_data->window_resized)
	{
		recreate_particle_framebuffer(platform_data->window_width, platform_data->window_height);
	}

	g_window.width = platform_data->window_width;
	g_window.height = platform_data->window_height;
	g_window.size = v2ii(g_window.width, g_window.height);
	g_window.center = v2_mul(g_window.size, 0.5f);

	#ifdef m_debug
	if(is_key_pressed(g_input, c_key_f8))
	{
		write_file("save_state", game, sizeof(*game));
	}

	if(is_key_pressed(g_input, c_key_f9))
	{
		char* data = read_file("save_state", frame_arena, null);
		if(data)
		{
			memcpy(game, data, sizeof(*game));
		}
	}
	#endif // m_debug


	if(is_key_pressed(g_input, c_key_f2))
	{
		game->current_resolution_index = g_platform_funcs.cycle_between_available_resolutions(game->current_resolution_index);
		g_platform_data->any_key_pressed = false;
	}
	g_platform_data->mouse.x *= c_base_res.x / (float)g_window.width;
	g_platform_data->mouse.y *= c_base_res.y / (float)g_window.height;

	game->update_timer += g_platform_data->time_passed;
	game->frame_count += 1;

	while(game->update_timer >= c_update_delay)
	{
		game->update_timer -= c_update_delay;

		game->transient.player.prev_pos = game->transient.player.pos;
		game->camera.prev_center = game->camera.center;
		update();

		for(int k_i = 0; k_i < c_max_keys; k_i++)
		{
			g_logic_input->keys[k_i].count = 0;
		}
		g_platform_data->any_key_pressed = false;
	}

	float interpolation_dt = (float)(game->update_timer / c_update_delay);
	render(interpolation_dt);

	// memset(game->e.drawn_last_render, true, sizeof(game->e.drawn_last_render));

	game->total_time += (float)platform_data->time_passed;

	frame_arena->used = 0;

	for(int k_i = 0; k_i < c_max_keys; k_i++)
	{
		g_input->keys[k_i].count = 0;
	}
}

#ifdef m_debug
}
#endif // m_debug

func void update()
{
	game->update_count += 1;
	float delta = c_delta;
	// g_platform_funcs.show_cursor(false);

	switch(game->state)
	{
		case e_state_main_menu:
		{
			game->title_pos = v2(
				c_half_res.x,
				range_lerp(sinf(game->total_time * 2), -1, 1, c_base_res.y * 0.2f, c_base_res.y * 0.4f)
			);
			game->title_color = v3(sinf2(game->total_time), 1, 1);

			if(g_platform_data->any_key_pressed)
			{
				game->state = e_state_game;
				reset_level();
			}
		} break;

		case e_state_game:
		{
			// s_rng* rng = &game->rng;
			game->transient.beat_time += delta;

			if(game->transient.in_upgrade_menu)
			{
				delta = 0;
			}

			#ifdef m_debug
			if(is_key_pressed(g_logic_input, c_key_f1))
			{
				game->in_debug_menu = !game->in_debug_menu;
			}

			if(is_key_pressed(g_logic_input, c_key_f3))
			{
				add_upgrade_to_queue();
			}

			if(!game->transient.won && is_key_pressed(g_logic_input, c_key_f4))
			{
				begin_winning();
			}

			if(game->in_debug_menu)
			{
				b8** target_vars = get_debug_vars();

				if(is_key_pressed(g_logic_input, c_key_up))
				{
					game->debug_menu_index = circular_index(game->debug_menu_index - 1, array_count(debug_text));
				}
				if(is_key_pressed(g_logic_input, c_key_down))
				{
					game->debug_menu_index = circular_index(game->debug_menu_index + 1, array_count(debug_text));
				}
				if(is_key_pressed(g_logic_input, c_key_enter))
				{
					*target_vars[game->debug_menu_index] = !*target_vars[game->debug_menu_index];
				}
			}
			#endif // m_debug


			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		reset game start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			if(game->reset_game)
			{
				game->reset_game = false;
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		reset game end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			if(!game->no_kill_area)
			{
				float speed = get_kill_area_speed();
				game->kill_area_bottom += delta * speed;
				game->transient.kill_area_speed += 0.5f * delta;
			}
			game->transient.kill_area_timer = at_most(c_kill_area_delay + delta, game->transient.kill_area_timer + delta);

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		update player start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_player* player = &game->transient.player;

				int how_many_blocks_can_dash_break = get_how_many_blocks_can_dash_break();
				float dash_cd = get_dash_cd();
				b8 can_dash = player->state != e_player_state_dashing && player->dash_cd_time >= dash_cd;
				if(can_dash && game->transient.upgrades_chosen[e_upgrade_dash] > 0 && is_key_pressed(g_logic_input, c_key_f))
				{
					player->dash_cd_time = 0;
					player->state = e_player_state_dashing;
					player->dash_time = 0;
					player->dash_dir = v2_normalized(get_world_mouse(game->camera) - player->pos);
					player->blocks_broken_with_dash = 0;
					player->vel.y = 0;
				}

				b8 dashing = player->state == e_player_state_dashing;

				int x_dir = 0;
				if(is_key_down(g_logic_input, c_key_a))
				{
					x_dir += -1;
					player->flip_x = true;
				}
				if(is_key_down(g_logic_input, c_key_d))
				{
					x_dir += 1;
					player->flip_x = false;
				}
				if(x_dir != 0 || dashing)
				{
					if(dashing)
					{
						player->flip_x = player->dash_dir.x > 0 ? false : true;
					}
					set_animation(player, e_animation_player_run);
				}
				else
				{
					set_animation(player, e_animation_player_idle);
				}

				if(dashing)
				{
					x_dir = player->dash_dir.x > 0 ? 1 : -1;
					player->x += player->dash_dir.x * c_dash_speed * delta;
					player->dash_time += delta;
					if(player->dash_time >= c_dash_duration)
					{
						player->state = e_player_state_default;
					}
				}
				else
				{
					player->x += get_movement_speed() * x_dir * delta;
				}

				if(!dashing)
				{
					player->dash_cd_time = at_most(dash_cd + delta, player->dash_cd_time + delta);
				}

				if(game->player_bounds)
				{
					player->x = at_least(c_player_size.x / 2, player->x);
					player->x = at_most(c_tiles_right * c_tile_size - c_player_size.x / 2, player->x);
				}

				// @Note(tkap, 30/09/2023): x collision
				{
					s_tile_collision collision = get_tile_collision(player->pos, c_player_size);
					if(collision.collided)
					{
						s_v2 center = get_tile_center(collision.index);
						if(dashing)
						{
							int type = game->tiles[collision.index.y][collision.index.x].type;
							if(g_tile_data[type].health != -1)
							{
								damage_tile(collision.index, 9999);
								player->blocks_broken_with_dash += 1;
								if(player->blocks_broken_with_dash >= how_many_blocks_can_dash_break)
								{
									player->state = e_player_state_default;
								}
							}
						}
						player->x = center.x - x_dir * (c_tile_size / 2 + c_player_size.x / 2) - x_dir * 0.01f;
					}
				}

				if(!dashing)
				{
					player->vel.y += c_gravity * delta;
					player->vel.y = at_most(get_max_y_vel(), player->vel.y);
					if(player->vel.y > 0)
					{
						player->state = e_player_state_default;
					}
				}

				if(player->state == e_player_state_jumping)
				{
					if(!is_key_down(g_logic_input, c_key_space))
					{
						player->state = e_player_state_default;
						player->vel.y *= 0.5f;
					}
				}

				b8 can_jump = player->jumps_done < get_max_jumps();
				if(can_jump && is_key_pressed(g_logic_input, c_key_space))
				{
					player->jumps_done += 1;
					play_sound_if_supported(e_sound_jump);
					player->state = e_player_state_jumping;
					player->vel.y = c_gravity * -0.42f;
				}

				if(dashing)
				{
					player->y += player->dash_dir.y * c_dash_speed * delta;
				}
				else
				{
					player->y += player->vel.y * delta;
				}

				// @Note(tkap, 30/09/2023): y collision
				{
					int y_dir = player->vel.y > 0 ? 1 : -1;
					if(dashing)
					{
						y_dir = player->dash_dir.y > 0 ? 1 : -1;
					}
					s_tile_collision collision = get_tile_collision(player->pos, c_player_size);
					if(collision.collided)
					{
						s_v2 center = get_tile_center(collision.index);
						if(dashing)
						{
							int type = game->tiles[collision.index.y][collision.index.x].type;
							if(g_tile_data[type].health != -1)
							{
								damage_tile(collision.index, 9999);
								player->blocks_broken_with_dash += 1;
								if(player->blocks_broken_with_dash >= how_many_blocks_can_dash_break)
								{
									player->state = e_player_state_default;
								}
							}
						}
						else
						{
							player->state = e_player_state_default;
							player->vel.y = 0;
							if(y_dir == 1)
							{
								player->jumps_done = 0;
							}
						}
						player->y = center.y - y_dir * (c_tile_size / 2 + c_player_size.y / 2) - y_dir * 0.01f;
					}
				}

				// @Note(tkap, 01/10/2023): Check for portal
				foreach_raw(portal_i, portal, game->transient.portals)
				{
					float distance = v2_distance(portal.pos, player->pos);
					if(distance < c_portal_size * 0.5f)
					{
						player->pos = portal.target_pos;
						play_sound_if_supported(e_sound_portal);
					}

					if(delta > 0)
					{
						spawn_particles(1, {
							.speed = 0.0f,
							.radius = 128.0f,
							.duration = delta + 0.0001f,
							.angle = 0,
							.pos = portal.pos,
							.color = v4(0.3f, 0.1f, 1.0f, c_portal_brightness),
							.shrink = false,
							.fade = false,
						});
				}
				}

				// @Note(tkap, 30/09/2023): Check for win
				if(!game->transient.winning && !game->transient.won)
				{
					int y_index = floorfi(player->y / (float)c_tile_size);
					if(y_index == c_tiles_down - 2)
					{
						begin_winning();
					}
				}

				game->camera.center = lerp_snap(game->camera.center, get_camera_wanted_center(*player), delta * 10);

				float dig_delay = get_dig_delay();
				player->dig_timer = at_most(dig_delay + delta, player->dig_timer + delta);
				if(delta > 0 && is_key_down(g_logic_input, c_left_mouse))
				{
					while(player->dig_timer >= dig_delay)
					{
						s_v2i hovered = get_closest_tile_to_mouse(game->camera);
						if(!is_valid_tile_index(hovered) || !is_tile_active(hovered)) { break; }
						s_tile tile = game->tiles[hovered.y][hovered.x];
						s_v2 tile_center = get_tile_center(hovered);
						float dist = v2_distance(player->pos, tile_center);
						if(dist > get_dig_range()) { break; }

						player->dig_timer -= dig_delay;
						int health = g_tile_data[tile.type].health;
						if(health != -1)
						{
							damage_tile(hovered, 1);
						}
					}
				}

				if(player->y < game->kill_area_bottom - 100)
				{
					if(game->transient.kill_area_timer >= c_kill_area_delay)
					{
						game->transient.kill_area_timer -= c_kill_area_delay;
						player->damage_taken += 1;
						play_sound_if_supported(e_sound_player_hurt);
						if(player->damage_taken >= get_max_health())
						{
							reset_level();
						}
					}
				}

				// @Note(tkap, 01/10/2023): Player "light"
				if(delta > 0)
				{
					spawn_particles(1, {
						.speed = 0.0f,
						.radius = 256.0f,
						.duration = delta + 0.0001f,
						.angle = 0,
						.pos = player->pos,
						.color = v4(1, 1, 1, c_light_brightness),
						.shrink = false,
						.fade = false,
					});
				}

				player->animation_timer += delta;

			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		update player end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		winning start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				if(game->transient.winning)
				{
					game->transient.winning_timer += delta;
					if(game->transient.winning_timer >= 3.0f)
					{
						game->state = e_state_victory;
						if(game->best_time > game->transient.beat_time)
						{
							game->best_time = game->transient.beat_time;
						}
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		winning end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			if(is_key_pressed(g_logic_input, c_key_r))
			{
				reset_level();
			}

		} break;

		case e_state_victory:
		{
		} break;

		invalid_default_case;

	}

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		update particles start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	foreach(particle_i, particle, game->particles)
	{
		particle->time += delta;
		particle->pos += particle->dir * particle->speed * delta;
		if(particle->time >= particle->duration)
		{
			game->particles.remove_and_swap(particle_i--);
		}
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		update particles end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		delayed sounds start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	foreach(sound_i, sound, game->delayed_sounds)
	{
		sound->time += delta;
		if(sound->time >= sound->delay)
		{
			g_platform_funcs.play_sound(sound->sound);
			game->delayed_sounds.remove_and_swap(sound_i--);
		}
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		delayed sounds end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

}

func void render(float dt)
{
	s_camera interpolated_camera = game->camera;
	switch(game->state)
	{
		case e_state_main_menu:
		{
			s_v2i res = c_resolutions[game->current_resolution_index];
			draw_text("DigHard", game->title_pos, 15, v4(hsv_to_rgb(game->title_color), 1), e_font_huge, true);
			draw_text("Press Any Key to Start", v2(c_half_res.x, c_base_res.y * 0.7f), 15, v4(1), e_font_big, true);
			draw_text(
				format_text("Press F2 to change resolution (%ix%i)", res.x, res.y), v2(c_half_res.x, c_base_res.y * 0.8f), 15, v4(1), e_font_medium, true
			);
			draw_text("Made Live at twitch.tv/Tkap1", v2(c_half_res.x, c_base_res.y * 0.9f), 15, make_color(0.5f), e_font_medium, true);
		} break;

		case e_state_game:
		{
			interpolated_camera.center = lerp(interpolated_camera.prev_center, interpolated_camera.center, dt);
			s_bounds cam_bounds = get_camera_bounds(interpolated_camera);

			// @Note(tkap, 01/10/2023): Draw timer
			draw_text(format_text("%.1f", game->transient.beat_time), c_base_res * v2(0.93f, 0.05f), 15, make_color(1), e_font_small, false);

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		exp bar start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				float bar_height = 24.0f;
				int bars = 10;

				draw_texture(
					v2(0, 0), e_layer_exp_bar, v2(c_base_res.x, bar_height), make_color(0.1f), game->sprite_data[e_sprite_rect], false, dt,
					{.origin_offset = c_origin_topleft}
				);
				for(int i = 0; i < bars; i++)
				{
					float p = i / (float)(bars - 1);
					draw_texture(
						v2(c_base_res.x * p - 4, 0.0f), e_layer_exp_bar, v2(8.0f, bar_height), make_color(0.25f), game->sprite_data[e_sprite_rect], false, dt,
						{.sublayer = 2, .origin_offset = c_origin_topleft}
					);
				}
				float exp_percent = game->transient.player.exp / (float)get_required_exp_to_level_up(game->transient.player.level);
				float mix_weight = 0;
				float elapsed = game->total_time - game->transient.exp_gained_time;
				if(elapsed <= 1)
				{
					mix_weight = 1 - powf(elapsed, 0.75f);
				}
				draw_texture(
						v2(0.0f, 0.0f), e_layer_exp_bar, v2(c_base_res.x * exp_percent, bar_height), rgb(0x41ACBF), game->sprite_data[e_sprite_rect], false, dt,
						{.sublayer = 1, .mix_weight = mix_weight, .origin_offset = c_origin_topleft}
					);
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		exp bar end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// @Note(tkap, 01/10/2023): Draw player
			{
				s_player player = game->transient.player;
				s_v2 pos = lerp(player.prev_pos, player.pos, dt);
				draw_texture(
					pos, e_layer_player, c_player_size, make_color(c_player_brightness),
					get_animation_frame(player.current_animation, player.animation_timer), true, dt, {.flip_x = player.flip_x}
				);
			}

			{
				s_v2 pos = v2(cam_bounds.left, cam_bounds.top);
				if(game->kill_area_bottom > pos.y)
				{
					draw_texture(
						pos, e_layer_kill_area, v2(c_base_res.x, game->kill_area_bottom - pos.y), v4(1,1,1,1), game->sprite_data[e_sprite_rect], true, dt,
						{.effect_id = 1, .origin_offset = c_origin_topleft}
					);
				}
			}

			// @Note(tkap, 01/10/2023): Background
			draw_texture(
				c_half_res, e_layer_background, c_base_res, make_color(1), game->sprite_data[e_sprite_rect], false, dt,
				{.effect_id = 2}
			);

			int left_tile = at_least(0, floorfi(cam_bounds.left / c_tile_size));
			int right_tile = at_most(c_tiles_right, ceilfi(cam_bounds.right / c_tile_size));
			int top_tile = at_least(0, floorfi(cam_bounds.top / c_tile_size));
			int bottom_tile = at_most(c_tiles_down, ceilfi(cam_bounds.bottom / c_tile_size));

			s_v2i hovered = get_closest_tile_to_mouse(interpolated_camera);
			for(int y = top_tile; y < bottom_tile; y++)
			{
				for(int x = left_tile; x < right_tile; x++)
				{
					if(!is_tile_active(x, y)) { continue; }

					s_tile tile = game->tiles[y][x];
					s_v4 mix_color = v4(1, 1, 1, 1);
					float mix_weight = 0;
					if(hovered == v2i(x, y))
					{
						mix_weight = 0.25f;
					}

					// @Note(tkap, 01/10/2023): Draw tiles
					draw_texture(
						v2(x * c_tile_size, y * c_tile_size), e_layer_tiles, v2(c_tile_size), make_color(c_tile_brightness),
						game->sprite_data[g_tile_data[tile.type].sprite],
						true, dt, {.mix_weight = mix_weight, .origin_offset = c_origin_topleft, .mix_color = mix_color}
					);

					if(tile.damage_taken > 0)
					{
						int damage_sprite = e_sprite_damage_0;
						float damage_percent = tile.damage_taken / (float)g_tile_data[tile.type].health;
						if(damage_percent >= 0.66f) { damage_sprite = e_sprite_damage_2; }
						else if(damage_percent >= 0.33f) { damage_sprite = e_sprite_damage_1; }
						draw_texture(
							v2(x * c_tile_size, y * c_tile_size), e_layer_tiles_decal, v2(c_tile_size), v4(1,1,1,1),
							game->sprite_data[damage_sprite],
							true, dt, {.mix_weight = mix_weight, .origin_offset = c_origin_topleft, .mix_color = mix_color}
						);
					}

					if(!game->transient.in_upgrade_menu && game->frame_count % 10 == 0 && (tile.type == e_tile_emerald || tile.type == e_tile_ruby))
					{
						s_v4 color = g_tile_data[tile.type].particle_color;
						color.w *= 0.1f;
						spawn_particles(2, {
							.speed = 30.0f,
							.radius = 32.0f,
							.duration = 2.5f,
							.angle = 0,
							.angle_rand = 1,
							.pos = get_tile_center(x, y),
							.color = color,
						});
					}
				}
			}

			// @Note(tkap, 01/10/2023): Portals
			foreach_raw(portal_i, portal, game->transient.portals)
			{
				draw_texture(
					portal.pos, e_layer_portal, v2(c_portal_size), make_color(0, 1, 0), game->sprite_data[e_sprite_rect], true, dt,
					{.effect_id = 3}
				);
				// draw_texture(portal.target_pos, e_layer_portal, v2(c_tile_size), make_color(1, 0, 0), game->sprite_data[e_sprite_player], true, dt);
			}

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		upgrade menu start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				if(game->transient.in_upgrade_menu)
				{
					e_font font_type = e_font_medium;
					float spacing = game->font_arr[font_type].size * 1.2f;

					draw_texture(
						c_half_res, 10, c_half_res, rgb(0.1f, 0.1f, 0.1f), game->sprite_data[e_sprite_rect], false, dt
					);

					s_label_group group = begin_label_group(c_half_res - v2(0.0f, spacing * 1.5f - game->font_arr[font_type].size / 2), font_type, game->transient.upgrade_index, spacing);
					for(int upgrade_i = 0; upgrade_i < 3; upgrade_i++)
					{
						int upgrade_id = game->transient.upgrade_choices[upgrade_i];
						int upgrade_level = game->transient.upgrades_chosen[upgrade_id];
						char* text = get_upgrade_description(upgrade_id, upgrade_level);
						s_ui_state state = add_label(&group, text);
						if(state.clicked)
						{
							apply_upgrade(upgrade_id);
						}
					}
					game->transient.upgrade_index = end_label_group(&group);
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		upgrade menu end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			#ifdef m_debug
			if(game->in_debug_menu)
			{

				b8** target_vars = get_debug_vars();
				s_v2 pos = v2(4, 4);
				e_font font_type = e_font_medium;
				for(int option_i = 0; option_i < array_count(debug_text); option_i++)
				{
					s_v4 color = zero;
					if(game->debug_menu_index == option_i)
					{
						color = v4(1, 1, 0, 1);
					}
					else if(*target_vars[option_i])
					{
						color = v4(0, 1, 0, 1);
					}
					else
					{
						color = v4(1, 0, 0, 1);
					}
					draw_text(debug_text[option_i], pos, 15, color, font_type, false);
					pos.y += game->font_arr[font_type].size;
				}
			}
			#endif // m_debug

			if(game->transient.beat_time < 5)
			{
				float alpha = game->transient.beat_time >= 4 ? (range_lerp((float)game->transient.beat_time, 4, 5, 1, 0)) : 1.0f;
				draw_texture(
					c_base_res * v2(0.82f, 0.1f) - v2(4), 14, v2(220, 100), v4(0.25f, 0.25f, 0.25f, alpha),
					game->sprite_data[e_sprite_rect], false, dt,
					{.origin_offset = c_origin_topleft}
				);
				draw_text("A/D: Movement", c_base_res * v2(0.82f, 0.1f), 15, v4(1, 1, 1, alpha), e_font_small, false);
				draw_text("Space: Jump", c_base_res * v2(0.82f, 0.13f), 15, v4(1, 1, 1, alpha), e_font_small, false);
				draw_text("Left click: Dig", c_base_res * v2(0.82f, 0.16f), 15, v4(1, 1, 1, alpha), e_font_small, false);
				draw_text("R: Restart", c_base_res * v2(0.82f, 0.19f), 15, v4(1, 1, 1, alpha), e_font_small, false);
			}

		} break;

		case e_state_victory:
		{
			draw_text("Congratulations! You win!", v2(c_half_res.x, c_base_res.y * 0.1f), 15, v4(hsv_to_rgb(game->title_color), 1), e_font_big, true);
			draw_text(format_text("%.1f seconds", game->transient.beat_time), c_base_res * v2(0.5f, 0.2f), 15, rgb(1, 1, 1), e_font_big, true);
			if(game->best_time < game->transient.beat_time)
			{
				draw_text(format_text("(record is %.1f seconds)", game->best_time), c_base_res * v2(0.5f, 0.3f), 15, make_color(0.75f), e_font_medium, true);
			}
			else
			{
				draw_text(format_text("New record!", game->best_time), c_base_res * v2(0.5f, 0.3f), 15, make_color(0.75f), e_font_medium, true);
			}
			draw_text("Press Enter to keep playing", v2(c_half_res.x, c_base_res.y * 0.45f), 15, v4(0.8f), e_font_medium, true);
			draw_text("Press R to restart", v2(c_half_res.x, c_base_res.y * 0.5f), 15, v4(0.8f), e_font_medium, true);
			draw_text("Press Escape to exit", v2(c_half_res.x, c_base_res.y * 0.55f), 15, v4(0.8f), e_font_medium, true);
			draw_text("Get the source code at github.com/Tkap1/ldjam54", v2(c_half_res.x, c_base_res.y * 0.8f), 15, make_color(0.5f), e_font_medium, true);
			draw_text("Made live at twitch.tv/Tkap1", v2(c_half_res.x, c_base_res.y * 0.9f), 15, make_color(0.5f), e_font_medium, true);

			if(is_key_pressed(g_input, c_key_enter))
			{
				game->state = e_state_game;
				game->transient.winning = false;
			}
			if(is_key_pressed(g_input, c_key_r))
			{
				game->state = e_state_game;
				reset_level();
			}
			if(is_key_pressed(g_input, c_key_escape))
			{
				ExitProcess(0);
			}
		} break;

		invalid_default_case;
	}

	foreach_raw(particle_i, particle, game->particles)
	{
		s_v4 color = particle.color;
		float percent = (particle.time / particle.duration);
		float percent_left = 1.0f - percent;
		if(particle.fade)
		{
			color.w *= powf(percent_left, 0.5f);
		}
		float radius = particle.radius;
		if(particle.shrink)
		{
			radius *= range_lerp(percent, 0, 1, 1, 0.2f);
		}
		draw_circle_p(particle.pos, e_layer_particles, radius, color, &interpolated_camera);
	}

	{
		glUseProgram(game->programs[e_shader_default]);
		// glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, g_window.width, g_window.height);

		{
			int location = glGetUniformLocation(game->programs[e_shader_default], "window_size");
			s_v2 window_size = v2(g_window.width, g_window.height);
			glUniform2fv(location, 1, &window_size.x);
		}
		{
			int location = glGetUniformLocation(game->programs[e_shader_default], "base_res");
			glUniform2fv(location, 1, &c_base_res.x);
		}
		{
			int location = glGetUniformLocation(game->programs[e_shader_default], "time");
			glUniform1f(location, game->total_time);
		}
		{
			s_v2 player_pos = lerp(game->transient.player.prev_pos, game->transient.player.pos, dt);
			int location = glGetUniformLocation(game->programs[e_shader_default], "player_pos");
			glUniform2fv(location, 1, &player_pos.x);
		}

		if(transforms.count > 0)
		{
			do_normal_render(game->atlas.id, 0);
			transforms.count = 0;
		}

		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		render particles start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		if(particles.count > 0)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, game->particle_fbo);
			glViewport(0, 0, g_window.width, g_window.height);
			// glViewport(0, 0, (int)c_base_res.x, (int)c_base_res.y);
			glClear(GL_COLOR_BUFFER_BIT);
			glBindVertexArray(game->default_vao);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			// glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			glBlendFunc(GL_ONE, GL_ONE);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(*particles.elements) * particles.count, particles.elements);
			glDrawArraysInstanced(GL_TRIANGLES, 0, 6, particles.count);
			particles.count = 0;

			draw_fbo(game->particle_texture);
			do_normal_render(game->particle_texture, 1);
			transforms.count = 0;
		}
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		render particles end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		for(int font_i = 0; font_i < e_font_count; font_i++)
		{
			glBindVertexArray(game->default_vao);
			if(text_arr[font_i].count > 0)
			{
				s_font* font = &game->font_arr[font_i];
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, font->texture.id);
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(*text_arr[font_i].elements) * text_arr[font_i].count, text_arr[font_i].elements);
				glDrawArraysInstanced(GL_TRIANGLES, 0, 6, text_arr[font_i].count);
				text_arr[font_i].count = 0;
			}
		}
	}

	#ifdef m_debug
	hot_reload_shaders();
	#endif // m_debug
}

func b8 check_for_shader_errors(u32 id, char* out_error)
{
	int compile_success;
	char info_log[1024];
	glGetShaderiv(id, GL_COMPILE_STATUS, &compile_success);

	if(!compile_success)
	{
		glGetShaderInfoLog(id, 1024, null, info_log);
		log("Failed to compile shader:\n%s", info_log);

		if(out_error)
		{
			strcpy(out_error, info_log);
		}

		return false;
	}
	return true;
}

func s_font load_font(const char* path, float font_size, s_lin_arena* arena)
{
	s_font font = zero;
	font.size = font_size;

	u8* file_data = (u8*)read_file(path, arena);
	assert(file_data);

	stbtt_fontinfo info = zero;
	stbtt_InitFont(&info, file_data, 0);

	stbtt_GetFontVMetrics(&info, &font.ascent, &font.descent, &font.line_gap);

	font.scale = stbtt_ScaleForPixelHeight(&info, font_size);
	constexpr int max_chars = 128;
	int bitmap_count = 0;
	u8* bitmap_arr[max_chars];
	const int padding = 10;

	int columns = floorfi(4096 / (font_size + padding));
	int rows = ceilfi((max_chars - columns) / (float)columns) + 1;

	int total_width = floorfi(columns * (font_size + padding));
	int total_height = floorfi(rows * (font_size + padding));

	for(int char_i = 0; char_i < max_chars; char_i++)
	{
		s_glyph glyph = zero;
		u8* bitmap = stbtt_GetCodepointBitmap(&info, 0, font.scale, char_i, &glyph.width, &glyph.height, 0, 0);
		stbtt_GetCodepointBox(&info, char_i, &glyph.x0, &glyph.y0, &glyph.x1, &glyph.y1);
		stbtt_GetGlyphHMetrics(&info, char_i, &glyph.advance_width, null);

		font.glyph_arr[char_i] = glyph;
		bitmap_arr[bitmap_count++] = bitmap;
	}

	// @Fixme(tkap, 23/06/2023): Use arena
	u8* gl_bitmap = (u8*)calloc(1, sizeof(u8) * 4 * total_width * total_height);

	for(int char_i = 0; char_i < max_chars; char_i++)
	{
		s_glyph* glyph = &font.glyph_arr[char_i];
		u8* bitmap = bitmap_arr[char_i];
		int column = char_i % columns;
		int row = char_i / columns;
		for(int y = 0; y < glyph->height; y++)
		{
			for(int x = 0; x < glyph->width; x++)
			{
				int current_x = floorfi(column * (font_size + padding));
				int current_y = floorfi(row * (font_size + padding));
				u8 src_pixel = bitmap[x + y * glyph->width];
				u8* dst_pixel = &gl_bitmap[((current_x + x) + (current_y + y) * total_width) * 4];
				dst_pixel[0] = 255;
				dst_pixel[1] = 255;
				dst_pixel[2] = 255;
				dst_pixel[3] = src_pixel;
			}
		}

		glyph->uv_min.x = column / (float)columns;
		glyph->uv_max.x = glyph->uv_min.x + (glyph->width / (float)total_width);

		glyph->uv_min.y = row / (float)rows;

		// @Note(tkap, 17/05/2023): For some reason uv_max.y is off by 1 pixel (checked the texture in renderoc), which causes the text to be slightly miss-positioned
		// in the Y axis. "glyph->height - 1" fixes it.
		glyph->uv_max.y = glyph->uv_min.y + (glyph->height / (float)total_height);

		// @Note(tkap, 17/05/2023): Otherwise the line above makes the text be cut off at the bottom by 1 pixel...
		// glyph->uv_max.y += 0.01f;
	}

	font.texture = load_texture_from_data(gl_bitmap, total_width, total_height, GL_LINEAR);

	for(int bitmap_i = 0; bitmap_i < bitmap_count; bitmap_i++)
	{
		stbtt_FreeBitmap(bitmap_arr[bitmap_i], null);
	}

	free(gl_bitmap);

	return font;
}

func s_texture load_texture_from_data(void* data, int width, int height, u32 filtering)
{
	assert(data);
	u32 id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);

	s_texture texture = zero;
	texture.id = id;
	texture.size = v22i(width, height);
	return texture;
}

func s_texture load_texture_from_file(char* path, u32 filtering)
{
	int width, height, num_channels;
	void* data = stbi_load(path, &width, &height, &num_channels, 4);
	s_texture texture = load_texture_from_data(data, width, height, filtering);
	stbi_image_free(data);
	return texture;
}

func s_v2 get_text_size_with_count(const char* text, e_font font_id, int count)
{
	assert(count >= 0);
	if(count <= 0) { return zero; }
	s_font* font = &game->font_arr[font_id];

	s_v2 size = zero;
	size.y = font->size;

	for(int char_i = 0; char_i < count; char_i++)
	{
		char c = text[char_i];
		s_glyph glyph = font->glyph_arr[c];
		size.x += glyph.advance_width * font->scale;
	}

	return size;
}

func s_v2 get_text_size(const char* text, e_font font_id)
{
	return get_text_size_with_count(text, font_id, (int)strlen(text));
}


#ifdef m_debug
func void hot_reload_shaders(void)
{
	for(int shader_i = 0; shader_i < e_shader_count; shader_i++)
	{
		s_shader_paths* sp = &shader_paths[shader_i];

		WIN32_FIND_DATAA find_data = zero;
		HANDLE handle = FindFirstFileA(sp->fragment_path, &find_data);
		if(handle == INVALID_HANDLE_VALUE) { continue; }

		if(CompareFileTime(&sp->last_write_time, &find_data.ftLastWriteTime) == -1)
		{
			// @Note(tkap, 23/06/2023): This can fail because text editor may be locking the file, so we check if it worked
			u32 new_program = load_shader(sp->vertex_path, sp->fragment_path);
			if(new_program)
			{
				if(game->programs[shader_i])
				{
					glUseProgram(0);
					glDeleteProgram(game->programs[shader_i]);
				}
				game->programs[shader_i] = load_shader(sp->vertex_path, sp->fragment_path);
				sp->last_write_time = find_data.ftLastWriteTime;
			}
		}

		FindClose(handle);
	}

}
#endif // m_debug

func u32 load_shader(const char* vertex_path, const char* fragment_path)
{
	u32 vertex = glCreateShader(GL_VERTEX_SHADER);
	u32 fragment = glCreateShader(GL_FRAGMENT_SHADER);
	const char* header = "#version 430 core\n";
	char* vertex_src = read_file(vertex_path, frame_arena);
	if(!vertex_src || !vertex_src[0]) { return 0; }
	char* fragment_src = read_file(fragment_path, frame_arena);
	if(!fragment_src || !fragment_src[0]) { return 0; }

	#ifdef m_debug
	const char* vertex_src_arr[] = {header, read_file("src/shader_shared.h", frame_arena), vertex_src};
	const char* fragment_src_arr[] = {header, read_file("src/shader_shared.h", frame_arena), fragment_src};
	#else // m_debug
	const char* vertex_src_arr[] = {header, vertex_src};
	const char* fragment_src_arr[] = {header, fragment_src};
	#endif // m_debug
	glShaderSource(vertex, array_count(vertex_src_arr), (const GLchar * const *)vertex_src_arr, null);
	glShaderSource(fragment, array_count(fragment_src_arr), (const GLchar * const *)fragment_src_arr, null);
	glCompileShader(vertex);
	char buffer[1024] = zero;
	check_for_shader_errors(vertex, buffer);
	glCompileShader(fragment);
	check_for_shader_errors(fragment, buffer);
	u32 program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	return program;
}

func b8 is_key_down(s_input* input, int key)
{
	assert(key < c_max_keys);
	return input->keys[key].is_down || input->keys[key].count >= 2;
}

func b8 is_key_up(s_input* input, int key)
{
	assert(key < c_max_keys);
	return !input->keys[key].is_down;
}

func b8 is_key_pressed(s_input* input, int key)
{
	assert(key < c_max_keys);
	return (input->keys[key].is_down && input->keys[key].count == 1) || input->keys[key].count > 1;
}

func b8 is_key_released(s_input* input, int key)
{
	assert(key < c_max_keys);
	return (!input->keys[key].is_down && input->keys[key].count == 1) || input->keys[key].count > 1;
}

void gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	unreferenced(userParam);
	unreferenced(length);
	unreferenced(id);
	unreferenced(type);
	unreferenced(source);
	if(severity >= GL_DEBUG_SEVERITY_HIGH)
	{
		printf("GL ERROR: %s\n", message);
		assert(false);
	}
}

func char* handle_plural(int num)
{
	return (num == 1 || num == -1) ? "" : "s";
}

func void spawn_particles(int count, s_particle_spawn_data data)
{
	for(int i = 0; i < count; i++)
	{
		s_particle p = zero;
		p.duration = data.duration * (1 - data.duration_rand * game->rng.randf32());
		p.speed = data.speed * (1 - data.speed_rand * game->rng.randf32());
		p.radius = data.radius * (1 - data.radius_rand * game->rng.randf32());
		p.pos = data.pos;

		float foo = (float)game->rng.randf2() * data.angle_rand * tau;
		float angle = data.angle + foo;
		p.dir = v2_from_angle(angle);
		p.color.x = data.color.x * (1 - data.color_rand * game->rng.randf32());
		p.color.y = data.color.y * (1 - data.color_rand * game->rng.randf32());
		p.color.z = data.color.z * (1 - data.color_rand * game->rng.randf32());
		p.color.w = data.color.w;
		p.fade = data.fade;
		p.shrink = data.shrink;
		game->particles.add_checked(p);
	}
}

func void play_delayed_sound(s_sound sound, float delay)
{
	s_delayed_sound s = zero;
	s.sound = sound;
	s.delay = delay;
	game->delayed_sounds.add_checked(s);
}

func void reset_level()
{
	g_ui = zero;
	s_rng* rng = &game->rng;
	memset(&game->transient, 0, sizeof(game->transient));
	memset(game->tiles_active, true, sizeof(game->tiles_active));
	int last_y_index = c_tiles_down - 1;
	for(int y = 0; y < c_tiles_down; y++)
	{
		s64* weights = get_tile_weights_for_y(y, e_tile_count);
		for(int x = 0; x < c_tiles_right; x++)
		{
			game->tiles[y][x] = zero;
			if(y == last_y_index)
			{
				game->tiles[y][x].type = e_tile_unbreakable;
			}
			else
			{
				game->tiles[y][x].type = pick_from_weights(weights, e_tile_count);
				if(rng->chance100(10))
				{
					game->tiles_active[y][x] = false;
				}
			}
		}
	}

	// @Note(tkap, 01/10/2023): Add portals
	b8 portals[c_tiles_down][c_tiles_right] = zero;
	for(int y = 1; y < c_tiles_down - c_portal_distance - 1; y++)
	{
		for(int x = 0; x < c_tiles_right; x++)
		{
			s_v2i target = v2i(rng->randu() % c_tiles_right, y + c_portal_distance);
			if(!portals[y][x] && !portals[target.y][target.x] && rng->randf32() <= 0.001f)
			{
				game->tiles_active[y][x] = false;
				game->tiles_active[target.y][target.x] = false;

				s_portal portal = zero;
				portal.pos = get_tile_center(v2i(x, y));
				portal.target_pos = get_tile_center(v2i(target.x, target.y));
				game->transient.portals.add_checked(portal);

				portals[y][x] = true;
				portals[target.y][target.x] = true;
			}
		}
	}


	s_player* player = &game->transient.player;
	player->pos.x = c_tile_size * c_tiles_right / 2;
	player->pos.y = -500;
	player->prev_pos = player->pos;
	player->level = 1;
	set_animation(player, e_animation_player_idle);
	game->transient.kill_area_speed = 80;
	game->camera.center = player->pos;
	game->camera.prev_center = game->camera.center;
	game->kill_area_bottom = -900;
}

func s_bounds get_camera_bounds(s_camera camera)
{
	s_bounds result;
	result.left = camera.center.x - c_base_res.x / 2;
	result.right = camera.center.x + c_base_res.x / 2;
	result.top = camera.center.y - c_base_res.y / 2;
	result.bottom = camera.center.y + c_base_res.y / 2;
	return result;
}

func s_tile_collision get_tile_collision(s_v2 pos, s_v2 size)
{
	s_tile_collision result = zero;
	int x_index = floorfi(pos.x / (float)c_tile_size);
	int y_index = floorfi(pos.y / (float)c_tile_size);

	for(int y = -1; y <= 1; y++)
	{
		int yy = y_index + y;
		for(int x = -1; x <= 1; x++)
		{
			int xx = x_index + x;
			if(!is_valid_tile_index(xx, yy)) { continue; }
			if(!is_tile_active(xx, yy)) { continue; }
			s_v2 tile_center = v2(xx * c_tile_size, yy * c_tile_size);
			tile_center += v2(c_tile_size) / 2;
			if(rect_collides_rect_center(pos, size, tile_center, v2(c_tile_size)))
			{
				result.collided = true;
				result.index = v2i(xx, yy);
				return result;
			}
		}
	}

	return result;
}

func b8 is_valid_tile_index(int x, int y)
{
	return x >= 0 && x < c_tiles_right && y >= 0 && y < c_tiles_down;
}

func b8 is_valid_tile_index(s_v2i p)
{
	return is_valid_tile_index(p.x, p.y);
}

func s_v2 get_world_mouse(s_camera camera)
{
	s_v2 result = zero;
	result.x = g_platform_data->mouse.x + (camera.center.x - c_half_res.x);
	result.y = g_platform_data->mouse.y + (camera.center.y - c_half_res.y);
	return result;
}

func s_v2i get_hovered_tile(s_camera camera)
{
	s_v2 mouse = get_world_mouse(camera);
	return point_to_tile(mouse);
}

func s_v2i point_to_tile(s_v2 pos)
{
	int x_index = floorfi(pos.x / (float)c_tile_size);
	int y_index = floorfi(pos.y / (float)c_tile_size);
	return s_v2i(x_index, y_index);
}

func float get_dig_delay()
{
	float result = c_dig_delay;
	float inc = 1;
	for(int i = 0; i < game->transient.upgrades_chosen[e_upgrade_dig_speed]; i++)
	{
		inc += get_upgrade_value(e_upgrade_dig_speed, i) / 100;
	}
	result /= inc;
	if(game->super_dig)
	{
		result *= 0.1f;
	}
	return result;
}

func float get_dig_range()
{
	float result = c_dig_range;

	float inc = 1;
	for(int i = 0; i < game->transient.upgrades_chosen[e_upgrade_dig_range]; i++)
	{
		inc += get_upgrade_value(e_upgrade_dig_range, i) / 100;
	}
	result *= inc;

	if(game->super_dig)
	{
		result *= 3;
	}
	return result;
}

func float get_movement_speed()
{
	float result = c_player_speed;

	float inc = 1;
	for(int i = 0; i < game->transient.upgrades_chosen[e_upgrade_movement_speed]; i++)
	{
		inc += get_upgrade_value(e_upgrade_movement_speed, i) / 100;
	}
	result *= inc;

	if(game->high_speed)
	{
		result *= 5;
	}
	return result;
}

func int get_max_health()
{
	int result = c_player_health;
	for(int i = 0; i < game->transient.upgrades_chosen[e_upgrade_health]; i++)
	{
		result += (int)get_upgrade_value(e_upgrade_health, i);
	}
	return result;
}

func b8** get_debug_vars()
{
	b8** result = (b8**)la_get(g_platform_data->frame_arena, array_count(debug_text) * sizeof(b8*));
	result[0] = &game->high_speed;
	result[1] = &game->super_dig;
	result[2] = &game->player_bounds;
	result[3] = &game->camera_bounds;
	result[4] = &game->high_gravity;
	result[5] = &game->no_kill_area;
	return result;
}

func int get_max_jumps()
{
	int result = 1;
	for(int i = 0; i < game->transient.upgrades_chosen[e_upgrade_extra_jump]; i++)
	{
		result += (int)get_upgrade_value(e_upgrade_extra_jump, i);
	}
	return result;
}

func int get_how_many_blocks_can_dash_break()
{
	int result = 0;
	for(int i = 0; i < game->transient.upgrades_chosen[e_upgrade_dash]; i++)
	{
		result += (int)get_upgrade_value(e_upgrade_dash, i);
	}
	return result;
}

func void add_upgrade_to_queue()
{
	game->transient.upgrades_queued += 1;

	if(game->transient.winning) { return; }

	if(game->transient.upgrades_queued == 1)
	{
		trigger_upgrade_menu();
	}
}

func void trigger_upgrade_menu()
{
	assert(!game->transient.in_upgrade_menu);
	s_rng* rng = &game->rng;
	game->transient.in_upgrade_menu = true;
	game->transient.upgrade_choices.count = 0;
	game->transient.upgrade_index = 0;

	s_sarray<int, e_upgrade_count> available_upgrades = zero;
	for(int upgrade_i = 0; upgrade_i < e_upgrade_count; upgrade_i++)
	{
		if(upgrade_i == e_upgrade_dash_cd && game->transient.upgrades_chosen[e_upgrade_dash] <= 0) { continue; }
		available_upgrades.add(upgrade_i);
	}

	for(int i = 0; i < 3; i++)
	{
		int rand_index = rng->randu() % available_upgrades.count;
		game->transient.upgrade_choices.add(available_upgrades[rand_index]);
		available_upgrades.remove_and_swap(rand_index);
	}
}

func b8 mouse_collides_rect_topleft(s_v2 mouse, s_v2 pos, s_v2 size)
{
	return rect_collides_rect_topleft(mouse, v2(1, 1), pos, size);
}

func void apply_upgrade(int index)
{
	game->transient.upgrades_chosen[index] += 1;
	game->transient.in_upgrade_menu = false;

	game->transient.upgrades_queued -= 1;
	if(game->transient.upgrades_queued > 0)
	{
		trigger_upgrade_menu();
	}
}

func char* get_upgrade_description(int id, int level)
{
	float value = get_upgrade_value(id, level);
	switch(id)
	{
		case e_upgrade_dig_speed:
		{
			return format_text("+%0.f%% digging speed", value);
		} break;

		case e_upgrade_dig_range:
		{
			return format_text("+%0.f%% dig range", value);
		} break;

		case e_upgrade_movement_speed:
		{
			return format_text("+%0.f%% movement speed", value);
		} break;

		case e_upgrade_health:
		{
			return format_text("+%0.f health", value);
		} break;

		case e_upgrade_extra_jump:
		{
			return format_text("+%0.f jump", value);
		} break;

		case e_upgrade_slower_kill_area:
		{
			return format_text("The void moves %0.f%% slower", value);
		} break;

		case e_upgrade_dash:
		{
			if(level == 0)
			{
				return format_text("Press F to dash, breaking %0.0f blocks", value);
			}
			else
			{
				return format_text("Dash can break an extra block");
			}
		} break;

		case e_upgrade_dash_cd:
		{
			return format_text("-%.0f%% dash cooldown", value);
		} break;

		invalid_default_case;
	}
	return null;
}

func float get_upgrade_value(int id, int level)
{
	switch(id)
	{
		case e_upgrade_dig_speed:
		{
			return 50.0f + 10 * level;
		} break;

		case e_upgrade_dig_range:
		{
			return 30.0f + 5 * level;
		} break;

		case e_upgrade_movement_speed:
		{
			return 25.0f + 5 * level;
		} break;

		case e_upgrade_health:
		{
			return 1;
		} break;

		case e_upgrade_extra_jump:
		{
			return 1;
		} break;

		case e_upgrade_slower_kill_area:
		{
			return 5;
		} break;

		case e_upgrade_dash:
		{
			if(level == 0) { return 3; }
			else { return 1; }
		} break;

		case e_upgrade_dash_cd:
		{
			return 10;
		} break;

		invalid_default_case;
	}
	return 0;
}

func s64* get_tile_weights_for_y(int y, int count)
{
	s64* weights = (s64*)la_get(g_platform_data->frame_arena, sizeof(s64) * count);
	for(int tile_i = 0; tile_i < count; tile_i++)
	{
		s64 weight = at_least((s64)0, (s64)floorfi(g_tile_data[tile_i].weight + g_tile_data[tile_i].weight_add * y));
		weights[tile_i] = weight;
	}
	return weights;
}

func int pick_from_weights(s64* weights, int count)
{
	s64 total = 0;
	for(int i = 0; i < count; i++)
	{
		s64 weight = weights[i];
		total += weight;
	}
	for(int i = 0; i < count; i++)
	{
		s64 weight = weights[i];
		s64 rand = game->rng.randu() % total;
		if(rand < weight) { return i; }
		total -= weight;
	}
	return -1;
}

func void add_exp(int exp)
{
	s_player* player = &game->transient.player;
	player->exp += exp;
	int required_exp = get_required_exp_to_level_up(player->level);
	game->transient.exp_gained_time = game->total_time;
	while(player->exp >= required_exp)
	{
		player->level += 1;
		player->exp -= required_exp;
		required_exp = get_required_exp_to_level_up(player->level);
		add_upgrade_to_queue();
	}
}

func int get_required_exp_to_level_up(int level)
{
	float result = 10;
	result += (level - 1) * 10;
	result = powf(result, 1.1f);
	return ceilfi(result);
}

func float get_max_y_vel()
{
	return 1000.0f * (game->high_gravity ? 5 : 1);
}

func void begin_winning()
{
	assert(!game->transient.winning);
	assert(!game->transient.won);
	game->transient.winning = true;
	game->transient.won = true;
	play_sound_if_supported(e_sound_win);
}

func b8 is_tile_active(int x, int y)
{
	assert(is_valid_tile_index(x, y));
	return game->tiles_active[y][x];
}

func b8 is_tile_active(s_v2i index)
{
	return is_tile_active(index.x, index.y);
}

func s_v2i get_closest_tile_to_mouse(s_camera camera)
{
	s_v2 mouse = get_world_mouse(camera);
	int mx = floorfi(mouse.x / c_tile_size);
	int my = floorfi(mouse.y / c_tile_size);

	// @Note(tkap, 01/10/2023): Let's just check on a 17x17 tile radius
	float shortest_dist = 999999.0f;
	s_v2i closest = v2i(-1, -1);
	for(int y = -8; y <= 8; y++)
	{
		int yy = my + y;
		for(int x = -8; x <= 8; x++)
		{
			int xx = mx + x;
			if(is_valid_tile_index(xx, yy) && is_tile_active(xx, yy))
			{
				float dist = v2_distance(mouse, get_tile_center(xx, yy));
				if(dist < shortest_dist)
				{
					shortest_dist = dist;
					closest = v2i(xx, yy);
				}
			}
		}
	}

	return closest;
}

func s_v2 get_tile_center(int x, int y)
{
	assert(is_valid_tile_index(x, y));
	return v2(
		x * c_tile_size + c_tile_size / 2.0f,
		y * c_tile_size + c_tile_size / 2.0f
	);
}

func s_v2 get_tile_center(s_v2i index)
{
	return get_tile_center(index.x, index.y);
}

func s_v2 world_to_screen(s_v2 pos, s_camera cam)
{
	s_v2 result;
	result.x = c_base_res.x / 2 - (cam.center.x - pos.x);
	result.y = c_base_res.y / 2 - (cam.center.y - pos.y);
	return result;
}

func s_v2 get_tile_pos(s_v2i index)
{
	assert(is_valid_tile_index(index));
	return v2(
		(float)(index.x * c_tile_size),
		(float)(index.y * c_tile_size)
	);
}

func void do_normal_render(u32 texture, int render_type)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindVertexArray(game->default_vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, game->noise.id);
	glUniform1i(1, 1);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GREATER);
	glEnable(GL_BLEND);
	if(render_type == 0)
	{
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else if(render_type == 1)
	{
		glBlendFunc(GL_ONE, GL_ONE);
	}
	invalid_else;

	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(*transforms.elements) * transforms.count, transforms.elements);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, transforms.count);
}

func void do_tile_particles(s_v2 pos, int tile_type, int type)
{
	s_v4 color = g_tile_data[tile_type].particle_color;
	color.w *= type == 0 ? 0.5f : 1.0f;
	spawn_particles(type == 0 ? 3 : 20, {
		.speed = 75.0f,
		.speed_rand = 0.5f,
		.radius = type == 0 ? 8.0f : 16.0f,
		.radius_rand = 0.5f,
		.duration = 0.5f,
		.duration_rand = 0.5f,
		.angle = 0,
		.angle_rand = 1,
		.pos = pos,
		.color = color,
	});
}

func void recreate_particle_framebuffer(int width, int height)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if(game->particle_fbo != 0)
	{
		assert(game->particle_texture != 0);
		glDeleteFramebuffers(1, &game->particle_fbo);
		glDeleteTextures(1, &game->particle_texture);

		game->particle_texture = 0;
		game->particle_fbo = 0;
	}
	glGenFramebuffers(1, &game->particle_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, game->particle_fbo);

	glGenTextures(1, &game->particle_texture);
	glBindTexture(GL_TEXTURE_2D, game->particle_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->particle_texture, 0);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

func s_label_group begin_label_group(s_v2 pos, e_font font_type, int selected, float spacing)
{
	s_label_group result = zero;
	result.pos = pos;
	result.font_type = font_type;
	result.default_selected = selected;
	result.spacing = spacing;
	return result;
}

func s_ui_state add_label(s_label_group* group, char* text)
{
	s_ui_state result = zero;
	u32 id = hash(text);
	int index = group->ids.count;

	if(index == group->default_selected && g_ui.selected.id == 0 && g_ui.hovered.id == 0 && g_ui.pressed.id == 0)
	{
		ui_request_selected(id, index);
	}

	group->ids.add(id);
	s_v4 color = rgb(0.7f, 0.7f, 0.7f);
	s_v2 label_size = get_text_size(text, group->font_type);
	s_v2 label_topleft = group->pos - label_size / 2;
	b8 hovered = mouse_collides_rect_topleft(g_platform_data->mouse, label_topleft, label_size);
	b8 selected = id == g_ui.selected.id;
	if(hovered || (selected && g_ui.hovered.id == 0))
	{
		ui_request_hovered(id, index);
	}
	if(g_ui.hovered.id == id)
	{
		color = rgb(1, 1, 0);
		if(hovered)
		{
			if(is_key_pressed(g_input, c_left_mouse))
			{
				ui_request_pressed(id, index);
			}
		}
		else
		{
			g_ui.hovered = zero;
		}
		if(is_key_pressed(g_input, c_key_enter))
		{
			result.clicked = true;
		}
	}
	if(g_ui.pressed.id == id)
	{
		color = color = rgb(0.7f, 0.7f, 0);
		if(is_key_released(g_input, c_left_mouse))
		{
			if(hovered)
			{
				ui_request_active(id);
				result.clicked = true;
			}
			else
			{
				g_ui.pressed = zero;
			}
		}
	}

	draw_text(text, group->pos, 15, color, group->font_type, true);
	group->pos.y += group->spacing;

	if(result.clicked)
	{
		g_ui = zero;
	}

	return result;
}

func int end_label_group(s_label_group* group)
{
	if(g_ui.hovered.id != 0) { return g_ui.hovered.index; }
	if(g_ui.pressed.id != 0) { return g_ui.pressed.index; }
	int selected = g_ui.selected.index;
	if(is_key_pressed(g_input, c_key_up))
	{
		selected = circular_index(selected - 1, group->ids.count);
	}
	if(is_key_pressed(g_input, c_key_down))
	{
		selected = circular_index(selected + 1, group->ids.count);
	}
	ui_request_selected(group->ids[selected], selected);
	return selected;
}

func u32 hash(const char* text)
{
	assert(text);
	u32 hash = 5381;
	while(true)
	{
		int c = *text;
		text += 1;
		if(!c) { break; }
		hash = ((hash << 5) + hash) + c;
	}
	return hash;
}

func void ui_request_selected(u32 id, int index)
{
	if(g_ui.hovered.id != 0) { return; }
	if(g_ui.pressed.id != 0) { return; }
	g_ui.selected.id = id;
	g_ui.selected.index = index;
}

func void ui_request_hovered(u32 id, int index)
{
	if(g_ui.pressed.id != 0) { return; }
	g_ui.hovered.id = id;
	g_ui.hovered.index = index;
	g_ui.selected.id = id;
	g_ui.selected.index = index;
}

func void ui_request_pressed(u32 id, int index)
{
	g_ui.hovered = zero;
	g_ui.pressed.id = id;
	g_ui.pressed.index = index;
}

func void ui_request_active(u32 id)
{
	unreferenced(id);
	g_ui.hovered = zero;
	g_ui.pressed = zero;
}

func s_v2 get_camera_wanted_center(s_player player)
{
	s_v2 result = player.pos;

	if(game->camera_bounds)
	{
		s_bounds cam_bounds = get_camera_bounds(game->camera);
		float right_limit = c_tiles_right * c_tile_size;
		result.x = at_least(c_half_res.x, result.x);
		result.x = at_most(right_limit - c_half_res.x, result.x);
	}
	return result;

}

func float get_kill_area_speed()
{
	float result = game->transient.kill_area_speed;

	float dec = 1;
	for(int i = 0; i < game->transient.upgrades_chosen[e_upgrade_slower_kill_area]; i++)
	{
		dec -= get_upgrade_value(e_upgrade_slower_kill_area, i) / 100;
	}
	dec = at_least(0.0f, dec);
	result *= dec;

	return result;
}

func void damage_tile(s_v2i index, int damage)
{
	assert(is_valid_tile_index(index));
	assert(is_tile_active(index));
	s_tile tile = game->tiles[index.y][index.x];
	int health = g_tile_data[tile.type].health;
	assert(health > 0);
	game->tiles[index.y][index.x].damage_taken += damage;
	if(game->tiles[index.y][index.x].damage_taken >= health)
	{
		add_exp(g_tile_data[tile.type].exp);

		do_tile_particles(
			random_point_in_rect_topleft(get_tile_pos(index), v2(c_tile_size), &game->rng),
			tile.type, 1
		);
		play_sound_if_supported(g_tile_data[tile.type].break_sound);
		game->tiles_active[index.y][index.x] = false;
	}
	else
	{
		s_v2 pos;
		if(get_hovered_tile(game->camera) == index) { pos = get_world_mouse(game->camera); }
		else { pos = random_point_in_rect_topleft(get_tile_pos(index), v2(c_tile_size), &game->rng); }
		do_tile_particles(
			pos, tile.type, 0
		);
		play_sound_if_supported(e_sound_dig);
	}
}

func float get_dash_cd()
{
	float result = c_dash_cd;

	float dec = 1;
	for(int i = 0; i < game->transient.upgrades_chosen[e_upgrade_dash_cd]; i++)
	{
		dec -= get_upgrade_value(e_upgrade_dash_cd, i) / 100;
	}
	dec = at_least(0.0f, dec);
	result *= dec;

	return result;
}

func s_sprite_data get_animation_frame(int animation_id, float time)
{
	float delay = g_animation[animation_id].delay;
	int index = roundfi(time / delay * g_animation[animation_id].frame_count);
	index %= g_animation[animation_id].frame_count;
	return g_animation[animation_id].frames[index];
}

func void set_animation(s_player* player, int animation_id)
{
	if(player->current_animation == animation_id) { return; }
	player->animation_timer = 0;
	player->current_animation = animation_id;
}