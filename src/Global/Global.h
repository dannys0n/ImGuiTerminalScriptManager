#pragma once
#include <string>   
#include <algorithm>
#include <cctype>
#include <vector>
#include <imgui.h>
#include <filesystem>
#include <optional>

namespace fs = std::filesystem;

static void TrimAll(std::string& s)
{
    // Remove leading/trailing whitespace including CR/LF/Tabs
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");

    if (start == std::string::npos)
    {
        s.clear();
        return;
    }

    s = s.substr(start, end - start + 1);

    // Remove all CR characters (Windows-style returns)
    s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
}

// Expands an InputText field so it fills the rest of the window width.
inline void RenderFullWidthInput(const char* id, char* buffer, size_t bufferSize)
{
    float fullWidth = ImGui::GetContentRegionAvail().x;
    ImGui::SetNextItemWidth(fullWidth);
    ImGui::InputText(id, buffer, bufferSize);
}

// Computes uniform button width from a list of button labels
inline float ComputeUniformButtonWidth(std::vector<std::string>& labels)
{
    float maxWidth = 0.0f;
    const ImGuiStyle& style = ImGui::GetStyle();

    for (auto text : labels)
    {
        ImVec2 size = ImGui::CalcTextSize(text.c_str());
        float padded = size.x + style.FramePadding.x * 2.0f;
        if (padded > maxWidth)
            maxWidth = padded;
    }
    return maxWidth;
}

static std::optional<std::string> FindScriptByPath(const std::string& name,
                   const std::vector<std::string>& searchPaths)
{
    fs::path base = fs::current_path();

    for (const auto& rel : searchPaths)
    {
        fs::path candidate = base / rel / (name + ".sh");
        if (fs::exists(candidate) && fs::is_regular_file(candidate))
            return rel;
    }

    return std::nullopt; // script not found anywhere
}
