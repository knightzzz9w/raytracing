//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include "Scene.hpp"
#include "vecmath.h"
#include "Renderer.hpp"
#include "Renderer.cuh"



inline float deg2rad(const float& deg) { return deg * M_PI / 180.0; }

const float EPSILON = 0.001;

// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
void Renderer::Render(const Scene& scene)
{
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = (float)scene.width / (float)scene.height;
    Vector3f eye_pos(-1, 5, 10); //视点位置
    int m = 0;
    int turn_try = 100;
    const int max_depth  = 5; // 光线最深追几层
    // #pragma omp parallel for 
    // for (uint32_t j = 0; j < scene.height; ++j) {
    //     for (uint32_t i = 0; i < scene.width; ++i) {
    //         Ray camRay = scene.camera->generateRay(Vector2f(i, j));
    //         Vector3f RayC(1,1,1);
    //         thread_local Vector3f color(0,0,0);

            
    //         for(int turn = 0; turn < turn_try; ++turn)
    //         {
    //             // std::cout << "i is  " << i   << "j is " << j << " " << "turn is "  <<turn <<std::endl;
    //             //  std::cout << "i is  " << i   << "j is " << j << " " << "turn is "  <<turn <<std::endl;
    //             // float _i = (rand() * 1.0 / RAND_MAX - 0.5) * 1 + i;
    //             // float _j =  (rand() * 1.0 / RAND_MAX - 0.5) * 1 + j;
    //             // float x = (2 * (i + 0.5) / (float)scene.width - 1) *
    //             // imageAspectRatio * scale;
    //             // float y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;
    //             // Vector3f dir = Vector3f(x, y, -1); // Don't forget to normalize this direction!
    //             // dir.normalized();
    //             // Ray camRay = scene.camera->generateRay(Vector2f(i, j));
    //             // //raycast(camRay, Vector3f(1, 1, 1), i, j, 0, current_e, 0);
    //             // // std::cout << "i is " << i << "j is " << j <<  std::endl;
    //             // // std::cout << "camera direction is " << camRay.direction << std::endl;
    //             // Vector3f RayC(1,1,1);
    //             color += scene.castRay(camRay, RayC , 0)/(float)turn_try;   //这条光线的追踪效果

    //         }
    //         framebuffer[j*scene.width + i] +=  color;
    //         #pragma one critical 
    //         {
    //             m++;
    //             UpdateProgress(m / (float)scene.width* (float)scene.height);
    //         }
    //     }
        
    // }
    renderall(framebuffer , scene.width ,  scene.height   , turn_try);
    UpdateProgress(1.f);

    // save framebuffer to file
    FILE* fp = fopen("binary.ppm", "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        static unsigned char color[3];
        color[0] = (unsigned char)(255 * clamp(0, 1, framebuffer[i].x()));
        color[1] = (unsigned char)(255 * clamp(0, 1, framebuffer[i].y()));
        color[2] = (unsigned char)(255 * clamp(0, 1, framebuffer[i].z()));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);    
}
