
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "opengl32.lib")

// @Note(tkap, 24/06/2023): We don't want this Madeg
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#include "external/wglext.h"
#include "external/glcorearb.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <intrin.h>

#include "types.h"
#include "math.h"
#include "utils.h"
#include "memory.h"
#include "str_builder.h"
#include "rng.h"