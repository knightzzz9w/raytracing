#ifndef RAYTRACING_MATERIAL_H
#define RAYTRACING_MATERIAL_H
#include "texture.hpp"
#include "vecmath.h"

// 粗糙漫反射，反射+折射，仅反射
enum MaterialType { DIFFUSE_AND_GLOSSY, REFLECTION_AND_REFRACTION, REFLECTION };

class Material{
public:
    MaterialType m_type;
    Vector3f m_color  = Vector3f(1,1,1);
    Vector3f m_emission; //emmision的光照
    Vector3f diffuseColor;  //漫反射的颜色
    float ior;  // 折射率
    float diff; //漫反射的系数
    float Ks ;
    float refl , refr;  //反射的系数
    float specularExponent = 5;   //高光的次数
    int tt = 0 ;//是否启用材质的标志
    texture *albedo;
    //Texture tex;

    Vector3f toWorld(const Vector3f &a, const Vector3f &N){
        Vector3f B, C;
        if (std::fabs(N.x()) > std::fabs(N.y())){
            float invLen = 1.0f / std::sqrt(N.x() * N.x() + N.z() * N.z());
            C = Vector3f(N.z() * invLen, 0.0f, -N.x() *invLen);
        }
        else {
            float invLen = 1.0f / std::sqrt(N.y() * N.y() + N.z() * N.z());
            C = Vector3f(0.0f, N.z() * invLen, -N.y() *invLen);
        }
        B = crossProduct(C, N);
        if(dotProduct(a, N) >= 0)
        {
            return a.x() * B + a.y() * C + a.z() * N;
        }
        else{
            return -a.x() * B + -a.y() * C + -a.z() * N;
        }
    }


    Material(const Vector3f &d_color, Vector3f &e_color,  Vector3f &color_ , float  refl_ = 0,  float diff_ = 0, float refr_ = 0 , 
    float tt_ = 0 ,  char *filename = nullptr , float Ks_ = 0  ,  float ior_ = 1.5  , float specularExponent_  = 5  ) : 
    diffuseColor(d_color), m_emission(e_color), m_color(color_), 
    refl(refl_), diff(diff_), refr(refr_), tt(tt_) , Ks(Ks_), ior(ior_) , specularExponent(specularExponent_)
    {
        // if(tt == 1) albedo = new checker_texture(Vector3f(0.2, 0.3, 0.1), Vector3f(0.9, 0.9, 0.9));
        if (tt == 0)
        {
            // 单色
            albedo = new solid_color(color_);
        }
        else if (tt == 1)
        {
            // 图片
            albedo = new image_texture(filename);
        }
    }
    inline Material( Vector3f color=Vector3f(1,1,1), Vector3f e=Vector3f(0,0,0));
    inline MaterialType getType();
    inline Vector3f getColor();
    inline Vector3f getColorAt(double u, double v , Vector3f& pos);
    inline Vector3f getEmission();
    inline bool hasEmission();

    inline Vector3f sample(const Vector3f &wi, const Vector3f &N);

    inline float pdf(const Vector3f &wi, const Vector3f &wo, const Vector3f &N);

    inline Vector3f eval(const Vector3f &wi, const Vector3f &wo, const Vector3f &N);


};


    

Vector3f Material::getColor(){return m_color;}
Vector3f Material::getEmission() {return m_emission;}
bool Material::hasEmission() {
    if (m_emission.norm() > EPSILON) return true;
    else return false;
}


MaterialType Material::getType()
{
    return m_type;
}

Vector3f Material::getColorAt(double u, double v , Vector3f& pos) {
    return albedo->value(u, v, pos);  //pos在这里面其实没有用到啊
}



Vector3f Material::sample(const Vector3f &wi, const Vector3f &N){
    if (diff > 0.0001)
    {
        // uniform sample on the hemisphere
        float x_1 = get_random_float(), x_2 = get_random_float();
        float z = std::fabs(1.0f - 2.0f * x_1);
        float r = std::sqrt(1.0f - z * z), phi = 2 * M_PI * x_2;
        Vector3f localRay(r*std::cos(phi), r*std::sin(phi), z);
        return toWorld(localRay, N);
        
    }
    else
    {
        return Vector3f(0,0,0);
    }
    
    
}

float Material::pdf(const Vector3f &wi, const Vector3f &wo, const Vector3f &N){
    // if (diff > 0.0001){
    //     // uniform sample probability 1 / (2 * PI)
    //     if (dotProduct(wo, N) > 0.0f)
            return 0.5f / M_PI;
        // else
        //     return 0.0f;
    //}
}

Vector3f Material::eval(const Vector3f &wi, const Vector3f &wo, const Vector3f &N){
    if (diff > 0.0001){
            // calculate the contribution of diffuse   model
            // float cosalpha = dotProduct(N, wo);
            // if (cosalpha > 0.0f) {
                Vector3f diffuse = diff / M_PI;
                return diffuse;
            // }
            // else
            //     return Vector3f(0.0f);
        
    }
}

#endif


