import { createRouter, createWebHistory } from "vue-router";
import HomeView from "./views/HomeView.vue";
import PlaygroundView from "./views/PlaygroundView.vue";
import DocsView from "./views/DocsView.vue";

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    { path: "/", component: HomeView },
    { path: "/playground", component: PlaygroundView },
    { path: "/docs/:pageId?", component: DocsView },
    { path: "/:pathMatch(.*)*", redirect: "/" }
  ]
});

export default router;
