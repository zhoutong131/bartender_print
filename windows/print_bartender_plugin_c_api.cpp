#include "include/print_bartender/print_bartender_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "print_bartender_plugin.h"

void PrintBartenderPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  print_bartender::PrintBartenderPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
