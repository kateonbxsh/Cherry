<template>
  <div class="hidden md:block">
    <NavBar :wide="true" brand-label="CHERRY DOCS" brand-to="/" />
  </div>

  <main class="max-w-[1500px] mx-auto p-4 md:p-6 pt-3 h-[calc(100vh-110px)] relative">
    <div class="md:hidden mb-3">
      <div class="flex items-center justify-between">
        <h1 class="text-lg font-bold tracking-tight accent">CHERRY DOCS</h1>
        <button
          class="glass p-2 rounded-md text-gray-200 hover:bg-white/10"
          @click="sidebarOpen = !sidebarOpen"
          aria-label="Toggle docs menu"
        >
          <svg v-if="!sidebarOpen" xmlns="http://www.w3.org/2000/svg" class="w-5 h-5" viewBox="0 0 24 24" fill="currentColor"><path d="M4 6h16v2H4zm0 5h16v2H4zm0 5h16v2H4z"/></svg>
          <svg v-else xmlns="http://www.w3.org/2000/svg" class="w-5 h-5" viewBox="0 0 24 24" fill="currentColor"><path d="M18.3 5.7L12 12l6.3 6.3-1.4 1.4L10.6 13.4 4.3 19.7 2.9 18.3 9.2 12 2.9 5.7 4.3 4.3l6.3 6.3 6.3-6.3z"/></svg>
        </button>
      </div>
      <div class="mt-2 text-xs text-gray-400">
        <span>{{ activeSectionTitle || "Docs" }}</span>
        <span class="mx-1 text-gray-500">/</span>
        <span class="text-gray-300">{{ activePageTitle || "Page" }}</span>
      </div>
    </div>

    <div v-if="sidebarOpen" class="fixed inset-0 bg-black/45 z-20" @click="sidebarOpen = false"></div>
    <aside
      ref="sidebarRefMobile"
      class="fixed top-[84px] left-4 bottom-4 w-[min(340px,calc(100vw-2rem))] glass rounded-2xl p-4 overflow-auto z-30 transition-transform duration-300 ease-out"
      :class="sidebarOpen ? 'translate-x-0' : '-translate-x-[120%]'"
    ></aside>

    <div class="hidden md:grid grid-cols-12 gap-4 h-full">
      <aside ref="sidebarRefDesktop" class="glass rounded-2xl p-4 col-span-3 overflow-auto"></aside>
      <section ref="contentRef" class="rounded-2xl p-4 md:p-6 col-span-9 h-full overflow-auto doc-content"></section>
    </div>
    <section ref="contentRefMobile" class="md:hidden rounded-2xl p-4 h-[calc(100%-68px)] overflow-auto doc-content"></section>
  </main>
</template>

<script setup>
import { onMounted, onBeforeUnmount, ref, watch } from "vue";
import { useRoute, useRouter } from "vue-router";
import { marked } from "marked";
import NavBar from "../components/NavBar.vue";

const BASE_URL = (import.meta.env.BASE_URL || "/").replace(/\/+$/, "") + "/";
const withBase = (path) => BASE_URL + String(path).replace(/^\/+/, "");

const route = useRoute();
const router = useRouter();
const sidebarRefDesktop = ref(null);
const sidebarRefMobile = ref(null);
const contentRef = ref(null);
const contentRefMobile = ref(null);
const sidebarOpen = ref(false);
const activeSectionTitle = ref("");
const activePageTitle = ref("");

let docsData = null;
let activeSectionId = null;
let activePageId = null;
let activePageHeadings = [];
const pageHeadingsById = new Map();
const openSections = new Set();
let pageRenderToken = 0;
const scrollPrefix = "cherry.docs.scroll.";
let docsAnsi = null;
let docsAnsiLoadPromise = null;
let docsRunCounter = 0;
const docsRunnerFrames = new Map();
let docsDestroyed = false;

function isMobileViewport() {
  return window.matchMedia("(max-width: 767px)").matches;
}

function getActiveContentElement() {
  if (isMobileViewport()) return contentRefMobile.value || contentRef.value;
  return contentRef.value || contentRefMobile.value;
}

