#include <iostream>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <cuda_runtime.h>
#include "kernel.h"
#include "kernel.cu"
#include "dev_array.h"
#include <math.h>

#include <chrono>

using namespace std;

int main()
{
    // Perform matrix multiplication C = A*B
    // where A, B and C are NxN matrices
    int N = 1000;
    int SIZE = N*N;

    std::cout << "N: " << N << std::endl;

    // Allocate memory on the host
    vector<float> h_A(SIZE);
    vector<float> h_B(SIZE);
    vector<float> h_C(SIZE);

    // Initialize matrices on the host
    for (int i=0; i<N; i++){
        for (int j=0; j<N; j++){
            h_A[i*N+j] = sin(i);
            h_B[i*N+j] = cos(j);
        }
    }

    auto start = std::chrono::high_resolution_clock::now();

    // Allocate memory on the device
    dev_array<float> d_A(SIZE);
    dev_array<float> d_B(SIZE);
    dev_array<float> d_C(SIZE);

    d_A.set(&h_A[0], SIZE);
    d_B.set(&h_B[0], SIZE);

    const auto dgpu1 = std::chrono::high_resolution_clock::now()-start;
    std::cout << "GPU data init+copy: " << std::chrono::duration_cast<std::chrono::milliseconds>(dgpu1).count() << " ms" << std::endl;

    matrixMultiplication(d_A.getData(), d_B.getData(), d_C.getData(), N);
    cudaDeviceSynchronize();

    d_C.get(&h_C[0], SIZE);
    cudaDeviceSynchronize();

    const auto dgpu = std::chrono::high_resolution_clock::now()-start;
    std::cout << "GPU total: " << std::chrono::duration_cast<std::chrono::milliseconds>(dgpu).count() << " ms" << std::endl;

    float *cpu_C;
    cpu_C=new float[SIZE];

    // Now do the matrix multiplication on the CPU
    double sum;
    start = std::chrono::high_resolution_clock::now();

    for (size_t row=0; row<N; ++row){
        for (size_t col=0; col<N; ++col){
            sum = 0.0;
            for (size_t n=0; n<N; ++n){
                sum += h_A[row*N+n]*h_B[n*N+col];
            }
            cpu_C[row*N+col] = sum;
        }
    }
    const auto dcpu = std::chrono::high_resolution_clock::now()-start;
    std::cout << "CPU: " << std::chrono::duration_cast<std::chrono::milliseconds>(dcpu).count() << " ms" << std::endl;

    std::cout << "CPU/GPU: " << dcpu/dgpu << std::endl;

    double err = 0.0;
    // Check the result and make sure it is correct
    for (size_t ROW=0; ROW < N; ++ROW) {
        for (size_t COL=0; COL < N; ++COL) {
            const size_t index = ROW*N+COL;
            err += cpu_C[index] - h_C[index];
        }
    }

    cout << "Error: " << err << endl;

    return 0;
}