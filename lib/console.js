// [WIP]

function serializeConsoleLog(...args) {
  let result = [];

  // Format if first argument is a string
  if (typeof args[0] === "string") {
    let formattedMessage = args.shift().replace(/%[csdifoO]/g, (match) => {
      // Keep raw token if no substitution args left
      if (args.length === 0) return match;

      switch (match) {
        // Formatting (omitted)
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

      // Keep raw token if not replaced
      return match;
    });

    if (formattedMessage.length > 0) {
      result.push(formattedMessage);
    }
  }

  // Serialize remaining arguments
  let formattedArgs = args.map((arg) =>
    typeof arg === "string" ? arg : JSON.stringify(arg)
  );
  result.push(...formattedArgs);

  return result.join(" ");
}

function log(...args) {
  const str = serializeConsoleLog(...args);

  __pure_stdout.write(str);
}

global.console = {
  log,
  info: log,
  warn: log,
  error: log,
};