function escapeHtml(value) {
  return value.replaceAll("&", "&amp;").replaceAll("<", "&lt;").replaceAll(">", "&gt;");
}

function loadScript(src) {
  return new Promise((resolve, reject) => {
    const existing = document.querySelector(`script[src="${src}"]`);
    if (existing) {
      if (existing.dataset.loaded === "1") {
        resolve();
        return;
      }
      existing.addEventListener("load", () => resolve(), { once: true });
      existing.addEventListener("error", () => reject(new Error(`Failed to load ${src}`)), { once: true });
      return;
    }

    const script = document.createElement("script");
    script.src = src;
    script.async = true;
    script.addEventListener("load", () => {
      script.dataset.loaded = "1";
      resolve();
    }, { once: true });
    script.addEventListener("error", () => reject(new Error(`Failed to load ${src}`)), { once: true });
    document.head.appendChild(script);
  });
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

async function ensureDocsAnsi() {
  if (docsAnsi) return docsAnsi;
  if (!docsAnsiLoadPromise) {
    docsAnsiLoadPromise = loadScript(withBase("ansi_up.js?v=2"))
      .catch(() => null)
      .then(() => {
        docsAnsi = createAnsiInstance();
        return docsAnsi;
      });
  }
  return docsAnsiLoadPromise;
}

function toBase64Utf8(value) {
  return btoa(unescape(encodeURIComponent(value)));
}

function outputChunkToHtml(text, ansi = null) {
  const normalized = String(text ?? "").replaceAll("\r\n", "\n").replaceAll("\r", "\n");
  const lines = normalized.split("\n");
  const parser = ansi || docsAnsi;
  if (parser) {
    return lines.map((line) => parser.ansi_to_html(line)).join("<br/>");
  }
  return lines.map((line) => escapeHtml(line)).join("<br/>");
}

function cleanupDocRunner(runId) {
  const frame = docsRunnerFrames.get(runId);
  if (frame && frame.parentNode) frame.parentNode.removeChild(frame);
  docsRunnerFrames.delete(runId);
}

function runDocSnippet(sourceCode, onOutput) {
  return new Promise((resolve, reject) => {
    if (docsDestroyed) {
      reject(new Error("docs view is unloading"));
      return;
    }

    const runId = ++docsRunCounter;
    const payload = toBase64Utf8(sourceCode);
    const frame = document.createElement("iframe");
    frame.style.display = "none";
    docsRunnerFrames.set(runId, frame);

    const onMessage = (event) => {
      const data = event.data;
      if (!data || data.__cherryDocRunId !== runId) return;
      if (data.type === "out") {
        onOutput(String(data.text ?? ""));
        return;
      }
      window.removeEventListener("message", onMessage);
      cleanupDocRunner(runId);
      if (data.type === "done") {
        resolve();
      } else {
        reject(new Error(String(data.text ?? "runtime failure")));
      }
    };

    window.addEventListener("message", onMessage);

    const wasmScriptUrl = withBase("wasm/Cherry.js");
    frame.srcdoc = `<!doctype html><html><body><script>
      (function () {
        var runId = ${runId};
        var code = decodeURIComponent(escape(atob("${payload}")));
        function post(type, text) {
          parent.postMessage({ __cherryDocRunId: runId, type: type, text: text }, "*");
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
        s.src = "${wasmScriptUrl}";
        s.onerror = function () { post("fail", "failed to load wasm runtime"); };
        document.body.appendChild(s);
      })();
    <\/script></body></html>`;

    document.body.appendChild(frame);
  });
}

function markdownToHtml(raw) {
  return marked.parse(raw, { gfm: true, breaks: false });
}

function highlightCherryCode(source) {
  const tokenRegex = /\/\/.*|\/\*[\s\S]*?\*\/|"([^"\\]|\\.)*"|'([^'\\]|\\.)*'|\b(typeof|until|infer|let|repeat|times|class|type|function|return|if|else|unless|while|do|for|try|catch|finally|throw|new|this|extends|when|default|display|operator|get|set|public|and|or|xor|private|protected|static|readonly|is|any|int|real|string|boolean|void)\b|\b(true|false|null)\b|\b([0-9]+(?:\.[0-9]+)?)\b/g;
  let result = "";
  let last = 0;
  let match = tokenRegex.exec(source);
  while (match) {
    if (match.index > last) result += escapeHtml(source.slice(last, match.index));
    const token = match[0];
    const escaped = escapeHtml(token);
    if (token.startsWith("//") || token.startsWith("/*")) result += `<span class=\"text-gray-500\">${escaped}</span>`;
    else if (token.startsWith("\"") || token.startsWith("'")) result += `<span class=\"text-emerald-300\">${escaped}</span>`;
    else if (match[3]) result += `<span class=\"text-pink-300 font-semibold\">${escaped}</span>`;
    else if (match[4]) result += `<span class=\"text-sky-300\">${escaped}</span>`;
    else if (match[5]) result += `<span class=\"text-amber-300\">${escaped}</span>`;
    else result += escaped;
    last = tokenRegex.lastIndex;
    match = tokenRegex.exec(source);
  }
  if (last < source.length) result += escapeHtml(source.slice(last));
  return result;
}

