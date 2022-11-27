#ifndef  TRIANGLE_H
#define TRIANGLE_H


#include "BVH.hpp"
#include "Intersection.hpp"
#include "Material.hpp"
#include "Object.hpp"
#include <cassert>
#include <array>

inline bool rayTriangleIntersect(const Vector3f& v0, const Vector3f& v1,
                          const Vector3f& v2, const Vector3f& orig,
                          const Vector3f& dir, float& tnear, float& u, float& v)
{
    //std::cout <<"v0 is " << v0 << " v1 is " << v1 << "v2 is " << v2 <<  std::endl;
    Vector3f edge1 = v1 - v0;
    Vector3f edge2 = v2 - v0;
    Vector3f T = orig - v0;
    Vector3f P = crossProduct(dir, edge2);
    Vector3f Q = crossProduct(T, edge1);
    float D = dotProduct( edge1 , P );  //最下面的参数
    if(abs(D)  < EPSILON)  //很小，病态或者无解
    {
        return false;
    }
    u = dotProduct(T, P)/D;   //先判断U的值
    if (u < 0.0f || u > 1)
        return false;

    v = dotProduct(dir, Q)/D;

    if (v < 0.0f || u + v > 1)
            return false;

    tnear = dotProduct(Q,edge2)/D;

    //std::cout <<"tnear  is "<< tnear << std::endl;
    if(tnear < 0)
    {
        return false;
    }
    return true;
}

class Triangle : public Object
{
public:
    Vector3f v0, v1, v2; // vertices A, B ,C , counter-clockwise order
    Vector3f e1, e2;     // 2 edges v1-v0, v2-v0;
    Vector3f t0, t1, t2; // texture coords
    Vector3f normal;
    float area;
    Material* m;

    Triangle(Vector3f _v0, Vector3f _v1, Vector3f _v2, Material* _m = nullptr)
        : v0(_v0), v1(_v1), v2(_v2), m(_m)
    {
        e1 = v1 - v0;
        e2 = v2 - v0;
        normal = normalize(crossProduct(e1, e2));
        area = crossProduct(e1, e2).norm()*0.5f;
    }

    bool intersect(const Ray& ray) override;
    bool intersect(const Ray& ray, float& tnear,
                   uint32_t& index  , float &u, float &v  ) const override;
    Intersection getIntersection(Ray ray) override;
    void getSurfaceProperties(const Vector3f& P, const Vector3f& I,
                              const uint32_t& index, const Vector2f& uv,
                              Vector3f& N, Vector2f& st) const override
    {
        N = normal;
        //        throw std::runtime_error("triangle::getSurfaceProperties not
        //        implemented.");
    }
    Vector3f evalDiffuseColor(const Vector2f&) const override;
    Bounds3 getBounds() override;
    void Sample(Intersection &pos, float &pdf){
        float x = std::sqrt(get_random_float()), y = get_random_float();
        pos.coords = v0 * (1.0f - x) + v1 * (x * (1.0f - y)) + v2 * (x * y);
        pos.normal = this->normal;
        pdf = 1.0f / area;
    }
    float getArea(){
        return area;
    }
    bool hasEmit(){
        return m->hasEmission();
    }
};

class MeshTriangle : public Object
{
public:
    MeshTriangle(const std::string& filename, Material *mt = new Material() , Matrix4f trans_matrix =  Matrix4f::identity())
    {
        objl::Loader loader;
        loader.LoadFile(filename);
        area = 0;
        m = mt;
        assert(loader.LoadedMeshes.size() == 1);
        auto mesh = loader.LoadedMeshes[0];

        Vector3f min_vert = Vector3f{std::numeric_limits<float>::infinity(),
                                     std::numeric_limits<float>::infinity(),
                                     std::numeric_limits<float>::infinity()};
        Vector3f max_vert = Vector3f{-std::numeric_limits<float>::infinity(),
                                     -std::numeric_limits<float>::infinity(),
                                     -std::numeric_limits<float>::infinity()};
        numTriangles = mesh.Vertices.size()/3;
        cout << "Num triangles: " << numTriangles << endl;
        for (int i = 0; i < mesh.Vertices.size(); i += 3) {
            std::array<Vector3f, 3> face_vertices;
            for (int j = 0; j < 3; j++) {
                auto vert_last = Vector3f(mesh.Vertices[i + j].Position.X,
                                     mesh.Vertices[i + j].Position.Y,
                                     mesh.Vertices[i + j].Position.Z);
                Vector4f v_cur(vert_last , 1);
                Vector4f v_cur_trans = trans_matrix*v_cur;
                Vector3f vert(v_cur_trans.x() , v_cur_trans.y() , v_cur_trans.z());
                face_vertices[j] = vert;
                vertices.push_back(vert); //当前三角形的顶点
                min_vert = Vector3f(std::min(min_vert.x(), vert.x()),
                                    std::min(min_vert.y(), vert.y()),
                                    std::min(min_vert.z(), vert.z()));
                max_vert = Vector3f(std::max(max_vert.x(), vert.x()),
                                    std::max(max_vert.y(), vert.y()),
                                    std::max(max_vert.z(), vert.z()));
            }

            triangles.emplace_back(face_vertices[0], face_vertices[1],
                                   face_vertices[2], mt);
            
        }

        bounding_box = Bounds3(min_vert, max_vert);

        std::vector<Object*> ptrs;
        for (auto& tri : triangles){
            ptrs.push_back(&tri);
            area += tri.area;
        }
        bvh = new BVHAccel(ptrs);
    }

    bool intersect(const Ray& ray) { return true; }

