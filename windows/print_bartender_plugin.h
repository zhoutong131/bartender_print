#ifndef FLUTTER_PLUGIN_PRINT_BARTENDER_PLUGIN_H_
#define FLUTTER_PLUGIN_PRINT_BARTENDER_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include <memory>

namespace print_bartender {

class PrintBartenderPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  PrintBartenderPlugin();

  virtual ~PrintBartenderPlugin();

  // Disallow copy and assign.
  PrintBartenderPlugin(const PrintBartenderPlugin&) = delete;
  PrintBartenderPlugin& operator=(const PrintBartenderPlugin&) = delete;

  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

}  // namespace print_bartender

#endif  // FLUTTER_PLUGIN_PRINT_BARTENDER_PLUGIN_H_