function ensureDocsEnhancementStyles() {
  let style = document.getElementById("docsEnhancementStyles");
  if (!style) {
    style = document.createElement("style");
    style.id = "docsEnhancementStyles";
    document.head.appendChild(style);
  }
  style.textContent = `
    .doc-content pre { background: rgba(0,0,0,.45); border: 1px solid rgba(255,255,255,.15); border-radius: .6rem; padding: .9rem; overflow: auto; margin: .9rem 0; }
    .doc-content code { color: #ffd2df; background: rgba(255,255,255,.08); border-radius: .35rem; padding: 0 .35rem; font-size: .84rem; }
    .doc-content pre code { color: inherit; background: transparent; border-radius: 0; padding: 0; }
    .doc-code-pre { position: relative; }
    .doc-code-toolbar { position: absolute; top: .45rem; right: .5rem; display: inline-flex; gap: .35rem; z-index: 2; opacity: 0; pointer-events: none; transform: translateY(-2px); transition: opacity .18s ease, transform .18s ease; }
    .doc-code-pre:hover .doc-code-toolbar, .doc-code-pre:focus-within .doc-code-toolbar { opacity: 1; pointer-events: auto; transform: translateY(0); }
    .doc-code-tool-btn { width: 1.75rem; height: 1.75rem; border-radius: .45rem; display: inline-flex; align-items: center; justify-content: center; border: 1px solid rgba(255,255,255,.12); background: rgba(255,255,255,.06); color: #f3f4f6; transition: background .2s ease, border-color .2s ease, transform .2s ease; }
    .doc-code-tool-btn:hover { background: rgba(255, 95, 122, .22); border-color: rgba(255, 95, 122, .45); transform: translateY(-1px); }
    .doc-code-tool-btn:disabled { opacity: .55; cursor: not-allowed; transform: none; }
    .doc-code-tool-btn svg { width: .9rem; height: .9rem; }
    .doc-code-tool-btn--copied { width: auto; min-width: 3.8rem; padding: 0 .5rem; font-size: .72rem; font-weight: 700; color: #bbf7d0; border-color: rgba(74, 222, 128, .6); background: rgba(74, 222, 128, .15); }
    .doc-code-output { margin-top: .45rem; border: 1px solid rgba(255,255,255,.1); border-radius: .55rem; background: rgba(0,0,0,.3); font-family: "JetBrains Mono", monospace; font-size: .78rem; line-height: 1.5; color: #e5e7eb; white-space: pre-wrap; overflow-wrap: anywhere; max-height: 0; opacity: 0; overflow: hidden; padding: 0 .7rem; transition: max-height .35s ease, opacity .25s ease, padding .25s ease; }
    .doc-code-output--open { max-height: 22rem; opacity: 1; padding: .65rem .7rem; overflow: auto; }
    .doc-code-output--loading { color: #fbcfe8; }
    .doc-code-loading { display: inline-flex; align-items: center; gap: .5rem; }
    .doc-code-spinner { width: .78rem; height: .78rem; border-radius: 999px; border: 2px solid rgba(255,255,255,.25); border-top-color: #ff7f9a; animation: docSpin .8s linear infinite; }
    @keyframes docSpin { to { transform: rotate(360deg); } }
    .doc-content code, .doc-code-line-num, .doc-code-line-text { font-family: "JetBrains Mono", monospace; }
    .doc-content ul { list-style: disc; margin-left: 1.4rem; }
    .doc-content h2 { font-size: 1.9rem; font-weight: 700; letter-spacing: .01em; margin: .35rem 0 .95rem; color: #ff9fbb; text-shadow: 0 0 18px rgba(255, 122, 158, .22); border-bottom: 1px solid rgba(255, 122, 158, .35); padding-bottom: .45rem; }
    .doc-content h3 { font-size: 1.15rem; font-weight: 600; margin: 1.2rem 0 .45rem; color: #e5e7eb; }
    .doc-content p, .doc-content li { color: #d1d5db; line-height: 1.7; font-size: .88rem; }
    .doc-content a { color: #ff9fbb; text-decoration: underline; text-decoration-color: rgba(255, 159, 187, .45); text-underline-offset: 2px; transition: color .2s ease, text-decoration-color .2s ease; }
    .doc-content a:hover { color: #ffd3df; text-decoration-color: rgba(255, 211, 223, .85); }
    .doc-content hr {
      border: 0;
      height: 1px;
      margin: 2rem 0;
      background: linear-gradient(
        to right,
        rgba(255, 255, 255, 0),
        rgba(255, 255, 255, 0.14),
        rgba(255, 255, 255, 0)
      );
    }
    .doc-tip { display: inline-flex; align-items: center; gap: .5rem; font-size: .75rem; margin: .5rem 0 1rem 0; padding: .35rem .65rem; border-radius: 9999px; border: 1px solid rgba(255, 122, 158, .45); background: rgba(255, 122, 158, .12); color: #ffd6e2; box-shadow: inset 0 0 0 1px rgba(255, 255, 255, .04);}
    .doc-tip::before { content: "TIP"; font-size: .65rem; font-weight: 700; letter-spacing: .08em; color: #ffb7cc; }
    .doc-tip--note { border-color: rgba(125, 211, 252, .45); background: rgba(56, 189, 248, .12); color: #d7f1ff; }
    .doc-tip--note::before { content: "NOTE"; color: #9fdcff; }
    .doc-tip--warn { border-color: rgba(251, 191, 36, .5); background: rgba(245, 158, 11, .14); color: #ffefc5; }
    .doc-tip--warn::before { content: "WARN"; color: #ffd580; }
    .doc-code { display: block; width: 100%; background: transparent; }
    .doc-code-line { display: grid; grid-template-columns: 2.35rem 1fr; gap: .45rem; min-height: 1.3rem; }
    .doc-code-line-num { user-select: none; text-align: right; color: rgba(255,255,255,.36); border-right: 1px solid rgba(255,255,255,.18); padding-right: .35rem; font-size: .78em; }
    .doc-code-line-text { white-space: pre-wrap; overflow-wrap: anywhere; word-break: break-word; color: #e5e7eb; font-size: .84rem; }
    .doc-content.fade-none { -webkit-mask-image: none; mask-image: none; }
    .doc-content.fade-top { -webkit-mask-image: linear-gradient(to bottom, transparent 0px, #000 28px, #000 100%); mask-image: linear-gradient(to bottom, transparent 0px, #000 28px, #000 100%); }
    .doc-content.fade-bottom { -webkit-mask-image: linear-gradient(to bottom, #000 0px, #000 calc(100% - 28px), transparent 100%); mask-image: linear-gradient(to bottom, #000 0px, #000 calc(100% - 28px), transparent 100%); }
    .doc-content.fade-both { -webkit-mask-image: linear-gradient(to bottom, transparent 0px, #000 28px, #000 calc(100% - 28px), transparent 100%); mask-image: linear-gradient(to bottom, transparent 0px, #000 28px, #000 calc(100% - 28px), transparent 100%); }
    .doc-content.doc-content-animate { animation: docContentAppear .5s cubic-bezier(.22, .72, .2, 1); }
    @keyframes docContentAppear { from { opacity: .2; transform: translateY(10px); } to { opacity: 1; transform: translateY(0); } }
    .docs-section-pages { min-height: 0; }
  `;
}

