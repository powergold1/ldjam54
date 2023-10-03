#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "Ole32.lib")

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#ifdef _WIN32
#include <windows.h>
#include <gl/GL.h>
#include "external/wglext.h"
#include <xinput.h>
#include <xaudio2.h>
#else
#ifdef m_use_sdl_platform
#include <SDL2/SDL.h>
#include <GL/gl.h>
#endif
#endif
#include "external/glcorearb.h"
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#include "types.h"
#include "rng.h"
#include "math.h"
#include "utils.h"
