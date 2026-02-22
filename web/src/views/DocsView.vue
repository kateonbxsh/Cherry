<template>
  <NavBar :wide="true" brand-label="CHERRY DOCS" brand-to="/" />

  <main class="max-w-[1500px] mx-auto p-6 pt-3 h-[calc(100vh-110px)]">
    <div class="grid grid-cols-12 gap-4 h-full">
      <aside ref="sidebarRef" class="glass rounded-2xl p-4 col-span-12 lg:col-span-3 overflow-auto"></aside>
      <section ref="contentRef" class="glass rounded-2xl p-6 col-span-12 lg:col-span-9 overflow-auto doc-content"></section>
    </div>
  </main>
</template>

<script setup>
import { onMounted, onBeforeUnmount, ref, watch } from "vue";
import { useRoute, useRouter } from "vue-router";
import NavBar from "../components/NavBar.vue";

const route = useRoute();
const router = useRouter();
const sidebarRef = ref(null);
const contentRef = ref(null);

let docsData = null;
let activeSectionId = null;
let activePageId = null;
const openSections = new Set();
let pageRenderToken = 0;
const scrollPrefix = "cherry.docs.scroll.";

function escapeHtml(value) {
  return value.replaceAll("&", "&amp;").replaceAll("<", "&lt;").replaceAll(">", "&gt;");
}

function highlightCherryCode(source) {
  const tokenRegex = /\/\/.*|\/\*[\s\S]*?\*\/|"([^"\\]|\\.)*"|'([^'\\]|\\.)*'|\b(class|type|function|return|if|else|unless|while|do|for|try|catch|finally|throw|new|this|extends|when|default|display|operator|get|set|public|private|protected|static|readonly|is|true|false|null|any)\b|\b(int|real|string|boolean|void)\b|\b([0-9]+(?:\.[0-9]+)?)\b/g;
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
  if (document.getElementById("docsEnhancementStyles")) return;
  const style = document.createElement("style");
  style.id = "docsEnhancementStyles";
  style.textContent = `
    .doc-content pre { background: rgba(0,0,0,.45); border: 1px solid rgba(255,255,255,.15); border-radius: .6rem; padding: .9rem; overflow: auto; margin: .9rem 0; }
    .doc-content code { color: #ffd2df; background: rgba(255,255,255,.08); border-radius: .35rem; padding: 0 .35rem; }
    .doc-content pre code { color: inherit; background: transparent; border-radius: 0; padding: 0; }
    .doc-content code, .doc-code-line-num, .doc-code-line-text { font-family: "JetBrains Mono", monospace; }
    .doc-content ul { list-style: disc; margin-left: 1.4rem; }
    .doc-content h2 { font-size: 1.9rem; font-weight: 700; letter-spacing: .01em; margin: .35rem 0 .95rem; color: #ff9fbb; text-shadow: 0 0 18px rgba(255, 122, 158, .22); border-bottom: 1px solid rgba(255, 122, 158, .35); padding-bottom: .45rem; }
    .doc-content h3 { font-size: 1.15rem; font-weight: 600; margin: 1.2rem 0 .45rem; color: #e5e7eb; }
    .doc-content p, .doc-content li { color: #d1d5db; line-height: 1.7; }
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
    .doc-tip { display: inline-flex; align-items: center; gap: .5rem; font-size: .75rem; margin-top: .5rem; padding: .35rem .65rem; border-radius: 9999px; border: 1px solid rgba(255, 122, 158, .45); background: rgba(255, 122, 158, .12); color: #ffd6e2; box-shadow: inset 0 0 0 1px rgba(255, 255, 255, .04); }
    .doc-tip::before { content: "TIP"; font-size: .65rem; font-weight: 700; letter-spacing: .08em; color: #ffb7cc; }
    .doc-tip--note { border-color: rgba(125, 211, 252, .45); background: rgba(56, 189, 248, .12); color: #d7f1ff; }
    .doc-tip--note::before { content: "NOTE"; color: #9fdcff; }
    .doc-tip--warn { border-color: rgba(251, 191, 36, .5); background: rgba(245, 158, 11, .14); color: #ffefc5; }
    .doc-tip--warn::before { content: "WARN"; color: #ffd580; }
    .doc-code { display: block; width: 100%; background: transparent; }
    .doc-code-line { display: grid; grid-template-columns: 3rem 1fr; gap: .75rem; min-height: 1.3rem; }
    .doc-code-line-num { user-select: none; text-align: right; color: rgba(255,255,255,.36); border-right: 1px solid rgba(255,255,255,.18); padding-right: .55rem; font-size: .78em; }
    .doc-code-line-text { white-space: pre; color: #e5e7eb; }
    .doc-content.fade-none { -webkit-mask-image: none; mask-image: none; }
    .doc-content.fade-top { -webkit-mask-image: linear-gradient(to bottom, transparent 0px, #000 28px, #000 100%); mask-image: linear-gradient(to bottom, transparent 0px, #000 28px, #000 100%); }
    .doc-content.fade-bottom { -webkit-mask-image: linear-gradient(to bottom, #000 0px, #000 calc(100% - 28px), transparent 100%); mask-image: linear-gradient(to bottom, #000 0px, #000 calc(100% - 28px), transparent 100%); }
    .doc-content.fade-both { -webkit-mask-image: linear-gradient(to bottom, transparent 0px, #000 28px, #000 calc(100% - 28px), transparent 100%); mask-image: linear-gradient(to bottom, transparent 0px, #000 28px, #000 calc(100% - 28px), transparent 100%); }
    .doc-content.doc-content-animate { animation: docContentAppear .5s cubic-bezier(.22, .72, .2, 1); }
    @keyframes docContentAppear { from { opacity: .2; transform: translateY(10px); } to { opacity: 1; transform: translateY(0); } }
    .docs-section-pages { min-height: 0; }
  `;
  document.head.appendChild(style);
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
  const lines = raw.replaceAll("\r\n", "\n").split("\n");
  return lines.map((line, i) => {
    const highlighted = line.length ? highlightCherryCode(line) : "&nbsp;";
    return `<span class=\"doc-code-line\"><span class=\"doc-code-line-num\">${i + 1}</span><span class=\"doc-code-line-text\">${highlighted}</span></span>`;
  }).join("");
}

