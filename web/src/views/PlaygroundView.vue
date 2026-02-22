<template>
  <NavBar :wide="true" brand-label="CHERRY PLAYGROUND" brand-to="/" />

  <main class="max-w-[1500px] mx-auto p-6 pt-3">
    <div class="grid grid-cols-12 gap-4 h-[78vh] min-h-0">
      <aside class="glass rounded-2xl p-4 col-span-12 lg:col-span-3 overflow-hidden flex flex-col min-h-0 min-w-0">
        <div class="text-sm font-semibold text-gray-300 mb-2 tracking-wide uppercase">Examples</div>
        <div class="overflow-auto pr-1 space-y-2">
          <button
            v-for="(example, index) in examples"
            :key="example.file"
            class="w-full text-left px-3 py-2 rounded-md border transition text-sm"
            :class="index === selectedExample ? 'border-[#ff7a9e55] bg-white/10' : 'border-white/10 hover:border-[#ff7a9e55] hover:bg-white/10'"
            @click="applyExample(index)"
          >
            <span class="text-gray-100">{{ example.title }}</span>
          </button>
        </div>
      </aside>

      <section class="glass p-4 rounded-2xl col-span-12 lg:col-span-6 flex flex-col min-h-0 min-w-0 overflow-hidden">
        <div class="flex items-center justify-between mb-2">
          <div class="text-sm font-semibold text-gray-300 tracking-wide uppercase">Code Editor</div>
          <button id="run" class="bg-[#ff7a9e] hover:bg-[#ff93b0] text-black font-semibold px-4 py-2 rounded-md transition" @click="runCode">Run Code</button>
        </div>
        <div ref="editorRef" class="grow min-h-0 min-w-0 overflow-hidden"></div>
      </section>

      <section class="glass p-4 rounded-2xl col-span-12 lg:col-span-3 flex flex-col min-h-0 min-w-0">
        <div class="flex items-center justify-between mb-2">
          <div class="text-sm font-semibold text-gray-300 tracking-wide uppercase">Output</div>
          <span class="text-xs" :class="runtimeReady ? 'text-emerald-300' : 'text-gray-400'">{{ runtimeReady ? 'wasm ready' : 'loading wasm...' }}</span>
        </div>
        <div ref="outputRef" style="white-space: pre-wrap" class="grow w-full bg-transparent outline-none resize-none text-gray-200 text-sm leading-6 overflow-auto"></div>
      </section>
    </div>
  </main>
</template>

<script setup>
import { onMounted, onBeforeUnmount, ref } from "vue";
import NavBar from "../components/NavBar.vue";

const EXAMPLE_MANIFEST = [
  { title: "Class definition", file: "class_definition.chry" },
  { title: "Turing Machine", file: "turing_machine.chry" },
  { title: "Generics", file: "generics.chry" },
  { title: "Closure madness", file: "closure_madness.chry" },
  { title: "Custom exceptions", file: "custom_exceptions.chry" },
  { title: "Operator overloading", file: "operator_overloading.chry" }
];

const editorRef = ref(null);
const outputRef = ref(null);
const examples = ref([]);
const selectedExample = ref(0);
const runtimeReady = ref(false);

let editor = null;
let runtimeModule = null;
let ansi = null;

function loadScript(src) {
  return new Promise((resolve, reject) => {
    const existing = document.querySelector(`script[src=\"${src}\"]`);
    if (existing) {
      if (existing.dataset.loaded === "1") { resolve(); return; }
      existing.addEventListener("load", () => resolve(), { once: true });
      existing.addEventListener("error", () => reject(new Error(`Failed to load ${src}`)), { once: true });
      return;
    }
    const s = document.createElement("script");
    s.src = src;
    s.async = true;
    s.addEventListener("load", () => { s.dataset.loaded = "1"; resolve(); }, { once: true });
    s.addEventListener("error", () => reject(new Error(`Failed to load ${src}`)), { once: true });
    document.head.appendChild(s);
  });
}

function appendOutput(text) {
  if (!outputRef.value) return;
  const body = (text || "").replaceAll("\n", "<br/>");
  const html = ansi ? ansi.ansi_to_html(body) : body;
  outputRef.value.innerHTML += `${html}\n`;
  outputRef.value.scrollTop = outputRef.value.scrollHeight;
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
      const response = await fetch(`/playground/examples/${item.file}`);
      if (!response.ok) throw new Error(`HTTP ${response.status}`);
      loaded.push({ ...item, code: await response.text() });
    } catch (error) {
      loaded.push({ ...item, code: `// Failed to load ${item.file}\n// ${String(error)}` });
    }
  }
  examples.value = loaded;
}

function applyExample(index) {
  selectedExample.value = index;
  if (editor && examples.value[index]) editor.setValue(examples.value[index].code);
}

function initEditor() {
  globalThis.require.config({ paths: { vs: "https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.51.0/min/vs" } });
  globalThis.require(["vs/editor/editor.main"], () => {
    globalThis.defineCherryLanguage(globalThis.monaco);
    editor = globalThis.monaco.editor.create(editorRef.value, {
      value: examples.value[0]?.code || "",
      language: "cherry",
      theme: "cherryDark",
      minimap: { enabled: false },
      fontSize: 14,
      fontFamily: "JetBrains Mono"
    });
  });
}

function runCode() {
  if (outputRef.value) outputRef.value.innerHTML = "";
  if (!runtimeReady.value) {
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

onMounted(async () => {
  await loadExamples();

  globalThis.Module = globalThis.Module || {};
  Object.assign(globalThis.Module, {
    print: (text) => appendOutput(text),
    printErr: (text) => appendOutput(text),
    onAbort: (reason) => appendOutput(String(reason ?? "unknown runtime abort")),
    onExit: (code) => {
      if (code !== 0) appendOutput(`program exited with code ${code}`);
    },
    onRuntimeInitialized: () => {
      runtimeModule = globalThis.Module || runtimeModule;
      runtimeReady.value = true;
    }
  });

  window.addEventListener("error", onWindowError);
  window.addEventListener("unhandledrejection", onWindowRejection);

  await loadScript("/ansi_up.js");
  ansi = new globalThis.AnsiUp();
  await loadScript("/cherry_monaco.js");
  await loadScript("https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.51.0/min/vs/loader.min.js");
  initEditor();
  await loadScript("/wasm/Cherry.js");
});

function onWindowError(ev) { appendOutput(ev.message); }
function onWindowRejection(ev) { appendOutput(String(ev.reason)); }

onBeforeUnmount(() => {
  window.removeEventListener("error", onWindowError);
  window.removeEventListener("unhandledrejection", onWindowRejection);
  if (editor) {
    editor.dispose();
    editor = null;
  }
});
</script>
