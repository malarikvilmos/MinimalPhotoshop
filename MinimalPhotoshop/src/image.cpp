#include "image.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb/stb_image_resize.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include "application.h"

#include <iostream>
#include <glad/glad.h>
#include <numeric>
#include <cmath>
#include <algorithm>

Image::Image(Image* image, void* app)
{
	Image::app = (Application*)app;
	width = image->width;
	height = image->height;
	channels = image->channels;
	this->data = new unsigned char[width * height * channels];
	memcpy(this->data, image->data, width * height * channels);
}

Image::Image(const char* filename, void* app, bool flip, bool rgba)
{
	Image::app = (Application*)app;
	stbi_set_flip_vertically_on_load(flip);

	if (rgba)
		data = stbi_load(filename, &width, &height, &channels, 4);
	else
		data = stbi_load(filename, &width, &height, &channels, 0);
	if (data == nullptr) {
		std::cout << "Error loading image at: " << filename << std::endl;
		return;
	}	

	std::cout << "width: " << width << ", height: " << height << ", channels: " << channels << std::endl;
}

Image::~Image()
{
	stbi_image_free(data);
}

void Image::Save(const char* path)
{
	if (this == nullptr) return;
	stbi_flip_vertically_on_write(1);
	stbi_write_png(path, width, height, channels, data, width * channels);
}

void Image::Invert(bool multithreaded)
{
	if (this == nullptr) return;

	if (multithreaded)
	{
		unsigned int blocksize = (width * height * channels) / threadcount;
		bool* completed = new bool[threadcount];
		for (int i = 0; i < threadcount; ++i) {
			threadpool.push([blocksize, i, this, completed](int) {
				for (int j = 0; j < blocksize; ++j) {
					((Application*)app)->altered->image->data[i * blocksize + j] = 255 - data[i * blocksize + j];
				}
				completed[i] = true;
			});
		}
		while (std::accumulate(completed, completed + threadcount, 0) != threadcount);
		delete[] completed;
	}
	else
	{
		for (int i = 0; i < width * height * channels; ++i) {
			((Application*)app)->altered->image->data[i] = 255 - data[i];
		}
	}
	((Application*)app)->altered->UpdateTexture();
}

void Image::GammaTransform(float gamma, bool multithreaded)
{
	if (this == nullptr) return;

	if (multithreaded)
	{
		unsigned int blocksize = (width * height * channels) / threadcount;
		bool* completed = new bool[threadcount];

		for (int i = 0; i < threadcount; ++i) {
			threadpool.push([blocksize, i, this, completed, gamma](int) {
				for (int j = 0; j < blocksize; ++j) {
					((Application*)app)->altered->image->data[i * blocksize + j] =
						std::clamp((int)pow(data[i * blocksize + j], gamma), 0, 255);
				}
				completed[i] = true;
			});
		}
		while (std::accumulate(completed, completed + threadcount, 0) != threadcount);
		delete[] completed;
	}
	else
	{
		for (int i = 0; i < width * height * channels; ++i) {
			((Application*)app)->altered->image->data[i] = 
				std::clamp((int)pow(data[i], gamma), 0, 255);
		}
	}
	((Application*)app)->altered->UpdateTexture();
}

void Image::LogarithmicTransform(int scalar, bool multithreaded)
{
	if (this == nullptr) return;

	if (multithreaded)
	{
		unsigned int blocksize = (width * height * channels) / threadcount;
		bool* completed = new bool[threadcount];

		for (int i = 0; i < threadcount; ++i) {
			threadpool.push([blocksize, i, this, completed, scalar](int) {
				for (int j = 0; j < blocksize; ++j) {
					((Application*)app)->altered->image->data[i*blocksize+j] = 
						std::clamp(scalar * (int)log(1 + data[i*blocksize+j]), 0, 255);
				}
				completed[i] = true;
			});
		}
		while (std::accumulate(completed, completed + threadcount, 0) != threadcount);
		delete[] completed;
	}
	else
	{
		for (int i = 0; i < width * height * channels; ++i) {
			((Application*)app)->altered->image->data[i] = 
				std::clamp(scalar * (int)log(1 + data[i]), 0, 255);
		}
	}
	((Application*)app)->altered->UpdateTexture();
}

