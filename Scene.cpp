#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
    this->useBVH = true;
}

Intersection Scene::intersect(const Ray &ray) const
{

    if (useBVH) {   //是否开启bvh的应用
        return this->bvh->Intersect(ray);
    } else {
        Intersection ret;
        float tNear = kInfinity;
         for (const auto & object : objects) {
            auto temp = object -> getIntersection(ray);
            if (temp.distance < tNear) {
                tNear = temp.distance;
                ret = temp;
            }
        }
        return ret;
    }
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}


bool Scene::trace (
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject) const
{
    *hitObject = nullptr;
    std::cout << "tracing" << std::endl;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        std::cout << "k is " << k << std::endl;
        std::cout << "k is " << k << std::endl;
        std::cout << "k is " << k << std::endl;
        float u , v;
        if (objects[k]->intersect(ray, tNearK, indexK , u, v) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of the Whitted-syle light transport algorithm (E [S*] (D|G) L)
//
// This function is the function that compute the color at the intersection point
// of a ray defined by a position and a direction. Note that thus function is recursive (it calls itself).
//
// If the material of the intersected object is either reflective or reflective and refractive,
// then we compute the reflection/refracton direction and cast two new rays into the scene
// by calling the castRay() function recursively. When the surface is transparent, we mix
// the reflection and refraction color using the result of the fresnel equations (it computes
// the amount of reflection and refractin depending on the surface normal, incident view direction
// and surface refractive index).
//
// If the surface is duffuse/glossy we use the Phong illumation model to compute the color
// at the intersection point
Vector3f Scene::castRay(const Ray &ray, Vector3f & RayC  ,   int depth) const
{  //还要有目前的打光 , 光照的强度全部体现在RayC 上面
    if (depth > this->maxDepth) {
        return Vector3f(0.0,0.0,0.0);
    }
    bool iftt; //是否有贴图
    int u , v;  //其中的交点u，v，来计算在其中贴图的颜色
    Intersection intersection = Scene::intersect(ray);
    Material *m = intersection.m;
    Object *hitObject = intersection.obj;
    Vector3f hitColor = Vector3f(0,0,0); //初始化为背景颜色
    Vector2f uv;
    uint32_t index = 0;
    if(intersection.happened) {

        if(intersection.emit.norm() > 0.002)
        {
            hitColor = Vector3f(1,1,1);
        } 
        else{
             Vector3f hitPoint = intersection.coords; //hitpoint和normal
            // std::cout << "Hit point is " << hitPoint << std::endl;
            Vector3f N = intersection.normal; // normal
            Vector2f st; // st coordinates
            //hitObject->getSurfaceProperties(hitPoint, ray.direction, index, uv, N, st);  //没有必要
        
            if(m->refl > 0.0001 )
            {
                //std::cout << "reflection" << std::endl;
                Vector3f reflect_dir = normalize(reflect(ray.direction, N));
//重新在hitPoint生成两条光线
                
                Vector3f new_reflect_Orig = hitPoint ;
                if(dotProduct(reflect_dir, N) < 0)   //在这个表面的外部
                {
                    new_reflect_Orig += -N * EPSILON;
                }
                else{
                    new_reflect_Orig += N * EPSILON;
                }

                Ray new_ray_reflect(new_reflect_Orig , reflect_dir);
                Vector3f newrayC = (RayC*m->refl)*intersection.color;  //反射的颜色
                Vector3f reflectionColor = castRay(new_ray_reflect , newrayC , depth + 1);  //物体本身的颜色
                float kr ;
                fresnel(ray.direction, N, m->ior  , kr);
                hitColor += reflectionColor * kr ;
            }

            if(m->refr > 0.0001)
            {
                //std::cout << "refraction" << std::endl;
                Vector3f refrACT_dir = normalize(refract(ray.direction, N, m->ior));

                if(refrACT_dir.norm() > 0.5)
                {
                    Vector3f new_refrACT_Orig = hitPoint ;
                    if(dotProduct(refrACT_dir, N) < 0)   //在这个表面的外部
                    {
                        new_refrACT_Orig += -N * EPSILON;
                    }
                    else{
                        new_refrACT_Orig += N * EPSILON;
                    }
                    Ray new_ray_refrACT(new_refrACT_Orig , refrACT_dir);
                    Vector3f newrayC = RayC*m->refr*intersection.color;
                    Vector3f refractionColor = castRay(new_ray_refrACT ,newrayC , depth + 1);
                    float kr ;
                    fresnel(ray.direction, N, m->ior  , kr);
                    hitColor  +=  refractionColor * (1 - kr);
                }

            }

            if(m->Ks > 0.001)
            {
                //std::cout << " KS IS " << m->Ks << std::endl;
                Vector3f specularColor;
                Vector3f wo = normalize(-ray.direction);   //ray的负向
                Vector3f p = intersection.coords; //与材质的交点
                for (uint32_t k = 0; k < 20; ++k) {
                    float pdf_light = 0.0f;
                    Intersection inter;
                    sampleLight(inter,pdf_light);  //light的朝向
                    Vector3f x = inter.coords;
                    Vector3f ws = normalize(x-p);   //到这个光源的向量 
                    Vector3f NN = normalize(inter.normal);    //交点的法向量
                     Vector3f new_p = p ;
                    if( dotProduct( ( x-p) , N) >  0)
                    {
                        new_p = p+N*EPSILON;
                    } 
                    else{
                        new_p = p-N*EPSILON;
                    }

                    Vector3f reflectionDirection = normalize(reflect(p-x, N));
                    Intersection new_intersect = intersect(Ray(new_p,ws));
                    // std::cout << " new_intersect.coords is " <<  new_intersect.coords<< std::endl;
                    // std::cout << " x  is " << x << std::endl;
                    float dot1 = dotProduct(ws,N);
                    if (dot1 < 0)
                    {
                        dot1 = -dot1;
                    }
                    float dot2 = dotProduct(-ws,NN);
                    if (dot2 < 0)
                    {
                        dot2 = -dot2;
                    }
                    if((  new_intersect.coords - x).norm() < 0.01)
                    {
                        specularColor +=RayC*1/20* powf(std::max(0.f, -dotProduct(reflectionDirection, ray.direction)),
                        m->specularExponent) * inter.emit * m->m_color*m->Ks*
                        intersection.color*dot1 * dot2 / (((x-p).norm()* (x-p).norm()) * pdf_light);;
                    }
                }
                hitColor  +=  specularColor;
            }

            if(m->diff > 0.0001)
            {
                //std::cout << " DIFFING " << std::endl;
                Vector3f wo = normalize(-ray.direction);   //ray的负向
                Vector3f p = intersection.coords; //与材质的交点
                Vector3f L_dir = Vector3f(0);  //
                float pdf_light = 0.0f;
                // for(uint32_t k = 0; k < 20; ++k)
                // {
                    Intersection inter;
                    sampleLight(inter,pdf_light);  //light的朝向
                    Vector3f x = inter.coords;
                    Vector3f ws = normalize(x-p);   //到这个光源的向量 
                    Vector3f NN = normalize(inter.normal);    //交点的法向量
                    //std::cout << " DIFFING 3 " << std::endl;
                    //direct light
                    Vector3f new_p = p ;
                    if( dotProduct( ( x-p) , N) > 0)
                    {
                        new_p = p+N*EPSILON;
                    } 
                    else{
                        new_p = p-N*EPSILON;
                    }
                     Intersection new_intersect = intersect(Ray(new_p,ws));
                    //  std::cout << " new_intersect.coords is " <<  new_intersect.coords<< std::endl;
                    //  std::cout << " x  is " << x << std::endl;
                    float dot1 = dotProduct(ws,N);
                    if (dot1 < 0)
                    {
                        dot1 = -dot1;
                    }
                    float dot2 = dotProduct(-ws,NN);
                    if (dot2 < 0)
                    {
                        dot2 = -dot2;
                    }
                    if((new_intersect.coords - x).norm() < 0.01)
                    {
                        L_dir += inter.emit * m->diffuseColor*intersection.color*RayC*intersection.m->eval(wo,ws,N)*dot1 * dot2 / (((x-p).norm()* (x-p).norm()) * pdf_light);
                    }
           // }
                //std::cout << " DIFFING 2 " << std::endl;

                Vector3f L_indir = Vector3f(0);
                Vector3f new_pp  = p;
                if(dotProduct(ray.direction, N) < 0)   //在这个表面的外部
                {
                    new_pp += N * EPSILON;
                }
                else{
                    new_pp += -N * EPSILON;
                }
                // for(int knew = 0; knew < 20; ++knew)
                // {
                    float P_RR = get_random_float();
                    //indirect 
                    if(P_RR < 0.8 )
                    {
                        Vector3f wi = intersection.m->sample(wo,N);
                        float dot1 = dotProduct(wi,N);
                        if (dot1 < 0)
                        {
                            dot1 = -dot1;
                        }
                        Vector3f newrayC = RayC *m->diffuseColor*intersection.color*intersection.m->eval(wi,wo,N) * dot1 / (intersection.m->pdf(wi,wo,N)*0.8) ;
                        //std::cout << "continue"  << std::endl;
                        L_indir += castRay(Ray(new_pp,wi),   newrayC   ,   depth+1);
                    }
                //}
                hitColor += (L_indir + L_dir);
            }

        }
    }  

    return hitColor;
        
}

    
