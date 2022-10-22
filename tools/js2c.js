/**
 * @file js2c.js
 * @author xiaoxiaojx (784487301@qq.com)
 * @brief 把 lib 库下面的 js 一起写入到二进制文件中, 实现参考于 Node.js js2c.py, 原理见 https://github.com/xiaoxiaojx/blog/issues/13
 * @version 0.1
 * @date 2022-10-22
 *
 * @copyright Copyright (c) 2022
 *
 */

const fs = require("fs");
const path = require("path");

function string2CharCode8([...string]) {
  // array of unicode
  return string.map((char) => {
    return char.charCodeAt(0);
  });
}

function string2CharCode16(str) {
  var buf = new ArrayBuffer(str.length * 2);
  var bufView = new Uint16Array(buf);
  for (var i = 0, strLen = str.length; i < strLen; i++) {
    bufView[i] = str.charCodeAt(i);
  }
  return bufView;
}

const LIB_DIR = path.join(__dirname, "../lib");
const OUTPUT_FILE = path.join(__dirname, "../src/pure_javascript.cc");

const FILES = fs.readdirSync(LIB_DIR).map((file) => {
  const filePath = path.join(LIB_DIR, file);
  const fileContent = fs.readFileSync(filePath, "utf-8");
  let relativePath = filePath.replace(LIB_DIR, "").replace(".js", "");

  const uint8 = [...fileContent].every((str) => str.charCodeAt(0) <= 127);

  relativePath = relativePath.startsWith("/")
    ? relativePath.slice(1)
    : relativePath;

  return {
    uint8,
    path: relativePath,
    content: uint8
      ? string2CharCode8(fileContent)
      : string2CharCode16(fileContent),
    id: relativePath.split("/").join("_") + "_raw",
  };
});

const MODULES = FILES.map((file) => {
  return `
    static const ${file.uint8 ? "uint8_t" : "uint16_t"} ${file.id}[] = {
        ${file.content}
    };
`;
});

const MODULE_MAP = FILES.map((file) => {
  return `
source_.emplace("pure:${file.path}", UnionBytes{${file.id}, ${file.content.length}});`;
});

const RESULT = `
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
        ${MODULES.join("\n")}

        void NativeModuleLoader::LoadJavaScriptSource()
        {
          ${MODULE_MAP.join("\n")}
        }
    }
}
`;

fs.writeFileSync(OUTPUT_FILE, RESULT);
