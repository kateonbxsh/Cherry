<template>
  <div class="min-h-screen overflow-x-hidden">
    <div class="absolute top-0 right-0 w-72 h-72 pointer-events-none opacity-50 blob-a"></div>
    <div class="absolute bottom-0 left-0 w-96 h-96 pointer-events-none opacity-40 blob-b"></div>
    <RouterView />
  </div>
</template>

<script setup>
import { RouterView } from "vue-router";
</script>
