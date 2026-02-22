import { defineConfig } from "vite";
import vue from "@vitejs/plugin-vue";

export default defineConfig({
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
});
