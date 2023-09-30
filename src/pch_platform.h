#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "Ole32.lib")

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <gl/GL.h>
#include "external/glcorearb.h"
#include "external/wglext.h"
#include <xinput.h>
#include <xaudio2.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#include "types.h"
#include "math.h"
#include "utils.h"