function normalizeTipBadges(container) {
  const candidates = [".doc-tip", "[data-tip]", "[data-note]", "[data-warn]", ".tip", ".note", ".warning"].join(",");
  container.querySelectorAll(candidates).forEach((el) => {
    const text = (el.textContent || "").trim().toLowerCase();
    const isWarn = el.hasAttribute("data-warn") || el.classList.contains("warning") || text.startsWith("warning:");
    const isNote = el.hasAttribute("data-note") || el.classList.contains("note") || text.startsWith("note:");
    el.className = `doc-tip ${isWarn ? "doc-tip--warn" : isNote ? "doc-tip--note" : ""}`.trim();
  });
}

function renderCodeWithLineNumbers(raw) {
  const normalized = raw.replaceAll("\r\n", "\n").replaceAll("\r", "\n");
  const lines = normalized.split("\n");
  while (lines.length > 1 && lines[lines.length - 1] === "") {
    lines.pop();
  }
  return lines.map((line, i) => {
    const highlighted = line.length ? highlightCherryCode(line) : "&nbsp;";
    return `<span class=\"doc-code-line\"><span class=\"doc-code-line-num\">${i + 1}</span><span class=\"doc-code-line-text\">${highlighted}</span></span>`;
  }).join("");
}

function slugifyHeading(value) {
  return value
    .toLowerCase()
    .trim()
    .replace(/[^a-z0-9\s-]/g, "")
    .replace(/\s+/g, "-")
    .replace(/-+/g, "-");
}

