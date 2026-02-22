# Cherry Web

Web entrypoints:

- `web/index.html` landing page
- `web/playground/index.html` online playground (Monaco + WASM runtime + examples)
- `web/docs/index.html` docs (JSON-driven, collapsible sidebar)

The visual style intentionally matches the original Cherry glass/pink theme.

## Build WebAssembly Runtime

From repository root:

```powershell
./scripts/build_wasm.ps1
```

or on Linux/macOS:

```bash
./scripts/build_wasm.sh
```

Or use CMake presets:

```powershell
./scripts/build_wasm.ps1 -UsePreset
```

```bash
./scripts/build_wasm.sh build-wasm web/wasm Release 1
```

This generates/copies:

- `web/wasm/Cherry.js`
- `web/wasm/Cherry.wasm` (and optional side assets)

## Run Locally

Use a static server from repository root so WASM can be fetched by the browser.

Example with Python:

```bash
python -m http.server 8000
```

Then open:

- `http://localhost:8000/web/`
- `http://localhost:8000/web/playground/`
- `http://localhost:8000/web/docs/`
