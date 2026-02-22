<template>
  <header :class="headerClass">
    <nav class="glass rounded-2xl px-4 py-3 md:flex md:items-center md:justify-between">
      <div class="flex items-center justify-between">
        <RouterLink :to="brandTo" class="font-extrabold tracking-tight text-xl"><span class="accent">{{ brandLabel }}</span></RouterLink>
        <button class="md:hidden px-2.5 py-2 rounded-md text-gray-300 hover:bg-white/10 inline-flex items-center" @click="menuOpen = !menuOpen" aria-label="Toggle menu">
          <svg v-if="!menuOpen" xmlns="http://www.w3.org/2000/svg" class="w-4 h-4" viewBox="0 0 24 24" fill="currentColor"><path d="M4 6h16v2H4zm0 5h16v2H4zm0 5h16v2H4z"/></svg>
          <svg v-else xmlns="http://www.w3.org/2000/svg" class="w-4 h-4" viewBox="0 0 24 24" fill="currentColor"><path d="M18.3 5.7L12 12l6.3 6.3-1.4 1.4L10.6 13.4 4.3 19.7 2.9 18.3 9.2 12 2.9 5.7 4.3 4.3l6.3 6.3 6.3-6.3z"/></svg>
        </button>
      </div>
      <div class="mt-3 md:mt-0 flex-col md:flex-row gap-2 text-sm md:ml-auto" :class="menuOpen ? 'flex' : 'hidden md:flex'">
        <RouterLink to="/" class="px-3 py-1 rounded-md inline-flex items-center gap-1.5" :class="isActive('/') ? 'bg-white/10 text-white' : 'hover:bg-white/10 text-gray-300'">
          <svg xmlns="http://www.w3.org/2000/svg" class="w-4 h-4" viewBox="0 0 24 24" fill="currentColor"><path d="M12 3l9 8h-3v9h-5v-6H11v6H6v-9H3l9-8z"/></svg>
          <span>home</span>
        </RouterLink>
        <RouterLink to="/playground" class="px-3 py-1 rounded-md inline-flex items-center gap-1.5" :class="isActive('/playground') ? 'bg-white/10 text-white' : 'hover:bg-white/10 text-gray-300'">
          <svg xmlns="http://www.w3.org/2000/svg" class="w-4 h-4" viewBox="0 0 24 24" fill="currentColor"><path d="M8 5v14l11-7zM4 5h2v14H4z"/></svg>
          <span>playground</span>
        </RouterLink>
        <RouterLink to="/docs" class="px-3 py-1 rounded-md inline-flex items-center gap-1.5" :class="isActive('/docs') ? 'bg-white/10 text-white' : 'hover:bg-white/10 text-gray-300'">
          <svg xmlns="http://www.w3.org/2000/svg" class="w-4 h-4" viewBox="0 0 24 24" fill="currentColor"><path d="M5 4h10a4 4 0 014 4v12H9a4 4 0 00-4 4V4zm12 14V8a2 2 0 00-2-2H7v13.1A5.98 5.98 0 019 18h8z"/></svg>
          <span>docs</span>
        </RouterLink>
        <a href="https://github.com/kateonbxsh/Cherry" target="_blank" rel="noreferrer" class="px-3 py-1 rounded-md hover:bg-white/10 text-gray-300 inline-flex items-center gap-1.5">
          <svg xmlns="http://www.w3.org/2000/svg" class="w-4 h-4" viewBox="0 0 24 24" fill="currentColor"><path d="M12 .5A12 12 0 000 12.7a12.2 12.2 0 008.2 11.6c.6.1.8-.2.8-.6v-2.2c-3.3.8-4-1.6-4-1.6-.5-1.4-1.3-1.8-1.3-1.8-1.1-.8.1-.8.1-.8 1.2.1 1.9 1.3 1.9 1.3 1.1 1.9 2.9 1.4 3.6 1.1.1-.8.4-1.4.8-1.7-2.7-.3-5.5-1.4-5.5-6.2 0-1.4.5-2.5 1.3-3.4-.1-.3-.6-1.6.1-3.3 0 0 1.1-.4 3.5 1.3a11.8 11.8 0 016.4 0c2.4-1.7 3.5-1.3 3.5-1.3.7 1.7.2 3 .1 3.3.8.9 1.3 2 1.3 3.4 0 4.8-2.9 5.9-5.6 6.2.5.4.9 1.2.9 2.4v3.6c0 .4.2.7.8.6A12.2 12.2 0 0024 12.7 12 12 0 0012 .5z"/></svg>
          <span>github repo</span>
        </a>
      </div>
    </nav>
  </header>
</template>

<script setup>
import { useRoute, RouterLink } from "vue-router";
import { computed, ref, watch } from "vue";

const props = defineProps({
  wide: { type: Boolean, default: false },
  brandLabel: { type: String, default: "CHERRY" },
  brandTo: { type: String, default: "/" }
});

const route = useRoute();
const menuOpen = ref(false);
const headerClass = computed(() => props.wide ? "max-w-[1500px] mx-auto p-6 pb-2" : "max-w-[70%] mx-auto px-6 pt-6");

function isActive(prefix) {
  return route.path === prefix || route.path.startsWith(`${prefix}/`);
}

watch(() => route.path, () => {
  menuOpen.value = false;
});
</script>
