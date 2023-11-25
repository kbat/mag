// Simple cuSparse test:
// create random dense matrix A an identity matrix I on cpu host, copy them to gpu device, convert to sparse CSR format,
// compute C = A*I in CSR sparse format, convert to dense and copy to cpu host, chek C == A
// examples based on https://github.com/NVIDIA/CUDALibrarySamples/tree/master/cuSPARSE
// cuSparse doc: https://docs.nvidia.com/cuda/cusparse/


#include <cuda_runtime_api.h> // cudaMalloc, cudaMemcpy, etc.
#include <cusparse.h>         // cusparseSparseToDense
#include <stdio.h>            // printf
#include <stdlib.h>           // EXIT_FAILURE

#define CHECK_CUDA(func)                                                       \
{                                                                              \
    cudaError_t status = (func);                                               \
    if (status != cudaSuccess) {                                               \
        printf("CUDA API failed at line %d with error: %s (%d)\n",             \
               __LINE__, cudaGetErrorString(status), status);                  \
        return EXIT_FAILURE;                                                   \
    }                                                                          \
}

#define CHECK_CUSPARSE(func)                                                   \
{                                                                              \
    cusparseStatus_t status = (func);                                          \
    if (status != CUSPARSE_STATUS_SUCCESS) {                                   \
        printf("CUSPARSE API failed at line %d with error: %s (%d)\n",         \
               __LINE__, cusparseGetErrorString(status), status);              \
        return EXIT_FAILURE;                                                   \
    }                                                                          \
}

// Row-major order
float* generate_random_dense_matrix(int rows, int cols)
{
    int i, j;
    float *matrix = (float *)malloc(sizeof(float) * rows * cols);
    for (i = 0; i < rows; i++)
    {   
        for (j = 0; j < cols; j++)
        {
            int r = rand();
            matrix[i * cols + j] = 0.0f;
            if (r % 3 == 0)
            {
                matrix[i * cols + j] = rand() % 100;
            }
        }
    }
    return matrix;
}

float* generate_identity_dense_matrix(int rows, int cols)
{
    int i, j;
    float *matrix = (float *)malloc(sizeof(float) * rows * cols);
    for (i = 0; i < rows; i++)
    {   
        for (j = 0; j < cols; j++)
        {
            matrix[i * cols + j] = 0.0f;
            if (i == j)
            {
                matrix[i * cols + j] = 1;
            }
        }
    }
    return matrix;
}