void Image::Grayscale(bool multithreaded)
{
	if (this == nullptr) return;

	if (multithreaded)
	{
		unsigned int blocksize = (width * height * channels) / threadcount;
		bool* completed = new bool[threadcount];

		for (int i = 0; i < threadcount; ++i) {
			threadpool.push([blocksize, i, this, completed](int) {
				for (int j = 1; j < blocksize - 1; j += 3) {
					int value = (data[i*blocksize+j-1] + data[i*blocksize+j] + data[i*blocksize+j+1]) / 3;
					((Application*)app)->altered->image->data[i * blocksize + j - 1] = value;
					((Application*)app)->altered->image->data[i * blocksize + j] = value;
					((Application*)app)->altered->image->data[i * blocksize + j + 1] = value;
				}
				completed[i] = true;
				});
		}
		while (std::accumulate(completed, completed + threadcount, 0) != threadcount);
		delete[] completed;
	}
	else
	{
		for (int i = 1; i < width * height * channels - 1; i += 3) {
			int value = (data[i - 1] + data[i] + data[i + 1]) / 3;
			((Application*)app)->altered->image->data[i-1] = value;
			((Application*)app)->altered->image->data[i] = value;
			((Application*)app)->altered->image->data[i+1] = value;
		}
	}
	((Application*)app)->altered->UpdateTexture();
}

void Image::HistogramCreate(float histogram[256], bool multithreaded)
{
	if (this == nullptr) return;

	if (multithreaded)
	{
		/*unsigned int blocksize = (width * height * channels) / threadcount;
		bool* completed = new bool[threadcount];

		for (int i = 0; i < threadcount; ++i) {
			threadpool.push([blocksize, i, this, completed, histogram](int) {
				for (int j = 0; j < blocksize; ++j) {
					++histogram[data[i * blocksize + j]];
				}
				completed[i] = true;
				});
		}
		while (std::accumulate(completed, completed + threadcount, 0) != threadcount);
		delete[] completed;*/

		for (int i = 0; i < width * height * channels; ++i) {
			++histogram[data[i]];
		}

		// Ezt végül csak egy szálon futtatom, mert az ImGUI realtime akarja frissíteni hisztogramot, amitõl a program lefagy.
	}
	else
	{
		for (int i = 0; i < width * height * channels; ++i) {
			++histogram[data[i]];
		}
	}
	for (int i = 0; i < 256; ++i) {
		histogram[i] /= width * height * channels;
	}
}

void Image::HistogramEqualization(bool multithreaded)
{
	if (this == nullptr) return;

	if (multithreaded)
	{
		unsigned int blocksize = (width * height * channels) / threadcount;
		bool* completed = new bool[threadcount];

		for (int i = 0; i < threadcount; ++i) {
			threadpool.push([blocksize, i, this, completed](int) {
				for (int j = 0; j < blocksize; ++j) {
					((Application*)app)->altered->image->data[i * blocksize + j] =
						std::clamp((int)(data[i * blocksize + j] * (1.0f / (128.0 - 1.0f)) * 255), 0, 255);
				}
				completed[i] = true;
			});
		}
		while (std::accumulate(completed, completed + threadcount, 0) != threadcount);
		delete[] completed;
	}
	else
	{
		for (int i = 0; i < width * height * channels; ++i)
		{
			((Application*)app)->altered->image->data[i] = 
				std::clamp((int)(data[i] * (1.0f / (128.0 - 1.0f)) * 255), 0, 255);
		}
	}
	((Application*)app)->altered->UpdateTexture();
}

