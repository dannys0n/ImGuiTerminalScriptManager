#pragma once
#include "Global/Global.h"
#include "lib_include.h"
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <thread>
#include <future>
#include <nlohmann/json.hpp>
#include "ScriptMacro.h"
using json = nlohmann::json;


class ButtonsWindow
{
public:

  std::vector<ScriptMacro> scripts;
  int selected = -1;
  char nameBuffer[128 * 2] = "";
  std::string editBuffer;

  void Render();
  void ReloadScripts(bool ParseMetadata = false);
  void ClearScripts();
  json Serialize() const;
  void Deserialize(const json &j);
  
  void LoadButtonSearchPaths();
  void SaveButtonSearchPaths();
  const std::vector<std::string> &GetSearchPaths() const { return scriptSearchPaths; }
  std::vector<std::string>& GetSearchPaths() { return scriptSearchPaths; }
  
  private:
  void RenderAddNewScript();
  void RenderScriptList();
  void OpenInEditor(const std::string &path);
  void AddExistingScriptPopup();
  void ToggleCategorizeMode() { categorizeMode = !categorizeMode; }
  std::vector<std::string> scriptSearchPaths;
  bool categorizeMode = false;
};