
global constexpr float pi =  3.1415926f;
global constexpr float tau = 6.283185f;
global constexpr float epsilon = 0.000001f;

struct s_v2
{
	float x;
	float y;
	//constexpr s_v2(float x, float y) : x(x), y(y) {}
	//constexpr s_v2(int x, int y) : x((float)x), y((float)y) {}
	//constexpr s_v2(float f) : x((float)f), y((float)f) {}
	//constexpr s_v2(int i) : x((float)i), y((float)i) {}
};

struct s_v2i
{
	int x;
	int y;
	//constexpr s_v2i(int x, int y) : x(x), y(y) {}
};

struct s_v3
{
	float x;
	float y;
	float z;
	//constexpr s_v3(float x, float y, float z) : x(x), y(y), z(z) {}
	//constexpr s_v3(float f) : x(f), y(f), z(f) {}
};

struct s_v4
{
	float x;
	float y;
	float z;
	float w;
	//constexpr s_v4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	//constexpr s_v4(float f) : x(f), y(f), z(f), w(f) {}
	//constexpr s_v4(s_v3 v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}
};

func constexpr s_v2 v2(float a, float b)
{
	return {a, b};
}

func constexpr s_v2 v2(float v)
{
	return {v, v};
}

func constexpr s_v2 v2(s_v2i v)
{
	return {.x=(float)v.x, .y=(float)v.y};
}

func s_v2 v22i(int x, int y)
{
	return {.x=(float)x, .y=(float)y};
}

func constexpr s_v2i v2i(int x, int y)
{
	return {.x=x, .y=y};
}

func s_v2 v2ii(int x, int y)
{
	return {.x=(float)x, .y=(float)y};
}

func s_v2 v2_mul(s_v2 a, float b)
{
	return {a.x * b, a.y * b};
}

func s_v3 v3(float x, float y, float z)
{
	return {x, y, z};
}

func s_v3 v3_mul(s_v3 a, float b)
{
	return {a.x * b, a.y * b, a.z * b};
}

func s_v4 v4(float x, float y, float z, float w)
{
	return {x, y, z, w};
}

template <typename t0, typename t1, typename t2, typename t3>
func s_v4 v4(t0 x, t1 y, t2 z, t3 w)
{
	return (s_v4){(float)x, (float)y, (float)z, (float)w};
}

template <typename t>
func s_v4 v4(t v)
{
	return (s_v4){(float)v, (float)v, (float)v, (float)v};
}

template <typename t>
func constexpr s_v4 make_color(t v)
{
	return {(float)v, (float)v, (float)v, 1};
}

template <typename t0, typename t1, typename t2>
func constexpr s_v4 make_color(t0 r, t1 g, t2 b)
{
	return {(float)r, (float)g, (float)b, 1};
}

func s_v4 v41f(float v)
{
	return {v, v, v, v};
}

func s_v4 v4(s_v3 v, float w)
{
	return {v.x, v.y, v.z, w};
}

func b8 rect_collides_circle(s_v2 rect_center, s_v2 rect_size, s_v2 center, float radius)
{
	b8 collision = false;

	float dx = fabsf(center.x - rect_center.x);
	float dy = fabsf(center.y - rect_center.y);

	if(dx > (rect_size.x/2.0f + radius)) { return false; }
	if(dy > (rect_size.y/2.0f + radius)) { return false; }

	if(dx <= (rect_size.x/2.0f)) { return true; }
	if(dy <= (rect_size.y/2.0f)) { return true; }

	float cornerDistanceSq = (dx - rect_size.x/2.0f)*(dx - rect_size.x/2.0f) +
													(dy - rect_size.y/2.0f)*(dy - rect_size.y/2.0f);

	collision = (cornerDistanceSq <= (radius*radius));

	return collision;
}

func s_v2 v2_from_angle(float angle)
{
	return {
		cosf(angle),
		sinf(angle)
	};
}

func s_v2 v2_sub(s_v2 a, s_v2 b)
{
	return {a.x - b.x, a.y - b.y};
}

func float v2_angle(s_v2 v)
{
	return atan2f(v.y, v.x);
}

func float lerp(float a, float b, float t)
{
	return a + (b - a) * t;
}

func int roundfi(float x)
{
	return (int)roundf(x);
}

func float sinf2(float t)
{
	return sinf(t) * 0.5f + 0.5f;
}

func float deg_to_rad(float d)
{
	return d * (pi / 180.f);
}

func float rad_to_deg(float r)
{
	return r * (180.f / pi);
}

template <typename t>
func t at_least(t a, t b)
{
	return a > b ? a : b;
}

template <typename t>
func t at_most(t a, t b)
{
	return b > a ? a : b;
}

func b8 floats_equal(float a, float b)
{
	return (a >= b - epsilon && a <= b + epsilon);
}

func float ilerp(float start, float end, float val)
{
	float b = end - start;
	if(floats_equal(b, 0)) { return val; }
	return (val - start) / b;
}

func int floorfi(float x)
{
	return (int)floorf(x);
}

