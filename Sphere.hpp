#pragma once

#include "Object.hpp"
#include "vecmath.h"
#include "Bounds3.hpp"
#include "Material.hpp"

class Sphere : public Object{
public:
    Vector3f center;
    float radius, radius2;
    Material *m;
    float area;
    Sphere(const Vector3f &c, const float &r , Material * new_m = new Material()) : center(c), radius(r), radius2(r * r), m(new_m) , area(4 * M_PI *r *r){}
    bool intersect(const Ray& ray) {
        // analytic solution
        Vector3f L = ray.origin - center;
        float a = dotProduct(ray.direction, ray.direction);
        float b = 2 * dotProduct(ray.direction, L);
        float c = dotProduct(L, L) - radius2;
        float t0, t1;
        float area = 4 * M_PI * radius2;
        if (!solveQuadratic(a, b, c, t0, t1)) return false;
        if (t0 < 0) t0 = t1;
        if (t0 < 0) return false;
        return true;
    }
    bool intersect(const Ray& ray, float &tnear, uint32_t &index , float &u , float &v) const
    {
        // analytic solution
        Vector3f L = ray.origin - center;
        float a = dotProduct(ray.direction, ray.direction);
        float b = 2 * dotProduct(ray.direction, L);
        float c = dotProduct(L, L) - radius2;
        float t0, t1;
        if (!
        solveQuadratic(a, b, c, t0, t1)) return false;
        if (t0 < 0) t0 = t1;
        if (t0 < 0) return false;
        tnear = t0;

        return true;
    }
    Intersection getIntersection(Ray ray){   //相交到了emmision上，那么返回的颜色是灯光的颜色
        Intersection result;
        result.happened = false;
        Vector3f L = ray.origin - center;
        // std::cout << "ray origin   is "<< ray.origin << std::endl;
        // std::cout << "ray direction   is "<< ray.direction << std::endl;
        // std::cout << "ball center   is "<< center << std::endl;
        // std::cout << "ball radius   is "<< radius << std::endl;
        float a = dotProduct(ray.direction, ray.direction);
        float b = 2 * dotProduct(ray.direction, L);
        float c = dotProduct(L, L) - radius2;
        float t0, t1;
        if (!solveQuadratic(a, b, c, t0, t1)) return result;
        if (t0 < 0) t0 = t1;
        if (t0 < 0) return result;
        //std::cout << "t0 and t1    is "<< t0 << t1 << std::endl;
        result.happened=true;
        result.emit =  this->m->m_emission ;

        result.coords = Vector3f(ray.origin + ray.direction * t0);
        //std::cout << "result.coords  is "<< result.coords  << std::endl;
        result.normal = normalize(Vector3f(result.coords - center));
        double u=0, v =0 ;
        if(this->m->tt == 1)
        {
            Sphere::get_sphere_uv(result.normal , u,v) ;  //材质的颜色
            result.iftt = true;
        }
        Vector3f cur(0,0,0);
        result.color = this->m->getColorAt(u,v , cur); //材质的颜色 , t0其实没有用上
        result.m = this->m;
        result.obj = this;
        result.distance = t0;
        return result;
    }
    void getSurfaceProperties(const Vector3f &P, const Vector3f &I, const uint32_t &index, const Vector2f &uv, Vector3f &N, Vector2f &st) const
    { N = normalize(P - center); }

    Vector3f evalDiffuseColor(const Vector2f &st)const {
        return m->getColor();
    }
    Bounds3 getBounds(){
        return Bounds3(Vector3f(center.x()-radius, center.y()-radius, center.z()-radius),
                       Vector3f(center.x()+radius, center.y()+radius, center.z()+radius));
    }

    void Sample(Intersection &pos, float &pdf){
        float theta = 2.0 * M_PI * get_random_float(), phi = M_PI * get_random_float();
        Vector3f dir(std::cos(phi), std::sin(phi)*std::cos(theta), std::sin(phi)*std::sin(theta));
        pos.coords = center + radius * dir;
        pos.normal = dir;
        pos.emit = m->getEmission();
        pdf = 1.0f / area;
    }
    float getArea(){
        return area;
    }
    bool hasEmit(){
        return m->hasEmission();
    }

    inline void roll_angle(float &angle, double a, double max)
    {
        if (angle > max - a)
            angle = angle + a - max;
        else
            angle += a;
    }

    void get_sphere_uv(const Vector3f &p, double &u, double &v)
    {
        float pi = 3.14159265;
        //std::cout << "get_sphere_uv p is " <<  p << std::endl;
        auto theta = acos(-p.y()); // [0, pi]
        auto phi = atan2(-p.z(), p.x()) + pi;
        // std::cout << "theta is  " <<  theta << std::endl;
        // std::cout << "phi is  " <<  phi << std::endl;
        //下面的东西都是图像在正中央的情况
        // roll_angle(theta, -pi / 14, pi);      //减小时向下
        // roll_angle(phi, 5 * pi / 14, 2 * pi); // 减小是向右
        u = phi / (2 * pi);
        v = theta / pi;
    }

};

