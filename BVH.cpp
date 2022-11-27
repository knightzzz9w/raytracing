#include <iostream>
#include <cassert>
#include <algorithm>
#include "BVH.hpp"


BVHAccel::BVHAccel(std::vector<Object*> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
      primitives(std::move(p))
{
    time_t start, stop;
    time(&start);
    if (primitives.empty())
        return;

    root = recursiveBuild(primitives);

    time(&stop);
    double diff = difftime(stop, start);
    int hrs = (int)diff / 3600;
    int mins = ((int)diff / 60) - (hrs * 60);
    int secs = (int)diff - (hrs * 3600) - (mins * 60);

    printf(
        "\rBVH Generation complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n",
        hrs, mins, secs);
}

BVHBuildNode* BVHAccel::recursiveBuild(std::vector<Object*> objects)
{
    // TODO: 对objects 递归构造BVH树型结构

   BVHBuildNode* tnode = new BVHBuildNode(); 
    // Compute bounds of all primitives in BVH node
    if(objects.size() == 1)
    {
        tnode->bounds = objects[0]->getBounds();
        tnode->object = objects[0];
        tnode -> left = nullptr;
        tnode->right = nullptr;
        tnode -> area = objects[0]->getArea();

        // std::cout << tnode->bounds.pMin  <<   "  " <<
        // tnode->bounds.pMax <<
        // std::endl;
        return tnode;
    }
    else if (objects.size() ==2)
    {
        tnode->left = recursiveBuild(std::vector<Object*>{objects[0]});
        tnode->right = recursiveBuild(std::vector<Object*>{objects[1]});
        tnode->bounds =  Union(objects[0]->getBounds()    ,   objects[1]->getBounds());
        tnode -> area = objects[0]->getArea() + objects[1]->getArea(); 
         return tnode;
    }

    float convx = 0; float convy = 0; float convz = 0;
    float meanx = 0; float meany = 0; float meanz = 0;
    for(int i = 0; i < objects.size(); i++)
    {
        Vector3f edge =  objects[i]->getBounds().Centroid();
        meanx = meanx*i/(i+1) + edge.x()/(i+1);
        meany = meany*i/(i+1) + edge.y()/(i+1);
        meanz = meanz*i/(i+1) + edge.z()/(i+1);
    }

    for(int i = 0; i < objects.size(); i++)
    {
        Vector3f edge =  objects[i]->getBounds().Centroid();
        convx = convx*i/(i+1) + (edge.x() - meanx)*(edge.x() - meanx)/(i+1);
        convy = convy*i/(i+1) + (edge.y() - meany)*(edge.y() - meany)/(i+1);
        convz = convz*i/(i+1) + (edge.z() - meanz)*(edge.z() - meanz)/(i+1);
    }

    int dim  =  2;

    if (convx > convy && convx > convz)
    {
        dim = 0;
    }
    else if (convy > convz)
    {
        dim = 1;
    }

    if (dim == 0) {
        std::sort(objects.begin(), objects.end(), [](Object* f1, Object* f2) {
            return f1->getBounds().Centroid().x() <
     f2->getBounds().Centroid().x(); });
    }

    else if (dim == 1) {
        std::sort(objects.begin(), objects.end(), [](Object* f1, Object* f2) {
            return f1->getBounds().Centroid().y() <
     f2->getBounds().Centroid().y(); });
    }
    else if(dim == 2) {
        std::sort(objects.begin(), objects.end(), [](Object* f1, Object* f2) {
            return f1->getBounds().Centroid().z() <
     f2->getBounds().Centroid().z(); });
    }
    assert(objects.size() == objects.size());
    auto firstptr   =   objects.begin();
    auto secondptr   =   objects.begin() + objects.size()/2;
    auto thirdptr   =   objects.end() ;


    std::vector <Object * >  leftspace , rightspace;
    leftspace.assign(firstptr, secondptr);
    rightspace.assign(secondptr, thirdptr);
    tnode ->left = recursiveBuild(leftspace);
    tnode ->right = recursiveBuild(rightspace);

    tnode -> bounds = Union( tnode ->left->bounds, tnode ->right->bounds);
    tnode->area = tnode->left->area + tnode->right->area;

    return tnode;


}

Intersection BVHAccel::Intersect(const Ray& ray) const
{
    Intersection isect;
    if (!root)
        return isect;
    isect = BVHAccel::getIntersection(root, ray);
    return isect;
}

Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
    // TODO: 这里求ray和BVH结点的交点
    
    std::array<int, 3> dirIsNeg;
    dirIsNeg[0] = ray.direction.x() < 0 ? 1  : 0;
    dirIsNeg[1] = ray.direction.y() < 0 ? 1  : 0;
    dirIsNeg[2] = ray.direction.z() < 0 ? 1  : 0;

    if(!node->bounds.IntersectP(ray , ray.direction_inv ,dirIsNeg ))  //没有相交
    {
        return Intersection();
    }

    if(node->left == nullptr && node->right == nullptr)
    {
        //std::cout << "we are in the box" << std::endl;
        //std::cout << "Ray direction is " << ray.direction << std::endl;
        Intersection iter = node->object->getIntersection(ray);
        //std::cout << "itersection is " << iter.coords << std::endl;
        return iter;
    }
        Intersection intersect1 =  this->getIntersection(node->right,  ray);
        Intersection intersect2 =  this->getIntersection(node->left,  ray);
        return intersect1.distance < intersect2.distance ? intersect1 : intersect2;  

}


void BVHAccel::getSample(BVHBuildNode* node, float p, Intersection &pos, float &pdf){
    if(node->left == nullptr || node->right == nullptr){
        node->object->Sample(pos, pdf);
        pdf *= node->area;
        return;
    }
    if(p < node->left->area) getSample(node->left, p, pos, pdf);
    else getSample(node->right, p - node->left->area, pos, pdf);
}

void BVHAccel::Sample(Intersection &pos, float &pdf){
    float p = std::sqrt(get_random_float()) * root->area;
    getSample(root, p, pos, pdf);
    pdf /= root->area;
}