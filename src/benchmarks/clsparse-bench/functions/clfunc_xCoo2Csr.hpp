/* ************************************************************************
 * Copyright 2015 Advanced Micro Devices, Inc.
 * ************************************************************************/
#pragma once
#ifndef CLSPARSE_BENCHMARK_COO2CSR_HXX__
#define CLSPARSE_BENCHMARK_COO2CSR_HXX__

#include "clSPARSE.h"
#include "clfunc_common.hpp"

template <typename T>
class xCoo2Csr: public clsparseFunc
{
public:
    xCoo2Csr( PFCLSPARSETIMER sparseGetTimer, size_t profileCount, cl_device_type devType ): clsparseFunc( devType, CL_QUEUE_PROFILING_ENABLE ), gpuTimer( nullptr ), cpuTimer( nullptr )
    {
        //	Create and initialize our timer class, if the external timer shared library loaded
        if( sparseGetTimer )
        {
            gpuTimer = sparseGetTimer( CLSPARSE_GPU );
            gpuTimer->Reserve( 1, profileCount );
            gpuTimer->setNormalize( true );

            cpuTimer = sparseGetTimer( CLSPARSE_CPU );
            cpuTimer->Reserve( 1, profileCount );
            cpuTimer->setNormalize( true );

            gpuTimerID = gpuTimer->getUniqueID( "GPU xCoo2Csr", 0 );
            cpuTimerID = cpuTimer->getUniqueID( "CPU xCoo2Csr", 0 );
        }


        clsparseEnableAsync( control, false );
    }

    ~xCoo2Csr( )
    {
    }

    void call_func( )
    {
      if( gpuTimer && cpuTimer )
      {
        gpuTimer->Start( gpuTimerID );
        cpuTimer->Start( cpuTimerID );

        xCoo2Csr_Function( false );

        cpuTimer->Stop( cpuTimerID );
        gpuTimer->Stop( gpuTimerID );
      }
      else
      {
        xCoo2Csr_Function( false );
      }
    }

    double gflops( )
    {
        return 0.0;
    }

    std::string gflops_formula( )
    {
        return "N/A";
    }

    double bandwidth( )
    {
        //  Assuming that accesses to the vector always hit in the cache after the first access
        //  There are NNZ integers in the cols[ ] array
        //  You access each integer value in row_delimiters[ ] once.
        //  There are NNZ float_types in the vals[ ] array
        //  You read num_cols floats from the vector, afterwards they cache perfectly.
        //  Finally, you write num_rows floats out to DRAM at the end of the kernel.
        return ( sizeof( cl_int )*( csrMtx.num_nonzeros + csrMtx.num_rows ) + sizeof( T ) * ( csrMtx.num_nonzeros + csrMtx.num_cols + csrMtx.num_rows ) ) / time_in_ns( );
    }

    std::string bandwidth_formula( )
    {
        return "GiB/s";
    }


    void setup_buffer( double pAlpha, double pBeta, const std::string& path )
    {
        sparseFile = path;

        cl_int status;

        int nnz1, row1, col1;
        clsparseStatus fileError = clsparseHeaderfromFile( &nnz1, &row1, &col1, path.c_str( ) );
        if( fileError != clsparseSuccess )
           throw std::runtime_error( "Could not read matrix market header from disk" );

        clsparseInitCooMatrix( &cooMatx );
        cooMatx.num_nonzeros = nnz1;
        cooMatx.num_rows = row1;
        cooMatx.num_cols = col1;
         
        cooMatx.values     = ::clCreateBuffer( ctx, CL_MEM_READ_ONLY,
                                               cooMatx.num_nonzeros * sizeof(T), NULL, &status );
        cooMatx.colIndices = ::clCreateBuffer( ctx, CL_MEM_READ_ONLY,
                                               cooMatx.num_nonzeros * sizeof( cl_int ), NULL, &status );
        cooMatx.rowIndices = ::clCreateBuffer( ctx, CL_MEM_READ_ONLY,
                                               cooMatx.num_nonzeros * sizeof( cl_int ), NULL, &status );

        //JPA: Bug fix: That will not work for double precision
        //clsparseCooMatrixfromFile( &cooMatx, path.c_str( ), control );
        if (typeid(T) == typeid(float))
            clsparseSCooMatrixfromFile(&cooMatx, path.c_str(), control);
        if (typeid(T) == typeid(double))
            clsparseDCooMatrixfromFile(&cooMatx, path.c_str(), control);
        
        //clsparseCsrMatrix csrMatx;
        clsparseInitCsrMatrix( &csrMtx );
 
        //JPA:: Shouldn't be CL_MEM_WRITE_ONLY since coo ---> csr???
        csrMtx.values = ::clCreateBuffer( ctx, CL_MEM_READ_ONLY,
                                           cooMatx.num_nonzeros * sizeof( T ), NULL, &status );

        csrMtx.colIndices = ::clCreateBuffer( ctx, CL_MEM_READ_ONLY,
                                               cooMatx.num_nonzeros * sizeof( cl_int ), NULL, &status );
        csrMtx.rowOffsets = ::clCreateBuffer( ctx, CL_MEM_READ_ONLY,
                                              ( cooMatx.num_rows + 1 ) * sizeof( cl_int ), NULL, &status );

    }

