#pragma once
#include "lib_include.h"
#include "ButtonsWindow/ButtonsWindow.h"
#include "ButtonsWindow/ScriptMacro.h"
#include <nlohmann/json.hpp>
#include <filesystem>
#include "Global/Global.h"

using json = nlohmann::json;

class SavesWindow
{
public:
    explicit SavesWindow(ButtonsWindow* buttons) : buttonsWindow(buttons) {}
    void Render();

    void ReloadSaves();
private:
    ButtonsWindow* buttonsWindow;
    std::vector<std::string> saveFiles;
};
