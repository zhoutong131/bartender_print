#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    // 单个键值对
    struct NativeKeyValuePair {
        const char* key;
        const char* value;
    };

    // 单个 map（字典）
    struct DictionaryDom {
        NativeKeyValuePair* entries;  // 键值对数组
        int count;              // 键值对数量
    };

    // 外层 vector（列表）
    struct DictionaryList {
        DictionaryDom* dictionaries;  // 字典数组
        int count;                 // 字典数量
    };
    struct PrintResult {
        const char* info;
        bool res;
    };
    // 函数指针类型定义
    typedef void(__cdecl* ProcessDataFunc)(const DictionaryList* data);

#ifdef __cplusplus
}
#endif