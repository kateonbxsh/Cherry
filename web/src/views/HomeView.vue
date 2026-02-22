<template>
  <NavBar :wide="false" brand-label="CHERRY" brand-to="/" />
  <main class="max-w-[70%] mx-auto px-6 py-10 md:py-16">
    <section class="grid grid-cols-1 w-full gap-8 items-center">
      <div class="space-y-6 fade-up">
        <p class="uppercase tracking-[0.25em] text-xs text-gray-400">The Cherry Language</p>
        <h1 class="text-4xl md:text-6xl leading-tight">
          Write expressive code,
          <span class="accent font-extrabold">run it anywhere.</span>
        </h1>
        <p class="text-gray-300 max-w-xl leading-relaxed">
          Classes, generics, operator overloads, dynamic types, indexers, exceptions, and rich diagnostics in one modern interpreted language.
        </p>
        <div class="glass rounded-xl px-5 py-4 min-h-[120px] flex items-start">
          <span ref="typewriterRef" class="type-line block whitespace-pre-wrap text-lg md:text-2xl font-semibold text-gray-100 leading-relaxed" style="font-family: 'JetBrains Mono', monospace;"></span>
        </div>
        <div class="flex flex-wrap gap-3 pt-1">
          <RouterLink to="/playground" class="bg-[#ff7a9e] hover:bg-[#ff93b0] text-black font-semibold px-5 py-3 rounded-lg transition inline-flex items-center gap-2">
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
