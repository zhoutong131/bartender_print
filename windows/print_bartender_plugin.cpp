#include "print_bartender_plugin.h"
#include"NativeStructs.h"
// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <sstream>
using flutter::EncodableList;
using flutter::EncodableMap;
using flutter::EncodableValue;

namespace print_bartender {
typedef PrintResult(*PrintFunction)(const char*, const DictionaryList*);

PrintResult printLabel(const char* path, const DictionaryList* info) {
    HINSTANCE hDll = LoadLibrary(L"bartender_print.dll");
    PrintResult resInfo = {};
    resInfo.res = true;
    resInfo.info = "success";
    if (hDll == NULL) {
        std::cerr << "Failed to load Bartender DLL." << std::endl;
        resInfo.res = false;
        resInfo.info = "Failed to load Bartender DLL";
        return resInfo;
    }
    // 获取函数地址
    PrintFunction printInfo = (PrintFunction)GetProcAddress(hDll, "printByBartender");
    if (printInfo == NULL) {
        std::cerr << "Failed to get function address." << std::endl;
        resInfo.res = false;
        resInfo.info = "Failed to get function address.";
        FreeLibrary(hDll);
        return resInfo;
    }
    resInfo = printInfo(path, info);
    FreeLibrary(hDll);
    return resInfo;
}

void FreeDictionaryList(DictionaryList* list) {
    if (!list) return;

    for (int i = 0; i < list->count; ++i) {
        DictionaryDom& dict = list->dictionaries[i];
        for (int j = 0; j < dict.count; ++j) {
            free(const_cast<char*>(dict.entries[j].key));  // 释放strdup内存
            free(const_cast<char*>(dict.entries[j].value));
        }
        delete[] dict.entries;
    }
    delete[] list->dictionaries;
    list->dictionaries = nullptr;
    list->count = 0;
}

DictionaryList ConvertDartDataToNative(const flutter::EncodableList& dartData) {
    DictionaryList nativeList{};
    nativeList.count = static_cast<int>(dartData.size());
    nativeList.dictionaries = new DictionaryDom[nativeList.count];

    for (size_t i = 0; i < dartData.size(); ++i) {
        const auto& dartMap = std::get<flutter::EncodableMap>(dartData[i]);
        DictionaryDom& nativeDict = nativeList.dictionaries[i];
        nativeDict.count = static_cast<int>(dartMap.size());
        nativeDict.entries = new NativeKeyValuePair[nativeDict.count];

        int j = 0;
        for (const auto& pair : dartMap) {
            std::string key = std::get<std::string>(pair.first);
            std::string value = std::get<std::string>(pair.second);
            nativeDict.entries[j].key = _strdup(key.c_str());
            nativeDict.entries[j].value = _strdup(value.c_str());
            if (!nativeDict.entries[j].key || !nativeDict.entries[j].value) {
                // 错误处理：释放已分配的内存
                FreeDictionaryList(&nativeList);
                throw std::bad_alloc();
            }
            ++j;
        }
    }
    return nativeList;
}

// static
void PrintBartenderPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "print_bartender",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<PrintBartenderPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

PrintBartenderPlugin::PrintBartenderPlugin() {}

PrintBartenderPlugin::~PrintBartenderPlugin() {}

void PrintBartenderPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (method_call.method_name().compare("getPlatformVersion") == 0) {
    std::ostringstream version_stream;
    version_stream << "Windows ";
    if (IsWindows10OrGreater()) {
      version_stream << "10+";
    } else if (IsWindows8OrGreater()) {
      version_stream << "8";
    } else if (IsWindows7OrGreater()) {
      version_stream << "7";
    }
    result->Success(flutter::EncodableValue(version_stream.str()));
  } else if (method_call.method_name().compare("printLabel") == 0) {
      // 调起bartender打印
      // 获取参数（假设参数是单个 Map）
      //const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
      const flutter::EncodableValue* arguments = method_call.arguments();
      // 检查参数是否为 Map
      if (!arguments || !std::holds_alternative<flutter::EncodableMap>(*arguments)) {
          result->Error("INVALID_ARGUMENT", "参数必须是 Map");
          return;
      }

      // 提取 Map
      const flutter::EncodableMap& params = std::get<flutter::EncodableMap>(*arguments);
      // 提取 arg1（字符串）
      auto arg1_it = params.find(flutter::EncodableValue("btwPath"));
      std::string btwPath;
      if (arg1_it != params.end() &&
          std::holds_alternative<std::string>(arg1_it->second)) {
          btwPath = std::get<std::string>(arg1_it->second);
      } else {
          result->Error("INVALID_BTW_PATH", "参数未包含btwPath");
          return;
      }
      // 提取 arg2（List<Map<String, String>>）
      auto arg2_it = params.find(flutter::EncodableValue("info"));
      if (arg2_it != params.end() && std::holds_alternative<flutter::EncodableList>(arg2_it->second)) {
          const flutter::EncodableList& list = std::get<flutter::EncodableList>(arg2_it->second);
          DictionaryList dlist = ConvertDartDataToNative(list);
          PrintResult pres = printLabel(btwPath.c_str(), &dlist);
          FreeDictionaryList(&dlist); //  释放内存
          EncodableMap encodableMap;
          encodableMap[EncodableValue("info")] = EncodableValue(pres.info);
          encodableMap[EncodableValue("res")] = EncodableValue(pres.res);
          result->Success(EncodableValue(encodableMap));
          return;
      } else {
          result->Error("INVALID_INFO", "参数未包含info");
          return;
      }
  } else {
    result->NotImplemented();
  }
}

}  // namespace print_bartender
