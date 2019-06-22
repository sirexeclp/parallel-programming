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
	cl::Program program(context, cl::Program::Sources(1, std::make_pair(kernelSource.c_str(), kernelSource.length())));

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
		result.push((in % 10) + '0');
		in /= 10;

	} while (in > 0);

	while (!result.empty())
	{
		std::cout << result.top();
		result.pop();
	}
}

// source: https://stackoverflow.com/a/24336429
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
        printf("usage: ./parsum <start> <end>\n");
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
//		std::cout << platforms.size() << " platforms found" << "\n";

//		for (auto p: platforms)
//		{
//			auto vendor =  p.getInfo<CL_PLATFORM_VENDOR>();
//			auto name = p.getInfo<CL_PLATFORM_NAME>();
//			std::cout << vendor << " | " << name << "\n";
//		}

		// Get first available GPU device which supports double precision.
		std::vector<cl::Device> devices;
		for (auto p: platforms)
		{
			std::vector<cl::Device> tmp;
			p.getDevices(CL_DEVICE_TYPE_ALL, &tmp);
//			for (auto d: tmp)
//			{
//				std::string name = d.getInfo<CL_DEVICE_NAME>();
//				std::cout << name << " " << "\n";
//			}
			devices.insert(devices.end(), tmp.begin(), tmp.end());
		}

		auto selectedDevice = devices[0];

		cl::Context context;
		context = cl::Context(selectedDevice);

		// Create command queue
		cl::CommandQueue queue(context, selectedDevice);

		auto kernel = compileKernelFromFile(context, selectedDevice, "reduce", "kernel.c");

		// Prepare input data.
		u_int64_t max_global_size = UINT16_MAX;
		int max_workgroup_size = selectedDevice.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
		// std::cout << max_workgroup_size << "\n";
		int num_groups = max_global_size / max_workgroup_size;
		max_global_size = num_groups * max_workgroup_size;

//		std::cout << "num_workpacks: " << num_workpacks << "\n";
		u_int64_t num_workpacks = std::ceil((range+1)/(float)max_global_size);
		
		// Allocate device buffers and transfer input data to device.
		cl::Buffer b_workpack_result_low(context, CL_MEM_READ_WRITE, sizeof(uint64_t) * num_groups);
		cl::Buffer b_workpack_result_high(context, CL_MEM_READ_WRITE, sizeof(uint64_t) * num_groups);

		kernel.setArg(1, b_workpack_result_low);
		kernel.setArg(2, b_workpack_result_high);
		kernel.setArg(3, sizeof(uint64_t) * max_workgroup_size, NULL);
		kernel.setArg(4, sizeof(uint64_t) * max_workgroup_size, NULL);

		uint128_t total = 0;
		// uint64_t tmp_low, tmp_high;
		std::vector<uint64_t> tmp_low(num_groups), tmp_high(num_groups);
		uint64_t offset = start; //+ i * max_workgroup_size;
		int local_kernel_range = std::min(end-offset+1,(uint64_t) max_workgroup_size);
		uint64_t global_kernel_range = std::min(end-offset+1,(uint64_t) max_global_size);




		auto dims =  selectedDevice.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();

		num_groups = global_kernel_range / local_kernel_range;
		// std::cout << "num_groups " << num_groups<< "\n";
		// std::cout << "max_global " << max_global_size << "\n"; 
		while (global_kernel_range>0)
		{

			u_int64_t x = std::min((uint64_t)local_kernel_range, dims[0]);
			u_int64_t y = std::min((uint64_t)local_kernel_range / x, dims[1]);
			u_int64_t z = std::min((uint64_t)local_kernel_range / (y * x), dims[2]);
			// std::cout << "optimal work item sizes\nx: " << x << "\ny: " << y << "\nz: " << z << "\n";
			// std::cout << "global_kernel_range " << global_kernel_range << "\n";
			
			kernel.setArg(0, offset);
			queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(global_kernel_range), cl::NDRange(x,y,z));
			queue.enqueueReadBuffer(b_workpack_result_low,  CL_TRUE, 0, sizeof(uint64_t)*num_groups, tmp_low.data());
			queue.enqueueReadBuffer(b_workpack_result_high, CL_TRUE, 0, sizeof(uint64_t)*num_groups, tmp_high.data());

			for(int j =0; j < num_groups; j++)
			{
				total += tmp_low[j];
				uint128_t high = tmp_high[j];
				total += (high << 64);
			}
			offset += global_kernel_range;
			local_kernel_range = std::min(end - offset + 1, (uint64_t) max_workgroup_size);
			global_kernel_range = std::min(end-offset+1,(uint64_t) max_global_size);
			num_groups = global_kernel_range / local_kernel_range;
			global_kernel_range =  local_kernel_range * num_groups;
			

		}

		print_u128_u(total);
		std::cout << "\n";
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
