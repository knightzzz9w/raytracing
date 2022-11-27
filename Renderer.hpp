#pragma once
#include "Scene.hpp"
#include "omp.h"


struct hit_payload
{
    float tNear;
    uint32_t index;
    Vector2f uv; // uv坐标系坐标
    Object* hit_obj; 
};

class Renderer
{
public:
    void Render(const Scene& scene);

private:
};
