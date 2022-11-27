#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <GL/freeglut.h>
#include "scene_parser.hpp"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DegreesToRadians(x) ((M_PI * x) / 180.0f)

SceneParser::SceneParser(const char *filename) {

    // initialize some reasonable default values
    //group = nullptr;
    camera = nullptr;
    background_color = Vector3f(0.5, 0.5, 0.5);
    //num_lights = 0;   
    num_materials = 0;
    materials = nullptr;
    current_material = nullptr;

    // parse the file
    assert(filename != nullptr);
    const char *ext = &filename[strlen(filename) - 4];

    if (strcmp(ext, ".txt") != 0) {
        printf("wrong file name extension\n");
        exit(0);
    }
    file = fopen(filename, "r");

    if (file == nullptr) {
        printf("cannot open scene file\n");
        exit(0);
    }
    parseFile();
    fclose(file);
    file = nullptr;

    // if (num_lights == 0) {
    //     printf("WARNING:    No lights specified\n");
    // }
}

SceneParser::~SceneParser() {  //等待处理

    //delete group;
    delete camera;

    int i;
    for (i = 0; i < num_materials; i++) {
        delete materials[i];
    }
    delete[] materials;
    // for (i = 0; i < num_lights; i++) {
    //     delete lights[i];
    // }
}

// ====================================================================
// ====================================================================

void SceneParser::parseFile() {
    //
    // at the top level, the scene can have a camera, 
    // background color and a group of objects
    // (we add lights and other things in future assignments)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    while (getToken(token)) {
        if (!strcmp(token, "PerspectiveCamera")) {
            parsePerspectiveCamera();
        } else if (!strcmp(token, "Background")) {
            parseBackground();
        }  else if (!strcmp(token, "Materials")) {
            parseMaterials();
        } else if (!strcmp(token, "Objects")) {
            parseObjects();
        } 
        else {
            printf("Unknown token in parseFile: '%s'\n", token);
            exit(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser::parsePerspectiveCamera() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the camera parameters
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "center"));
    Vector3f center = readVector3f();
    getToken(token);
    assert (!strcmp(token, "direction"));
    Vector3f direction = readVector3f();
    getToken(token);
    assert (!strcmp(token, "up"));
    Vector3f up = readVector3f();
    getToken(token);
    assert (!strcmp(token, "angle"));
    float angle_degrees = readFloat();
    float angle_radians = DegreesToRadians(angle_degrees);
    getToken(token);
    assert (!strcmp(token, "width"));
    int width = readInt();
    getToken(token);
    assert (!strcmp(token, "height"));
    int height = readInt();
    getToken(token);
    assert (!strcmp(token, "}"));
    camera =  new PerspectiveCamera(center, direction, up, width, height, angle_radians);     //返回的是相机的视角  qaqqaq，完结
}

void SceneParser::parseBackground() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the background color
    getToken(token);
    assert (!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (!strcmp(token, "}")) {
            break;
        } else if (!strcmp(token, "color")) {
            background_color = readVector3f();
        } else {
            printf("Unknown token in parseBackground: '%s'\n", token);
            assert(0);
        }
    }
}   //背景的颜色

// ====================================================================
// ====================================================================





// ====================================================================
// ====================================================================

