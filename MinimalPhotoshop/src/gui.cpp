#include "gui.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui_dialog/ImGuiFileDialog.h>

#include <iostream>
#include "benchmark.h"
#include <glad/glad.h>

void GUI::Init(Application* app)
{
    GUI::app = app;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(GUI::app->window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void GUI::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GUI::Begin()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::End()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::DrawMenuBar()
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.30f, 0.30f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.50f, 0.50f, 0.50f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));

    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByContainedInFullName, ".png", ImVec4(0.2f, 0.8f, 0.0f, 0.9f));
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByContainedInFullName, ".jpg", ImVec4(0.2f, 0.8f, 0.0f, 0.9f));
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Load Image"))
            {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".png,.jpg", ".");
            }
            if (ImGui::MenuItem("Save Image"))
            {
                ImGuiFileDialog::Instance()->OpenDialog("SaveFileDlgKey", "Save as...", ".png,.jpg", ".");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit"))
            {
                GUI::app->Quit();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            if (GUI::image != nullptr)
                delete GUI::image;
            GUI::image = new Image(filePathName.c_str(), app, true);
            app->original->SwitchImage(GUI::image);
            app->altered->SwitchImage(GUI::image);
            if (GUI::state == State::Histogram) 
            {
                GUI::image->HistogramCreate(histogramLeft, multithreading);
                GUI::app->altered->image->HistogramCreate(histogramRight, multithreading);
            }
        }
        ImGuiFileDialog::Instance()->Close();
    }
    if (ImGuiFileDialog::Instance()->Display("SaveFileDlgKey"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            app->altered->image->Save(filePathName.c_str());
        }
        ImGuiFileDialog::Instance()->Close();
    }
    ImGui::PopStyleColor(11);
}

void GUI::DrawEffectsBar()
{
    ImGui::Begin("Effects", (bool*)0,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.30f, 0.30f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.50f, 0.50f, 0.50f, 1.0f));
    ImGui::SetWindowPos(ImVec2(0, 20));
    if (ImGui::Button("Invert"))
    {
        GUI::state = State::Invert;
    } ImGui::SameLine();

    if (ImGui::Button("Gamma transform"))
    {
        GUI::state = State::GammaTransform;
    } ImGui::SameLine();

    if (ImGui::Button("Logarithmic transform"))
    {
        GUI::state = State::LogarithmicTransform;
    } ImGui::SameLine();
    if (ImGui::Button("Grayscale"))
    {
        GUI::state = State::Grayscale;
    } ImGui::SameLine();
    if (ImGui::Button("Histogram"))
    {
        GUI::state = State::Histogram;
        GUI::image->HistogramCreate(GUI::histogramLeft, multithreading);
        GUI::app->altered->image->HistogramCreate(GUI::histogramRight, multithreading);
    } ImGui::SameLine();
    if (ImGui::Button("Box filter")) 
    {
        GUI::state = State::BoxFilter;
    } ImGui::SameLine();
    if (ImGui::Button("Gauss filter"))
    {
        GUI::state = State::GaussFilter;
    } ImGui::SameLine();
    if (ImGui::Button("Sobel edge detection"))
    {
        GUI::state = State::SobelEdge;
    } ImGui::SameLine();
    if (ImGui::Button("Laplace edge detection"))
    {
        GUI::state = State::LaplaceEdge;
    } ImGui::SameLine();
    if (ImGui::Button("Moravec corner detection"))
    {
        GUI::state = State::FeatureDetection;
    } ImGui::SameLine();
    
    ImGui::PopStyleColor(3);
    ImGui::End();
}

