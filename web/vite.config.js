import { defineConfig } from "vite";
import vue from "@vitejs/plugin-vue";

export default defineConfig(({ command }) => ({
  // Local dev serves from root, production deploy uses project-path base.
  base: command === "serve" ? "/" : (process.env.VITE_BASE_PATH || "/Cherry/"),
  plugins: [
    vue(),
    {
      name: "cherry-docs-full-reload",
      handleHotUpdate(ctx) {
        const normalized = ctx.file.replaceAll("\\", "/");
        const shouldReload =
          normalized.includes("/web/public/docs/pages/") ||
          normalized.endsWith("/web/public/docs/docs_data.json");

        if (shouldReload) {
          ctx.server.ws.send({ type: "full-reload" });
          return [];
        }
      }
    }
  ]
}));
