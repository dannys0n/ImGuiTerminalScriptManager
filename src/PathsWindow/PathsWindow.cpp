#include "PathsWindow.h"
using json = nlohmann::json;

void PathsWindow::Render()
{
  ImGui::Separator();
  ImGui::Text("Add New Search Path (relative to tool)");

  static char inputBuf[512] = "";
    // --- Compute uniform width for "Add" + "Browse…" buttons ---
  float buttonWidth = ComputeUniformButtonWidth({"Add", "Browse…"});
  if (ImGui::Button("Add", ImVec2(buttonWidth, 0)))
  {
    if (strlen(inputBuf) > 0)
    {
      std::string normalized = NormalizeRelative(inputBuf);
      scriptSearchPaths->push_back(normalized);
      SaveSearchPaths();
      inputBuf[0] = 0;
    }
  }
  ImGui::SameLine();
  // --- Full-width input field ---
  RenderFullWidthInput("##newpath", inputBuf, sizeof(inputBuf));

  // --- OS directory picker ---
  if (ImGui::Button("Browse…"))
  {
    char buffer[512] = {0};

#if defined(_WIN32)
    FILE *pipe = _popen(
        "powershell -NoProfile -Command \""
        "$f = New-Object System.Windows.Forms.FolderBrowserDialog;"
        "if ($f.ShowDialog() -eq 'OK') { Write-Output $f.SelectedPath }\"",
        "r");

#elif defined(__linux__)
    FILE *pipe = popen("zenity --file-selection --directory", "r");
#else
    FILE *pipe = nullptr;
#endif

    if (pipe)
    {
      fgets(buffer, sizeof(buffer), pipe);
#if defined(_WIN32)
      _pclose(pipe);
#else
      pclose(pipe);
#endif

      std::string path = buffer;

      // trim newline
      path.erase(std::remove(path.begin(), path.end(), '\n'), path.end());
      path.erase(std::remove(path.begin(), path.end(), '\r'), path.end());

      if (!path.empty())
      {
        std::string rel = NormalizeRelative(path);
        scriptSearchPaths->push_back(rel);
        SaveSearchPaths();
      }
    }
  }

  ImGui::Separator();

  ImGui::Text("Script Search Paths");
  ImGui::Separator();

  float maxWidth = ComputeMaxPathWidth();
  // ---- Existing paths ----
  for (int i = 0; i < scriptSearchPaths->size(); ++i)
  {
    RenderPathRow(i, maxWidth);
  }
}

float PathsWindow::ComputeMaxPathWidth() const
{
    float maxWidth = 0.0f;

    for (const auto& p : *scriptSearchPaths)
    {
        ImVec2 ts = ImGui::CalcTextSize(p.c_str());
        float padded = ts.x + ImGui::GetStyle().FramePadding.x * 2.0f;
        if (padded > maxWidth)
            maxWidth = padded;
    }

    return maxWidth;
}

void PathsWindow::RenderPathRow(int index, float maxWidth)
{
    ImGui::PushID(index);

    // Reserve fixed width space for the text
    ImGui::BeginGroup();
    {
        // Invisible button that only reserves space
        ImGui::InvisibleButton("##slot", ImVec2(maxWidth, ImGui::GetFontSize()));

        // Draw text inside the slot manually by positioning cursor
        ImVec2 pos = ImGui::GetItemRectMin();
        ImGui::SetCursorScreenPos(pos);
        ImGui::TextUnformatted((*scriptSearchPaths)[index].c_str());
    }
    ImGui::EndGroup();

    ImGui::SameLine();

    // Remove button aligned in a column
    if (ImGui::Button("Remove"))
    {
        scriptSearchPaths->erase(scriptSearchPaths->begin() + index);
        SaveSearchPaths();
    }

    ImGui::PopID();
}


std::string PathsWindow::NormalizeRelative(const std::string &path)
{
  namespace fs = std::filesystem;

  fs::path abs = fs::absolute(path);
  fs::path base = fs::current_path();

  std::error_code ec;
  fs::path rel = fs::relative(abs, base, ec);

  // If relative conversion worked always use it
  if (!ec && !rel.empty())
  {
    return rel.string().append("/");
  }
  // Fallback: return empty to signal failure
  return "";
}

void PathsWindow::SaveSearchPaths()
{
  json j;
  j["scriptPaths"] = *scriptSearchPaths;

  if (!std::filesystem::exists("Config"))
    std::filesystem::create_directory("Config");

  std::ofstream("Config/paths.json") << std::setw(2) << j;

  
  buttonsWindow->SaveButtonSearchPaths();
  buttonsWindow->ReloadScripts();
}

float PathsWindow::ComputeUniformButtonWidth(std::initializer_list<const char*> labels) const
{
    float maxWidth = 0.0f;
    auto& style = ImGui::GetStyle();

    for (auto* text : labels)
    {
        ImVec2 size = ImGui::CalcTextSize(text);
        float padded = size.x + style.FramePadding.x * 2.0f;
        if (padded > maxWidth)
            maxWidth = padded;
    }

    return maxWidth;
}

void PathsWindow::RenderFullWidthInput(const char* id, char* buffer, size_t bufferSize)
{
    float fullWidth =
        ImGui::GetContentRegionAvail().x;  // width remaining on this line

    ImGui::SetNextItemWidth(fullWidth);
    ImGui::InputText(id, buffer, bufferSize);
}