void GUI::DrawOptionsBar()
{
    ImGui::SetNextWindowPos(ImVec2(1400 * 0.5f, 720 * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(300, 400));

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.30f, 0.30f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.50f, 0.50f, 0.50f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    switch (GUI::state)
    {
    case State::None:
        ImGui::Begin("None", (bool*)0,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetCursorPosX(120);
        ImGui::SetCursorPosY(360);
        ImGui::End();
        break;
    case State::Invert:
        ImGui::Begin("Invert", (bool*)0,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetCursorPosX(120);
        ImGui::SetCursorPosY(360);
        if (ImGui::Button("Process")) {
            Benchmark::MeasureBegin();
            GUI::image->Invert(GUI::multithreading);
            Benchmark::MeasureEnd();
        }
        ImGui::End();
        break;
    case State::GammaTransform:
        ImGui::Begin("Gamma Transform", (bool*)0,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SliderFloat("Gamma", &GUI::gammaGamma, 0.1f, 2.0f);
        ImGui::SetCursorPosX(120);
        ImGui::SetCursorPosY(360);
        if (ImGui::Button("Process")) {
            Benchmark::MeasureBegin();
            GUI::image->GammaTransform(GUI::gammaGamma, GUI::multithreading);
            Benchmark::MeasureEnd();
        }
        ImGui::End();
        break;
    case State::LogarithmicTransform:
        ImGui::Begin("Logarithmic Transform", (bool*)0,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);

        ImGui::SliderInt("Scalar", &GUI::scalarLogarithmic, 0, 100);
        ImGui::SetCursorPosX(120);
        ImGui::SetCursorPosY(360);
        if (ImGui::Button("Process")) {
            Benchmark::MeasureBegin();
            GUI::image->LogarithmicTransform(GUI::scalarLogarithmic, GUI::multithreading);
            Benchmark::MeasureEnd();
        }
        ImGui::End();
        break;
    case State::Grayscale:
        ImGui::Begin("Grayscale", (bool*)0,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);

        ImGui::SetCursorPosX(120);
        ImGui::SetCursorPosY(360);
        if (ImGui::Button("Process")) {
            Benchmark::MeasureBegin();
            GUI::image->Grayscale(GUI::multithreading);
            Benchmark::MeasureEnd();
        }
        ImGui::End();
        break;
    case State::Histogram:
        ImGui::Begin("Histogram", (bool*)0,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::PlotHistogram("Left", GUI::histogramLeft, 256, 0, (const char*)0, 3.4028235E38F, 3.4028235E38F, ImVec2(220, 50));
        ImGui::PlotHistogram("Right", GUI::histogramRight, 256, 0, (const char*)0, 3.4028235E38F, 3.4028235E38F, ImVec2(220, 50));
        ImGui::SetCursorPosX(120);
        ImGui::SetCursorPosY(360);
        if (ImGui::Button("Process")) {
            Benchmark::MeasureBegin();
            GUI::image->HistogramEqualization(multithreading);
            GUI::app->altered->image->HistogramCreate(histogramRight, multithreading);
            Benchmark::MeasureEnd();
        }
        ImGui::End();
        break;
    case State::BoxFilter:
        ImGui::Begin("Box Filter", (bool*)0,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetCursorPosX(120);
        ImGui::SetCursorPosY(360);
        if (ImGui::Button("Process")) {
            Benchmark::MeasureBegin();
            GUI::image->BoxFilter(multithreading);
            Benchmark::MeasureEnd();
        }
        ImGui::End();
        break;
    case State::GaussFilter:
        ImGui::Begin("Gauss Filter", (bool*)0,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetCursorPosX(120);
        ImGui::SetCursorPosY(360);
        if (ImGui::Button("Process")) {
            Benchmark::MeasureBegin();
            GUI::image->GaussFilter(multithreading);
            Benchmark::MeasureEnd();
        }
        ImGui::End();
        break;
    case State::SobelEdge:
        ImGui::Begin("Sobel Edge", (bool*)0,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SliderInt("Threshold", &GUI::threshold, 0, 300);
        ImGui::SetCursorPosX(120);
        ImGui::SetCursorPosY(360);
        if (ImGui::Button("Process")) {
            Benchmark::MeasureBegin();
            GUI::image->Sobel(GUI::threshold, multithreading);
            Benchmark::MeasureEnd();
        }
        ImGui::End();
        break;
    case State::LaplaceEdge:
        ImGui::Begin("Laplace Edge", (bool*)0,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetCursorPosX(120);
        ImGui::SetCursorPosY(360);
        if (ImGui::Button("Process")) {
            Benchmark::MeasureBegin();
            GUI::image->Laplace(multithreading);
            Benchmark::MeasureEnd();
        }
        ImGui::End();
        break;
    case State::FeatureDetection:
        ImGui::Begin("Moravec", (bool*)0,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SliderInt("Threshold", &GUI::thresholdMoravec, 10, 1000);
        ImGui::SetCursorPosX(120);
        ImGui::SetCursorPosY(360);
        if (ImGui::Button("Process")) {
            Benchmark::MeasureBegin();
            GUI::image->Moravec(GUI::thresholdMoravec, multithreading);
            Benchmark::MeasureEnd();
        }
        ImGui::End();
        break;
    }
    ImGui::PopStyleColor(11);
}

void GUI::DrawTimeTaken()
{
    ImGui::SetNextWindowPos(ImVec2(1400 * 0.5f, 720 * 0.1f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::Begin("msElapsed", (bool*)0,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
    ImGui::Text(std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(Benchmark::duration).count()).c_str());
    ImGui::SameLine();
    ImGui::Text("ms");
    ImGui::End();
}

void GUI::DrawMultithreaded()
{
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.30f, 0.30f, 0.30f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.50f, 0.50f, 0.50f, 1.0f));

    ImGui::SetNextWindowPos(ImVec2(1400 * 0.5f, 720 * 0.12f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::Begin("Multithreaded", (bool*)0,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
    ImGui::Checkbox("Multithreading", &GUI::multithreading);
    ImGui::End();
    ImGui::PopStyleColor(4);
}
