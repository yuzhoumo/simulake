#ifndef SIMULAKE_HPP
#define SIMULAKE_HPP

/*
 * NOTE(vir):
 * central include for opengl/cl because ordering matters
 */

#ifdef __APPLE__

#include <OpenCL/opencl.h>
#include <OpenGL/OpenGL.h>

#pragma OPENCL EXTENSION cl_khr_gl_sharing : enable
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#else

#include <CL/cl.hpp>
// #error "ONLY MACOS SUPPORTED"

#endif

#endif
