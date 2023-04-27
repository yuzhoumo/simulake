#ifndef SIMULAKE_HPP
#define SIMULAKE_HPP

/*
 * NOTE(vir):
 * central include for glad and glfw because ordering matters
 *
 */

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <OpenGL/OpenGL.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#pragma OPENCL EXTENSION cl_khr_gl_sharing : enable
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#else
#error "ONLY MACOS SUPPORTED"
#endif

#endif
