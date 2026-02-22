const ansi = new AnsiUp();
let editor = null;
let runtimeReady = false;
let examples = [];
const EXAMPLES_BASE_PATH = "./examples";
let runtimeModule = null;

const EXAMPLE_MANIFEST = [
  { title: "Class definition", file: "class_definition.chry" },
  { title: "Turing Machine", file: "turing_machine.chry" },
  { title: "Generics", file: "generics.chry" },
  { title: "Closure madness", file: "closure_madness.chry" },
  { title: "Custom exceptions", file: "custom_exceptions.chry" },
  { title: "Operator overloading", file: "operator_overloading.chry" }
];

globalThis.Module = globalThis.Module || {};
Object.assign(globalThis.Module, {
  print: (text) => appendOutput(text),
  printErr: (text) => {
    appendOutput(text);
  },
  onAbort: (reason) => {
    appendOutput(String(reason ?? "unknown runtime abort"));
  },
  onExit: (code) => {
    if (code !== 0) appendOutput(`program exited with code ${code}`);
  },
  onRuntimeInitialized: () => {
    runtimeModule = globalThis.Module || runtimeModule;
    runtimeReady = true;
    const badge = document.getElementById("runtimeBadge");
    badge.textContent = "wasm ready";
    badge.classList.remove("text-gray-400");
    badge.classList.add("text-emerald-300");
  }
});

function appendOutput(text) {
  const out = document.getElementById("output");
  out.innerHTML += ansi.ansi_to_html((text || "").replaceAll("\n", "<br/>")) + "\n";
  out.scrollTop = out.scrollHeight;
}

function resolveRuntimeApis() {
  const mod = runtimeModule || globalThis.Module || null;
  const fs = (mod && mod.FS) || globalThis.FS || null;
  const callMain = (mod && mod.callMain) || globalThis.callMain || null;
  return { fs, callMain };
}

async function loadExamples() {
  const loaded = [];
  for (const item of EXAMPLE_MANIFEST) {
    try {
      const response = await fetch(`${EXAMPLES_BASE_PATH}/${item.file}`);
      if (!response.ok) throw new Error(`HTTP ${response.status}`);
      loaded.push({ ...item, code: await response.text() });
    } catch (error) {
      loaded.push({ ...item, code: `// Failed to load ${item.file}\n// ${String(error)}` });
    }
  }
  examples = loaded;
}

function buildExamples() {
  const list = document.getElementById("exampleList");
  list.innerHTML = "";
  examples.forEach((example, i) => {
    const btn = document.createElement("button");
    btn.className = "w-full text-left px-3 py-2 rounded-md border border-white/10 hover:border-[#ff7a9e55] hover:bg-white/10 transition text-sm";
    btn.innerHTML = `<span class="text-gray-100">${example.title}</span>`;
    btn.addEventListener("click", () => {
      if (editor) editor.setValue(example.code);
    });
    list.appendChild(btn);
    if (i === 0 && editor) editor.setValue(example.code);
  });
}

function initEditor() {
  require.config({ paths: { vs: "https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.51.0/min/vs" } });
  require(["vs/editor/editor.main"], () => {
    defineCherryLanguage(monaco);
    editor = monaco.editor.create(document.getElementById("editor"), {
      value: examples[0]?.code || "",
      language: "cherry",
      theme: "cherryDark",
      minimap: { enabled: false },
      fontSize: 14,
      fontFamily: "JetBrains Mono"
    });
    buildExamples();
  });
}

function runCode() {
  const out = document.getElementById("output");
  out.innerHTML = "";
  if (!runtimeReady) {
    appendOutput("Runtime is still loading...");
    return;
  }
  if (!editor) {
    appendOutput("Editor is not ready.");
    return;
  }

  try {
    const { fs, callMain } = resolveRuntimeApis();
    if (!fs || !callMain) {
      appendOutput("Runtime APIs are unavailable (FS/callMain). Rebuild WASM with exported runtime methods.");
      return;
    }
    fs.writeFile("/input.chry", editor.getValue());
    callMain(["/input.chry"]);
  } catch (e) {
    appendOutput(e && e.stack ? e.stack : String(e));
  }
}

document.addEventListener("DOMContentLoaded", async () => {
  window.addEventListener("error", (ev) => {
    appendOutput(ev.message);
  });
  window.addEventListener("unhandledrejection", (ev) => {
    appendOutput(String(ev.reason));
  });

  await loadExamples();
  initEditor();
  document.getElementById("run").addEventListener("click", runCode);
});
