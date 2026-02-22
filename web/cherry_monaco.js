(function (global) {
  function defineCherryLanguage(monaco) {
    monaco.languages.register({ id: "cherry" });

    monaco.languages.setMonarchTokensProvider("cherry", {
      keywords: [
        "let", "infer", "if", "unless", "else", "for", "while", "do", "repeat", "until", "times",
        "try", "catch", "finally", "throw", "return",
        "class", "type", "extends", "when", "default", "new", "this", "display",
        "public", "private", "protected", "readonly", "static", "sealed",
        "operator", "get", "set",
        "and", "or", "xor", "not", "mod", "div", "is",
        "true", "false", "null"
      ],
      typeKeywords: ["int", "real", "string", "boolean", "function", "type", "any"],
      operators: [
        "=", "==", "!=", ">", "<", ">=", "<=", "=>", "...",
        "+", "-", "*", "/", "%", "^", "|", "&", "!|", "!||",
        "+=", "-=", "*=", "/=", "^=", "%=", "mod=", "div=",
        ";", ",", ":", "."
      ],
      symbols: /[=><!~?:&|+\-*\/\^%;,.]+/,
      tokenizer: {
        root: [
          [/\b[a-zA-Z_][\w]*\b/, {
            cases: {
              "@keywords": "keyword",
              "@typeKeywords": "keyword.type",
              "@default": "identifier"
            }
          }],
          [/\d*\.\d+([eE][\-+]?\d+)?f?/, "number.float"],
          [/\d+/, "number"],
          [/"([^"\\]|\\.)*$/, "string.invalid"],
          [/"/, { token: "string.quote", bracket: "@open", next: "@string" }],
          { include: "@whitespace" },
          [/@symbols/, {
            cases: {
              "@operators": "operator",
              "@default": ""
            }
          }],
          [/[{}()\[\]]/, "@brackets"],
          [/[;]/, "delimiter"]
        ],
        whitespace: [
          [/[ \t\r\n]+/, "white"],
          [/\/\/.*$/, "comment"],
          [/\/\*/, "comment", "@comment"]
        ],
        comment: [
          [/[^\/*]+/, "comment"],
          [/\/\*/, "comment", "@push"],
          [/\*\//, "comment", "@pop"],
          [/[\/*]/, "comment"]
        ],
        string: [
          [/[^\\"]+/, "string"],
          [/\\./, "string.escape"],
          [/"/, { token: "string.quote", bracket: "@close", next: "@pop" }]
        ]
      }
    });

    monaco.editor.defineTheme("cherryDark", {
      base: "vs-dark",
      inherit: true,
      rules: [
        { token: "keyword", foreground: "ff7a9e", fontStyle: "bold" },
        { token: "keyword.type", foreground: "ffa2c9" },
        { token: "number", foreground: "ff93b0" },
        { token: "string", foreground: "ffcfe0" },
        { token: "operator", foreground: "ff78a8" },
        { token: "comment", foreground: "777777", fontStyle: "italic" },
        { token: "identifier", foreground: "e5e7eb" }
      ],
      colors: {
        "editor.background": "#1a1a1a",
        "editorCursor.foreground": "#ff7a9e",
        "editorLineNumber.foreground": "#555",
        "editorLineNumber.activeForeground": "#ff7a9e",
        "editor.selectionBackground": "#ff7a9e22",
        "editor.inactiveSelectionBackground": "#ff7a9e11"
      }
    });
  }

  global.defineCherryLanguage = defineCherryLanguage;
})(window);