function collectHeadings(container) {
  const seen = new Set();
  const headings = [];
  container.querySelectorAll("h3").forEach((h3, index) => {
    const text = (h3.textContent || "").trim();
    if (!text) return;
    const base = slugifyHeading(text) || `section-${index + 1}`;
    let id = base;
    let suffix = 2;
    while (seen.has(id)) {
      id = `${base}-${suffix}`;
      suffix += 1;
    }
    seen.add(id);
    h3.id = id;
    headings.push({ id, title: text });
  });
  return headings;
}

function colorizeCodeBlocks(container) {
  const copyIcon = `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" aria-hidden="true">
      <path d="M9 9a2 2 0 0 1 2-2h8a2 2 0 0 1 2 2v10a2 2 0 0 1-2 2h-8a2 2 0 0 1-2-2V9Z"/>
      <path d="M5 5a2 2 0 0 1 2-2h8v2H7v10H5V5Z"/>
    </svg>`;
  const runIcon = `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor" aria-hidden="true">
      <path d="M8 5.14v14l11-7-11-7Z"/>
    </svg>`;

  container.querySelectorAll("pre code").forEach((block) => {
    const pre = block.parentElement;
    if (!pre) return;

    const raw = block.textContent || "";
    const isRunnable = /\blanguage-(chry|cherry)\b/.test(block.className || "");
    block.className = "doc-code";
    block.innerHTML = renderCodeWithLineNumbers(raw);
    pre.classList.add("doc-code-pre");

    pre.querySelector(".doc-code-toolbar")?.remove();
    const toolbar = document.createElement("div");
    toolbar.className = "doc-code-toolbar";
    toolbar.innerHTML = `
      <button class="doc-code-tool-btn" type="button" title="Copy code" aria-label="Copy code">${copyIcon}</button>
      ${isRunnable ? `<button class="doc-code-tool-btn" type="button" title="Run code" aria-label="Run code">${runIcon}</button>` : ""}
    `;
    pre.appendChild(toolbar);

    let output = pre.nextElementSibling;
    if (isRunnable) {
      if (!output || !output.classList.contains("doc-code-output")) {
        output = document.createElement("div");
        output.className = "doc-code-output";
        pre.insertAdjacentElement("afterend", output);
      } else {
        output.className = "doc-code-output";
        output.innerHTML = "";
      }
    } else if (output && output.classList.contains("doc-code-output")) {
      output.remove();
      output = null;
    }

    const [copyButton, runButton] = toolbar.querySelectorAll("button");
    const originalCopyButtonContent = copyButton.innerHTML;
    let copyResetTimer = null;

    copyButton.addEventListener("click", async () => {
      try {
        await navigator.clipboard.writeText(raw);
        if (copyResetTimer) clearTimeout(copyResetTimer);
        copyButton.classList.add("doc-code-tool-btn--copied");
        copyButton.textContent = "Copied";
        copyResetTimer = setTimeout(() => {
          copyButton.classList.remove("doc-code-tool-btn--copied");
          copyButton.innerHTML = originalCopyButtonContent;
          copyResetTimer = null;
        }, 1200);
      } catch {
        if (output) {
          output.classList.add("doc-code-output--open");
          output.innerHTML = `<span class="text-red-300">Unable to copy code.</span>`;
        }
      }
    });

    if (!isRunnable || !runButton || !output) return;

    runButton.addEventListener("click", async () => {
      if (runButton.disabled) return;
      runButton.disabled = true;
      output.classList.add("doc-code-output--open", "doc-code-output--loading");
      output.innerHTML = `<span class="doc-code-loading"><span class="doc-code-spinner"></span>Running snippet...</span>`;

      await ensureDocsAnsi();
      const runAnsi = createAnsiInstance() || docsAnsi;

      let producedOutput = false;
      try {
        await runDocSnippet(raw, (chunk) => {
          if (!producedOutput) {
            producedOutput = true;
            output.innerHTML = "";
            output.classList.remove("doc-code-output--loading");
            const resetPrefix = "\u001b[0m";
            output.innerHTML += outputChunkToHtml(resetPrefix, runAnsi);
          }
          output.innerHTML += outputChunkToHtml(chunk, runAnsi) + "<br/>";
          output.scrollTop = output.scrollHeight;
        });
        if (!producedOutput) {
          output.classList.remove("doc-code-output--loading");
          output.innerHTML = `<span class="text-gray-400">(no output)</span>`;
        }
      } catch (error) {
        output.classList.remove("doc-code-output--loading");
        output.innerHTML = `<span class="text-red-300">${escapeHtml(String(error?.message || error))}</span>`;
      } finally {
        runButton.disabled = false;
      }
    });
  });
}

