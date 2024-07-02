#pragma once
#include <map>
#include <string>
#include <filesystem>
#include <glm/glm.hpp>

class Shader
{
public:
	uint32_t ID;

	Shader(const char* vshader, const char* fshader) noexcept;
	~Shader() noexcept;
	void Bind() noexcept;
	void SetUniformMat4(glm::mat4& uniform, const char* name) noexcept;
	void SetUniformSampler2D(uint32_t value, const char* name) noexcept;
	void SetUniformFloat(float value, const char* name) noexcept;
	void SetUniformVec3(glm::vec3& uniform, const char* name) noexcept;

private:
	inline static uint32_t currentlyBoundID = 0;
};

class ShaderLoader
{
public:
	inline static std::map<const std::string, std::string> shaders = {};

	static void Load() noexcept;

private:
	ShaderLoader();
	static void Read(const std::filesystem::path& path) noexcept;
};
