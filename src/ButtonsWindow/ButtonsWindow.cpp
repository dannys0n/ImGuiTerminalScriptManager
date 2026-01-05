#include "ButtonsWindow.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

void ButtonsWindow::Render()
{
  // --- Add New Script ---
  RenderAddNewScript();
  ImGui::Separator();
  // --- UI for Refresh and Categorize ---
  if (ImGui::Button("Refresh"))
    ReloadScripts(true);
  ImGui::SameLine();
  if (ImGui::Button("Categorize Mode"))
  {
    ToggleCategorizeMode();
    ReloadScripts(true);
  }
  ImGui::Separator();
  // --- Script buttons ---
  RenderScriptList();
}

void ButtonsWindow::ReloadScripts(bool ParseMetadata)
{
  scripts.clear();

  if (scriptSearchPaths.empty())
    LoadSearchPaths(scriptSearchPaths);

  namespace fs = std::filesystem;
  fs::path base = fs::current_path();

  for (const auto &rel : scriptSearchPaths)
  {
    fs::path dir = base / rel;

    if (!fs::exists(dir) || !fs::is_directory(dir))
      continue;

    for (auto &entry : fs::directory_iterator(dir))
    {
      if (entry.path().extension() == ".sh")
      {
        ScriptMacro macro;
        macro.name = entry.path().stem().string();
        macro.path = entry.path().string(); // absolute path

        // --- LOAD ENTIRE SCRIPT CONTENT ---
        {
          std::ifstream f(entry.path());
          if (f.is_open())
          {
            std::stringstream ss;
            ss << f.rdbuf();
            macro.content = ss.str();
          }
        }

        // --- PARSE METADATA FROM CONTENT ---
        if (ParseMetadata)
          ParseScriptMetadata(macro.content, macro);

        scripts.push_back(std::move(macro));
      }
    }
  }
}


bool IsWSL()
{
#ifdef __linux__
  std::ifstream f("/proc/version");
  if (f.is_open())
  {
    std::string version;
    std::getline(f, version);
    return (version.find("Microsoft") != std::string::npos ||
            version.find("WSL") != std::string::npos);
  }
#endif
  return false;
}

void ButtonsWindow::OpenInEditor(const std::string &path)
{
#if defined(_WIN32)
  std::string command = "start \"\" \"" + path + "\"";

#elif defined(__APPLE__)
  std::string command = "open \"" + path + "\"";

#else // Linux and WSL
  if (IsWSL())
  {
    // Convert Linux path to Windows path for WSL
    std::string winPath = path;
    if (winPath.rfind("/mnt/", 0) == 0)
    {
      // /mnt/c/... → C:\...
      winPath = winPath.substr(5);
      winPath[0] = std::toupper(winPath[0]);
      for (auto &c : winPath)
        if (c == '/')
          c = '\\';
    }

    // Try opening in Windows default editor (like Notepad)
    std::string command = "cmd.exe /C start \"\" \"" + winPath + "\"";
    std::system(command.c_str());
    return;
  }
  else
  {
    // Native Linux
    std::string command = "xdg-open \"" + path + "\"";
  }
#endif

  // std::system(command.c_str());
}

void ButtonsWindow::ClearScripts()
{
  scripts.clear();
  selected = -1;
}

json ButtonsWindow::Serialize() const
{
  json j;
  for (const auto &s : scripts)
  {
    j["scripts"].push_back({{"name", s.name}});
  }
  return j;
}

void ButtonsWindow::Deserialize(const json &j)
{
  scripts.clear();
  if (!j.contains("scripts"))
    return;

  for (auto &item : j["scripts"])
  {
    ScriptMacro sm;
    sm.name = item.value("name", "");

    // Load the actual content from file
    std::string path = "Scripts/" + sm.name + ".sh";
    if (std::filesystem::exists(path))
    {
      std::ifstream file(path);
      sm.content.assign(
          std::istreambuf_iterator<char>(file),
          std::istreambuf_iterator<char>());
    }
    else
    {
      sm.content = "# Missing script file\n";
      fprintf(stderr, "[WARN] Missing script: %s\n", path.c_str());
    }

    ParseScriptMetadata(sm.content, sm);

    scripts.push_back(sm);
  }
}



