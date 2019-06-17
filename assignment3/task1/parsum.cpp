#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <streambuf>

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#define __ERR_STR(x) #x

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

		// cl_mem_flags flags = 0;
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

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("usage: ./parsum <start> <end>");
        return EXIT_FAILURE;
    }

    long long start = atoll(argv[1]);
    long long end = atoll(argv[2]);
	const size_t N = 1000;
	
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
				std::cout << name << "\n";
			}
		}

		auto selectedDevice = devices[0];
		context = cl::Context(selectedDevice);

		// // Create command queue.
		GenericQueue queue(context, selectedDevice);

		auto kernel = compileKernelFromFile(context, selectedDevice,"sum", "kernel.c");

		// // Prepare input data.
		std::vector<long> a(N, 1);
		std::vector<long> b(N/2);
		// std::vector<double> c(N);

		// // Allocate device buffers and transfer input data to device.
		GenericBuffer<long> A(context, a, CL_MEM_READ_ONLY);
		GenericBuffer<long> B(context, b, CL_MEM_READ_WRITE);
		// GenericBuffer<double> C(context, c, false);

		std::cout << "test";
		// Set kernel parameters.
		kernel.setArg(0, A);
		kernel.setArg(1, B);
		std::cout << "test2";

		queue.enqueueWriteBuffer(A,CL_TRUE);

		// // Launch kernel on the compute device.
		queue.enqueueNDRangeKernel(kernel, cl::NullRange, N/2 , cl::NullRange);

		// // Get result back to host.
		queue.enqueueReadBuffer(B, CL_TRUE, 0, b.size() * sizeof(long), b.data());

		// // Should get '3' here.
		std::cout << b[0] << std::endl;
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