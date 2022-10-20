// [WIP]


const fs = require("fs");
const path = require("path");

const string_to_char_code = ([...string]) => {
  const array = string.map((char) => {
    return char.charCodeAt(0);
  });
  // array of unicode
  return array;
};

const LIB_DIR = path.join(__dirname, "../lib");

const FILES = fs.readdirSync(LIB_DIR).map((file) => {
  const filePath = path.join(LIB_DIR, file);
  return {
    path: filePath.replace(LIB_DIR, ""),
    content: string_to_char_code(fs.readFileSync(filePath, "utf-8")),
    id:
      filePath.replace(LIB_DIR, "").replace(".", "").split("/").join("_") +
      "_raw",
  };
});

const MODULES = FILES.map((file) => {
  return `
    static const uint8_t ${file.id}[] = {
        ${file.content}
    }
`;
});
// static std::map<std::string, int> NativeModuleRecordMap
const MODULE_MAP = FILES.map((file) => {
  return `
    {"${file.path}, "}
    
`;
});

console.log(CODES);