function colorizeCodeBlocks(container) {
  container.querySelectorAll("pre code").forEach((block) => {
    const raw = block.textContent || "";
    block.className = "doc-code";
    block.innerHTML = renderCodeWithLineNumbers(raw);
  });
}

function updateContentFade() {
  const content = contentRef.value;
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
  const content = contentRef.value;
  if (!content || !pageId) return;
  sessionStorage.setItem(getScrollKey(pageId), String(content.scrollTop));
}

function restoreScrollPosition(pageId) {
  const content = contentRef.value;
  if (!content || !pageId) return;
  const raw = sessionStorage.getItem(getScrollKey(pageId));
  const saved = raw == null ? 0 : Number(raw);
  content.scrollTop = Number.isFinite(saved) ? saved : 0;
  updateContentFade();
}

function animateContentChange() {
  const content = contentRef.value;
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

function findPageById(pageId) {
  for (const section of docsData.sections) {
    const page = section.pages.find((p) => p.id === pageId);
    if (page) return { sectionId: section.id, page };
  }
  return null;
}

async function renderActivePage() {
  const content = contentRef.value;
  const page = findPage(activeSectionId, activePageId);
  const token = ++pageRenderToken;
  if (!content) return;

  if (!page) {
    content.innerHTML = "<p class='text-gray-300'>No content.</p>";
    updateContentFade();
    return;
  }

  content.innerHTML = "<p class='text-gray-400'>Loading...</p>";
  try {
    const res = await fetch(`/docs/${page.file}`);
    if (!res.ok) throw new Error(`Failed to load ${page.file} (${res.status})`);
    const html = await res.text();
    if (token !== pageRenderToken) return;
    content.innerHTML = html;
    restoreScrollPosition(activePageId);
    normalizeTipBadges(content);
    colorizeCodeBlocks(content);
    animateContentChange();
    updateContentFade();
  } catch (err) {
    if (token !== pageRenderToken) return;
    content.innerHTML = `<p class='text-red-300'>${escapeHtml(String(err))}</p>`;
    updateContentFade();
  }
}

function renderSidebar() {
  const sidebar = sidebarRef.value;
  if (!sidebar || !docsData) return;

  sidebar.innerHTML = docsData.sections.map((section) => {
    const isOpen = openSections.has(section.id);
    const pages = section.pages.map((page) => {
      const active = section.id === activeSectionId && page.id === activePageId;
      return `<button data-page=\"${section.id}:${page.id}\" class=\"w-full text-left text-sm px-3 py-2 rounded-md mt-1 border ${active ? "border-[#ff7a9e66] bg-[#ff7a9e22] text-white" : "border-white/10 hover:border-[#ff7a9e55] text-gray-300"}\">${page.title}</button>`;
    }).join("");

    return `<div class=\"mb-2\"><button data-section=\"${section.id}\" class=\"w-full flex items-center justify-between text-left px-3 py-2 rounded-md border border-white/10 hover:border-[#ff7a9e55] bg-white/[0.03] text-sm\"><span class=\"text-[#ff9fbb] font-semibold uppercase tracking-[0.1em] text-[11px]\">${section.title}</span><span class=\"text-[#ff7a9e]\">${isOpen ? "-" : "+"}</span></button><div class=\"docs-section-pages overflow-hidden transition-all duration-300 ease-out ${isOpen ? "max-h-[1200px] opacity-100 mt-1" : "max-h-0 opacity-0"}\">${pages}</div></div>`;
  }).join("");

  sidebar.querySelectorAll("[data-section]").forEach((el) => {
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
  });

  sidebar.querySelectorAll("[data-page]").forEach((el) => {
    el.addEventListener("click", async () => {
      const [sid, pid] = el.dataset.page.split(":");
      activeSectionId = sid;
      activePageId = pid;
      await router.push(`/docs/${pid}`);
      renderSidebar();
      renderActivePage();
    });
  });

}

async function initDocs() {
  ensureDocsEnhancementStyles();
  const content = contentRef.value;
  if (content) {
    content.addEventListener("scroll", onContentScroll, { passive: true });
  }

  const res = await fetch("/docs/docs_data.json");
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
  renderSidebar();
  await renderActivePage();
});

onMounted(initDocs);
onBeforeUnmount(() => {
  const content = contentRef.value;
  saveScrollPosition(activePageId);
  if (content) content.removeEventListener("scroll", onContentScroll);
});
</script>