void Image::BoxFilter(bool multithreaded)
{
	if (this == nullptr) return;

	if (multithreaded)
	{
		const int wch = width * channels;
		const unsigned int blocksize = (width * height * channels) / threadcount;
		bool* completed = new bool[threadcount];

		threadpool.push([blocksize, wch, this, completed](int) {
			for (int i = wch + 2; i < blocksize; ++i)
			{
				int box = (data[i - wch - 3] + data[i - wch] + data[i - wch + 3]
					+ data[i - 3] + data[i] + data[i + 3]
					+ data[i + wch - 3] + data[i + wch] + data[i + wch + 3]) / 9;
				((Application*)app)->altered->image->data[i] = box;
			}
			completed[0] = true;
		});
		for (int i = 1; i < threadcount - 1; ++i) {
			threadpool.push([blocksize, wch, i, this, completed](int) {
				for (int j = 0; j < blocksize; ++j) {
					const int index = i * blocksize + j;
					int box = (data[index - wch - 3] + data[index - wch] + data[index - wch + 3]
						+ data[index - 3] + data[index] + data[index + 3]
						+ data[index + wch - 3] + data[index + wch] + data[index + wch + 3]) / 9;
					((Application*)app)->altered->image->data[index] = box;
				}
				completed[i] = true;
			});
		}
		threadpool.push([blocksize, wch, this, completed](int) {
			for (int i = blocksize * (threadcount - 1); i < blocksize * (threadcount) - wch - 3; ++i) {
				int box = (data[i - wch - 3] + data[i - wch] + data[i - wch + 3]
					+ data[i - 3] + data[i] + data[i + 3]
					+ data[i + wch - 3] + data[i + wch] + data[i + wch + 3]) / 9;
				((Application*)app)->altered->image->data[i] = box;
			}
			completed[threadcount - 1] = true;
			});

		completed[threadcount - 1] = true;
		while (std::accumulate(completed, completed + threadcount, 0) != threadcount);	
		delete[] completed;
	}
	else
	{
		const int wch = width * channels;
		for (int i = wch + 2; i < width * height * channels - (wch - 3); ++i)
		{
			int box = (data[i - wch - 3] + data[i - wch] + data[i - wch + 3]
				+ data[i - 3] + data[i] + data[i + 3]
				+ data[i + wch - 3] + data[i + wch] + data[i + wch + 3]) / 9;
			((Application*)app)->altered->image->data[i] = box;
		}
	}
	((Application*)app)->altered->UpdateTexture();
}