    void initialize_cpu_buffer( )
    {
    }

    void initialize_gpu_buffer( )
    {
		
    }

    void reset_gpu_write_buffer( )
    {
		
		int scalar_i = 0;
		T scalar_f = 0;
		CLSPARSE_V( ::clEnqueueFillBuffer( queue, csrMtx.rowOffsets, &scalar_i, sizeof( int ), 0,
                              sizeof( int ) * (csrMtx.num_rows + 1), 0, NULL, NULL ), "::clEnqueueFillBuffer row" ); 
		CLSPARSE_V( ::clEnqueueFillBuffer( queue, csrMtx.colIndices, &scalar_i, sizeof( int ), 0,
                              sizeof( int ) * csrMtx.num_nonzeros, 0, NULL, NULL ), "::clEnqueueFillBuffer col" );
		CLSPARSE_V( ::clEnqueueFillBuffer( queue, csrMtx.values, &scalar_f, sizeof( T ), 0,
                              sizeof( T ) * csrMtx.num_nonzeros, 0, NULL, NULL ), "::clEnqueueFillBuffer values" );
    }

    void read_gpu_buffer( )
    {
    }

    void cleanup( )
    {	
        if( gpuTimer && cpuTimer )
        {
          std::cout << "clSPARSE matrix: " << sparseFile << std::endl;
          size_t sparseBytes = 0;
          cpuTimer->pruneOutliers( 3.0 );
          cpuTimer->Print( sparseBytes, "GiB/s" );
          cpuTimer->Reset( );

          gpuTimer->pruneOutliers( 3.0 );
          gpuTimer->Print( sparseBytes, "GiB/s" );
          gpuTimer->Reset( );
        }

        //this is necessary since we are running a iteration of tests and calculate the average time. (in client.cpp)
        //need to do this before we eventually hit the destructor
        CLSPARSE_V( ::clReleaseMemObject( csrMtx.values ), "clReleaseMemObject csrMtx.values" );
        CLSPARSE_V( ::clReleaseMemObject( csrMtx.colIndices ), "clReleaseMemObject csrMtx.colIndices" );
        CLSPARSE_V( ::clReleaseMemObject( csrMtx.rowOffsets ), "clReleaseMemObject csrMtx.rowOffsets" );

        CLSPARSE_V( ::clReleaseMemObject( cooMatx.values ), "clReleaseMemObject cooMtx.values" );
        CLSPARSE_V( ::clReleaseMemObject( cooMatx.colIndices ), "clReleaseMemObject cooMtx.colIndices" );
        CLSPARSE_V( ::clReleaseMemObject( cooMatx.rowIndices ), "clReleaseMemObject cooMtx.rowOffsets" );
    }

private:
    void xCoo2Csr_Function( bool flush );

    //  Timers we want to keep
    clsparseTimer* gpuTimer;
    clsparseTimer* cpuTimer;
    size_t gpuTimerID, cpuTimerID;

    std::string sparseFile;

    //device values
    clsparseCsrMatrix csrMtx;
    clsparseCooMatrix cooMatx;

    //matrix dimension
    int m;
    int n;
    int nnz;
	
    //  OpenCL state
    cl_command_queue_properties cqProp;

}; // class xCoo2Csr

template<> void
xCoo2Csr<float>::xCoo2Csr_Function( bool flush )
{
    clsparseStatus status = clsparseScoo2csr(&cooMatx, &csrMtx, control);	
    if( flush )
        clFinish( queue );
}

template<> void
xCoo2Csr<double>::xCoo2Csr_Function( bool flush )
{
    clsparseStatus status = clsparseDcoo2csr(&cooMatx, &csrMtx, control);	

    if( flush )
        clFinish( queue );
}

#endif // ifndef CLSPARSE_BENCHMARK_COO2CSR_HXX__