func int ceilfi(float x)
{
	return (int)ceilf(x);
}

func float fract(float x)
{
	return x - (int)x;
}

func s_v2 operator-(s_v2 a, s_v2 b)
{
	return {a.x - b.x, a.y - b.y};
}

func s_v2 operator+(s_v2 a, s_v2 b)
{
	return {a.x + b.x, a.y + b.y};
}

func s_v2 operator*(s_v2 a, float b)
{
	return {a.x * b, a.y * b};
}

func s_v2 operator/(s_v2 a, float b)
{
	return {a.x / b, a.y / b};
}

func s_v2 operator/(s_v2 a, s_v2 b)
{
	return {a.x / b.x, a.y / b.y};
}

func void operator-=(s_v2& a, s_v2 b)
{
	a.x -= b.x;
	a.y -= b.y;
}

func void operator*=(s_v2& a, s_v2 b)
{
	a.x *= b.x;
	a.y *= b.y;
}

func void operator*=(s_v2& a, float b)
{
	a.x *= b;
	a.y *= b;
}


func float v2_length(s_v2 a)
{
	return sqrtf(a.x * a.x + a.y * a.y);
}

func float v2_distance(s_v2 a, s_v2 b)
{
	return v2_length(a - b);
}


func s_v2 lerp(s_v2 a, s_v2 b, float t)
{
	return {lerp(a.x, b.x, t), lerp(a.y, b.y, t)};
}

func s_v2 lerp_snap(s_v2 a, s_v2 b, float t)
{
	float dist = v2_distance(a, b);
	return lerp(a, b, dist < 1.0f ? 1.0f : t);
}

func s_v4 lerp(s_v4 a, s_v4 b, float t)
{
	return {
		lerp(a.x, b.x, t),
		lerp(a.y, b.y, t),
		lerp(a.z, b.z, t),
		lerp(a.w, b.w, t)
	};
}

func s_v2 v2_normalized(s_v2 v)
{
	float length = v2_length(v);
	return length == 0 ? v : (s_v2){v.x / length, v.y / length};
}

func float range_lerp(float input_val, float input_start, float input_end, float output_start, float output_end)
{
	return output_start + ((output_end - output_start) / (input_end - input_start)) * (input_val - input_start);
}

func s_v2 operator*(s_v2 left, s_v2 right)
{
	return {left.x * right.x, left.y * right.y};
}

func void operator+=(s_v2& left, s_v2 right)
{
	left.x += right.x;
	left.y += right.y;
}

func b8 circle_collides_circle(s_v2 center1, float radius1, s_v2 center2, float radius2)
{
	float dx = center2.x - center1.x;
	float dy = center2.y - center1.y;

	float distance = dx*dx + dy*dy;

	float combined_radius = radius1 + radius2;
	combined_radius *= combined_radius;

	if(distance <= combined_radius) { return true; }
	return false;
}

func s_v3 hsv_to_rgb(s_v3 color)
{
	if(color.y <= 0.0f)
	{
		return {color.z, color.z, color.z};
	}

	color.x *= 360.0f;
	if(color.x < 0.0f || color.x >= 360.0f)
		color.x = 0.0f;
	color.x /= 60.0f;

	u32 i = (u32)color.x;
	float ff = color.x - i;
	float p = color.z * (1.0f - color.y );
	float q = color.z * (1.0f - (color.y * ff));
	float t = color.z * (1.0f - (color.y * (1.0f - ff)));

	switch(i)
	{
	case 0: return {color.z, t, p};
	case 1: return {q, color.z, p};
	case 2: return {p, color.z, t};
	case 3: return {p, q, color.z};
	case 4: return {t, p, color.z};
	default: return {color.z, p, q};
	}
}

template <typename t>
func t max(t a, t b)
{
	return a >= b ? a : b;
}

template <typename t>
func t clamp(t current, t min_val, t max_val)
{
	return at_most(max_val, at_least(min_val, current));
}

func b8 rect_collides_rect_topleft(s_v2 pos0, s_v2 size0, s_v2 pos1, s_v2 size1)
{
	return pos0.x + size0.x > pos1.x && pos0.x < pos1.x + size1.x &&
		pos0.y + size0.y > pos1.y && pos0.y < pos1.y + size1.y;
}

func b8 rect_collides_rect_center(s_v2 pos0, s_v2 size0, s_v2 pos1, s_v2 size1)
{
	s_v2 half_size0 = size0 / 2;
	s_v2 half_size1 = size1 / 2;
	float x_dist = fabsf(pos0.x - pos1.x);
	float y_dist = fabsf(pos0.y - pos1.y);
	return x_dist < half_size0.x + half_size1.x && y_dist < half_size0.y + half_size1.y;
}

func b8 operator==(s_v2i a, s_v2i b)
{
	return a.x == b.x && a.y == b.y;
}

[[nodiscard]]
func s_v2 random_point_in_rect_topleft(s_v2 pos, s_v2 size, s_rng* rng)
{
	return {
		pos.x + rng->randf32() * size.x,
		pos.y + rng->randf32() * size.y
	};
}
