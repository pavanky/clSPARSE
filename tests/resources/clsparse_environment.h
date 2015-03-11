#ifndef _CL_SPARSE_ENVIRONMENT_H_
#define _CL_SPARSE_ENVIRONMENT_H_

#include <gtest/gtest.h>

#include <clSPARSE.h>
#include "opencl_utils.h"



class ClSparseEnvironment : public ::testing::Environment
{
public:

    ClSparseEnvironment()
    {
        // init cl environment
        cl_int status = CL_SUCCESS;
        cl_platform_id* platforms = NULL;
        cl_uint num_platforms = 0;

        status = getPlatforms(&platforms, &num_platforms);
        if (status != CL_SUCCESS)
        {
            std::cerr << "Problem with setting up platforms" << std::endl;
            exit(-1);
        }

        printPlatforms(platforms, num_platforms);

        cl_device_id device = NULL;
        status = getDevice(platforms[0], &device, CL_DEVICE_TYPE_GPU);
        if (status != CL_SUCCESS)
        {
            std::cerr <<"Problem with initializing GPU device" << std::endl;
            exit(-2);
        }

        printDeviceInfo(device);

        cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
        cl_command_queue queue = clCreateCommandQueue(context, device, 0, NULL);

        clsparseSetup();

        control = clsparseCreateControl(queue, NULL);

        free(platforms);
    }


    void SetUp()
    {
    }

    //cleanup
    void TearDown()
    {
        //release cl structures
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        cl_int status  = clsparseReleaseControl(control);
        if (status != CL_SUCCESS)
        {
            std::cout << "Problem with releasing control object" << std::endl;
        }

        clsparseTeardown();
    }

    static cl_context context;
    static cl_command_queue queue;
    static clsparseControl control;

};


#endif //_CL_SPARSE_ENVIRONMENT_H_