    bool intersect(const Ray& ray, float& tnear, uint32_t& index , float &final_u , float &final_v) const
    {
        bool intersect = false;
        for (uint32_t k = 0; k < numTriangles; ++k) {
            const Vector3f& v0 = vertices[k * 3];
            const Vector3f& v1 = vertices[k * 3 + 1];
            const Vector3f& v2 = vertices[k * 3 + 2];
            float t, u, v;
            if (rayTriangleIntersect(v0, v1, v2, ray.origin, ray.direction, t,
                                     u, v) &&
                t < tnear) {
                tnear = t;
                index = k;
                intersect |= true;
                final_u = u; final_v = v;
            }
        }

        return intersect;
    }  //vertexIndex之后也没有什么用了

    Bounds3 getBounds() { return bounding_box; }

    void getSurfaceProperties(const Vector3f &P, const Vector3f &I,  const uint32_t& index, const Vector2f& uv,
                              Vector3f& N, Vector2f& st) const
    {
        const Vector3f& v0 = vertices[index * 3];
        const Vector3f& v1 = vertices[index * 3 + 1];
        const Vector3f& v2 = vertices[index * 3 + 2];
        Vector3f e0 = normalize(v1 - v0);
        Vector3f e1 = normalize(v2 - v1);
        N = normalize(crossProduct(e0, e1));
        const Vector2f& st0 = stCoordinates[index * 3];
        const Vector2f& st1 = stCoordinates[index * 3 + 1];
        const Vector2f& st2 = stCoordinates[index * 3 + 2];
        st = st0 * (1 - uv.x() - uv.y()) + st1 * uv.x() + st2 * uv.y();
    }

    Vector3f evalDiffuseColor(const Vector2f& st) const
    {
        float scale = 5;
        float pattern =
            (fmodf(st.x() * scale, 1) > 0.5) ^ (fmodf(st.y() * scale, 1) > 0.5);
        return Vector3f::lerp(Vector3f(0.815, 0.235, 0.031),
                    Vector3f(0.937, 0.937, 0.231), pattern);
    }

    Intersection getIntersection(Ray ray)
    {
        Intersection intersec;


        // std::cout << "numTriangles is : " << numTriangles << std::endl;
        // std::cout << "vertices size is : " << vertices.size() << std::endl;
        // std::cout << "object material is : " << this->m->tt << std::endl;
        // std::cout << "index size is : " << sizeof(vertexIndex)/sizeof(uint32_t) << std::endl;
        // std::cout << "stCoordinates size is : " << sizeof(stCoordinates)/sizeof(Vector2f) << std::endl;
        
        //std::cout << "here triangles bvh : " << vertices.size() << std::endl;
        if (bvh) {
            intersec = bvh->Intersect(ray);//现在交点的信息以及材质是什么
            //std::cout << "here we use bvh instead " << std::endl;
        }
        
        // else if( this-> m->tt == 1)  //需要求出交点的u和v的信息
        // {
        //     float tnear_ ; uint32_t index_;
        //     float u,v;
        //     if(intersect( ray,  tnear_,   index_ , u,v))
        //     {
        //         Vector2f st; Vector3f N , P , I;
        //          getSurfaceProperties(P,I, index_,  Vector2f(u,v),  N, st) ;
        //          Vector3f cur(0,0,0);
        //         intersec.color = this->m->getColorAt(  st.x()   ,  st.y() ,   cur); //材质的颜色 , t0其实没有用上
        //         intersec.m = this->m;
        //         intersec.obj = this;
        //         intersec.distance = tnear_;
        //         intersec.iftt = true;
        //     } 
        // }

        return intersec;
    }
    
    void Sample(Intersection &pos, float &pdf){
        bvh->Sample(pos, pdf);
        pos.emit = m->getEmission();
    }
    float getArea(){
        return area;
    }
    bool hasEmit(){
        return m->hasEmission();
    }

    Bounds3 bounding_box;
    std::vector<Vector3f> vertices;
    uint32_t numTriangles;
    std::unique_ptr<uint32_t[]> vertexIndex;
    std::unique_ptr<Vector2f[]> stCoordinates;

    std::vector<Triangle> triangles;

    BVHAccel* bvh;
    float area;

    Material* m;
};

inline bool Triangle::intersect(const Ray& ray) { return true; }
inline bool Triangle::intersect(const Ray& ray, float& tnear,
                                uint32_t& index , float &u, float &v) const
{
    return false;
}

inline Bounds3 Triangle::getBounds() { 
    //std::cout <<    "v0 is  " << v0   <<"and v1 " <<  v1 <<"and v2 " << v2  << std::endl;
    return Union(Bounds3(v0, v1), v2); }

inline Intersection Triangle::getIntersection(Ray ray)
{
    Intersection inter;
    float  u,v,tmp;
    bool intersect_result = rayTriangleIntersect(v0 , v1  ,v2 , ray.origin , ray.direction , tmp , u, v );
    if(intersect_result == false) return inter;
    inter.happened = true;
    inter.coords = ray.origin+ tmp* ray.direction;
    Vector3f pos_cur;
    inter.color = m->getColorAt(u,v,pos_cur);
    inter.normal = normal;
    inter.distance = tmp;//已经归一化了
     //   std::cout << "tmp is " <<  tmp << std::endl;
    inter.obj = this;
    inter.m = m;
    inter.emit  = m -> m_emission;

    return inter;
}

inline Vector3f Triangle::evalDiffuseColor(const Vector2f&) const
{
    return Vector3f(0.5, 0.5, 0.5);
}


#endif