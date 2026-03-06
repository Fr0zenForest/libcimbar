/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#ifdef _WIN32
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>
#else
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif
