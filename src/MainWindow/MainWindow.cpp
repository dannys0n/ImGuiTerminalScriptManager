#include "MainWindow.h"
#include <filesystem>
namespace fs = std::filesystem;

MainWindow::MainWindow() : App(AppProperties{.imgui_viewports_enable = false}),
pathsWindow(&(buttonsWindow.GetSearchPaths()), &buttonsWindow)
{
}

void MainWindow::OnStart()
{
  buttonsWindow.LoadButtonSearchPaths();
  buttonsWindow.ReloadScripts(true);
  ImFontConfig cfg;
  cfg.SizePixels = 32.0f;
  ImGui::GetIO().Fonts->AddFontDefault(&cfg);
}

void MainWindow::OnUpdate()
{
}

void MainWindow::OnRender()
{
  static int activeWindow = 0;

  ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::Begin("My Tools###ToolsWindow", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing);
  
  ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
  if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
  {
    if (ImGui::BeginTabItem("Buttons"))
    {
      buttonsWindow.Render();
      activeWindow = 0;
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Paths"))
    {
      pathsWindow.Render();
      activeWindow = 1;
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Saves"))
    {
      if (activeWindow != 2)
        savesWindow.ReloadSaves();  // auto-refresh
      savesWindow.Render();
      activeWindow = 2;
      ImGui::EndTabItem();
    }
    
    ImGui::EndTabBar();
  }
  ImGui::End();
}

void MainWindow::OnPostRender()
{
}

void MainWindow::OnShutdown()
{
}