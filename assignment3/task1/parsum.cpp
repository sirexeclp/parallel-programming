#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <streambuf>
#include <cstdint>
#include <math.h> 
#include <stack>
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#define __ERR_STR(x) #x

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

typedef unsigned __int128 uint128_t;
#define INT_TYPE  uint128_t

cl::Kernel compileKernelFromFile(cl::Context context,cl::Device device, std::string kernelName, std::string fileName = "kernel.c")
{
	std::ifstream kernelSourceFile(fileName);
	std::stringstream sstr;
	sstr << kernelSourceFile.rdbuf();
	std::string kernelSource = sstr.str();
	// Compile OpenCL program for found device.
	cl::Program program(context, cl::Program::Sources(
										1, std::make_pair(kernelSource.c_str(), kernelSource.length())));

	try
	{
		program.build(std::vector<cl::Device>({device}));
	}
	catch (const cl::Error e)
	{
		std::cerr
			<< "OpenCL compilation error" << std::endl
			<< program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device)
			<< std::endl;
		throw e;
	}

	return cl::Kernel(program, kernelName.c_str());
}

/*print_u128_u function from https://stackoverflow.com/questions/11656241/how-to-print-uint128-t-number-using-gcc*/
/*      UINT64_MAX 18446744073709551615ULL */
#define P10_UINT64 10000000000000000000ULL   /* 19 zeroes */
#define E10_UINT64 19

#define STRINGIZER(x)   # x
#define TO_STRING(x)    STRINGIZER(x)

static int print_u128_u(uint128_t u128) {
    int rc;
    if (u128 > UINT64_MAX) {
        uint128_t leading = u128 / P10_UINT64;
        uint64_t trailing = u128 % P10_UINT64;
        rc = print_u128_u(leading);
        rc += printf("%." TO_STRING(E10_UINT64)
        PRIu64, trailing);
    } else {
        uint64_t u64 = u128;
        rc = printf("%"
        PRIu64, u64);
    }
    return rc;
}

void print128(uint128_t in)
{
	std::stack<char> result;
	do 
	{
		result.push((in%10)+'0');
		in /=10;

	}while(in >0);
	while (!result.empty())
	{
		std::cout << result.top();
		result.pop();
	}
	
}

const char *getErrorString(cl_int error)
{
switch(error){
    // run-time and JIT compiler errors
    case 0: return "CL_SUCCESS";
    case -1: return "CL_DEVICE_NOT_FOUND";
    case -2: return "CL_DEVICE_NOT_AVAILABLE";
    case -3: return "CL_COMPILER_NOT_AVAILABLE";
    case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case -5: return "CL_OUT_OF_RESOURCES";
    case -6: return "CL_OUT_OF_HOST_MEMORY";
    case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case -8: return "CL_MEM_COPY_OVERLAP";
    case -9: return "CL_IMAGE_FORMAT_MISMATCH";
    case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case -11: return "CL_BUILD_PROGRAM_FAILURE";
    case -12: return "CL_MAP_FAILURE";
    case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case -15: return "CL_COMPILE_PROGRAM_FAILURE";
    case -16: return "CL_LINKER_NOT_AVAILABLE";
    case -17: return "CL_LINK_PROGRAM_FAILURE";
    case -18: return "CL_DEVICE_PARTITION_FAILED";
    case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

    // compile-time errors
    case -30: return "CL_INVALID_VALUE";
    case -31: return "CL_INVALID_DEVICE_TYPE";
    case -32: return "CL_INVALID_PLATFORM";
    case -33: return "CL_INVALID_DEVICE";
    case -34: return "CL_INVALID_CONTEXT";
    case -35: return "CL_INVALID_QUEUE_PROPERTIES";
    case -36: return "CL_INVALID_COMMAND_QUEUE";
    case -37: return "CL_INVALID_HOST_PTR";
    case -38: return "CL_INVALID_MEM_OBJECT";
    case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case -40: return "CL_INVALID_IMAGE_SIZE";
    case -41: return "CL_INVALID_SAMPLER";
    case -42: return "CL_INVALID_BINARY";
    case -43: return "CL_INVALID_BUILD_OPTIONS";
    case -44: return "CL_INVALID_PROGRAM";
    case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
    case -46: return "CL_INVALID_KERNEL_NAME";
    case -47: return "CL_INVALID_KERNEL_DEFINITION";
    case -48: return "CL_INVALID_KERNEL";
    case -49: return "CL_INVALID_ARG_INDEX";
    case -50: return "CL_INVALID_ARG_VALUE";
    case -51: return "CL_INVALID_ARG_SIZE";
    case -52: return "CL_INVALID_KERNEL_ARGS";
    case -53: return "CL_INVALID_WORK_DIMENSION";
    case -54: return "CL_INVALID_WORK_GROUP_SIZE";
    case -55: return "CL_INVALID_WORK_ITEM_SIZE";
    case -56: return "CL_INVALID_GLOBAL_OFFSET";
    case -57: return "CL_INVALID_EVENT_WAIT_LIST";
    case -58: return "CL_INVALID_EVENT";
    case -59: return "CL_INVALID_OPERATION";
    case -60: return "CL_INVALID_GL_OBJECT";
    case -61: return "CL_INVALID_BUFFER_SIZE";
    case -62: return "CL_INVALID_MIP_LEVEL";
    case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
    case -64: return "CL_INVALID_PROPERTY";
    case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
    case -66: return "CL_INVALID_COMPILER_OPTIONS";
    case -67: return "CL_INVALID_LINKER_OPTIONS";
    case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

    // extension errors
    case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
    case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
    case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
    case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
    case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
    case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
    default: return "Unknown OpenCL error";
    }
}