void ButtonsWindow::RenderScriptList()
{
  float maxButtonWidth = 0.0f;

  // --- Group scripts by category ---
  std::unordered_map<std::string, std::vector<ScriptMacro>> categorized;
  for (const auto &script : scripts)
  {
    if (!categorizeMode)
    {
      categorized["All Scripts"].push_back(script);
    }
    else
    {
      std::string category = script.category.empty() ? "Uncategorized" : script.category;
      TrimAll(category);
      categorized[category].push_back(script);
    }


    // get longest length to size buttons
    const std::string label = script.name;
    ImVec2 size = ImGui::CalcTextSize(label.c_str());
    float paddedWidth =
        size.x + ImGui::GetStyle().FramePadding.x * 2.0f;
    if (paddedWidth > maxButtonWidth)
      maxButtonWidth = paddedWidth;
  }

  // load in buttons
  for (auto &[category, group] : categorized)
  {
    // dropdown for category
    if (ImGui::TreeNodeEx(category.c_str(), ImGuiTreeNodeFlags_DefaultOpen))

    //if (ImGui::CollapsingHeader(category.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
      for (int i = 0; i < group.size(); ++i)
      {
        const auto &script = group[i];
        // std::string buttonId = script.title + "##" + script.name; // avoid ID conflicts
        std::string buttonId = script.name;

        if (ImGui::Button(buttonId.c_str(), ImVec2(maxButtonWidth, 0)))
          LaunchScript(script);

        // Tooltip
        if (ImGui::IsItemHovered() && !script.description.empty())
          ImGui::SetTooltip("%s", script.description.c_str());

        ImGui::SameLine();
        if (ImGui::Button(("Edit##" + script.name).c_str()))
        {
          auto found = FindScriptByPath(script.name, scriptSearchPaths);
          if (found.has_value())
          {
            if (found->back() != '/')
              found->append("/");

              #if defined(_WIN32)
              OpenInEditor(found.value() + script.name + ".sh");
            #elif defined(__linux__)
              PlatformOpen::OpenFile(found.value() + script.name + ".sh");
            #endif
          }
          else
          {
            fprintf(stderr, "[ERROR] Could not locate script '%s' in any search path\n",
                    script.name.c_str());
          }
        }

        ImGui::SameLine();
        if (ImGui::Button(("Remove##" + script.name).c_str()))
        {
          // Remove reference from main scripts list
          auto it = std::find_if(scripts.begin(), scripts.end(), [&](const auto &s)
                                 { return s.name == script.name; });
          if (it != scripts.end())
            scripts.erase(it);
          break;
        }
      }
      ImGui::TreePop();
      ImGui::Spacing();
    }
  }
}

void ButtonsWindow::RenderAddNewScript()
{
  if (ImGui::Button("New Script"))
  {
    if (strlen(nameBuffer) > 0)
    {
      std::string name = std::string(nameBuffer);
      std::string path = "Scripts/" + name + ".sh";
      
      // --- Ensure directory exists ---
      fs::path dir = fs::current_path() / "Scripts";
      std::error_code ec;
      fs::create_directories(dir, ec);   // safe: no error if it already exists
      if (ec)
      {
          fprintf(stderr, "[ERROR] Failed to create Scripts directory: %s\n",
                  ec.message().c_str());
          return;
      }
      
      // --- Generate metadata stub ---
      std::ostringstream stub;
      stub << "#!/usr/bin/env bash\n";
      //stub << "# @title: " << name << "\n"; // not using
      stub << "# @desc: on button hover description.\n";
      stub << "# @category: general\n";
      stub << "# ==========================================\n\n";

      // --- Write the file ---
      std::ofstream file(path, std::ios::out | std::ios::binary);
      file << stub.str();
      file.close();
      
      // Make executable
      std::filesystem::permissions(path,
      std::filesystem::perms::owner_exec |
      std::filesystem::perms::owner_read |
      std::filesystem::perms::owner_write);
      
      ReloadScripts(true);
      OpenInEditor(path);
      printf("[INFO] Created new stub: %s\n", path.c_str());
    }
  }
  ImGui::SameLine();
  RenderFullWidthInput("Script Name", nameBuffer, IM_ARRAYSIZE(nameBuffer));
}

void ButtonsWindow::SaveButtonSearchPaths()
{
  SaveSearchPaths(scriptSearchPaths);
}

void ButtonsWindow::LoadButtonSearchPaths()
{
  LoadSearchPaths(scriptSearchPaths);
}

