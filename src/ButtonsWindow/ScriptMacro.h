#pragma once
#include <string>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

struct ScriptMacro
{
  std::string name;
  std::string path;
  std::string content;
  std::string title;
  std::string description;
  std::string category;
};

static void ParseScriptMetadata(const std::string &content, ScriptMacro &macro)
{
  std::istringstream stream(content);
  std::string line;
  int lineCount = 0;

  while (std::getline(stream, line) && lineCount < 10)
  {
    lineCount++;
    if (line.starts_with("# @"))
    {
      auto keyEnd = line.find(':');
      if (keyEnd != std::string::npos)
      {
        std::string key = line.substr(2, keyEnd - 2);
        std::string value = line.substr(keyEnd + 1);
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);

        if (key == "@title")
          macro.title = value;
        else if (key == "@desc")
          macro.description = value;
        else if (key == "@category")
          macro.category = value;
      }
    }
  }

  // Fallbacks
  if (macro.title.empty())
    macro.title = macro.name;
}


static void LaunchScript(const ScriptMacro &script)
{
  std::string scriptPath = script.path;

  // Detect WSL
  bool isWSL = false;
  {
    std::ifstream f("/proc/version");
    if (f.is_open())
    {
      std::string version;
      std::getline(f, version);
      if (version.find("Microsoft") != std::string::npos ||
          version.find("WSL") != std::string::npos)
        isWSL = true;
    }
  }

  std::string command;

  if (isWSL)
  {
    // Correct absolute-path safe WSL command
    command =
        "/mnt/c/Windows/System32/cmd.exe /C start \"\" wt.exe wsl.exe "
        "-d \"$WSL_DISTRO_NAME\" -- bash -lc \"bash '" +
        scriptPath + "'\"";
  }
  else
  {
    if (std::system("command -v konsole > /dev/null 2>&1") == 0)
      command = "konsole --hold -e bash \"" + scriptPath + "\"";
    else if (std::system("command -v gnome-terminal > /dev/null 2>&1") == 0)
      command = "gnome-terminal -- bash -c 'bash \"" + scriptPath + "\"; exec bash'";
    else if (std::system("command -v xfce4-terminal > /dev/null 2>&1") == 0)
      command = "xfce4-terminal --hold -e bash \"" + scriptPath + "\"";
    else if (std::system("command -v x-terminal-emulator > /dev/null 2>&1") == 0)
      command = "x-terminal-emulator -e bash \"" + scriptPath + "\"";
    else
      command = "bash \"" + scriptPath + "\"";
  }

  std::thread([command]()
              {
        int result = std::system(command.c_str());
        if (result != 0)
            fprintf(stderr, "[ERROR] Script failed: %s (code %d)\n", command.c_str(), result); })
      .detach();
}

static void SaveSearchPaths(std::vector<std::string>& scriptSearchPaths)
{
  fs::path configDir = fs::current_path() / "Config";
  fs::path configPath = configDir / "paths.json";

  if (!fs::exists(configDir))
    fs::create_directory(configDir);

  json j;
  j["scriptPaths"] = scriptSearchPaths;

  std::ofstream f(configPath);
  if (f.is_open())
    f << std::setw(2) << j;
}

static void LoadSearchPaths(std::vector<std::string>& scriptSearchPaths)
{
  fs::path configDir = fs::current_path() / "Config";
  fs::path configPath = configDir / "paths.json";

  if (!fs::exists(configDir))
    fs::create_directory(configDir);

  scriptSearchPaths.clear();

  if (!fs::exists(configPath))
  {
    // Default: just the Scripts folder
    scriptSearchPaths.push_back("Scripts");
    SaveSearchPaths(scriptSearchPaths);
    return;
  }

  std::ifstream f(configPath);
  if (!f.is_open())
  {
    // Fallback: default
    scriptSearchPaths.push_back("Scripts");
    return;
  }

  json j;
  f >> j;

  if (j.contains("scriptPaths") && j["scriptPaths"].is_array())
  {
    for (auto &p : j["scriptPaths"])
      scriptSearchPaths.push_back(p.get<std::string>());
  }

  if (scriptSearchPaths.empty())
    scriptSearchPaths.push_back("Scripts");
}

static void AddSearchPath(const std::string &relPath, std::vector<std::string>& scriptSearchPaths)
{
  if (relPath.empty())
  return;
  
  // avoid duplicates
  for (auto &p : scriptSearchPaths)
  if (p == relPath)
  return;
  
  scriptSearchPaths.push_back(relPath);
  SaveSearchPaths(scriptSearchPaths);
}

static void RemoveSearchPath(size_t index, std::vector<std::string>& scriptSearchPaths)
{
  if (index >= scriptSearchPaths.size())
  return;
  
  scriptSearchPaths.erase(scriptSearchPaths.begin() + index);
  if (scriptSearchPaths.empty())
  scriptSearchPaths.push_back("Scripts");
  
  SaveSearchPaths(scriptSearchPaths);
}
