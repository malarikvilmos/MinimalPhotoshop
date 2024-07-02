#include "quad.h"
#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>
#include "application.h"

Quad::Quad(Image* image, glm::mat4 view, void* app)
{
	if (image != nullptr) {
		this->image = new Image(image, (Application*)app);
	}
		
	projection = glm::perspective(glm::radians(45.0f), 1400.0f / 720.0f, 0.1f, 100.0f);

	shader = new Shader(ShaderLoader::shaders["image.vert"].c_str(), ShaderLoader::shaders["image.frag"].c_str());
	shader->SetUniformMat4(projection, "projection");
	shader->SetUniformMat4(view, "view");
	shader->SetUniformMat4(model, "model");

	glGenVertexArrays(1, &Quad::VAO);
	glBindVertexArray(Quad::VAO);

	glGenBuffers(1, &Quad::VBO);
	glBindBuffer(GL_ARRAY_BUFFER, Quad::VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Quad::vertices), Quad::vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glDeleteTextures(1, &texture1);
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

Quad::~Quad()
{
	delete shader;
}

void Quad::SwitchImage(Image* image)
{
	if (this->image != nullptr)
	{
		delete this->image;
	}
		
	this->image = new Image(image, Image::app);
	UpdateTexture();
}

void Quad::UpdateTexture()
{
	this->shader->Bind();
	if (image) {
		if (image->channels == 3) 
		{
			glBindTexture(GL_TEXTURE_2D, this->texture1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->image->width, this->image->height, 0,
				GL_RGB, GL_UNSIGNED_BYTE, this->image->data);
		}
		else if (image->channels == 4)
		{
			glBindTexture(GL_TEXTURE_2D, this->texture1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->image->width, this->image->height, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, this->image->data);
		}
	}
}

void Quad::Draw()
{
	this->shader->Bind();
	if (image != nullptr)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, Quad::indices);
}