function updateContentFade() {
  const content = getActiveContentElement();
  if (!content) return;
  const max = Math.max(0, content.scrollHeight - content.clientHeight);
  const y = content.scrollTop;
  content.classList.remove("fade-none", "fade-top", "fade-bottom", "fade-both");
  if (max <= 1) content.classList.add("fade-none");
  else if (y <= 1) content.classList.add("fade-bottom");
  else if (y >= max - 1) content.classList.add("fade-top");
  else content.classList.add("fade-both");
}

function getScrollKey(pageId) {
  return `${scrollPrefix}${pageId || "index"}`;
}

function saveScrollPosition(pageId) {
  const content = getActiveContentElement();
  if (!content || !pageId) return;
  sessionStorage.setItem(getScrollKey(pageId), String(content.scrollTop));
}

function restoreScrollPosition(pageId) {
  const content = getActiveContentElement();
  if (!content || !pageId) return;
  const raw = sessionStorage.getItem(getScrollKey(pageId));
  const saved = raw == null ? 0 : Number(raw);
  content.scrollTop = Number.isFinite(saved) ? saved : 0;
  updateContentFade();
}

function animateContentChange() {
  const content = getActiveContentElement();
  if (!content) return;
  content.classList.remove("doc-content-animate");
  void content.offsetWidth;
  content.classList.add("doc-content-animate");
}

function findPage(sectionId, pageId) {
  const section = docsData.sections.find((s) => s.id === sectionId);
  if (!section) return null;
  return section.pages.find((p) => p.id === pageId) || null;
}

function updateActiveTitles() {
  if (!docsData) return;
  const section = docsData.sections.find((s) => s.id === activeSectionId) || null;
  const page = section?.pages.find((p) => p.id === activePageId) || null;
  activeSectionTitle.value = section?.title || "";
  activePageTitle.value = page?.title || "";
}

