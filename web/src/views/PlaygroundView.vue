<template>
  <div class="hidden md:block">
    <NavBar :wide="true" brand-label="CHERRY PLAYGROUND" brand-to="/" />
  </div>

  <main class="max-w-[1500px] mx-auto p-4 md:p-6 pt-3 relative">
    <div class="md:hidden mb-3">
      <div class="flex items-center justify-between">
        <h1 class="text-lg font-bold tracking-tight accent">CHERRY PLAYGROUND</h1>
        <button
          class="glass p-2 rounded-md text-gray-200 hover:bg-white/10"
          @click="examplesDrawerOpen = !examplesDrawerOpen"
          aria-label="Toggle examples menu"
        >
          <svg v-if="!examplesDrawerOpen" xmlns="http://www.w3.org/2000/svg" class="w-5 h-5" viewBox="0 0 24 24" fill="currentColor"><path d="M4 6h16v2H4zm0 5h16v2H4zm0 5h16v2H4z"/></svg>
          <svg v-else xmlns="http://www.w3.org/2000/svg" class="w-5 h-5" viewBox="0 0 24 24" fill="currentColor"><path d="M18.3 5.7L12 12l6.3 6.3-1.4 1.4L10.6 13.4 4.3 19.7 2.9 18.3 9.2 12 2.9 5.7 4.3 4.3l6.3 6.3 6.3-6.3z"/></svg>
        </button>
      </div>
    </div>

    <div v-if="examplesDrawerOpen" class="fixed inset-0 bg-black/45 z-20" @click="examplesDrawerOpen = false"></div>
    <aside
      class="fixed top-[84px] left-4 bottom-4 w-[min(340px,calc(100vw-2rem))] glass rounded-2xl p-4 overflow-hidden flex flex-col min-h-0 min-w-0 z-30 transition-transform duration-300 ease-out"
      :class="examplesDrawerOpen ? 'translate-x-0' : '-translate-x-[120%]'"
    >
      <div class="mb-3 md:hidden">
        <div class="text-[10px] uppercase tracking-[0.12em] text-gray-400 mb-2">Navigate</div>
        <div class="space-y-1">
          <a href="/" class="block text-xs px-2 py-1.5 rounded-md hover:bg-white/[0.06] text-gray-300">home</a>
          <a href="/playground" class="block text-xs px-2 py-1.5 rounded-md bg-white/[0.06] text-white">playground</a>
          <a href="/docs" class="block text-xs px-2 py-1.5 rounded-md hover:bg-white/[0.06] text-gray-300">docs</a>
        </div>
      </div>

      <div class="flex items-center justify-between mb-2">
        <div class="text-sm font-semibold text-gray-300 tracking-wide uppercase">Examples</div>
        <button class="text-gray-400 hover:text-gray-200 inline-flex items-center" @click="examplesDrawerOpen = false" aria-label="Close examples">
          <svg xmlns="http://www.w3.org/2000/svg" class="w-4 h-4" viewBox="0 0 24 24" fill="currentColor"><path d="M18.3 5.7L12 12l6.3 6.3-1.4 1.4L10.6 13.4 4.3 19.7 2.9 18.3 9.2 12 2.9 5.7 4.3 4.3l6.3 6.3 6.3-6.3z"/></svg>
        </button>
      </div>

      <div class="overflow-auto pr-1 space-y-2 min-h-0">
          <button
            v-for="(example, index) in examples"
            :key="example.file"
            class="w-full text-left px-3 py-2 rounded-md border transition text-sm"
            :class="index === selectedExample ? 'border-[#ff5f7a66] bg-white/10' : 'border-white/10 hover:border-[#ff5f7a66] hover:bg-white/10'"
            @click="applyExample(index)"
          >
          <span class="text-gray-100">{{ example.title }}</span>
        </button>
      </div>
    </aside>

    <div class="grid grid-cols-12 gap-4 h-[calc(100vh-160px)] min-h-[520px]">
      <section class="glass p-4 rounded-2xl col-span-12 lg:col-span-8 flex flex-col min-h-0 min-w-0 overflow-hidden">
        <div class="flex items-center justify-between mb-2">
          <div class="text-sm font-semibold text-gray-300 tracking-wide uppercase">Code Editor</div>
          <button id="run" class="bg-[#ff5f7a] hover:bg-[#ff7a93] text-black font-semibold px-4 py-2 rounded-md transition" @click="runCode">Run Code</button>
        </div>
        <div ref="editorRef" class="grow min-h-0 min-w-0 overflow-hidden"></div>
        <div class="mt-2 hidden md:flex justify-start">
          <button class="text-sm text-gray-300 hover:text-white bg-transparent border-0 px-1 py-0.5" @click="examplesDrawerOpen = !examplesDrawerOpen">code examples</button>
        </div>
      </section>

      <section class="glass p-4 rounded-2xl col-span-12 lg:col-span-4 flex flex-col min-h-0 min-w-0">
        <div class="flex items-center justify-between mb-2">
          <div class="text-sm font-semibold text-gray-300 tracking-wide uppercase">Output</div>
        </div>
        <div ref="outputRef" style="white-space: pre-wrap" class="grow w-full bg-transparent outline-none resize-none text-gray-200 text-sm leading-6 overflow-auto font-mono"></div>
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
const examplesDrawerOpen = ref(false);
const runInProgress = ref(false);

