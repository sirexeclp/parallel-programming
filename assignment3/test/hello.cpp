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
	GenericBuffer(const cl::Context &context,
				  std::vector<T> vector,
				  bool readOnly,
				  bool useHostPtr = false,
				  bool copyHostPtr = false,
				  cl_int *err = NULL) : cl::Buffer()
	{
		cl_int error;

		cl_mem_flags flags = 0;
		if (readOnly)
		{
			flags |= CL_MEM_READ_ONLY;
		}
		else
		{
			flags |= CL_MEM_READ_WRITE;
		}
		if (useHostPtr)
		{
			flags |= CL_MEM_USE_HOST_PTR;
		}
		if (copyHostPtr)
			flags |= CL_MEM_COPY_HOST_PTR;

		size_t size = sizeof(T) * vector.size();

		if (useHostPtr || copyHostPtr)
		{
			object_ = clCreateBuffer(context(), flags, size, static_cast<T *>(&*vector.begin()), &error);
		}
		else
		{
			object_ = clCreateBuffer(context(), flags, size, 0, &error);
		}

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

int main()
{
	const size_t N = 1 << 20;
	try
	{
		// Get list of OpenCL platforms.
		std::vector<cl::Platform> platform;
		cl::Platform::get(&platform);

		if (platform.empty())
		{
			std::cerr << "OpenCL platforms not found." << std::endl;
			return 1;
		}

		// Get first available GPU device which supports double precision.
		cl::Context context;
		std::vector<cl::Device> device;
		for (auto p = platform.begin(); device.empty() && p != platform.end(); p++)
		{
			std::vector<cl::Device> pldev;

			try
			{
				p->getDevices(CL_DEVICE_TYPE_GPU, &pldev);

				for (auto d = pldev.begin(); device.empty() && d != pldev.end(); d++)
				{
					if (!d->getInfo<CL_DEVICE_AVAILABLE>())
						continue;

					std::string ext = d->getInfo<CL_DEVICE_EXTENSIONS>();

					if (
						ext.find("cl_khr_fp64") == std::string::npos &&
						ext.find("cl_amd_fp64") == std::string::npos)
						continue;

					device.push_back(*d);
					context = cl::Context(device);
				}
			}
			catch (...)
			{
				device.clear();
			}
		}

		if (device.empty())
		{
			std::cerr << "GPUs with double precision not found." << std::endl;
			return 1;
		}
		std::cout << device[0].getInfo<CL_DEVICE_NAME>() << std::endl;

		// Create command queue.
		cl::CommandQueue queue(context, device[0]);

		std::ifstream kernelSourceFile("kernel.c");
		std::stringstream sstr;
		sstr << kernelSourceFile.rdbuf();
		std::string kernelSource = sstr.str();
		// Compile OpenCL program for found device.
		cl::Program program(context, cl::Program::Sources(
										 1, std::make_pair(kernelSource.c_str(), kernelSource.length())));

		try
		{
			program.build(device);
		}
		catch (const cl::Error &)
		{
			std::cerr
				<< "OpenCL compilation error" << std::endl
				<< program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device[0])
				<< std::endl;
			return 1;
		}

		cl::Kernel add(program, "add");

		// Prepare input data.
		std::vector<double> a(N, 1);
		std::vector<double> b(N, 2);
		std::vector<double> c(N);

		// Allocate device buffers and transfer input data to device.
		GenericBuffer<double> A(context, a, true, false, true);
		GenericBuffer<double> B(context, b, true, false, true);
		GenericBuffer<double> C(context, c, false);

		// Set kernel parameters.
		add.setArg(0, static_cast<cl_ulong>(N));
		add.setArg(1, A);
		add.setArg(2, B);
		add.setArg(3, C);

		// Launch kernel on the compute device.
		queue.enqueueNDRangeKernel(add, cl::NullRange, N, cl::NullRange);

		// Get result back to host.
		queue.enqueueReadBuffer(C, CL_TRUE, 0, c.size() * sizeof(double), c.data());

		// Should get '3' here.
		std::cout << c[42] << std::endl;
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