void Image::GaussFilter(bool multithreaded)
{
	double GKernel[3][3];
	constexpr const float PI = 3.14159265358979f;
	constexpr const double sigma = 1.0, s = 2.0 * sigma * sigma;
	double r;

	double sum = 0.0;

	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			r = sqrt(x * x + y * y);
			GKernel[x + 1][y + 1] = (exp(-(r * r) / s)) / (PI * s);
			sum += GKernel[x + 1][y + 1];
		}
	}

	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			GKernel[i][j] /= sum;

	if (this == nullptr) return;

	if (multithreaded)
	{
		const int wch = width * channels;
		const unsigned int blocksize = (width * height * channels) / threadcount;
		bool* completed = new bool[threadcount];

		threadpool.push([blocksize, GKernel, wch, this, completed](int) {
			for (int i = wch + 2; i < blocksize; ++i)
			{
				int box = 
					data[i - wch - 3] * GKernel[0][0] + data[i - wch] * GKernel[0][1] + data[i - wch + 3] * GKernel[0][2]
					+ data[i - 3] * GKernel[1][0] + data[i] * GKernel[1][1] + data[i + 3] * GKernel[1][2]
					+ data[i + wch - 3] * GKernel[2][0] + data[i + wch] * GKernel[2][1] + data[i + wch + 3] * GKernel[2][2];
				((Application*)app)->altered->image->data[i] = box;
			}
			completed[0] = true;
			});
		for (int i = 1; i < threadcount - 1; ++i) {
			threadpool.push([blocksize, GKernel, wch, i, this, completed](int) {
				for (int j = 0; j < blocksize; ++j) {
					const int index = i * blocksize + j;
					int box = 
						data[index - wch - 3] * GKernel[0][0] + data[index - wch] * GKernel[0][1] + data[index - wch + 3] * GKernel[0][2]
						+ data[index - 3] * GKernel[1][0] + data[index] * GKernel[1][1] + data[index + 3] * GKernel[1][2]
						+ data[index + wch - 3] * GKernel[2][0] + data[index + wch] * GKernel[2][1] + data[index + wch + 3] * GKernel[2][2];
					((Application*)app)->altered->image->data[index] = box;
				}
				completed[i] = true;
				});
		}
		threadpool.push([blocksize, GKernel, wch, this, completed](int) {
			for (int i = blocksize * (threadcount - 1); i < blocksize * (threadcount)-wch - 3; ++i) {
				int box = 
					data[i - wch - 3] * GKernel[0][0] + data[i - wch] * GKernel[0][1] + data[i - wch + 3] * GKernel[0][2]
					+ data[i - 3] * GKernel[1][0] + data[i] * GKernel[1][1] + data[i + 3] * GKernel[1][2]
					+ data[i + wch - 3] * GKernel[2][0] + data[i + wch] * GKernel[2][1] + data[i + wch + 3] * GKernel[2][2];
				((Application*)app)->altered->image->data[i] = box;
			}
			completed[threadcount - 1] = true;
			});

		completed[threadcount - 1] = true;
		while (std::accumulate(completed, completed + threadcount, 0) != threadcount);
		delete[] completed;
	}
	else
	{
		const int wch = width * channels;
		for (int i = wch + 2; i < width * height * channels - (wch - 3); ++i)
		{
			int x = i % wch, y = i / wch;

			int box = 
				data[i - wch - 3] * GKernel[0][0] + data[i - wch] * GKernel[0][1] + data[i - wch + 3] * GKernel[0][2]
				+ data[i - 3] * GKernel[1][0] + data[i] * GKernel[1][1] + data[i + 3] * GKernel[1][2]
				+ data[i + wch - 3] * GKernel[2][0] + data[i + wch] * GKernel[2][1] + data[i + wch + 3] * GKernel[2][2];

			((Application*)app)->altered->image->data[i] = box;
		}
	}
	((Application*)app)->altered->UpdateTexture();
}