let editor = null;
let ansi = null;
let editorResizeHandler = null;
let layoutRaf = null;
let editorResizeObserver = null;
let observerLayoutTimeout = null;
let lastObservedEditorWidth = -1;
let lastObservedEditorHeight = -1;
let currentRunId = 0;
let currentRunnerFrame = null;
let currentRunCleanup = null;
let destroyed = false;

function loadScript(src) {
  return new Promise((resolve, reject) => {
    const existing = document.querySelector(`script[src="${src}"]`);
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

function appendOutput(text, appendLineBreak = false) {
  if (!outputRef.value) return;
  const normalized = String(text ?? "").replaceAll("\r\n", "\n").replaceAll("\r", "\n");
  const lines = normalized.split("\n");
  const escapedLines = lines.map((line) => {
    if (ansi) return ansi.ansi_to_html(line);
    return line
      .replaceAll("&", "&amp;")
      .replaceAll("<", "&lt;")
      .replaceAll(">", "&gt;");
  });
  let withBreaks = escapedLines.join("<br/>");
  if (appendLineBreak) withBreaks += "<br/>";
  outputRef.value.innerHTML += withBreaks;
  outputRef.value.scrollTop = outputRef.value.scrollHeight;
}

function resolveAnsiUpCtor() {
  const direct = globalThis.AnsiUp;
  if (typeof direct === "function") return direct;
  const nested = globalThis.ansi_up && globalThis.ansi_up.AnsiUp;
  if (typeof nested === "function") return nested;
  return null;
}

function createAnsiInstance() {
  const ctor = resolveAnsiUpCtor();
  if (!ctor) return null;
  try {
    return new ctor();
  } catch {
    return null;
  }
}

function toBase64Utf8(value) {
  return btoa(unescape(encodeURIComponent(value)));
}

function removeRunnerFrame() {
  if (currentRunnerFrame && currentRunnerFrame.parentNode) {
    currentRunnerFrame.parentNode.removeChild(currentRunnerFrame);
  }
  currentRunnerFrame = null;
}

function runInIsolatedFrame(sourceCode) {
  return new Promise((resolve, reject) => {
    if (destroyed) {
      reject(new Error("playground is unloading"));
      return;
    }
    removeRunnerFrame();
    if (currentRunCleanup) {
      currentRunCleanup();
      currentRunCleanup = null;
    }

    const runId = ++currentRunId;
    const payload = toBase64Utf8(sourceCode);
    const iframe = document.createElement("iframe");
    iframe.style.display = "none";
    currentRunnerFrame = iframe;

    const onMessage = (event) => {
      if (destroyed) return;
      const data = event.data;
      if (!data || data.__cherryRunId !== runId) return;
      if (data.type === "out") appendOutput(String(data.text ?? ""), true);
      if (data.type === "done") {
        window.removeEventListener("message", onMessage);
        removeRunnerFrame();
        resolve();
      }
      if (data.type === "fail") {
        window.removeEventListener("message", onMessage);
        removeRunnerFrame();
        reject(new Error(String(data.text ?? "runtime failure")));
      }
    };

    window.addEventListener("message", onMessage);
    currentRunCleanup = () => {
      window.removeEventListener("message", onMessage);
      removeRunnerFrame();
    };

    iframe.srcdoc = `<!doctype html><html><body><script>
      (function () {
        var runId = ${runId};
        var code = decodeURIComponent(escape(atob("${payload}")));
        function post(type, text) {
          parent.postMessage({ __cherryRunId: runId, type: type, text: text }, "*");
        }
        window.Module = {
          noInitialRun: true,
          noExitRuntime: true,
          print: function (t) { post("out", t); },
          printErr: function (t) { post("out", t); },
          onAbort: function (reason) { post("fail", String(reason || "unknown runtime abort")); },
          onRuntimeInitialized: function () {
            try {
              Module.FS.writeFile("/input.chry", code);
              Module.callMain(["/input.chry"]);
              post("done", "");
            } catch (e) {
              post("fail", e && e.stack ? e.stack : String(e));
            }
          }
        };
        var s = document.createElement("script");
        s.src = "/wasm/Cherry.js";
        s.onerror = function () { post("fail", "failed to load wasm runtime"); };
        document.body.appendChild(s);
      })();
    <\/script></body></html>`;

    document.body.appendChild(iframe);
  });
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
  examplesDrawerOpen.value = false;
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
      automaticLayout: false,
      fontSize: 14,
      fontFamily: "JetBrains Mono"
    });
    if (examples.value.length && !editor.getValue()) editor.setValue(examples.value[0].code);
    editorResizeHandler = () => {
      if (!editor) return;
      if (layoutRaf != null) cancelAnimationFrame(layoutRaf);
      layoutRaf = requestAnimationFrame(() => {
        layoutRaf = null;
        if (editor) editor.layout();
      });
    };
    window.addEventListener("resize", editorResizeHandler);
    editorResizeHandler();
    if (editorRef.value && "ResizeObserver" in window) {
      editorResizeObserver = new ResizeObserver((entries) => {
        const entry = entries[0];
        if (!entry) return;
        const width = Math.round(entry.contentRect.width);
        const height = Math.round(entry.contentRect.height);
        if (width === lastObservedEditorWidth && height === lastObservedEditorHeight) return;
        lastObservedEditorWidth = width;
        lastObservedEditorHeight = height;
        if (observerLayoutTimeout != null) clearTimeout(observerLayoutTimeout);
        observerLayoutTimeout = setTimeout(() => {
          observerLayoutTimeout = null;
          editorResizeHandler();
        }, 0);
      });
      editorResizeObserver.observe(editorRef.value);
    }
  });
}