function findPageById(pageId) {
  for (const section of docsData.sections) {
    const page = section.pages.find((p) => p.id === pageId);
    if (page) return { sectionId: section.id, page };
  }
  return null;
}

async function renderActivePage() {
  const activeContent = getActiveContentElement();
  const page = findPage(activeSectionId, activePageId);
  const token = ++pageRenderToken;
  if (!activeContent) return;

  if (!page) {
    activePageHeadings = [];
    activeContent.innerHTML = "<p class='text-gray-300'>No content.</p>";
    renderSidebar();
    updateContentFade();
    return;
  }

  activePageHeadings = pageHeadingsById.get(page.id) || [];
  renderSidebar();
  activeContent.innerHTML = "<p class='text-gray-400'>Loading...</p>";
  try {
    const res = await fetch(withBase(`docs/${page.file}`));
    if (!res.ok) throw new Error(`Failed to load ${page.file} (${res.status})`);
    const raw = await res.text();
    const html = page.file.endsWith(".md") ? markdownToHtml(raw) : raw;
    if (token !== pageRenderToken) return;
    activeContent.innerHTML = html;
    activePageHeadings = collectHeadings(activeContent);
    pageHeadingsById.set(page.id, activePageHeadings);
    restoreScrollPosition(activePageId);
    normalizeTipBadges(activeContent);
    colorizeCodeBlocks(activeContent);
    animateContentChange();
    renderSidebar();
    updateContentFade();
  } catch (err) {
    if (token !== pageRenderToken) return;
    activePageHeadings = pageHeadingsById.get(page.id) || [];
    activeContent.innerHTML = `<p class='text-red-300'>${escapeHtml(String(err))}</p>`;
    renderSidebar();
    updateContentFade();
  }
}

function renderSidebar() {
  const desktop = sidebarRefDesktop.value;
  const mobile = sidebarRefMobile.value;
  if (!docsData) return;

  const navHtml = `
    <div class="mb-3">
      <div class="text-[10px] uppercase tracking-[0.12em] text-gray-400 mb-2">Navigate</div>
      <div class="space-y-1">
        <a href="${withBase("")}" class="block text-xs px-2 py-1.5 rounded-md hover:bg-white/[0.06] text-gray-300">home</a>
        <a href="${withBase("playground")}" class="block text-xs px-2 py-1.5 rounded-md hover:bg-white/[0.06] text-gray-300">playground</a>
        <a href="${withBase("docs")}" class="block text-xs px-2 py-1.5 rounded-md bg-white/[0.06] text-white">docs</a>
      </div>
    </div>
    <div class="mb-3">
      <div class="h-px bg-white/10 my-2"></div>
      <div class="text-[10px] uppercase tracking-[0.12em] text-gray-400">Doc Sections</div>
    </div>
  `;

  const sectionsHtml = docsData.sections.map((section) => {
    const isOpen = openSections.has(section.id);
    const pages = section.pages.map((page) => {
      const active = section.id === activeSectionId && page.id === activePageId;
      const headingLinks = active && activePageHeadings.length
        ? `<div class="mt-1 ml-3 border-l border-white/10 pl-2 space-y-1">${activePageHeadings.map((heading) => (
            `<button data-heading="${heading.id}" class="w-full text-left text-[11px] px-2 py-1 rounded text-gray-400 hover:text-gray-200 hover:bg-white/[0.05]">${heading.title}</button>`
          )).join("")}</div>`
        : "";
      return `<div><button data-page=\"${section.id}:${page.id}\" class=\"w-full text-left text-xs px-2 py-1.5 rounded-md mt-1 ${active ? "bg-[#ff5f7a2a] text-white" : "hover:bg-white/[0.06] text-gray-300"}\">${page.title}</button>${headingLinks}</div>`;
    }).join("");

    return `<div class=\"mb-2\"><button data-section=\"${section.id}\" class=\"w-full flex items-center justify-between text-left px-2 py-1.5 rounded-md hover:bg-white/[0.06] bg-white/[0.03] text-xs\"><span class=\"text-[#ff8ea4] font-semibold uppercase tracking-[0.1em] text-[10px]\">${section.title}</span><span class=\"text-[#ff5f7a]\">${isOpen ? "-" : "+"}</span></button><div class=\"docs-section-pages overflow-hidden transition-all duration-300 ease-out ${isOpen ? "max-h-[1200px] opacity-100 mt-1" : "max-h-0 opacity-0"}\">${pages}</div></div>`;
  }).join("");

  if (desktop) desktop.innerHTML = sectionsHtml;
  if (mobile) mobile.innerHTML = navHtml + sectionsHtml;

  [desktop, mobile].forEach((sidebar) => sidebar?.querySelectorAll("[data-section]").forEach((el) => {
    el.addEventListener("click", () => {
      const id = el.dataset.section;
      const panel = el.parentElement?.querySelector(".docs-section-pages");
      const indicator = el.querySelector("span:last-child");
      if (!panel) return;

      const isOpen = openSections.has(id);
      if (isOpen) {
        openSections.delete(id);
        panel.classList.remove("max-h-[1200px]", "opacity-100", "mt-1");
        panel.classList.add("max-h-0", "opacity-0");
        if (indicator) indicator.textContent = "+";
      } else {
        openSections.add(id);
        panel.classList.remove("max-h-0", "opacity-0");
        panel.classList.add("max-h-[1200px]", "opacity-100", "mt-1");
        if (indicator) indicator.textContent = "-";
      }
    });
  }));

  [desktop, mobile].forEach((sidebar) => sidebar?.querySelectorAll("[data-page]").forEach((el) => {
    el.addEventListener("click", async () => {
      const [sid, pid] = el.dataset.page.split(":");
      activeSectionId = sid;
      activePageId = pid;
      sidebarOpen.value = false;
      await router.push(`/docs/${pid}`);
      renderSidebar();
      renderActivePage();
    });
  }));

  [desktop, mobile].forEach((sidebar) => sidebar?.querySelectorAll("[data-heading]").forEach((el) => {
    el.addEventListener("click", () => {
      const headingId = el.dataset.heading;
      const content = getActiveContentElement();
      if (!content || !headingId) return;
      const target = content.querySelector(`#${CSS.escape(headingId)}`);
      if (!target) return;
      target.scrollIntoView({ behavior: "smooth", block: "start" });
      saveScrollPosition(activePageId);
    });
  }));

}