void Image::Sobel(int threshold, bool multithreaded)
{
	if (this == nullptr) return;

	constexpr const int KernelX[9] = { 1, 0, -1, 2, 0, -2,  1,  0, -1 };
	constexpr const int KernelY[9] = { 1, 2,  1, 0, 0,  0, -1, -2, -1 };

	if (multithreaded)
	{
		const int wch = width * channels;
		const unsigned int blocksize = (width * height * channels) / threadcount;
		bool* completed = new bool[threadcount];

		threadpool.push([blocksize, KernelX, KernelY, threshold, wch, this, completed](int) {
			for (int i = wch + 2; i < blocksize; i += 3)
			{
				int x = (data[i - wch - 3] * KernelX[0] + data[i - wch] * KernelX[1] + data[i - wch + 3] * KernelX[2]
					+ data[i - 3] * KernelX[3] + data[i] * KernelX[4] + data[i + 3] * KernelX[5]
					+ data[i + wch - 3] * KernelX[6] + data[i + wch] * KernelX[7] + data[i + wch + 3] * KernelX[8]);
				int y = (data[i - wch - 3] * KernelY[0] + data[i - wch] * KernelY[1] + data[i - wch + 3] * KernelY[2]
					+ data[i - 3] * KernelY[3] + data[i] * KernelY[4] + data[i + 3] * KernelY[5]
					+ data[i + wch - 3] * KernelY[6] + data[i + wch] * KernelY[7] + data[i + wch + 3] * KernelY[8]);
				int value = (sqrt((x * x) + (y * y)) > threshold) * 255;
				((Application*)app)->altered->image->data[i-2] = value;
				((Application*)app)->altered->image->data[i-1] = value;
				((Application*)app)->altered->image->data[i] = value;
			}
			completed[0] = true;
			});
		for (int i = 1; i < threadcount - 1; ++i) {
			threadpool.push([blocksize, KernelX, KernelY, threshold, wch, i, this, completed](int) {
				for (int j = 0; j < blocksize; j += 3) {
					const int index = i * blocksize + j;
					
					int x = (data[index - wch - 3] * KernelX[0] + data[index - wch] * KernelX[1] + data[index - wch + 3] * KernelX[2]
					+ data[index - 3] * KernelX[3] + data[index] * KernelX[4] + data[index + 3] * KernelX[5]
					+ data[index + wch - 3] * KernelX[6] + data[index + wch] * KernelX[7] + data[index + wch + 3] * KernelX[8]);
					
					int y = (data[index - wch - 3] * KernelY[0] + data[index - wch] * KernelY[1] + data[index - wch + 3] * KernelY[2]
					+ data[index - 3] * KernelY[3] + data[index] * KernelY[4] + data[index + 3] * KernelY[5]
					+ data[index + wch - 3] * KernelY[6] + data[index + wch] * KernelY[7] + data[index + wch + 3] * KernelY[8]);

					int value = (sqrt((x * x) + (y * y)) > threshold) * 255;
					((Application*)app)->altered->image->data[index-2] = value;
					((Application*)app)->altered->image->data[index-1] = value;
					((Application*)app)->altered->image->data[index] = value;
				}
				completed[i] = true;
				});
		}
		threadpool.push([blocksize, KernelX, KernelY, threshold, wch, this, completed](int) {
			for (int i = blocksize * (threadcount - 1); i < blocksize * (threadcount)-wch - 3; i += 3) {
				int x = (data[i - wch - 3] * KernelX[0] + data[i - wch] * KernelX[1] + data[i - wch + 3] * KernelX[2]
					+ data[i - 3] * KernelX[3] + data[i] * KernelX[4] + data[i + 3] * KernelX[5]
					+ data[i + wch - 3] * KernelX[6] + data[i + wch] * KernelX[7] + data[i + wch + 3] * KernelX[8]);
				int y = (data[i - wch - 3] * KernelY[0] + data[i - wch] * KernelY[1] + data[i - wch + 3] * KernelY[2]
					+ data[i - 3] * KernelY[3] + data[i] * KernelY[4] + data[i + 3] * KernelY[5]
					+ data[i + wch - 3] * KernelY[6] + data[i + wch] * KernelY[7] + data[i + wch + 3] * KernelY[8]);
				int value = (sqrt((x * x) + (y * y)) > threshold) * 255;
				((Application*)app)->altered->image->data[i-2] = value;
				((Application*)app)->altered->image->data[i-1] = value;
				((Application*)app)->altered->image->data[i] = value;
			}
			completed[threadcount - 1] = true;
			});

		completed[threadcount - 1] = true;
		while (std::accumulate(completed, completed + threadcount, 0) != threadcount);
		delete[] completed;
	}
	else
	{
		const int wch = width * channels;
		for (int i = wch + 2; i < width * height * channels - (wch - 3); i += 3)
		{
			int x = (data[i - wch - 3] * KernelX[0] + data[i - wch] * KernelX[1] + data[i - wch + 3] * KernelX[2]
				+ data[i - 3] * KernelX[3] + data[i] * KernelX[4] + data[i + 3] * KernelX[5]
				+ data[i + wch - 3] * KernelX[6] + data[i + wch] * KernelX[7] + data[i + wch + 3] * KernelX[8]);

			int y = (data[i - wch - 3] * KernelY[0] + data[i - wch] * KernelY[1] + data[i - wch + 3] * KernelY[2]
				+ data[i - 3] * KernelY[3] + data[i] * KernelY[4] + data[i + 3] * KernelY[5]
				+ data[i + wch - 3] * KernelY[6] + data[i + wch] * KernelY[7] + data[i + wch + 3] * KernelY[8]);

			int value = (sqrt((x * x) + (y * y)) > threshold) * 255;
			((Application*)app)->altered->image->data[i-2] = value;
			((Application*)app)->altered->image->data[i-1] = value;
			((Application*)app)->altered->image->data[i] = value;
		}
	}
	((Application*)app)->altered->UpdateTexture();
}