function runCode() {
  if (runInProgress.value) return;
  runInProgress.value = true;
  if (outputRef.value) outputRef.value.innerHTML = "";

  Promise.resolve().then(async () => {
    if (!editor) {
      appendOutput("Editor is not ready.");
      return;
    }
    await runInIsolatedFrame(editor.getValue());
  }).catch((e) => {
    appendOutput(e && e.stack ? e.stack : String(e));
  }).finally(() => {
    runInProgress.value = false;
  });
}

onMounted(async () => {
  await loadExamples();

  window.addEventListener("error", onWindowError);
  window.addEventListener("unhandledrejection", onWindowRejection);

  await loadScript("/ansi_up.js?v=2");
  ansi = createAnsiInstance();
  await loadScript("/cherry_monaco.js");
  await loadScript("https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.51.0/min/vs/loader.min.js");
  initEditor();
});

function onWindowError(ev) {
  appendOutput(ev.message);
}

function onWindowRejection(ev) {
  appendOutput(String(ev.reason));
}

onBeforeUnmount(() => {
  destroyed = true;
  window.removeEventListener("error", onWindowError);
  window.removeEventListener("unhandledrejection", onWindowRejection);
  if (editor) {
    editor.dispose();
    editor = null;
  }
  if (editorResizeHandler) {
    window.removeEventListener("resize", editorResizeHandler);
    editorResizeHandler = null;
  }
  if (editorResizeObserver) {
    editorResizeObserver.disconnect();
    editorResizeObserver = null;
  }
  if (layoutRaf != null) {
    cancelAnimationFrame(layoutRaf);
    layoutRaf = null;
  }
  if (observerLayoutTimeout != null) {
    clearTimeout(observerLayoutTimeout);
    observerLayoutTimeout = null;
  }
  if (currentRunCleanup) {
    currentRunCleanup();
    currentRunCleanup = null;
  } else {
    removeRunnerFrame();
  }
});
</script>