async function initDocs() {
  ensureDocsEnhancementStyles();
  const desktop = contentRef.value;
  const mobile = contentRefMobile.value;
  if (desktop) desktop.addEventListener("scroll", onContentScroll, { passive: true });
  if (mobile) mobile.addEventListener("scroll", onContentScroll, { passive: true });

  const res = await fetch(withBase("docs/docs_data.json"));
  docsData = await res.json();

  const pageId = route.params.pageId ? String(route.params.pageId) : null;
  const routed = pageId ? findPageById(pageId) : null;

  if (routed) {
    activeSectionId = routed.sectionId;
    activePageId = routed.page.id;
  } else {
    activeSectionId = docsData.sections[0].id;
    activePageId = docsData.sections[0].pages[0].id;
    await router.replace(`/docs/${activePageId}`);
  }

  openSections.add(activeSectionId);
  updateActiveTitles();
  renderSidebar();
  await renderActivePage();
}

function onContentScroll() {
  updateContentFade();
  saveScrollPosition(activePageId);
}

watch(() => route.params.pageId, async (next) => {
  if (!docsData || !next) return;
  const resolved = findPageById(String(next));
  if (!resolved) return;
  activeSectionId = resolved.sectionId;
  activePageId = resolved.page.id;
  openSections.add(activeSectionId);
  updateActiveTitles();
  renderSidebar();
  await renderActivePage();
});

onMounted(() => {
  docsDestroyed = false;
  initDocs();
});
onBeforeUnmount(() => {
  docsDestroyed = true;
  saveScrollPosition(activePageId);
  const desktop = contentRef.value;
  const mobile = contentRefMobile.value;
  if (desktop) desktop.removeEventListener("scroll", onContentScroll);
  if (mobile) mobile.removeEventListener("scroll", onContentScroll);
  for (const runId of Array.from(docsRunnerFrames.keys())) {
    cleanupDocRunner(runId);
  }
});
</script>
