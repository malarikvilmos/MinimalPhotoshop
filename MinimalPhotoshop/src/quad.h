#pragma once
#include "shader.h"
#include "image.h"
#include <glm/glm.hpp>

class Quad
{
public:
	Image* image = nullptr;

	Quad(Image* image, glm::mat4 view, void* app);
	~Quad();
	void SwitchImage(Image* image);
	void UpdateTexture();
	void Draw();

private:
	unsigned int texture1;
	Shader* shader = nullptr;
	glm::mat4 projection = glm::mat4(1.0f);
	glm::mat4 model = glm::mat4(1.0f);

	inline static float vertices[20] {
		 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
		 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
		-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
	};
	inline static unsigned int indices[6] {
		0, 1, 3,
		1, 2, 3
	};
	inline static unsigned int VBO, VAO;
};