int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("usage: ./parsum <start> <end>");
        return EXIT_FAILURE;
    }

    uint64_t start = atoll(argv[1]);
	uint64_t end = atoll(argv[2]);
	uint64_t range = end - start;
    try
	{
		// Get list of OpenCL platforms.
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);

		if (platforms.empty())
		{
			std::cerr << "OpenCL platforms not found." << "\n";
			return EXIT_FAILURE;
		}
		std::cout << platforms.size() << " platforms found" << "\n";
		for(auto p: platforms)
		{
			auto vendor =  p.getInfo<CL_PLATFORM_VENDOR>();
			auto name = p.getInfo<CL_PLATFORM_NAME>();
			std::cout << vendor << " | " << name << "\n";
		}

		// Get first available GPU device which supports double precision.
		cl::Context context;
		std::vector<cl::Device> devices;
		for(auto p: platforms)
		{
			p.getDevices(CL_DEVICE_TYPE_GPU, &devices);

			for (auto d: devices)
			{
				// if (!d->getInfo<CL_DEVICE_AVAILABLE>())
				// 	continue;

				std::string name = d.getInfo<CL_DEVICE_NAME>();
				std::cout << name << " " << "\n";
			}
		}

		auto selectedDevice = devices[0];
		context = cl::Context(selectedDevice);

		// // Create command queue.
		cl::CommandQueue queue(context, selectedDevice);

		auto kernel = compileKernelFromFile(context, selectedDevice, "reduce", "kernel.c");
		
		// // auto reduceKernel = compileKernelFromFile(context, selectedDevice, "reduce", "init.c");
	
		// // // // Prepare input data.

		// // // std::vector<double> c(N);

		int max_workgroup_size = selectedDevice.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
		u_int64_t max_n = 250;//
		auto dims =  selectedDevice.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
		

		u_int16_t num_workpacks = std::ceil((range +1)/ float(max_n));

		std::cout << "num_workpacks: " << num_workpacks << "\n";
		// // // // Allocate device buffers and transfer input data to device.
		// // // cl::Buffer A(context, a.begin(),a.end(), false);
		// auto prefSize = kernel.getInfo<CL_KERNEL_WORK_GROUP_SIZE>();
		// std::cout << prefSize[0] <<"\n";

		cl::Buffer b_workpack_result_low(context, CL_MEM_READ_WRITE, sizeof( uint64_t) *(num_workpacks ));
		cl::Buffer b_workpack_result_high(context, CL_MEM_READ_WRITE, sizeof( uint64_t) *(num_workpacks ));

		// // // GenericBuffer<double> C(context, c, false);

		// std::vector<uint128_t> combined_result;

		// kernel.setArg(0, start);
		// kernel.setArg(1, A);
		// uint64_t vals [] = {0,CL_ULONG_MAX,0,1};
		// kernel.setArg(0, vals[0]);
		// kernel.setArg(1, vals[1]);
		// kernel.setArg(2, vals[2]);
		// kernel.setArg(3, vals[3]);
		// kernel.setArg(4, A);
		// kernel.setArg(5, B);

		// std::vector<uint64_t> results;

		// for(int workpackId = 0; workpackId<num_workpacks; workpackId++)
		// {
			
			u_int64_t threads =  range +1;//std::min(workpack_end[workpackId]- workpack_start[workpackId]+1,max_n);

			u_int64_t x = threads % UINT16_MAX;//std::min(threads, dims[0]);
			u_int64_t y = threads / UINT16_MAX+1;//std::min(threads/x, dims[1]);
			u_int64_t z = 1;//std::min(threads/(y*x), dims[2]);
			std::cout << "optimal work item sizes\nx: " << x << "\ny: " << y << "\nz: " << z << "\n"; 

			size_t start_x = start % UINT16_MAX;
			size_t start_y = start / UINT16_MAX;

			// kernel.setArg(0, (uint64_t) workpackId);
			// kernel.setArg(1, b_workpack_start);
			// kernel.setArg(2, b_workpack_end);
			kernel.setArg(0, b_workpack_result_low);
			kernel.setArg(1, b_workpack_result_high);

			kernel.setArg(2, max_workgroup_size * sizeof(uint64_t), NULL);
			kernel.setArg(3, max_workgroup_size * sizeof(uint64_t), NULL);

			// queue.enqueueNDRangeKernel(kernel, x, y , z);
			std::cout << "start:"<< (start_y << 32) + start_x << "\n";
			queue.enqueueNDRangeKernel(kernel, cl::NDRange(start_x,start_y,0) , cl::NDRange(x,y,z) , cl::NullRange);
			std::vector<uint64_t> result_low(num_workpacks,0);
			queue.enqueueReadBuffer(b_workpack_result_low, CL_TRUE, 0, sizeof(uint64_t) * num_workpacks, result_low.data());

			std::vector<uint64_t> result_high(num_workpacks,0);
			queue.enqueueReadBuffer(b_workpack_result_high, CL_TRUE, 0, sizeof(uint64_t) * num_workpacks, result_high.data());
			
			// for(int i =0)
			//results.push_back(result);
			std::cout << result_low[0] << "\n";
		// }

		uint128_t total;
		for(int i =0; i< num_workpacks;i++)
		{
			total += result_low[i];
			uint128_t high = result_high[i];
			total += (high << 64);
		}

		// // kernel.setArg(1, A);
		// // kernel.setArg(0, B);
		// // queue.enqueueNDRangeKernel(kernel, cl::NullRange, 1 , cl::NullRange);

		// //Launch kernel on the compute device.
		// int flag = 0;
		// for (int i = ceil((N+2)/2.); i > 1; i=ceil(i/2.))
		// {
		// 	// Set kernel parameters.
		// 	reduceKernel.setArg(flag, A);
		// 	reduceKernel.setArg(!flag, B);
		// 	flag = !flag;
		// 	queue.enqueueNDRangeKernel(reduceKernel, cl::NullRange, i , cl::NullRange);
		// }
		// // // Get result back to host.
		// u_int64_t result;
		// queue.enqueueReadBuffer((flag ? B:A), CL_TRUE, 0, sizeof(result), &result);
		// uint64_t result1;
		// uint64_t result2;

		// queue.enqueueReadBuffer(b_workpack_result, CL_TRUE, 0, sizeof(result1), &result1);
		//std::cout << total << "\n";
		// queue.enqueueReadBuffer(B, CL_TRUE, 0, sizeof(result2),  &result2 );
		// // Should get '3' here.
		// std::cout << result2 << result1;
		// uint128_t combined_result = UINT64_MAX;
		// combined_result += result1 *  UINT64_MAX;
		 print_u128_u(total);
		// print128(total);
	}
	catch (const cl::Error &err)
	{
		std::cerr
			<< "OpenCL error: "
			<< err.what() << "(" << getErrorString(err.err()) << ")"
			<< std::endl;
		return 1;
	}
	return EXIT_SUCCESS;
}