void Image::Laplace(bool multithreaded)
{
	if (this == nullptr) return;

	constexpr const int LoG[] = {
		 -1, -2, -1,
		 -2, 16, -2,
		 -1, -2, -1,
	};

	if (multithreaded)
	{
		const int wch = width * channels;
		const unsigned int blocksize = (width * height * channels) / threadcount;
		bool* completed = new bool[threadcount];

		threadpool.push([blocksize, LoG, wch, this, completed](int) {
			for (int i = wch + 2; i < blocksize; i += 3)
			{
				int box = (data[i - wch - 3] * LoG[0] + data[i - wch] * LoG[1] + data[i - wch + 3] * LoG[2]
					+ data[i - 3] * LoG[3] + data[i] * LoG[4] + data[i + 3] * LoG[5]
					+ data[i + wch - 3] * LoG[6] + data[i + wch] * LoG[7] + data[i + wch + 3] * LoG[8]);
				int value = (box < 0) * 255;
				((Application*)app)->altered->image->data[i - 2] = value;
				((Application*)app)->altered->image->data[i - 1] = value;
				((Application*)app)->altered->image->data[i] = value;
			}
			completed[0] = true;
			});
		for (int i = 1; i < threadcount - 1; ++i) {
			threadpool.push([blocksize, LoG, wch, i, this, completed](int) {
				for (int j = 2; j < blocksize; j += 3) {
					const int index = i * blocksize + j;

					int box = (data[index - wch - 3] * LoG[0] + data[index - wch] * LoG[1] + data[index - wch + 3] * LoG[2]
						+ data[index - 3] * LoG[3] + data[index] * LoG[4] + data[index + 3] * LoG[5]
						+ data[index + wch - 3] * LoG[6] + data[index + wch] * LoG[7] + data[index + wch + 3] * LoG[8]);

					int value = (box < 0) * 255;
					((Application*)app)->altered->image->data[index - 2] = value;
					((Application*)app)->altered->image->data[index - 1] = value;
					((Application*)app)->altered->image->data[index] = value;
				}
				completed[i] = true;
				});
		}
		threadpool.push([blocksize, LoG, wch, this, completed](int) {
			for (int i = blocksize * (threadcount - 1); i < blocksize * (threadcount)-wch - 3; i += 3) {
				int box = (data[i - wch - 3] * LoG[0] + data[i - wch] * LoG[1] + data[i - wch + 3] * LoG[2]
					+ data[i - 3] * LoG[3] + data[i] * LoG[4] + data[i + 3] * LoG[5]
					+ data[i + wch - 3] * LoG[6] + data[i + wch] * LoG[7] + data[i + wch + 3] * LoG[8]);
				int value = (box < 0) * 255;
				((Application*)app)->altered->image->data[i - 2] = value;
				((Application*)app)->altered->image->data[i - 1] = value;
				((Application*)app)->altered->image->data[i] = value;
			}
			completed[threadcount - 1] = true;
			});

		completed[threadcount - 1] = true;
		while (std::accumulate(completed, completed + threadcount, 0) != threadcount);
		delete[] completed;
	}
	else
	{
		const int wch = width * channels;
		for (int i = wch + 2; i < width * height * channels - (wch - 3); i += 3)
		{
			int box = (data[i - wch - 3] * LoG[0] + data[i - wch] * LoG[1] + data[i - wch + 3] * LoG[2]
				+ data[i - 3] * LoG[3] + data[i] * LoG[4] + data[i + 3] * LoG[5]
				+ data[i + wch - 3] * LoG[6] + data[i + wch] * LoG[7] + data[i + wch + 3] * LoG[8]);

			int value = (box < 0) * 255;
			((Application*)app)->altered->image->data[i - 2] = value;
			((Application*)app)->altered->image->data[i - 1] = value;
			((Application*)app)->altered->image->data[i] = value;
		}
	}
	((Application*)app)->altered->UpdateTexture();
}

