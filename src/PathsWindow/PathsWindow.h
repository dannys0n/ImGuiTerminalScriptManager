#pragma once
#include "lib_include.h"
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include "ButtonsWindow/ButtonsWindow.h"
#include "ButtonsWindow/ScriptMacro.h"
#include "Global/Global.h"

class PathsWindow
{
public:
    PathsWindow(std::vector<std::string>* pathsRef, ButtonsWindow* buttons)
        : scriptSearchPaths(pathsRef), buttonsWindow(buttons) {}

    void Render();

private:
    std::vector<std::string>* scriptSearchPaths;
    ButtonsWindow* buttonsWindow;

public:
    void SaveSearchPaths();
    float ComputeMaxPathWidth() const;
    void RenderPathRow(int index, float maxWidth);
    float ComputeUniformButtonWidth(std::initializer_list<const char*> labels) const;
    void RenderFullWidthInput(const char* id, char* buffer, size_t bufferSize);
    std::string NormalizeRelative(const std::string& path);
};