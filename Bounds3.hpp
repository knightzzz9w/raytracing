#pragma once

#include "Ray.hpp"
#include "vecmath.h"
#include <limits>
#include <array>

// 包围盒类
class Bounds3
{
public:
    Vector3f pMin, pMax; // two points to specify the bounding box
    Bounds3()
    {
        double minNum = std::numeric_limits<double>::lowest();
        double maxNum = std::numeric_limits<double>::max();
        pMax = Vector3f(minNum, minNum, minNum);
        pMin = Vector3f(maxNum, maxNum, maxNum);
    }
    Bounds3(const Vector3f p) : pMin(p), pMax(p) {}


    Bounds3 &operator=(const Bounds3 &arr){  //重载赋值运算符
        if( this != &arr){  //判断是否是给自己赋值
            this->pMax = arr.pMax;
            this->pMin = arr.pMin;
        }
        return *this;
    }
    Bounds3(const Vector3f p1, const Vector3f p2)
    {
        pMin = Vector3f(fmin(p1.x(), p2.x()), fmin(p1.y(), p2.y()), fmin(p1.z(), p2.z()));
        pMax = Vector3f(fmax(p1.x(), p2.x()), fmax(p1.y(), p2.y()), fmax(p1.z(), p2.z()));
    }

    Vector3f Diagonal() const { return pMax - pMin; }
    int maxExtent() const
    {
        Vector3f d = Diagonal();
        if (d.x() > d.y() && d.x() > d.z())
            return 0;
        else if (d.y() > d.z())
            return 1;
        else
            return 2;
    }

    double SurfaceArea() const
    {
        Vector3f d = Diagonal();
        return 2 * (d.x() * d.y() + d.x() * d.z() + d.y() * d.z());
    }

    Vector3f Centroid() { return 0.5 * pMin + 0.5 * pMax; }
    Bounds3 Intersect(const Bounds3& b)
    {
        return Bounds3(Vector3f(fmax(pMin.x(), b.pMin.x()), fmax(pMin.y(), b.pMin.y()),
                                fmax(pMin.z(), b.pMin.z())),
                       Vector3f(fmin(pMax.x(), b.pMax.x()), fmin(pMax.y(), b.pMax.y()),
                                fmin(pMax.z(), b.pMax.z())));
    }

    Vector3f Offset(const Vector3f& p) const
    {
        Vector3f o = p - pMin;
        if (pMax.x() > pMin.x())
            o.x() /= pMax.x() - pMin.x();
        if (pMax.y() > pMin.y())
            o.y() /= pMax.y() - pMin.y();
        if (pMax.z() > pMin.z())
            o.z() /= pMax.z() - pMin.z();
        return o;
    }

    bool Overlaps(const Bounds3& b1, const Bounds3& b2)
    {
        bool x = (b1.pMax.x() >= b2.pMin.x()) && (b1.pMin.x() <= b2.pMax.x());
        bool y = (b1.pMax.y() >= b2.pMin.y()) && (b1.pMin.y() <= b2.pMax.y());
        bool z = (b1.pMax.z() >= b2.pMin.z()) && (b1.pMin.z() <= b2.pMax.z());
        return (x && y && z);
    }

    bool Inside(const Vector3f& p, const Bounds3& b)
    {
        return (p.x() >= b.pMin.x() && p.x() <= b.pMax.x() && p.y() >= b.pMin.y() &&
                p.y() <= b.pMax.y() && p.z() >= b.pMin.z() && p.z() <= b.pMax.z());
    }
    inline const Vector3f& operator[](int i) const
    {
        return (i == 0) ? pMin : pMax;
    }

    inline bool IntersectP(const Ray& ray, const Vector3f& invDir,
                           const std::array<int, 3>& dirisNeg) const;
};



inline bool Bounds3::IntersectP(const Ray& ray, const Vector3f& invDir,
                                const std::array<int, 3>& dirIsNeg) const
{
// TODO: 求包围盒与光线的是否相交

float t_minx = invDir[0] * (pMin.x() - ray.origin.x());
float t_maxx = invDir[0] * (pMax.x() - ray.origin.x()); 
if (dirIsNeg[0]) {
    float tmp = t_minx;
    t_minx = t_maxx;
    t_maxx = tmp;
}
float t_miny = invDir[1] * (pMin.y() - ray.origin.y());
float t_maxy = invDir[1] * (pMax.y() - ray.origin.y());
if (dirIsNeg[1]) {
    float tmp = t_miny;
    t_miny = t_maxy;
    t_maxy = tmp;
}
float t_minz = invDir[2] * (pMin.z() - ray.origin.z());
float t_maxz = invDir[2] * (pMax.z() - ray.origin.z());
    //注意如果射线是反的则需要交换tMin和tMax的位置

if (dirIsNeg[2]) {
    float tmp = t_minz;
    t_minz = t_maxz;
    t_maxz = tmp;
}
float t_min = std::max(t_miny, std::max(t_minx, t_minz));
float t_max = std::min(t_maxx, std::min(t_maxy, t_maxz));
if (t_min < t_max+0.001 && t_max >= 0) {
    return true;
}
return false;
}

inline Bounds3 Union(const Bounds3& b1, const Bounds3& b2)
{
    Bounds3 ret;
    ret.pMin = Vector3f::Min(b1.pMin, b2.pMin);
    ret.pMax = Vector3f::Max(b1.pMax, b2.pMax);
    return ret;
}

inline Bounds3 Union(const Bounds3& b, const Vector3f& p)
{
    Bounds3 ret;
    ret.pMin = Vector3f::Min(b.pMin, p);
    ret.pMax = Vector3f::Max(b.pMax, p);
    return ret;
}

