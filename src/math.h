
global constexpr float pi =  3.1415926f;
global constexpr float tau = 6.283185f;
global constexpr float epsilon = 0.000001f;

struct s_v2
{
	float x;
	float y;
};

struct s_v2i
{
	int x;
	int y;
};

struct s_v3
{
	float x;
	float y;
	float z;
};

struct s_v4
{
	float x;
	float y;
	float z;
	float w;
};

template <typename T>
func constexpr s_v2 v2(T x, T y)
{
	s_v2 result;
	result.x = (float)x;
	result.y = (float)y;
	return result;
}

template <typename T>
func constexpr s_v2 v2(T v)
{
	s_v2 result;
	result.x = (float)v;
	result.y = (float)v;
	return result;
}

func s_v2 v22i(int x, int y)
{
	s_v2 result;
	result.x = (float)x;
	result.y = (float)y;
	return result;
}

func constexpr s_v2i v2i(int x, int y)
{
	s_v2i result;
	result.x = x;
	result.y = y;
	return result;
}

func s_v2 v2ii(int x, int y)
{
	s_v2 result;
	result.x = (float)x;
	result.y = (float)y;
	return result;
}

func s_v2 v2_mul(s_v2 a, float b)
{
	s_v2 result;
	result.x = a.x * b;
	result.y = a.y * b;
	return result;
}

func s_v3 v3(float x, float y, float z)
{
	s_v3 result;
	result.x = x;
	result.y = y;
	result.z = z;
	return result;
}

func s_v3 v3_mul(s_v3 a, float b)
{
	s_v3 result;
	result.x = a.x * b;
	result.y = a.y * b;
	result.z = a.z * b;
	return result;
}

func s_v4 v4(float x, float y, float z, float w)
{
	s_v4 result;
	result.x = x;
	result.y = y;
	result.z = z;
	result.w = w;
	return result;
}

template <typename t0, typename t1, typename t2, typename t3>
func s_v4 v4(t0 x, t1 y, t2 z, t3 w)
{
	s_v4 result;
	result.x = (float)x;
	result.y = (float)y;
	result.z = (float)z;
	result.w = (float)w;
	return result;
}

template <typename t>
func s_v4 v4(t v)
{
	s_v4 result;
	result.x = (float)v;
	result.y = (float)v;
	result.z = (float)v;
	result.w = (float)v;
	return result;
}

template <typename t>
func constexpr s_v4 make_color(t v)
{
	s_v4 result;
	result.x = (float)v;
	result.y = (float)v;
	result.z = (float)v;
	result.w = 1;
	return result;
}

template <typename t0, typename t1, typename t2>
func constexpr s_v4 make_color(t0 r, t1 g, t2 b)
{
	s_v4 result;
	result.x = (float)r;
	result.y = (float)g;
	result.z = (float)b;
	result.w = 1;
	return result;
}

func s_v4 v41f(float v)
{
	s_v4 result;
	result.x = v;
	result.y = v;
	result.z = v;
	result.w = v;
	return result;
}

func s_v4 v4(s_v3 v, float w)
{
	s_v4 result;
	result.x = v.x;
	result.y = v.y;
	result.z = v.z;
	result.w = w;
	return result;
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
	return v2(
		cosf(angle),
		sinf(angle)
	);
}

func s_v2 v2_sub(s_v2 a, s_v2 b)
{
	s_v2 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	return result;
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
	s_v2 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	return result;
}

func s_v2 operator+(s_v2 a, s_v2 b)
{
	s_v2 result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return result;
}

func s_v2 operator*(s_v2 a, float b)
{
	s_v2 result;
	result.x = a.x * b;
	result.y = a.y * b;
	return result;
}

func s_v2 operator/(s_v2 a, float b)
{
	s_v2 result;
	result.x = a.x / b;
	result.y = a.y / b;
	return result;
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

func s_v2 lerp(s_v2 a, s_v2 b, float t)
{
	s_v2 result;
	result.x = lerp(a.x, b.x, t);
	result.y = lerp(a.y, b.y, t);
	return result;
}

func s_v4 lerp(s_v4 a, s_v4 b, float t)
{
	s_v4 result;
	result.x = lerp(a.x, b.x, t);
	result.y = lerp(a.y, b.y, t);
	result.z = lerp(a.z, b.z, t);
	result.w = lerp(a.w, b.w, t);
	return result;
}

func float v2_length(s_v2 a)
{
	return sqrtf(a.x * a.x + a.y * a.y);
}

func float v2_distance(s_v2 a, s_v2 b)
{
	return v2_length(a - b);
}

func s_v2 v2_normalized(s_v2 v)
{
	s_v2 result;
	float length = v2_length(v);
	if(length != 0)
	{
		result.x = v.x / length;
		result.y = v.y / length;
	}
	else
	{
		result = v;
	}
	return result;
}

func float range_lerp(float input_val, float input_start, float input_end, float output_start, float output_end)
{
	return output_start + ((output_end - output_start) / (input_end - input_start)) * (input_val - input_start);
}

func s_v2 operator*(s_v2 left, s_v2 right)
{
	s_v2 result;
	result.x = left.x * right.x;
	result.y = left.y * right.y;
	return result;
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

func s_v3 hsv_to_rgb(s_v3 colour)
{
	s_v3 rgb;

	if(colour.y <= 0.0f)
	{
		rgb.x = colour.z;
		rgb.y = colour.z;
		rgb.z = colour.z;
		return rgb;
	}

	colour.x *= 360.0f;
	if(colour.x < 0.0f || colour.x >= 360.0f)
		colour.x = 0.0f;
	colour.x /= 60.0f;

	u32 i = (u32)colour.x;
	float ff = colour.x - i;
	float p = colour.z * (1.0f - colour.y );
	float q = colour.z * (1.0f - (colour.y * ff));
	float t = colour.z * (1.0f - (colour.y * (1.0f - ff)));

	switch(i)
	{
	case 0:
		rgb.x = colour.z;
		rgb.y = t;
		rgb.z = p;
		break;

	case 1:
		rgb.x = q;
		rgb.y = colour.z;
		rgb.z = p;
		break;

	case 2:
		rgb.x = p;
		rgb.y = colour.z;
		rgb.z = t;
		break;

	case 3:
		rgb.x = p;
		rgb.y = q;
		rgb.z = colour.z;
		break;

	case 4:
		rgb.x = t;
		rgb.y = p;
		rgb.z = colour.z;
		break;

	default:
		rgb.x = colour.z;
		rgb.y = p;
		rgb.z = q;
		break;
	}

	return rgb;
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
	return v2(
		pos.x + rng->randf32() * size.x,
		pos.y + rng->randf32() * size.y
	);
}
