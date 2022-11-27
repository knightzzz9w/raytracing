#include "Renderer.cuh"


__global__ void renderoneray(Vector3f * framebuffer ,  int image_width, int image_height   , int turn_try)
{
	//blockIdx代表block的索引,blockDim代表block的大小，threadIdx代表thread线程的索引，因此对于一维的block和thread索引的计算方式如下
    __shared__ int m = 0; 
    int i =  blockIdx.x;    //blocksize , 最上层的执行
    int j =  threadIdx.x;   //blockdim ， 下面一层
    
    int i_circ = image_width/gridDim.x; // 有多少次循环
    int j_circ = image_height/blockDim.x; //x

    int i_left = image_width%gridDim.x; // 有多少次循环
    int j_left = image_height%blockDim.x; //x
    
    for(int i1  = i;  i1 < image_width; i1+=  gridDim.x)
    {
        for(int i2  = j;  i2 < image_height; i2+=  blockDim.x)
        {
            Ray camRay = scene.camera->generateRay(Vector2f(i1, i2));
            Vector3f RayC(1,1,1);
            Vector3f color(0,0,0);
             for(int turn = 0; turn < turn_try ;  ++turn)
            {
                color += scene.castRay(camRay, RayC , 0)/(float)turn_try;   //这条光线的追踪效果
            }
            framebuffer[i2*image_width + i1] +=  color;
            m++;
            UpdateProgress(m / (float)image_width* (float)image_height);
        }
    }
        
}



void renderall(std::vector<Vector3f> framebuffer ,  int image_width, int image_height   , int turn_try)
{
    assert(image_width * image_height == framebuffer.size());
    int device = 0;//设置使用第0块GPU进行运算
    cudaSetDevice(device);//设置运算显卡
    cudaDeviceProp devProp;
    cudaGetDeviceProperties(&devProp, device);//获取对应设备属性
    int threadMaxSize = devProp.maxThreadsPerBlock > image_height ? image_height : devProp.maxThreadsPerBlock  ;//每个线程块的最大线程数
    int blockSize =  devProp.maxGridSize > image_width ? image_width :  devProp.maxGridSize;//计算Block大小,block一维度是最大的，一般不会溢出
    dim3 thread(threadMaxSize);//设置thread
    dim3 block(blockSize);//设置block
    int size = length * sizeof(float);//计算空间大小
    float *sum =(float *)malloc(size) ;//开辟动态内存空间
    //开辟显存空间
    int *image_widthGPU,*image_heightGPU , *turn_tryGPU;
    cudaMalloc((void**)&image_widthGPU,sizeof(int));
    cudaMalloc((void**)&image_heightGPU,sizeof(int));
    cudaMalloc((void**)&turn_tryGPU,sizeof(int));
    //内存->显存
    cudaMemcpy((void*)image_widthGPU,(void*)image_width,sizeof(int),cudaMemcpyHostToDevice);
    cudaMemcpy((void*)image_heightGPU,(void*)image_height,sizeof(int),cudaMemcpyHostToDevice);
    cudaMemcpy((void*)turn_tryGPU,(void*)turn_try,sizeof(int),cudaMemcpyHostToDevice);
    //运算

    Vector3f * framebuffer_GPU;
    cutilSafeCall( cudaMalloc((void**) &framebuffer_GPU, sizeof(Vector3f) * framebuffer.size()));  // 申请显存空间
    cutilSafeCall( cudaMemset(framebuffer_GPU, &framebuffer[0] , sizeof(Vector3f) * framebuffer.size() ,  cudaMemcpyHostToDevice));
 

     
    renderoneray<<<block,thread>>>(framebuffer_GPU, *image_widthGPU , *image_heightGPU , *turn_tryGPU);
    //cudaThreadSynchronize();
    //显存->内存

    Vector3f * framebuffer_CPU;
    framebuffer_CPU = (Vector3f*)calloc(framebuffer.size(), sizeof(Vector3f)); 
    cudaMemcpy(framebuffer_CPU, framebuffer_GPU, sizeof(Vector3f) * framebuffer.size() , cudaMemcpyDeviceToHost);
    
    
    for(uint32_t i = 0; i <  framebuffer.size(); ++i)   // 打印Host端data
    {
        framebuffer[i] = framebuffer_CPU[i];
    }
    //释放显存
    cudaFree(framebuffer_GPU);
    cudaFree(image_widthGPU);
    cudaFree(image_heightGPU);
    cudaFree(turn_tryGPU);
    free(framebuffer_CPU);
}
 