void SceneParser::parseMaterials() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    // read in the number of objects
    getToken(token);
    assert (!strcmp(token, "numMaterials"));
    num_materials = readInt();
    materials = new Material *[num_materials];
    // read in the objects
    int count = 0;
    while (num_materials > count) {
        getToken(token);
        if (!strcmp(token, "Material") ||
            !strcmp(token, "PhongMaterial")) {
            materials[count] = parseMaterial();
        } else {
            printf("Unknown token in parseMaterial: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token);
    assert (!strcmp(token, "}"));
}    //Material的个数


Material *SceneParser::parseMaterial() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    char filename[MAX_PARSER_TOKEN_LENGTH];
    filename[0] = 0;
    Vector3f diffuseColor(1, 1, 1), specularColor(0, 0, 0), emissionColor(0,0,0);
    int type = 2;
    float refl = 0, diff = 0, spec = 0, refr = 0 , ior = 1.5  , specularExponent  = 5, Ks = 0;
    Vector3f color(0,0,0);
    int tt = 0;//储存是否是纹理
    float shininess = 0;
    getToken(token);
    assert (!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (strcmp(token, "diffuseColor") == 0) {
            diffuseColor = readVector3f();   //仅仅是光源的反射光
        } else if (strcmp(token, "emissionColor") == 0) {
            emissionColor = readVector3f();
        }  else if (strcmp(token, "color") == 0) {
            color = readVector3f();     //非光源的物体，自身带有的光的属性
        } else if (strcmp(token, "refl") == 0) {
            refl = readFloat();  //反射的效率
        }   else if (strcmp(token, "diff") == 0) {  //Kd
            diff = readFloat();
        }  else if (strcmp(token, "refr") == 0) {   //refr
            refr = readFloat();
        }   else if (strcmp(token, "texture") == 0) {
            // Optional: read in texture and draw it.
            getToken(filename);
            tt = 1;  //是否是带有纹理的物体
        }
        else if (strcmp(token, "Ks") == 0) {
            // Optional: read in texture and draw it.
            Ks = readFloat();   //高光 反射
        }  
        else if (strcmp(token, "ior") == 0) {
            // Optional: read in texture and draw it.
            ior = readFloat(); //是否是带有纹理的物体
        } 
        else if (strcmp(token, "specularExponent") == 0) {
            // Optional: read in texture and draw it.
            specularExponent = readFloat(); //是否是带有纹理的物体
        } 
        else {   //哪种颜色的材质在这里没有什么用处，只分贴图，和光源材质
            assert (!strcmp(token, "}"));
            break;
        }
    }
    auto *answer = 
    new Material(diffuseColor, emissionColor,  color , refl, diff,refr ,  tt, filename, Ks, ior, specularExponent);  //Material的构造函数
    return answer;
}

// ====================================================================
// ====================================================================


void SceneParser::parseObjects() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    // read in the number of objects
    getToken(token);
    assert (!strcmp(token, "numObjects"));
    num_objects = readInt();
    objects = new Object *[num_objects];
    // read in the objects
    int count = 0;
    while (num_objects > count) {
        //std::cout << "count is " << count << std::endl;
        getToken(token);
        if (!strcmp(token, "Sphere") ) {
            objects[count] = parseSphere();
        } else if (!strcmp(token, "Mesh"))
        {
            objects[count] = parseTriangleMesh();
        }
        else if (!strcmp(token, "Rectangle"))
        {
            objects[count] = parseRectangle();
    }
        count++;
    }
    getToken(token);
    std::cout << "token is " << token << std::endl;
    assert (!strcmp(token, "}"));
}    //Material的个数



// ====================================================================
// ====================================================================


// ====================================================================
// ====================================================================

Sphere *SceneParser::parseSphere() {
    std::cout << "parseSphere" << std::endl;
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "radius"));
    float radius = readFloat();
    getToken(token);
    assert (!strcmp(token, "center"));
    Vector3f center = readVector3f();
    getToken(token);
    assert (!strcmp(token, "MaterialIndex"));
    int Material_ind = readInt();
    getToken(token);
    //std::cout << "token is " <<     token << std::endl;
    assert (strcmp(token, "}") == 0);
    //std::cout << "continue one sphere" << std::endl;
    //assert (current_material != nullptr);
    return new Sphere(center, radius, materials[Material_ind - 1]);   //记住要减一
}





MeshTriangle *SceneParser::parseTriangleMesh() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    char filename[MAX_PARSER_TOKEN_LENGTH];
    // get the filename
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "filepath"));

    getToken(filename);
    const char *ext = &filename[strlen(filename) - 4];
    assert(!strcmp(ext, ".obj"));

    getToken(token);
    assert (!strcmp(token, "MaterialIndex"));
    int Material_ind = readInt();

    getToken(token);    
    assert (!strcmp(token, "Transform"));
    Matrix4f parse_Transform = parseTransform();   //得到物体的旋转矩阵
    getToken(token);
    assert (!strcmp(token, "}"));//所有的矩阵加在一起
    return  new MeshTriangle( filename , materials[Material_ind-1] , parse_Transform);
}