void Image::Moravec(int threshold, bool multithreaded)
{
	if (this == nullptr) return;

	if (multithreaded)
	{
		const int wch = width * channels;
		const unsigned int blocksize = (width * height * channels) / threadcount;
		bool* completed = new bool[threadcount];

		threadpool.push([blocksize, threshold, wch, this, completed](int) {
			for (int i = wch + 6; i < blocksize; i += 3)
			{
				int right = (data[i + 3] - data[i]) * (data[i + 3] - data[i]);
				int top = (data[i - wch] - data[i]) * (data[i - wch] - data[i]);
				int right_top = (data[i - wch + 3] - data[i]) * (data[i - wch + 3] - data[i]);
				int right_bottom = (data[i + wch + 3] - data[i]) * (data[i + wch + 3] - data[i]);
			
				int min_error = std::min(std::min(right, top), std::min(right_top, right_bottom));
			
				int value = 0;
				if (min_error > threshold) value = min_error;
			
				((Application*)app)->altered->image->data[i] = value;
				((Application*)app)->altered->image->data[i + 1] = value;
				((Application*)app)->altered->image->data[i + 2] = value;
			}
			completed[0] = true;
			});
		for (int i = 1; i < threadcount - 1; ++i) {
			threadpool.push([blocksize, threshold, wch, i, this, completed](int) {
				for (int j = 0; j < blocksize; j += 3) {
					const int index = i * blocksize + j;

					int right = (data[index + 3] - data[index]) * (data[index + 3] - data[index]);
					int top = (data[index - wch] - data[index]) * (data[index - wch] - data[index]);
					int right_top = (data[index - wch + 3] - data[index]) * (data[index - wch + 3] - data[index]);
					int right_bottom = (data[index + wch + 3] - data[index]) * (data[index + wch + 3] - data[index]);

					int min_error = std::min(std::min(right, top), std::min(right_top, right_bottom));

					int value = 0;
					if (min_error > threshold) value = min_error;

					((Application*)app)->altered->image->data[index] = value;
					((Application*)app)->altered->image->data[index + 1] = value;
					((Application*)app)->altered->image->data[index + 2] = value;
				}
				completed[i] = true;
				});
		}
		threadpool.push([blocksize, threshold, wch, this, completed](int) {
			for (int i = blocksize * (threadcount - 1); i < blocksize * (threadcount)-wch - 3; i += 3) {
				int right = (data[i + 3] - data[i]) * (data[i + 3] - data[i]);
				int top = (data[i - wch] - data[i]) * (data[i - wch] - data[i]);
				int right_top = (data[i - wch + 3] - data[i]) * (data[i - wch + 3] - data[i]);
				int right_bottom = (data[i + wch + 3] - data[i]) * (data[i + wch + 3] - data[i]);

				int min_error = std::min(std::min(right, top), std::min(right_top, right_bottom));

				int value = 0;
				if (min_error > threshold) value = min_error;

				((Application*)app)->altered->image->data[i] = value;
				((Application*)app)->altered->image->data[i + 1] = value;
				((Application*)app)->altered->image->data[i + 2] = value;
			}
			completed[threadcount - 1] = true;
			});

		completed[threadcount - 1] = true;
		while (std::accumulate(completed, completed + threadcount, 0) != threadcount);
		delete[] completed;
	}
	else
	{
		const int wch = width * channels;
		for (int i = wch + 6; i < width * height * channels - (wch - 9); i += 3)
		{
			int right = (data[i + 3] - data[i]) * (data[i + 3] - data[i]);
			int top = (data[i - wch] - data[i]) * (data[i - wch] - data[i]);
			int right_top = (data[i - wch + 3] - data[i]) * (data[i - wch + 3] - data[i]);
			int right_bottom = (data[i + wch + 3] - data[i]) * (data[i + wch + 3] - data[i]);

			int min_error = std::min(std::min(right, top), std::min(right_top, right_bottom));

			int value = 0;
			if (min_error > threshold) value = min_error;

			((Application*)app)->altered->image->data[i]     = value;
			((Application*)app)->altered->image->data[i + 1] = value;
			((Application*)app)->altered->image->data[i + 2] = value;
		}
	}

	((Application*)app)->altered->UpdateTexture();
}