int main(void) {
    // Host problem definition
    int   num_rows   = 1024;
    int   num_cols   = 1024;
    int   ld         = num_cols;    // leading dimension
    int   dense_size = num_cols * num_rows;

    float               alpha       = 1.0f;
    float               beta        = 0.0f;
    cusparseOperation_t opA         = CUSPARSE_OPERATION_NON_TRANSPOSE;
    cusparseOperation_t opB         = CUSPARSE_OPERATION_NON_TRANSPOSE;
    cudaDataType        computeType = CUDA_R_32F;

    float* h_dense = generate_random_dense_matrix(num_rows, num_cols);
    float* h_idense = generate_identity_dense_matrix(num_rows, num_cols);
    float* h_bdense = (float *)malloc(sizeof(float) * num_rows * num_cols);

    //--------------------------------------------------------------------------
    // Device memory management
    int   *d_csr_offsets, *d_csr_columns, *d_icsr_offsets, *d_icsr_columns, *d_bcsr_offsets, *d_bcsr_columns;
    float *d_csr_values, *d_icsr_values, *d_bcsr_values,  *d_dense, *d_idense, *d_bdense;

    CHECK_CUDA( cudaMalloc((void**) &d_dense, dense_size * sizeof(float)))
    CHECK_CUDA( cudaMalloc((void**) &d_idense, dense_size * sizeof(float)))
    CHECK_CUDA( cudaMalloc((void**) &d_bdense, dense_size * sizeof(float)))

    CHECK_CUDA( cudaMalloc((void**) &d_csr_offsets, (num_rows + 1) * sizeof(int)) )
    CHECK_CUDA( cudaMalloc((void**) &d_icsr_offsets, (num_rows + 1) * sizeof(int)) )
    CHECK_CUDA( cudaMalloc((void**) &d_bcsr_offsets, (num_rows + 1) * sizeof(int)) )
    
    CHECK_CUDA( cudaMemcpy(d_dense, h_dense, dense_size * sizeof(float),
                           cudaMemcpyHostToDevice) )
    CHECK_CUDA( cudaMemcpy(d_idense, h_idense, dense_size * sizeof(float),
                            cudaMemcpyHostToDevice) )
    //--------------------------------------------------------------------------
    // CUSPARSE APIs
    cusparseHandle_t     handle = NULL;
    cusparseSpMatDescr_t matSA;
    cusparseSpMatDescr_t matSI;
    cusparseSpMatDescr_t matSB;
    cusparseDnMatDescr_t matDA;
    cusparseDnMatDescr_t matDI;
    cusparseDnMatDescr_t matDB;
    
    void*                dBuffer    = NULL;
    size_t               bufferSize = 0;

    CHECK_CUSPARSE( cusparseCreate(&handle) )
    // Create dense matrix A, create dense matrix I
    CHECK_CUSPARSE( cusparseCreateDnMat(&matDA, num_rows, num_cols, ld, d_dense,
                                        computeType, CUSPARSE_ORDER_ROW) )
    CHECK_CUSPARSE( cusparseCreateDnMat(&matDI, num_rows, num_cols, ld, d_idense,
                                        computeType, CUSPARSE_ORDER_ROW) )
    
    // Create sparse matrix A in CSR format, create sparse matrix I in CSR format
    CHECK_CUSPARSE( cusparseCreateCsr(&matSA, num_rows, num_cols, 0,
                                      d_csr_offsets, NULL, NULL,
                                      CUSPARSE_INDEX_32I, CUSPARSE_INDEX_32I,
                                      CUSPARSE_INDEX_BASE_ZERO, computeType) )
    CHECK_CUSPARSE( cusparseCreateCsr(&matSI, num_rows, num_cols, 0,
                                      d_icsr_offsets, NULL, NULL,
                                      CUSPARSE_INDEX_32I, CUSPARSE_INDEX_32I,
                                      CUSPARSE_INDEX_BASE_ZERO, computeType) )

    // allocate an external buffer if needed
    CHECK_CUSPARSE( cusparseDenseToSparse_bufferSize(
                                        handle, matDA, matSA,
                                        CUSPARSE_DENSETOSPARSE_ALG_DEFAULT,
                                        &bufferSize) )

    CHECK_CUDA( cudaMalloc(&dBuffer, bufferSize) )

    // execute Dense to Sparse conversion
    CHECK_CUSPARSE( cusparseDenseToSparse_analysis(handle, matDA, matSA,
                                        CUSPARSE_DENSETOSPARSE_ALG_DEFAULT,
                                        dBuffer) )
    // get number of non-zero elements
    int64_t num_rows_tmp, num_cols_tmp, nnz;
    CHECK_CUSPARSE( cusparseSpMatGetSize(matSA, &num_rows_tmp, &num_cols_tmp,
                                         &nnz) )

    // allocate CSR column indices and values
    CHECK_CUDA( cudaMalloc((void**) &d_csr_columns, nnz * sizeof(int))   )
    CHECK_CUDA( cudaMalloc((void**) &d_csr_values,  nnz * sizeof(float)) )
    // reset offsets, column indices, and values pointers
    CHECK_CUSPARSE( cusparseCsrSetPointers(matSA, d_csr_offsets, d_csr_columns,
                                           d_csr_values) )
    // execute Sparse to Dense conversion
    CHECK_CUSPARSE( cusparseDenseToSparse_convert(handle, matDA, matSA,
                                        CUSPARSE_DENSETOSPARSE_ALG_DEFAULT,
                                        dBuffer) )

    CHECK_CUDA( cudaFree(dBuffer) )
   
    // the same for I
    CHECK_CUSPARSE( cusparseDenseToSparse_bufferSize(
                                        handle, matDI, matSI,
                                        CUSPARSE_DENSETOSPARSE_ALG_DEFAULT,
                                        &bufferSize) )
    CHECK_CUDA( cudaMalloc(&dBuffer, bufferSize) )
    CHECK_CUSPARSE( cusparseDenseToSparse_analysis(handle, matDI, matSI,
                                        CUSPARSE_DENSETOSPARSE_ALG_DEFAULT,
                                        dBuffer) )
    CHECK_CUSPARSE( cusparseSpMatGetSize(matSI, &num_rows_tmp, &num_cols_tmp,
                                         &nnz) )
    CHECK_CUDA( cudaMalloc((void**) &d_icsr_columns, nnz * sizeof(int))   )
    CHECK_CUDA( cudaMalloc((void**) &d_icsr_values,  nnz * sizeof(float)) )
    CHECK_CUSPARSE( cusparseCsrSetPointers(matSI, d_icsr_offsets, d_icsr_columns,
                                           d_icsr_values) )
    CHECK_CUSPARSE( cusparseDenseToSparse_convert(handle, matDI, matSI,
                                        CUSPARSE_DENSETOSPARSE_ALG_DEFAULT,
                                        dBuffer) )
    CHECK_CUDA( cudaFree(dBuffer) )

    
    CHECK_CUSPARSE( cusparseCreateCsr(&matSB, num_rows,  num_cols, 0,
                                      NULL, NULL, NULL,
                                      CUSPARSE_INDEX_32I, CUSPARSE_INDEX_32I,
                                      CUSPARSE_INDEX_BASE_ZERO, computeType) )

    //--------------------------------------------------------------------------
    // SpGEMM Computation
    cusparseSpGEMMDescr_t spgemmDesc;
    CHECK_CUSPARSE( cusparseSpGEMM_createDescr(&spgemmDesc) )

    void*  dBuffer2   = NULL;
    size_t bufferSize2 = 0;

    // ask bufferSize bytes for external memory
    CHECK_CUSPARSE(
        cusparseSpGEMM_workEstimation(handle, opA, opB,
                                      &alpha, matSA, matSI, &beta, matSB,
                                      computeType, CUSPARSE_SPGEMM_DEFAULT,
                                      spgemmDesc, &bufferSize, NULL) )
    CHECK_CUDA( cudaMalloc((void**) &dBuffer, bufferSize) )
    // inspect the matrices SA and SI to understand the memory requirement for
    // the next step
    CHECK_CUSPARSE(
        cusparseSpGEMM_workEstimation(handle, opA, opB,
                                      &alpha, matSA, matSI, &beta, matSB,
                                      computeType, CUSPARSE_SPGEMM_DEFAULT,
                                      spgemmDesc, &bufferSize, dBuffer) )

    // ask bufferSize2 bytes for external memory
    CHECK_CUSPARSE(
        cusparseSpGEMM_compute(handle, opA, opB,
                               &alpha, matSA, matSI, &beta, matSB,
                               computeType, CUSPARSE_SPGEMM_DEFAULT,
                               spgemmDesc, &bufferSize2, NULL) )
    CHECK_CUDA( cudaMalloc((void**) &dBuffer2, bufferSize2) )

    // compute the intermediate product of A * I
    CHECK_CUSPARSE( cusparseSpGEMM_compute(handle, opA, opB,
                                           &alpha, matSA, matSI, &beta, matSB,
                                           computeType, CUSPARSE_SPGEMM_DEFAULT,
                                           spgemmDesc, &bufferSize2, dBuffer2) )
    // get matrix B non-zero entries B_nnz1
    int64_t B_num_rows1, B_num_cols1, B_nnz1;
    CHECK_CUSPARSE( cusparseSpMatGetSize(matSB, &B_num_rows1, &B_num_cols1,
                                         &B_nnz1) )
    // allocate matrix B
    CHECK_CUDA( cudaMalloc((void**) &d_bcsr_columns, B_nnz1 * sizeof(int))   )
    CHECK_CUDA( cudaMalloc((void**) &d_bcsr_values,  B_nnz1 * sizeof(float)) )

    // NOTE: if 'beta' != 0, the values of B must be update after the allocation
    //       of d_bcsr_values, and before the call of cusparseSpGEMM_copy

    // update matB with the new pointers
    CHECK_CUSPARSE(
        cusparseCsrSetPointers(matSB, d_bcsr_offsets, d_bcsr_columns, d_bcsr_values) )


    // if beta != 0, cusparseSpGEMM_copy reuses/updates the values of dC_values

    // copy the final products to the matrix B
    CHECK_CUSPARSE(
        cusparseSpGEMM_copy(handle, opA, opB,
                            &alpha, matSA, matSI, &beta, matSB,
                            computeType, CUSPARSE_SPGEMM_DEFAULT, spgemmDesc) )



    CHECK_CUDA( cudaFree(dBuffer) )
    CHECK_CUDA( cudaFree(dBuffer2) )

    // Create dense matrix B
    CHECK_CUSPARSE( cusparseCreateDnMat(&matDB, num_rows, num_cols, ld, d_bdense,
                                        computeType, CUSPARSE_ORDER_ROW) )

    // allocate an external buffer if needed
    CHECK_CUSPARSE( cusparseSparseToDense_bufferSize(
                                        handle, matSB, matDB,
                                        CUSPARSE_SPARSETODENSE_ALG_DEFAULT,
                                        &bufferSize) )

    CHECK_CUDA( cudaMalloc(&dBuffer, bufferSize) )

    // execute Sparse to Dense conversion
    CHECK_CUSPARSE( cusparseSparseToDense(handle, matSB, matDB,
                                          CUSPARSE_SPARSETODENSE_ALG_DEFAULT,
                                          dBuffer) )

    // destroy matrix/vector descriptors
    CHECK_CUSPARSE( cusparseDestroyDnMat(matDA) )
    CHECK_CUSPARSE( cusparseDestroySpMat(matSA) )
    CHECK_CUSPARSE( cusparseDestroyDnMat(matDI) )
    CHECK_CUSPARSE( cusparseDestroySpMat(matSI) )
    CHECK_CUSPARSE( cusparseDestroyDnMat(matDB) )
    CHECK_CUSPARSE( cusparseDestroySpMat(matSB) )
    CHECK_CUSPARSE( cusparseDestroy(handle) )

    //--------------------------------------------------------------------------
    // device result check
    CHECK_CUDA( cudaMemcpy(h_bdense, d_bdense, dense_size * sizeof(float),
                           cudaMemcpyDeviceToHost) )

    int correct = 1;
    for (int i = 0; i < num_rows; i++) {
        for (int j = 0; j < num_cols; j++) {
            if (h_dense[i * ld + j] != h_bdense[i * ld + j]) {
                correct = 0;
                break;
            }
        }
    }
    if (correct)
        printf("Test PASSED\n");
    else
        printf("Test FAILED: wrong result\n");

    //--------------------------------------------------------------------------
    // device memory deallocation
    CHECK_CUDA( cudaFree(dBuffer) )
    CHECK_CUDA( cudaFree(d_csr_offsets) )
    CHECK_CUDA( cudaFree(d_csr_columns) )
    CHECK_CUDA( cudaFree(d_csr_values) )
    CHECK_CUDA( cudaFree(d_dense) )
    CHECK_CUDA( cudaFree(d_icsr_offsets) )
    CHECK_CUDA( cudaFree(d_icsr_columns) )
    CHECK_CUDA( cudaFree(d_icsr_values) )
    CHECK_CUDA( cudaFree(d_idense) )
    CHECK_CUDA( cudaFree(d_bcsr_offsets) )
    CHECK_CUDA( cudaFree(d_bcsr_columns) )
    CHECK_CUDA( cudaFree(d_bcsr_values) )
    CHECK_CUDA( cudaFree(d_bdense) )

    // host dealloc
    free(h_dense);
    free(h_idense);
    free(h_bdense);
    return EXIT_SUCCESS;
}
