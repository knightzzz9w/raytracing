#ifndef SCENE_PARSER_H
#define SCENE_PARSER_H

#include <cassert>
#include <vecmath.h>
#include "Triangle.hpp"
#include <GL/freeglut.h>
#include "camera.hpp"
#include "Material.hpp"
#include "Sphere.hpp"
//#include "transform.hpp"


#define MAX_PARSER_TOKEN_LENGTH 1024

class SceneParser {
public:

    SceneParser() = delete;
    SceneParser(const char *filename);

    ~SceneParser();

    Camera *getCamera() const {
        return camera;
    }

    Vector3f getBackgroundColor() const {
        return background_color;
    }

    // int getNumLights() const {
    //     return num_lights;
    // }

    // Light *getLight(int i) const {
    //     assert(i >= 0 && i < num_lights);
    //     return lights[i];
    // }

    int getNumMaterials() const {
        return num_materials;
    }

    Material *getMaterial(int i) const {
        assert(i >= 0 && i < num_materials);
        return materials[i];
    }

    // Group *getGroup() const {
    //     return group;
    // }

    void parseFile();
    void parsePerspectiveCamera();
    void parseBackground();   //对应的背景颜色是什么
    // void parseLights();
    // Light *parsePointLight();
    // Light *parseDirectionalLight();   
    void parseMaterials();   //存储在materials的二维数组里面
    Material *parseMaterial();
    void parseObjects();   //所有的物体并且赋予旋转矩阵
    //Group *parseGroup();
    Sphere *parseSphere();    //返回这个球的所有参数
    MeshTriangle *parseTriangleMesh() ;
    MeshTriangle *parseRectangle() ;
    Matrix4f parseTransform();    //要返回物体所对应的矩阵是什么

    int getToken(char token[MAX_PARSER_TOKEN_LENGTH]);  

    Vector3f readVector3f();

    float readFloat();
    int readInt();

    FILE *file;
    Camera *camera;
    Vector3f background_color;
    int num_objects;   //并且要设置这些物体的颜色哦;
    int num_materials;
    Material **materials;
    Material *current_material;
    Object **objects;
    Object *current_object;
};

#endif // SCENE_PARSER_H
