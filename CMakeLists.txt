cmake_minimum_required(VERSION 3.10)
project(RayTracing)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Debug")
enable_language(CUDA)
find_package(CUDA REQUIRED)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
find_package(GLUT REQUIRED)
#find_package(Eigen3 REQUIRED)
include_directories(  ${GLUT_INCLUDE_DIRS} include  include/vecmath/include ${CUDA_INCLUDE_DIRS})
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb -g")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
SET(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_61,code=sm_61;)

#aux_source_directory(${PROJECT_SOURCE_DIR}/include  DIR_SRCS)
file(GLOB_RECURSE DIR_SRCS include/*.h   include/*.hpp   include/*.cpp)
message(STATUS "ALL_src IS  ${DIR_SRCS} " )
add_library( ALL_LIB STATIC ${DIR_SRCS}   )
target_link_libraries(ALL_LIB PUBLIC ${PROJECT_SOURCE_DIR}/lib/libglut.so.3)
message(STATUS "ALL_LIB IS  ${ALL_LIB} " )
message(STATUS "ALL_LIB IS  ${GLUT_LIBRARIES} " )

link_directories(${LIBRARY_OUTPUT_PATH})

add_executable(RayTracing Object.hpp   Sphere.hpp global.hpp Triangle.hpp  main.cpp Scene.cpp
Scene.hpp Light.hpp AreaLight.hpp BVH.cpp BVH.hpp Bounds3.hpp Ray.hpp Material.hpp Intersection.hpp
Renderer.cpp Renderer.hpp  scene_parser.hpp  scene_parser.cpp texture.hpp  texture.cpp stb_image.h camera.hpp Renderer.cu Renderer.cuh)
target_link_libraries(RayTracing PUBLIC ALL_LIB ${PROJECT_SOURCE_DIR}/lib/libglut.so
${PROJECT_SOURCE_DIR}/lib/libglut.a ${PROJECT_SOURCE_DIR}/lib/libglut.so.3
)