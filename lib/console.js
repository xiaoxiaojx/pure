/**
 * @file console.js
 * @author xiaoxiaojx(784487301@qq.com)
 * @brief JavaScript console 简单的实现
 * @version 0.1
 * @date 2022-10-22
 *
 * @copyright Copyright (c) 2022
 *
 */

function serializeConsoleLog(args) {
  if (args.length === 1) {
    return args.join("");
  }

  let result = [];

  if (typeof args[0] === "string") {
    let formattedMessage = args.shift().replace(/%[csdifoO]/g, (match) => {
      if (args.length === 0) return match;

      switch (match) {
        case "%c":
          args.shift();
          return "";

        // String
        case "%s":
          return String(args.shift());

        // Integer
        case "%d":
        case "%i":
          return parseInt(args.shift());

        // Float
        case "%f":
          return parseFloat(args.shift());

        // Object
        case "%o":
        case "%O":
          return JSON.stringify(args.shift());
      }

      return match;
    });

    if (formattedMessage.length > 0) {
      result.push(formattedMessage);
    }
  }

  let formattedArgs = args.map((arg) =>
    typeof arg === "string" ? arg : JSON.stringify(arg)
  );
  result.push(...formattedArgs);

  return result.join(" ");
}

function log(...args) {
  const str = serializeConsoleLog(args);

  __pure_stdout.write(str);
}

global.console = {
  log,
  info: log,
  warn: log,
  error: log,
};
