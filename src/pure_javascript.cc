
/**
 * @file pure_javascript.cc
 * @author xiaoxiaojx (784487301@qq.com)
 * @brief 由 ./tools/js2c.js 脚本自动生成, 请勿手动修改, 原理见 https://github.com/xiaoxiaojx/blog/issues/13
 * @version 0.1
 * @date 2022-10-22
 *
 * @copyright Copyright (c) 2022
 *
 */                                                                 
                                                                  
#include <map>
#include "pure_union_bytes.h"
#include "pure_native_module.h"

namespace pure
{
    namespace native_module
    {
        
    static const uint16_t console_raw[] = {
        47,42,42,10,32,42,32,64,102,105,108,101,32,99,111,110,115,111,108,101,46,106,115,10,32,42,32,64,97,117,116,104,111,114,32,120,105,97,111,120,105,97,111,106,120,40,55,56,52,52,56,55,51,48,49,64,113,113,46,99,111,109,41,10,32,42,32,64,98,114,105,101,102,32,74,97,118,97,83,99,114,105,112,116,32,99,111,110,115,111,108,101,32,31616,21333,30340,23454,29616,44,32,67,43,43,32,31867,20284,30340,23454,29616,32,83,80,114,105,110,116,70,32,35265,32,115,114,99,47,100,101,98,117,103,95,117,116,105,108,115,45,105,110,108,46,104,10,32,42,32,64,118,101,114,115,105,111,110,32,48,46,49,10,32,42,32,64,100,97,116,101,32,50,48,50,50,45,49,48,45,50,50,10,32,42,10,32,42,32,64,99,111,112,121,114,105,103,104,116,32,67,111,112,121,114,105,103,104,116,32,40,99,41,32,50,48,50,50,10,32,42,10,32,42,47,10,10,102,117,110,99,116,105,111,110,32,115,101,114,105,97,108,105,122,101,67,111,110,115,111,108,101,76,111,103,40,97,114,103,115,41,32,123,10,32,32,105,102,32,40,97,114,103,115,46,108,101,110,103,116,104,32,61,61,61,32,49,41,32,123,10,32,32,32,32,114,101,116,117,114,110,32,97,114,103,115,46,106,111,105,110,40,39,39,41,59,10,32,32,125,10,10,32,32,108,101,116,32,114,101,115,117,108,116,32,61,32,91,93,59,10,10,32,32,105,102,32,40,116,121,112,101,111,102,32,97,114,103,115,91,48,93,32,61,61,61,32,39,115,116,114,105,110,103,39,41,32,123,10,32,32,32,32,108,101,116,32,102,111,114,109,97,116,116,101,100,77,101,115,115,97,103,101,32,61,32,97,114,103,115,46,115,104,105,102,116,40,41,46,114,101,112,108,97,99,101,40,47,37,91,99,115,100,105,102,111,79,93,47,103,44,32,40,109,97,116,99,104,41,32,61,62,32,123,10,32,32,32,32,32,32,105,102,32,40,97,114,103,115,46,108,101,110,103,116,104,32,61,61,61,32,48,41,32,114,101,116,117,114,110,32,109,97,116,99,104,59,10,10,32,32,32,32,32,32,115,119,105,116,99,104,32,40,109,97,116,99,104,41,32,123,10,32,32,32,32,32,32,32,32,99,97,115,101,32,39,37,99,39,58,10,32,32,32,32,32,32,32,32,32,32,97,114,103,115,46,115,104,105,102,116,40,41,59,10,32,32,32,32,32,32,32,32,32,32,114,101,116,117,114,110,32,39,39,59,10,10,32,32,32,32,32,32,32,32,47,47,32,83,116,114,105,110,103,10,32,32,32,32,32,32,32,32,99,97,115,101,32,39,37,115,39,58,10,32,32,32,32,32,32,32,32,32,32,114,101,116,117,114,110,32,83,116,114,105,110,103,40,97,114,103,115,46,115,104,105,102,116,40,41,41,59,10,10,32,32,32,32,32,32,32,32,47,47,32,73,110,116,101,103,101,114,10,32,32,32,32,32,32,32,32,99,97,115,101,32,39,37,100,39,58,10,32,32,32,32,32,32,32,32,99,97,115,101,32,39,37,105,39,58,10,32,32,32,32,32,32,32,32,32,32,114,101,116,117,114,110,32,112,97,114,115,101,73,110,116,40,97,114,103,115,46,115,104,105,102,116,40,41,41,59,10,10,32,32,32,32,32,32,32,32,47,47,32,70,108,111,97,116,10,32,32,32,32,32,32,32,32,99,97,115,101,32,39,37,102,39,58,10,32,32,32,32,32,32,32,32,32,32,114,101,116,117,114,110,32,112,97,114,115,101,70,108,111,97,116,40,97,114,103,115,46,115,104,105,102,116,40,41,41,59,10,10,32,32,32,32,32,32,32,32,47,47,32,79,98,106,101,99,116,10,32,32,32,32,32,32,32,32,99,97,115,101,32,39,37,111,39,58,10,32,32,32,32,32,32,32,32,99,97,115,101,32,39,37,79,39,58,10,32,32,32,32,32,32,32,32,32,32,114,101,116,117,114,110,32,74,83,79,78,46,115,116,114,105,110,103,105,102,121,40,97,114,103,115,46,115,104,105,102,116,40,41,41,59,10,32,32,32,32,32,32,125,10,10,32,32,32,32,32,32,114,101,116,117,114,110,32,109,97,116,99,104,59,10,32,32,32,32,125,41,59,10,10,32,32,32,32,105,102,32,40,102,111,114,109,97,116,116,101,100,77,101,115,115,97,103,101,46,108,101,110,103,116,104,32,62,32,48,41,32,123,10,32,32,32,32,32,32,114,101,115,117,108,116,46,112,117,115,104,40,102,111,114,109,97,116,116,101,100,77,101,115,115,97,103,101,41,59,10,32,32,32,32,125,10,32,32,125,10,10,32,32,108,101,116,32,102,111,114,109,97,116,116,101,100,65,114,103,115,32,61,32,97,114,103,115,46,109,97,112,40,40,97,114,103,41,32,61,62,10,32,32,32,32,116,121,112,101,111,102,32,97,114,103,32,61,61,61,32,39,115,116,114,105,110,103,39,32,63,32,97,114,103,32,58,32,74,83,79,78,46,115,116,114,105,110,103,105,102,121,40,97,114,103,41,10,32,32,41,59,10,32,32,114,101,115,117,108,116,46,112,117,115,104,40,46,46,46,102,111,114,109,97,116,116,101,100,65,114,103,115,41,59,10,10,32,32,114,101,116,117,114,110,32,114,101,115,117,108,116,46,106,111,105,110,40,39,32,39,41,59,10,125,10,10,102,117,110,99,116,105,111,110,32,108,111,103,40,46,46,46,97,114,103,115,41,32,123,10,32,32,99,111,110,115,116,32,115,116,114,32,61,32,115,101,114,105,97,108,105,122,101,67,111,110,115,111,108,101,76,111,103,40,97,114,103,115,41,59,10,10,32,32,95,95,112,117,114,101,95,115,116,100,111,117,116,46,119,114,105,116,101,40,115,116,114,41,59,10,125,10,10,103,108,111,98,97,108,46,99,111,110,115,111,108,101,32,61,32,123,10,32,32,108,111,103,44,10,32,32,105,110,102,111,58,32,108,111,103,44,10,32,32,119,97,114,110,58,32,108,111,103,44,10,32,32,101,114,114,111,114,58,32,108,111,103,44,10,125,59,10
    };


        void NativeModuleLoader::LoadJavaScriptSource()
        {
          
source_.emplace("pure:console", UnionBytes{console_raw, 1397});
        }
    }
}
