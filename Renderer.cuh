#ifndef RENDER_CUH
#define RENDER_CUH

#include "Renderer.hpp"
#include "Ray.hpp"
#include "camera.hpp"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "vecmath.h"
#include <stdio.h>
//非模板函数才可以进行C连接
extern "C" void renderall(std::vector<Vector3f> framebuffer ,  int image_width, int image_height   , int turn_try);

#endif
