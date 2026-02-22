# Cherry

Cherry is a custom interpreted programming language written in modern C++.
It is expression-oriented, class-capable, and includes language features like `unless`, indexer methods, dynamic types, operator overloading, and structured exception handling.
Source and test programs in this repository use the `.chry` extension.

## Highlights

- `if` and `unless`
- Classes with:
  - fields and methods
  - static fields and methods
  - constructors
  - access flags (`public`, `private`, `protected`, `readonly`)
  - operator overloading (`operator +`, etc.) with static binary dispatch
  - indexer methods (`get[...]`, `set[...]`)
- Lambdas and closures
- Variadic parameters (`...`) for lambdas, methods, and internal built-ins
- Generic type parameters with defaults and constraints
- Dynamic type declarations and `is` checks
- `throw`, `try/catch/finally`, built-in exception hierarchy, and formatted runtime errors
- Built-in runtime classes:
  - `Array<T = any>`
  - `Map<U = string, V = any>`
  - `Standard` (`print`, `println`, `format` with formatting)

## Quick Example

```typescript
class Vec {
  public int x;

  public Vec(int x) {
    this.x = x;
  }

  public static operator+(Vec a, Vec b) {
    return new Vec(a.x + b.x);
  }

  display {
    return "x: " + this.x;
  }
}

Vec a = new Vec(4);
function makeIncrementer = () => {
  let b = new Vec(2);
  return () => {
    b += a;
    return b;
  };
};
let incrementer = makeIncrementer();
Standard.println("iteration 1 = {}", incrementer()); // x: 6
Standard.println("iteration 2 = {}", incrementer()); // x: 10
```

## Build

### Requirements

- CMake 3.22+
- C++23-compatible compiler

### Configure + Build

```bash
cmake -S . -B build
cmake --build build --config Release
```

On Windows, the executable is typically:

- `build/Cherry.exe` (or `build/Release/Cherry.exe` depending on generator)

On Linux/macOS:

- `build/Cherry`

## Run

```bash
# Windows example
.\build\Cherry.exe .\test\cases\success\basic_arith.chry

# Linux/macOS example
./build/Cherry ./test/cases/success/basic_arith.chry
```

Debug mode:

```bash
.\build\Cherry.exe .\test\cases\success\basic_arith.chry --debug
```

## Test Suite

The repository includes a manifest-driven test harness covering:

- success paths
- compile-time failures
- runtime failures
- edge cases (precedence, operators, generics, variadics, indexers, exceptions, etc.)

Run all tests:

```bash
python scripts/run_cherry_tests.py
```

Use a custom executable path:

```bash
python scripts/run_cherry_tests.py --exe build/Release/Cherry.exe
```

Test definitions live in:

- `test/tests_manifest.json`
- `test/cases/success`
- `test/cases/compile_error`
- `test/cases/runtime_error`

## CI

GitHub Actions workflow:

- `.github/workflows/cherry-tests.yml`

It builds Cherry and runs the full manifest suite on:

- `ubuntu-latest`
- `windows-latest`

## Repository Layout

- `src/` language implementation (lexer, parser, interpreter, runtime)
- `test/` test cases and manifest
- `scripts/run_cherry_tests.py` local/CI test runner
- `.github/workflows/` CI

## Web Playground (WASM)

Build browser runtime (manual mode):

```powershell
./scripts/build_wasm.ps1
```

or

```bash
./scripts/build_wasm.sh
```

Build browser runtime using CMake presets (`CMakePresets.json`):

```powershell
./scripts/build_wasm.ps1 -UsePreset
```

or

```bash
./scripts/build_wasm.sh build-wasm web/wasm Release 1
```

This generates/copies the runtime files consumed by the playground:

- `web/wasm/Cherry.js`
- `web/wasm/Cherry.wasm`

Serve repository root and open:

- `http://localhost:8000/web/`

## Notes

- Cherry is actively evolving; syntax and diagnostics are still being refined.
- The test suite is the current source of truth for expected behavior.

