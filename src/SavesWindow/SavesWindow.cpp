#include "SavesWindow.h"
#include <fstream>

void SavesWindow:: ReloadSaves()
{
    saveFiles.clear();
    if (!std::filesystem::exists("Saves"))
        std::filesystem::create_directory("Saves");

    for (auto& entry : std::filesystem::directory_iterator("Saves"))
        if (entry.path().extension() == ".json")
            saveFiles.push_back(entry.path().stem().string());
}

void SavesWindow::Render()
{
    static char nameBuffer[512] = "";

    // --- Save current references ---
    if (ImGui::Button("Create Save"))
    {
        if (strlen(nameBuffer) > 0)
        {
            json j = buttonsWindow->Serialize();
            std::string path = "Saves/" + std::string(nameBuffer) + ".json";
            std::ofstream file(path);
            file << std::setw(2) << j;
            file.close();
            ReloadSaves();
        }
    }
    ImGui::SameLine();
    //ImGui::InputText("Save Name", nameBuffer, sizeof(nameBuffer));
    RenderFullWidthInput("Save Name", nameBuffer, sizeof(nameBuffer));

    ImGui::Separator();

    if (ImGui::Button("Refresh"))
        ReloadSaves();

    ImGui::Separator();

    // --- Compute uniform width for save buttons ---
    float buttonWidth = ComputeUniformButtonWidth(saveFiles);
    // --- Load referenced scripts ---
    for (auto& name : saveFiles)
    {
        if (ImGui::Button(name.c_str(), ImVec2(buttonWidth, 0)))
        {
            std::string path = "Saves/" + name + ".json";
            std::ifstream file(path);
            json j;
            file >> j;
            file.close();
            buttonsWindow->ClearScripts();
            buttonsWindow->Deserialize(j);
        }
    }
}
