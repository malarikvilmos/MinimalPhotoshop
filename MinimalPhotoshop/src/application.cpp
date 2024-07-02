#include <glad/glad.h>
#include "application.h"
#include <iostream>
#include "benchmark.h"
#include "gui.h"
#include "image.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_transform.hpp>

Application::Application()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); 

    window = glfwCreateWindow(1400, 720, "MinimalPhotoshop", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);

    Image image("images/icon.png", this, false, true);
    GLFWimage images[1] = {};
    images[0].pixels = image.data;
    images[0].width = image.width;
    images[0].height = image.height;
    glfwSetWindowIcon(window, 1, images);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glfwSwapInterval(1);
}

Application::~Application()
{
    GUI::Shutdown();
    glfwTerminate();
}

void Application::Run()
{
    GUI::Init(this);
    ShaderLoader::Load();

    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 viewOriginal = glm::translate(view, glm::vec3(-1.45f, 0.0f, -3.0f));
    viewOriginal = glm::scale(viewOriginal, glm::vec3(1.75f, 1.75f, 1.0f));
    glm::mat4 viewAltered = glm::translate(view, glm::vec3(1.45f, 0.0f, -3.0f));
    viewAltered = glm::scale(viewAltered, glm::vec3(1.75f, 1.75f, 1.0f));

    original = new Quad(nullptr, viewOriginal, this);
    altered = new Quad(nullptr, viewAltered, this);
    
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    while (!this->quit && !glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        original->Draw();
        altered->Draw();

        GUI::Begin();
        GUI::DrawMenuBar();
        GUI::DrawEffectsBar();
        GUI::DrawOptionsBar();
        GUI::DrawTimeTaken();
        GUI::DrawMultithreaded();
        GUI::End();      

        glfwSwapBuffers(window);
        glfwWaitEvents();
        
        Benchmark::DoBenchmark();
    }
    delete original;
    delete altered;
}

void Application::ProcessInput() {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        Quit();
    }  
}

void Application::Quit()
{
    this->quit = true;
}
