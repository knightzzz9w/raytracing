#pragma once

// Intersection class 是用来记录光线Ray和物体相交的信息的
// 例如：是否相交，相交点的坐标，相交点的法向量，相交点的材质，相交点的物体，相交点到光线原点的距离

#include "vecmath.h"
#include "Material.hpp"
class Object;
class Sphere;


struct Intersection
{
    Intersection(){
        happened=false;
        coords=Vector3f();
        normal=Vector3f();
        color = Vector3f();
        distance= std::numeric_limits<double>::max();
        obj =nullptr;
        m=nullptr;
    }
    bool happened;
    Vector3f coords;
    Vector3f normal;
    Vector3f emit;
    Vector3f color;  //物体表面的color是什么
    double distance;
    float u,v;  //在材质上相交的位置是什么
    Object* obj;
    Material* m;
    bool iftt = false;  //是否与材质表面相交

};
