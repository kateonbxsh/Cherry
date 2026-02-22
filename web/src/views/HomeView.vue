<template>
  <div class="hidden md:block">
    <NavBar :wide="false" brand-label="CHERRY" brand-to="/" />
  </div>
  <div class="md:hidden max-w-[95%] mx-auto px-4 pt-4 relative">
    <div class="flex items-center justify-between">
      <div class="font-extrabold tracking-tight text-xl accent">CHERRY</div>
      <button class="glass p-2 rounded-md text-gray-200 hover:bg-white/10" @click="menuOpen = !menuOpen" aria-label="Toggle menu">
        <svg v-if="!menuOpen" xmlns="http://www.w3.org/2000/svg" class="w-5 h-5" viewBox="0 0 24 24" fill="currentColor"><path d="M4 6h16v2H4zm0 5h16v2H4zm0 5h16v2H4z"/></svg>
        <svg v-else xmlns="http://www.w3.org/2000/svg" class="w-5 h-5" viewBox="0 0 24 24" fill="currentColor"><path d="M18.3 5.7L12 12l6.3 6.3-1.4 1.4L10.6 13.4 4.3 19.7 2.9 18.3 9.2 12 2.9 5.7 4.3 4.3l6.3 6.3 6.3-6.3z"/></svg>
      </button>
    </div>
  </div>
  <div v-if="menuOpen" class="fixed inset-0 bg-black/45 z-20 md:hidden" @click="menuOpen = false"></div>
  <aside
    class="fixed top-[72px] left-4 bottom-4 w-[min(340px,calc(100vw-2rem))] glass rounded-2xl p-4 overflow-auto z-30 transition-transform duration-300 ease-out md:hidden"
    :class="menuOpen ? 'translate-x-0' : '-translate-x-[120%]'"
  >
    <div class="text-[10px] uppercase tracking-[0.12em] text-gray-400 mb-2">Navigate</div>
    <div class="space-y-1">
      <RouterLink to="/" class="block text-xs px-2 py-1.5 rounded-md bg-white/[0.06] text-white">home</RouterLink>
      <RouterLink to="/playground" class="block text-xs px-2 py-1.5 rounded-md hover:bg-white/[0.06] text-gray-300">playground</RouterLink>
      <RouterLink to="/docs" class="block text-xs px-2 py-1.5 rounded-md hover:bg-white/[0.06] text-gray-300">docs</RouterLink>
    </div>
  </aside>

  <main class="max-w-[96%] md:max-w-[70%] mx-auto px-4 md:px-6 py-8 md:py-16">
    <section class="grid grid-cols-1 w-full gap-8 items-center">
      <div class="space-y-6 fade-up">
        <p class="uppercase tracking-[0.25em] text-xs text-gray-400">The Cherry Language</p>
        <h1 class="text-3xl md:text-6xl leading-tight">
          Write expressive code,
          <span class="accent font-extrabold">run it anywhere.</span>
        </h1>
        <p class="text-gray-300 max-w-xl leading-relaxed text-sm md:text-base">
          Classes, generics, operator overloads, dynamic types, indexers, exceptions, and rich diagnostics in one modern interpreted language.
        </p>
        <div class="glass rounded-xl px-5 py-4 min-h-[120px] flex items-start">
          <span ref="typewriterRef" class="type-line block whitespace-pre-wrap text-lg md:text-2xl font-semibold text-gray-100 leading-relaxed" style="font-family: 'JetBrains Mono', monospace;"></span>
        </div>
        <div class="flex flex-wrap gap-3 pt-1">
          <RouterLink to="/playground" class="bg-[#ff5f7a] hover:bg-[#ff7a93] text-black font-semibold px-5 py-3 rounded-lg transition inline-flex items-center gap-2">
            <svg xmlns="http://www.w3.org/2000/svg" class="w-4 h-4" viewBox="0 0 24 24" fill="currentColor"><path d="M8 5v14l11-7zM4 5h2v14H4z"/></svg>
            <span>Open Online Playground</span>
          </RouterLink>
          <RouterLink to="/docs" class="glass hover:bg-white/10 px-5 py-3 rounded-lg transition inline-flex items-center gap-2">
            <svg xmlns="http://www.w3.org/2000/svg" class="w-4 h-4" viewBox="0 0 24 24" fill="currentColor"><path d="M5 4h10a4 4 0 014 4v12H9a4 4 0 00-4 4V4zm12 14V8a2 2 0 00-2-2H7v13.1A5.98 5.98 0 019 18h8z"/></svg>
            <span>Read Docs</span>
          </RouterLink>
        </div>
      </div>
    </section>
  </main>
</template>

<script setup>
import { RouterLink } from "vue-router";
import { onMounted, onBeforeUnmount, ref } from "vue";
import NavBar from "../components/NavBar.vue";

const typewriterRef = ref(null);
const menuOpen = ref(false);
const lines = [
  "Standard.println(\"good morning!\") unless night == true;",
  "let a = 1;\nfunction incrementA = () => {\n  a += 1;\n}",
  "class Array3D {\n  public get[int x, int y, int z] { ... }\n}",
  "type Username = string x when x.length() > 5 default \"sample\";",
  "type Optional<T> = T | null;"
];

let lineIndex = 0;
let charIndex = 0;
let deleting = false;
let timer = null;

function escapeHtml(value) {
  return value.replaceAll("&", "&amp;").replaceAll("<", "&lt;").replaceAll(">", "&gt;");
}

function highlightSnippet(snippet) {
  const tokenRegex = /"([^"\\]|\\.)*"|\b(function|class|public|private|protected|static|type|when|default|unless|let|null|true|false|get|set)\b|\b(int|real|string|any|void)\b|\b([0-9]+(?:\.[0-9]+)?)\b/g;
  let result = "";
  let last = 0;
  let match = tokenRegex.exec(snippet);

  while (match) {
    if (match.index > last) result += escapeHtml(snippet.slice(last, match.index));
    const token = match[0];
    const escaped = escapeHtml(token);
    if (token.startsWith("\"")) result += `<span class=\"text-emerald-300\">${escaped}</span>`;
    else if (match[2]) result += `<span class=\"text-pink-300 font-semibold\">${escaped}</span>`;
    else if (match[3]) result += `<span class=\"text-sky-300\">${escaped}</span>`;
    else if (match[4]) result += `<span class=\"text-amber-300\">${escaped}</span>`;
    else result += escaped;
    last = tokenRegex.lastIndex;
    match = tokenRegex.exec(snippet);
  }
  if (last < snippet.length) result += escapeHtml(snippet.slice(last));
  return result;
}

function tick() {
  const el = typewriterRef.value;
  if (!el) return;
  const current = lines[lineIndex];
  if (!deleting) {
    charIndex += 1;
    el.innerHTML = highlightSnippet(current.slice(0, charIndex));
    if (charIndex >= current.length) {
      deleting = true;
      timer = setTimeout(tick, 2500);
      return;
    }
    timer = setTimeout(tick, 45 + Math.random() * 35);
  } else {
    charIndex -= 1;
    el.innerHTML = highlightSnippet(current.slice(0, Math.max(0, charIndex)));
    if (charIndex <= 0) {
      deleting = false;
      lineIndex = (lineIndex + 1) % lines.length;
      timer = setTimeout(tick, 250);
      return;
    }
    timer = setTimeout(tick, 12);
  }
}

onMounted(() => tick());
onBeforeUnmount(() => { if (timer) clearTimeout(timer); });
</script>
