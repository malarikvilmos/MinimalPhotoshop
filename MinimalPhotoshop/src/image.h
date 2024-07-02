#pragma once
#include <thread>
#include <ctpl/ctpl_stl.h>

class Image
{
public:
	inline static void* app = nullptr;
	unsigned char* data = nullptr;
	int width, height, channels;

	Image(Image* image, void* app);
	Image(const char* filename, void* app, bool flip = false, bool rgba = false);
	~Image();

	void Save(const char* path);

	void Invert(bool multithreaded = false);
	void GammaTransform(float gamma, bool multithreaded = false);
	void LogarithmicTransform(int scalar, bool multithreaded = false);
	void Grayscale(bool multithreaded = false);
	void HistogramCreate(float histogram[256], bool multithreaded = false);
	void HistogramEqualization(bool multithreaded = false);
	void BoxFilter(bool multithreaded = false);
	void GaussFilter(bool multithreaded = false);
	void Sobel(int treshold, bool multithreaded = false);
	void Laplace(bool multithreaded = false);
	void Moravec(int threshold, bool multithreaded = false);

private:
	inline static unsigned int threadcount = std::thread::hardware_concurrency() - 1;
	inline static ctpl::thread_pool threadpool = ctpl::thread_pool(threadcount);
};
