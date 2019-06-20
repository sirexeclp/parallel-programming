#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <streambuf>
#include <cstdint>
#include <math.h> 
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#define __ERR_STR(x) #x

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

typedef unsigned __int128 uint128_t;
#define INT_TYPE  uint128_t

template <class T>
class GenericBuffer : public cl::Buffer
{
public:
	size_t size_b;
	GenericBuffer(const cl::Context &context,
					std::vector<T> vector,
					cl_mem_flags flags,
					cl_int *err = NULL) : cl::Buffer()
	{
		cl_int error;
		// c__uint128_tl_mem_flags flags = 0;
		// if (readOnly)
		// {
		// 	flags |= CL_MEM_READ_ONLY;
		// }
		// else
		// {
		// 	flags |= CL_MEM_READ_WRITE;
		// }
		// if (useHostPtr)
		// {
		// 	flags |= CL_MEM_USE_HOST_PTR;
		// }
		// if (copyHostPtr)
		// 	flags |= CL_MEM_COPY_HOST_PTR;

		size_t size = sizeof(T) * vector.size();
		size_b = size;

		// if (useHostPtr || copyHostPtr)
		// {
			object_ = clCreateBuffer(context(), flags, size, static_cast<T *>(&*vector.begin()), &error);
		// }
		// else
		// {
		// 	object_ = clCreateBuffer(context(), flags, size, 0, &error);
		// }

		cl::detail::errHandler(error, __ERR_STR(clCreateBuffer));
		if (err != NULL)
		{
			*err = error;
		}

		// if( !useHostPtr ) {
		//     error = cl::copy(startIterator, endIterator, *this);
		//     detail::errHandler(error, __CREATE_BUFFER_ERR);
		//     if (err != NULL) {
		//         *err = error;
		//     }
		// }
	}
};

class GenericQueue : public cl::CommandQueue
{
	public:
		GenericQueue(const cl::Context& context,
        const cl::Device& device):CommandQueue(context,device)
		 {

		 }

		template <typename T>
		cl_int enqueueWriteBuffer( const GenericBuffer<T>& buffer,
        cl_bool blocking)
		{
			return CommandQueue::enqueueWriteBuffer(buffer, blocking, 0, buffer.size_b, &buffer);
		}
};

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

		u_int64_t max_n = selectedDevice.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
		auto dims =  selectedDevice.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
		

		u_int16_t num_workpacks = std::ceil(range / float(max_n));

		std::cout << "num_workpacks: " << num_workpacks << "\n";
		// // // // Allocate device buffers and transfer input data to device.
		// // // cl::Buffer A(context, a.begin(),a.end(), false);



		std::vector<uint64_t> workpack_start(num_workpacks);
		std::vector<uint64_t> workpack_end(num_workpacks);
		
		for(int i = 0; i<num_workpacks; i++)
		{
			workpack_start[i] = start + (i*max_n);
			workpack_end[i] = std::min(workpack_start[i] + max_n, end);
			std::cout << workpack_start[i] << " " << workpack_end[i] << "\n";
		}

		cl::Buffer b_workpack_start(context, CL_MEM_READ_WRITE, sizeof( uint64_t) * num_workpacks);
		cl::Buffer b_workpack_end(context, CL_MEM_READ_WRITE, sizeof( uint64_t) * num_workpacks);
		cl::Buffer b_workpack_result(context, CL_MEM_READ_WRITE, sizeof( uint64_t) *(num_workpacks +2));
		// // // GenericBuffer<double> C(context, c, false);



		queue.enqueueWriteBuffer(b_workpack_start,CL_TRUE,0,sizeof(uint64_t) * workpack_start.size() ,workpack_start.data());
		queue.enqueueWriteBuffer(b_workpack_end,CL_TRUE,0,sizeof(uint64_t) * workpack_end.size() ,workpack_end.data());


		// kernel.setArg(0, start);
		// kernel.setArg(1, A);
		// uint64_t vals [] = {0,CL_ULONG_MAX,0,1};
		// kernel.setArg(0, vals[0]);
		// kernel.setArg(1, vals[1]);
		// kernel.setArg(2, vals[2]);
		// kernel.setArg(3, vals[3]);
		// kernel.setArg(4, A);
		// kernel.setArg(5, B);

		std::vector<uint64_t> results;

		for(int workpackId = 0; workpackId<num_workpacks; workpackId++)
		{

			u_int64_t threads = std::min(workpack_end[workpackId]- workpack_start[workpackId]+1,max_n);

			u_int64_t x = std::min(threads, dims[0]);
			u_int64_t y = std::min(threads/x, dims[1]);
			u_int64_t z = std::min(threads/(y*x), dims[2]);
			std::cout << "optimal work item sizes\nx: " << x << "\ny: " << y << "\nz: " << z << "\n"; 


			kernel.setArg(0, (uint64_t) workpackId);
			kernel.setArg(1, b_workpack_start);
			kernel.setArg(2, b_workpack_end);
			kernel.setArg(3, b_workpack_result);
			kernel.setArg(4, (num_workpacks +2) * sizeof(uint64_t), NULL);

			// queue.enqueueNDRangeKernel(kernel, x, y , z);
			queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(x,y,z) , cl::NullRange);
			uint64_t result;
			queue.enqueueReadBuffer(b_workpack_result, CL_TRUE, 0, sizeof(result), &result);
			results.push_back(result);
		}

		uint64_t total;
		for(auto i: results)
		{
			total +=i;
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
		std::cout << total << "\n";
		// queue.enqueueReadBuffer(B, CL_TRUE, 0, sizeof(result2),  &result2 );
		// // Should get '3' here.
		// std::cout << result2 << result1;
		// uint128_t combined_result = result2;
		// combined_result += result1 *  UINT64_MAX;
		// print_u128_u(combined_result);
	}
	catch (const cl::Error &err)
	{
		std::cerr
			<< "OpenCL error: "
			<< err.what() << "(" << err.err() << ")"
			<< std::endl;
		return 1;
	}
}