
#pragma once
#include "App.h"
#include <iostream>
class MyApp : public App
{
    public:
    MyApp() : App(AppProperties{}) {}
   void OnStart() override {}
   void OnUpdate() override {}
   void OnRender() override {
    ImGui::ShowDemoWindow();
    ImPlot::ShowDemoWindow();
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("My window",nullptr,ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing);
    static int HowMuchFUcking =0;
    ImGui::SliderInt("Fucking amount ",&HowMuchFUcking,0,100);
    if (ImGui::Button("Button that fucks you"))
    {
        std::cerr << "Fuck you about " << HowMuchFUcking << " times" <<  std::endl;
    }
    ImGui::End();
   }
   void OnPostRender() override {}
   void OnShutdown()override {}
};


