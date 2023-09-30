
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

		game->sprite_data[e_sprite_player].pos = v2i(16, 0);
		game->sprite_data[e_sprite_player].size = v2i(16, 32);

		game->sprite_data[e_sprite_dirt].pos = v2i(0, 0);
		game->sprite_data[e_sprite_dirt].size = v2i(16, 16);

		game->sprite_data[e_sprite_grass].pos = v2i(0, 16);
		game->sprite_data[e_sprite_grass].size = v2i(16, 16);

		game->sprite_data[e_sprite_damage_0].pos = v2i(0, 32);
		game->sprite_data[e_sprite_damage_0].size = v2i(16, 16);

		game->sprite_data[e_sprite_damage_1].pos = v2i(0, 48);
		game->sprite_data[e_sprite_damage_1].size = v2i(16, 16);

		game->sprite_data[e_sprite_damage_2].pos = v2i(0, 64);
		game->sprite_data[e_sprite_damage_2].size = v2i(16, 16);

		game->sprite_data[e_sprite_rect].pos = v2i(32, 0);
		game->sprite_data[e_sprite_rect].size = v2i(16, 16);
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
	g_platform_data->mouse.x *= c_base_res.x / (float)c_resolutions[game->current_resolution_index].x;
	g_platform_data->mouse.y *= c_base_res.y / (float)c_resolutions[game->current_resolution_index].y;

	game->update_timer += g_platform_data->time_passed;
	game->frame_count += 1;

	while(game->update_timer >= c_update_delay)
	{
		game->update_timer -= c_update_delay;

		game->player.prev_pos = game->player.pos;
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
				trigger_upgrade_menu();
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

			game->kill_area_bottom += delta * 80;
			game->kill_area_timer = at_most(c_kill_area_delay + delta, game->kill_area_timer + delta);

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		update player start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_player* player = &game->player;

				float x_dir = 0;
				if(is_key_down(g_logic_input, c_key_a))
				{
					x_dir = -1;
				}
				if(is_key_down(g_logic_input, c_key_d))
				{
					x_dir = 1;
				}
				player->x += get_movement_speed() * x_dir * delta;

				// @Note(tkap, 30/09/2023): x collision
				{
					s_tile_collision collision = get_tile_collision(player->pos, c_player_size);
					if(collision.collided)
					{
						player->x = collision.tile_center.x - x_dir * (c_tile_size / 2 + c_player_size.x / 2) - x_dir * 0.01f;
					}
				}

				player->vel.y += c_gravity * delta;
				player->vel.y = at_most(1000.0f, player->vel.y);
				if(player->vel.y > 0)
				{
					player->jumping = false;
				}

				if(player->jumping)
				{
					if(!is_key_down(g_logic_input, c_key_space))
					{
						player->jumping = false;
						player->vel.y *= 0.5f;
					}
				}

				b8 can_jump = player->jumps_done < get_max_jumps();
				if(can_jump && is_key_pressed(g_logic_input, c_key_space))
				{
					player->jumps_done += 1;
					player->jumping = true;
					player->vel.y = c_gravity * -0.4f;
				}

				player->y += player->vel.y * delta;

				// @Note(tkap, 30/09/2023): y collision
				{
					int y_dir = player->vel.y > 0 ? 1 : -1;
					s_tile_collision collision = get_tile_collision(player->pos, c_player_size);
					if(collision.collided)
					{
						player->jumping = false;
						player->y = collision.tile_center.y - y_dir * (c_tile_size / 2 + c_player_size.y / 2) - y_dir * 0.01f;
						player->vel.y = 0;
						if(y_dir == 1)
						{
							player->jumps_done = 0;
						}
					}
				}

				if(game->player_bounds)
				{
					player->x = at_least(c_player_size.x / 2, player->x);
					player->x = at_most(c_tiles_right * c_tile_size - c_player_size.x / 2, player->x);
				}
				game->camera.center = player->pos;

				if(game->camera_bounds)
				{
					s_bounds cam_bounds = get_camera_bounds(game->camera);
					float right_limit = c_tiles_right * c_tile_size;
					game->camera.center.x = at_least(c_half_res.x, game->camera.center.x);
					game->camera.center.x = at_most(right_limit - c_half_res.x, game->camera.center.x);
				}

				float dig_delay = get_dig_delay();
				player->dig_timer = at_most(dig_delay + delta, player->dig_timer + delta);
				if(is_key_down(g_logic_input, c_left_mouse))
				{
					while(player->dig_timer >= dig_delay)
					{
						s_v2i hovered = get_hovered_tile(game->camera);
						if(!is_valid_tile_index(hovered) && !game->tiles_active[hovered.y][hovered.x]) { break; }
						s_v2 tile_center = tile_index_to_tile_center(hovered);
						float dist = v2_distance(player->pos, tile_center);
						if(dist > get_dig_range()) { break; }

						player->dig_timer -= dig_delay;
						game->tiles[hovered.y][hovered.x].damage_taken += 1;
						if(game->tiles[hovered.y][hovered.x].damage_taken >= 4)
						{
							game->tiles_active[hovered.y][hovered.x] = false;
						}
					}
				}

				if(player->y >= c_depth_goals[game->transient.current_depth_index])
				{
					game->transient.current_depth_index += 1;
					trigger_upgrade_menu();
				}

				if(player->y < game->kill_area_bottom)
				{
					if(game->kill_area_timer >= c_kill_area_delay)
					{
						game->kill_area_timer -= c_kill_area_delay;
						player->damage_taken += 1;
						if(player->damage_taken >= get_max_health())
						{
							reset_level();
						}
					}
				}

			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		update player end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
	switch(game->state)
	{
		case e_state_main_menu:
		{
			s_v2i res = c_resolutions[game->current_resolution_index];
			draw_text("GAME NAME", game->title_pos, 15, v4(hsv_to_rgb(game->title_color), 1), e_font_huge, true);
			draw_text("Press Any Key to Start", v2(c_half_res.x, c_base_res.y * 0.7f), 15, v4(1), e_font_big, true);
			draw_text(
				format_text("Press F2 to change resolution (%ix%i)", res.x, res.y), v2(c_half_res.x, c_base_res.y * 0.8f), 15, v4(1), e_font_medium, true
			);
			draw_text("Made Live at twitch.tv/Tkap1", v2(c_half_res.x, c_base_res.y * 0.9f), 15, make_color(0.5f), e_font_medium, true);
		} break;

		case e_state_game:
		{
			s_camera interpolated_camera = game->camera;
			interpolated_camera.center = lerp(interpolated_camera.prev_center, interpolated_camera.center, dt);
			s_bounds cam_bounds = get_camera_bounds(interpolated_camera);

			{
				s_v2 pos = lerp(game->player.prev_pos, game->player.pos, dt);
				draw_texture(pos, e_layer_player, c_player_size, v4(1,1,1,1), game->sprite_data[e_sprite_player], true, dt);
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

			int left_tile = at_least(0, floorfi(cam_bounds.left / c_tile_size));
			int right_tile = at_most(c_tiles_right, ceilfi(cam_bounds.right / c_tile_size));
			int top_tile = at_least(0, floorfi(cam_bounds.top / c_tile_size));
			int bottom_tile = at_most(c_tiles_down, ceilfi(cam_bounds.bottom / c_tile_size));

			s_v2i hovered = get_hovered_tile(interpolated_camera);
			for(int y = top_tile; y < bottom_tile; y++)
			{
				for(int x = left_tile; x < right_tile; x++)
				{
					if(!game->tiles_active[y][x]) { continue; }

					s_tile tile = game->tiles[y][x];
					s_v4 mix_color = v4(1, 1, 1, 1);
					float mix_weight = 0;
					if(hovered == v2i(x, y))
					{
						mix_weight = 0.5f;
					}
					draw_texture(
						v2(x * c_tile_size, y * c_tile_size), e_layer_tiles, v2(c_tile_size), v4(1,1,1,1), game->sprite_data[tile.sprite_index],
						true, dt, {.mix_weight = mix_weight, .origin_offset = c_origin_topleft, .mix_color = mix_color}
					);

					if(tile.damage_taken > 0)
					{
						draw_texture(
							v2(x * c_tile_size, y * c_tile_size), e_layer_tiles_decal, v2(c_tile_size), v4(1,1,1,1),
							game->sprite_data[e_sprite_damage_0 + (tile.damage_taken - 1)],
							true, dt, {.mix_weight = mix_weight, .origin_offset = c_origin_topleft, .mix_color = mix_color}
						);
					}
				}
			}

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		upgrade menu start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				if(game->transient.in_upgrade_menu)
				{
					s_v2 base_pos = c_half_res;
					e_font font_type = e_font_medium;
					float spacing = game->font_arr[font_type].size * 1.2f;
					base_pos.y -= spacing * 1.5f;

					draw_texture(
						c_half_res, 10, c_half_res, v4(0.1f,0.1f,0.1f,1), game->sprite_data[e_sprite_rect], false, dt
					);

					if(is_key_pressed(g_input, c_key_up) || is_key_pressed(g_input, c_key_w))
					{
						game->transient.upgrade_index = circular_index(game->transient.upgrade_index - 1, 3);
					}
					if(is_key_pressed(g_input, c_key_down) || is_key_pressed(g_input, c_key_s))
					{
						game->transient.upgrade_index = circular_index(game->transient.upgrade_index + 1, 3);
					}

					int selected = game->transient.upgrade_index;
					{
						s_v2 pos = base_pos;
						for(int upgrade_i = 0; upgrade_i < 3; upgrade_i++)
						{
							int upgrade_id = game->transient.upgrade_choices[upgrade_i];
							int upgrade_level = game->transient.upgrades_chosen[upgrade_id];
							char* text = get_upgrade_description(upgrade_id, upgrade_level);
							s_v2 size = get_text_size(text, font_type);
							s_v2 temp_pos = pos;
							temp_pos -= size / 2;
							if(mouse_collides_rect_topleft(g_platform_data->mouse, temp_pos, size))
							{
								game->transient.upgrade_index = upgrade_i;
								selected = upgrade_i;
								break;
							}
							pos.y += spacing;
						}
					}

					{
						s_v2 pos = base_pos;
						for(int upgrade_i = 0; upgrade_i < 3; upgrade_i++)
						{
							s_v4 color = rgb(0.7f, 0.7f, 0.7f);
							int upgrade_id = game->transient.upgrade_choices[upgrade_i];
							int upgrade_level = game->transient.upgrades_chosen[upgrade_id];
							char* text = get_upgrade_description(upgrade_id, upgrade_level);
							s_v2 size = get_text_size(text, font_type);
							s_v2 temp_pos = pos;
							temp_pos -= size / 2;

							if(upgrade_i == selected)
							{
								color = rgb(1, 1, 0);
							}

							draw_text(text, pos, 15, color, font_type, true);
							pos.y += spacing;
						}
					}

					if(is_key_pressed(g_input, c_key_enter))
					{
						apply_upgrade(game->transient.upgrade_choices[selected]);
					}
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
		} break;

		case e_state_victory:
		{
			draw_text("Congratulations! You win!", v2(c_half_res.x, c_base_res.y * 0.3f), 15, v4(hsv_to_rgb(game->title_color), 1), e_font_big, true);
			draw_text("Press Any Key to Replay", v2(c_half_res.x, c_base_res.y * 0.5f), 15, v4(1), e_font_big, true);
			draw_text("Press Escape to Exit", v2(c_half_res.x, c_base_res.y * 0.6f), 15, v4(1), e_font_big, true);
			draw_text("Get the source code at https://github.com/Tkap1/FIXME", v2(c_half_res.x, c_base_res.y * 0.8f), 15, make_color(0.5f), e_font_medium, true);
			draw_text("Made Live at twitch.tv/Tkap1", v2(c_half_res.x, c_base_res.y * 0.9f), 15, make_color(0.5f), e_font_medium, true);
		} break;

		invalid_default_case;
	}

	foreach_raw(particle_i, particle, game->particles)
	{
		s_v4 color = particle.color;
		float percent = (particle.time / particle.duration);
		float percent_left = 1.0f - percent;
		color.w = powf(percent_left, 0.5f);
		if(particle.render_type == 0)
		{
			draw_circle(particle.pos, 1, particle.radius * range_lerp(percent, 0, 1, 1, 0.2f), color);
		}
		else
		{
			draw_circle_p(particle.pos, 1, particle.radius * range_lerp(percent, 0, 1, 1, 0.2f), color);
		}
	}

	{
		glUseProgram(game->programs[e_shader_default]);
		// glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, g_window.width, g_window.height);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_GREATER);

		{
			int location = glGetUniformLocation(game->programs[e_shader_default], "window_size");
			glUniform2fv(location, 1, &c_base_res.x);
		}
		{
			int location = glGetUniformLocation(game->programs[e_shader_default], "time");
			glUniform1f(location, game->total_time);
		}

		if(transforms.count > 0)
		{
			glBindVertexArray(game->default_vao);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, game->atlas.id);
			glUniform1i(1, 1);
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			// glBlendFunc(GL_ONE, GL_ONE);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(*transforms.elements) * transforms.count, transforms.elements);
			glDrawArraysInstanced(GL_TRIANGLES, 0, 6, transforms.count);
			transforms.count = 0;
		}

		if(particles.count > 0)
		{
			glBindVertexArray(game->default_vao);
			glEnable(GL_BLEND);
			// glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			glBlendFunc(GL_ONE, GL_ONE);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(*particles.elements) * particles.count, particles.elements);
			glDrawArraysInstanced(GL_TRIANGLES, 0, 6, particles.count);
			particles.count = 0;
		}

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
	const char* vertex_src_arr[] = {header, read_file("src/shader_shared.h", frame_arena), vertex_src};
	const char* fragment_src_arr[] = {header, read_file("src/shader_shared.h", frame_arena), fragment_src};
	// const char* vertex_src_arr[] = {header, vertex_src};
	// const char* fragment_src_arr[] = {header, fragment_src};
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
		p.render_type = data.render_type;
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
	s_rng* rng = &game->rng;
	memset(game->tiles_active, true, sizeof(game->tiles_active));
	for(int y = 0; y < c_tiles_down; y++)
	{
		for(int x = 0; x < c_tiles_right; x++)
		{
			game->tiles[y][x] = zero;
			game->tiles[y][x].sprite_index = rng->rand_range_ii(e_sprite_dirt, e_sprite_grass);
			if(rng->chance100(10))
			{
				game->tiles_active[y][x] = false;
			}
		}
	}
	game->player = zero;
	game->player.pos.x = c_tile_size * c_tiles_right / 2;
	game->player.pos.y = -500;
	game->player.prev_pos = game->player.pos;
	game->camera.center = game->player.pos;
	game->camera.prev_center = game->camera.center;
	game->kill_area_bottom = -900;
	game->kill_area_timer = 0;
	memset(&game->transient, 0, sizeof(game->transient));
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
			if(!game->tiles_active[yy][xx]) { continue; }
			s_v2 tile_center = v2(xx * c_tile_size, yy * c_tile_size);
			tile_center += v2(c_tile_size) / 2;
			if(rect_collides_rect_center(pos, size, tile_center, v2(c_tile_size)))
			{
				result.collided = true;
				result.tile_center = tile_center;
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

func s_v2 tile_index_to_tile_center(s_v2i index)
{
	assert(is_valid_tile_index(index));
	s_v2 result;
	result.x = index.x * c_tile_size + c_tile_size / 2.0f;
	result.y = index.y * c_tile_size + c_tile_size / 2.0f;
	return result;
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
			return 40.0f + 10 * level;
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

		invalid_default_case;
	}
	return 0;
}

