#pragma once
#include "App.h"
#include "ButtonsWindow/ButtonsWindow.h"
#include "ButtonsWindow/ScriptMacro.h"
#include "SavesWindow/SavesWindow.h"
#include "PathsWindow/PathsWindow.h"
#include <iostream>

class MainWindow : public App
{
public:
  MainWindow();
  void OnStart() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnPostRender() override;
  void OnShutdown() override;

private:
  ButtonsWindow buttonsWindow;
  SavesWindow savesWindow{&buttonsWindow};
  PathsWindow pathsWindow;
};