MeshTriangle *SceneParser::parseRectangle() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    char filename[MAX_PARSER_TOKEN_LENGTH];
    // get the filename
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "filepath"));

    getToken(filename);
    const char *ext = &filename[strlen(filename) - 4];
    assert(!strcmp(ext, ".obj"));

    getToken(token);
    assert (!strcmp(token, "MaterialIndex"));
    int Material_ind = readInt();

    getToken(token);    
    assert (!strcmp(token, "Transform"));
    Matrix4f parse_Transform = parseTransform();   //得到物体的旋转矩阵
    getToken(token);
    assert (!strcmp(token, "}"));//所有的矩阵加在一起
    MeshTriangle* newmeshtri =   new MeshTriangle( filename , materials[Material_ind-1] , parse_Transform);
    Vector2f st[6] = {{0, 0}, {1, 0}, {1, 1}, {0, 0} , {1,1}  , {0,1}  };  
    newmeshtri->stCoordinates = std::unique_ptr<Vector2f[]>(new Vector2f[6]);
    memcpy(newmeshtri->stCoordinates.get(), st, sizeof(Vector2f) * 6);

    uint32_t vertIndex[6] = {0, 1, 2, 0, 2, 3};
    newmeshtri->vertexIndex = std::unique_ptr<uint32_t[]>(new uint32_t[6]);
    memcpy(newmeshtri->vertexIndex.get(), vertIndex, sizeof(uint32_t) * 2 * 3);
    return newmeshtri;
}




Matrix4f SceneParser::parseTransform() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Matrix4f matrix = Matrix4f::identity();
    getToken(token);
    assert (!strcmp(token, "{"));
    // read in transformations: 
    // apply to the LEFT side of the current matrix (so the first
    // transform in the list is the last applied to the object)
    Vector3f s;
    Vector3f t;
    while (true) {
        getToken(token);
        if (!strcmp(token, "Scale")) {
            s = readVector3f();
            matrix =  Matrix4f::scaling(s[0], s[1], s[2])*matrix;
        } else if (!strcmp(token, "Translate")) {
            t = readVector3f();
            matrix = Matrix4f::translation(t)*matrix;
        } else if (!strcmp(token, "XRotate")) {
            matrix = Matrix4f::rotateX(DegreesToRadians(readFloat()))*matrix;;
        } else if (!strcmp(token, "YRotate")) {
            matrix = Matrix4f::rotateY(DegreesToRadians(readFloat()))*matrix ;
        } else if (!strcmp(token, "ZRotate")) {
            matrix =  Matrix4f::rotateZ(DegreesToRadians(readFloat()))*matrix;
        } else if (!strcmp(token, "Rotate")) {
            getToken(token);
            assert (!strcmp(token, "{"));
            Vector3f axis = readVector3f();
            float degrees = readFloat();
            float radians = DegreesToRadians(degrees);
            matrix = Matrix4f::rotation(axis, radians)*matrix ;
            getToken(token);
            assert (!strcmp(token, "}"));
        } else if (!strcmp(token, "Matrix4f")) {
            Matrix4f matrix2 = Matrix4f::identity();
            getToken(token);
            assert (!strcmp(token, "{"));
            for (int j = 0; j < 4; j++) {
                for (int i = 0; i < 4; i++) {
                    float v = readFloat();
                    matrix2(i, j) = v;
                }
            }
            getToken(token);
            assert (!strcmp(token, "}"));
            matrix = matrix2 * matrix;
        } else {
            std::cout << "token is " << token << std::endl;
            assert (!strcmp(token, "}"));
            break;
        }
    }

    return matrix;
}

// ====================================================================
// ====================================================================

int SceneParser::getToken(char token[MAX_PARSER_TOKEN_LENGTH]) {
    // for simplicity, tokens must be separated by whitespace
    assert (file != nullptr);
    int success = fscanf(file, "%s ", token);
    if (success == EOF) {
        token[0] = '\0';
        return 0;
    }
    return 1;
}


Vector3f SceneParser::readVector3f() {
    float x, y, z;
    int count = fscanf(file, "%f %f %f", &x, &y, &z);
    std::cout << count << " " << x << " " << y << " " << z << std::endl;
    if (count != 3) {
        printf("Error trying to read 3 floats to make a Vector3f\n");
        assert (0);
    }
    return Vector3f(x, y, z);
}


float SceneParser::readFloat() {
    float answer;
    int count = fscanf(file, "%f", &answer);
    if (count != 1) {
        printf("Error trying to read 1 float\n");
        assert (0);
    }
    return answer;
}


int SceneParser::readInt() {
    int answer;
    int count = fscanf(file, "%d", &answer);
    if (count != 1) {
        printf("Error trying to read 1 int\n");
        assert (0);
    }
    return answer;
}
