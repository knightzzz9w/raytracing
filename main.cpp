#include "Renderer.hpp"
#include "OBJ_Loader.hpp"
#include "scene_parser.hpp"
#include "Scene.hpp"
#include "Sphere.hpp"
#include "vecmath.h"
#include "global.hpp"
#include <chrono>

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for th#include "vecmath.h"e render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char** argv)
{
    // 构建场景
    //Scene scene(1280, 960);

    // MeshTriangle bunny("../models/bunny/bunny.obj");
    
    // // 添加场景内的点光源和面光源
    // //std::cout << "scene obj size is " << scene.objects.size() <<std::endl;

    // MeshTriangle dragon("../models/Armadillo.obj");    
    //auto sph1 =  new Sphere(Vector3f(-1, 0, -12), 10.0f);
    SceneParser parser(argv[1]);  //文件的位置
    Scene scene(parser.getCamera()->getWidth() , parser.getCamera()->getHeight());
    for(int i = 0; i < parser.num_objects;  i++ )
    {
        scene.Add(parser.objects[i]);
        std::cout << "parser.num_objects i  " << i << std::endl;
    }
    scene.camera = parser.getCamera();
    scene.backgroundColor = parser.background_color;






    // scene.Add(sph1);
    // scene.Add(&bunny);
    //scene.Add(&dragon);
    // scene.Add(std::make_unique<Light>(Vector3f(-20, 70, 20), 1));
    // scene.Add(std::make_unique<Light>(Vector3f(20, 70, 20), 1));
    // 如果使用BVH就需要build build后之后Scene.useBVH将会使用BVH
    scene.buildBVH();

    // 渲染器
    Renderer r;

    auto start = std::chrono::system_clock::now();
    std::cout << "Starting "<< std::endl;
    std::cout << "object size is  "<< scene.objects.size()  <<std::endl;
    r.Render(scene);
    auto stop = std::chrono::system_clock::now();

    std::cout << "Render complete: \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";

    return 0;
}