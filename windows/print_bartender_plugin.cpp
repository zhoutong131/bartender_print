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
typedef PrintResult(*PrintFunction)(const char*, const char*, const DictionaryList*);

PrintResult printLabel(const char* path,const char* printerName, const DictionaryList* info) {
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
    PrintFunction printInfo = (PrintFunction)GetProcAddress(hDll, "printByBartender");
    if (printInfo == NULL) {
        std::cerr << "Failed to get function address." << std::endl;
        resInfo.res = false;
        resInfo.info = "Failed to get function address.";
        FreeLibrary(hDll);
        return resInfo;
    }
    resInfo = printInfo(path,printerName, info);
    FreeLibrary(hDll);
    return resInfo;
}

void FreeDictionaryList(DictionaryList* list) {
    if (!list) return;

    for (int i = 0; i < list->count; ++i) {
        DictionaryDom& dict = list->dictionaries[i];
        for (int j = 0; j < dict.count; ++j) {
            free(const_cast<char*>(dict.entries[j].key));
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

flutter::EncodableValue WideStringToEncodable(const std::wstring& wstr) {
    if (wstr.empty()) {
        return flutter::EncodableValue(""); // 返回空字符串
    }

    // 计算转换后 UTF-8 字符串所需的缓冲区大小
    int utf8_size = WideCharToMultiByte(
        CP_UTF8,            // 目标编码为 UTF-8
        0,                  // 无特殊标志
        wstr.c_str(),       // 输入宽字符串
        -1,                 // 自动计算长度（包括终止符）
        nullptr,            // 不接收输出（仅计算大小）
        0,                  // 输出缓冲区大小为0
        nullptr, nullptr    // 默认处理不可转换字符
    );

    if (utf8_size == 0) {
        // 转换失败，返回空或抛出异常
        DWORD error = GetLastError();
        // 处理错误，例如输出日志
        return flutter::EncodableValue("");
    }

    // 分配缓冲区并实际转换
    std::string utf8_str(utf8_size, 0);
    WideCharToMultiByte(
        CP_UTF8, 0, wstr.c_str(), -1,
        utf8_str.data(), utf8_size,
        nullptr, nullptr
    );

    // 移除转换后可能的多余终止符
    utf8_str.resize(utf8_size - 1);

    // 封装为 EncodableValue
    return flutter::EncodableValue(utf8_str);
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
      //const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
      const flutter::EncodableValue* arguments = method_call.arguments();
      if (!arguments || !std::holds_alternative<flutter::EncodableMap>(*arguments)) {
          result->Error("INVALID_ARGUMENT", "INVALID_ARGUMENT");
          return;
      }
      const flutter::EncodableMap& params = std::get<flutter::EncodableMap>(*arguments);
      auto arg1_it = params.find(flutter::EncodableValue("btwPath"));
      std::string btwPath;
      if (arg1_it != params.end() &&
          std::holds_alternative<std::string>(arg1_it->second)) {
          btwPath = std::get<std::string>(arg1_it->second);
      } else {
          result->Error("INVALID_BTW_PATH", "INVALID_BTW_PATH");
          return;
      }
      auto arg2_it = params.find(flutter::EncodableValue("info"));
      if (arg2_it != params.end() && std::holds_alternative<flutter::EncodableList>(arg2_it->second)) {
          const flutter::EncodableList& list = std::get<flutter::EncodableList>(arg2_it->second);
          DictionaryList dlist = ConvertDartDataToNative(list);
          auto pName = params.find(flutter::EncodableValue("printerName"));
          PrintResult pres = printLabel(btwPath.c_str(),(std::get<std::string>(pName->second)).c_str(), & dlist);
          FreeDictionaryList(&dlist);
          EncodableMap encodableMap;
          encodableMap[EncodableValue("info")] = EncodableValue(pres.info);
          encodableMap[EncodableValue("res")] = EncodableValue(pres.res);
          result->Success(EncodableValue(encodableMap));
          return;
      } else {
          result->Error("INVALID_INFO", "INVALID_INFO");
          return;
      }
  } else if (method_call.method_name().compare("getPrintList") == 0) {
      // 枚举已连接的打印机
      DWORD needed = 0, returned = 0;
      BOOL printResult = EnumPrinters(
          PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, // 枚举本地和网络打印机
          nullptr, 4, nullptr, 0, &needed, &returned
      );

      std::vector<BYTE> buffer;
      if (!printResult) {
          DWORD error = GetLastError();
          if (error != ERROR_INSUFFICIENT_BUFFER) {
              std::cerr << "EnumPrinters 失败。错误代码: " << error << std::endl;
              return;
          }
          buffer.resize(needed);
          if (!EnumPrinters(
              PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
              nullptr, 4, buffer.data(), needed, &needed, &returned
          )) {
              std::cerr << "EnumPrinters 失败。错误代码: " << GetLastError() << std::endl;
              return;
          }
      }

      PRINTER_INFO_4* printers = reinterpret_cast<PRINTER_INFO_4*>(buffer.data());
      EncodableList printList;
      for (DWORD i = 0; i < returned; ++i) {
          std::wstring name = printers[i].pPrinterName ? printers[i].pPrinterName : L"";
          printList.push_back(EncodableValue(WideStringToEncodable(name)));
      }
      result->Success(EncodableValue(printList));
  }
  else if (method_call.method_name().compare("getDefaultPrinter") == 0) {
      // 获取默认打印机
      DWORD size = 0;
      BOOL lastResult = GetDefaultPrinter(nullptr, &size);
      if (!result) {
          DWORD error = GetLastError();
          if (error != ERROR_INSUFFICIENT_BUFFER) {
              std::cerr << "未设置默认打印机"  << std::endl;
              return ;
          }
      }

      std::vector<wchar_t> defaultPrinter(size);
      if (!GetDefaultPrinter(defaultPrinter.data(), &size)) {
          std::cerr << "获取默认打印机失败。错误代码: " << GetLastError() << std::endl;
          return ;
      }
      result->Success(EncodableValue(WideStringToEncodable(defaultPrinter.data())));
      return;
  } else {
    result->NotImplemented();
  }
}

}  // namespace print_bartender
