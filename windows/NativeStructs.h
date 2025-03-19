#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    struct NativeKeyValuePair {
        const char* key;
        const char* value;
    };

    struct DictionaryDom {
        NativeKeyValuePair* entries;
        int count;
    };

    struct DictionaryList {
        DictionaryDom* dictionaries;
        int count;
    };
    struct PrintResult {
        const char* info;
        bool res;
    };
    //typedef void(__cdecl* ProcessDataFunc)(const DictionaryList* data);

#ifdef __cplusplus
}
#endif