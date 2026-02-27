# Cherry

Cherry is a custom interpreted programming language written in modern C++.
It is expression-oriented, class-capable, and includes language features like `unless`, indexer methods, dynamic types, operator overloading, and structured exception handling.
Source and test programs in this repository use the `.chry` extension.

This repository contains the full source code, the test suite, the configurations for native binary compilation and WASM compilation, and the web pages for documentation and Online playground.

## Background 

Like most of my projects, this started of as a curious dive into programming languages and how they work, I initially wanted to make a language that natively supported the use of `unless` as a keyword, then slowly evolved into this mix of ideas I could have in my own language. I really like TypeScript but TypeScript's types only lived up to its transpilation into JS, I wanted to have an interpreted language that supports those types even on runtime.

## Highlights

- Types in Cherry are **runtime values**, you can pass them around, modify them, and reference them wherever you can reference a value. That unfortunately means there is low type checking at **compile time**, but it offers powerful syntactic advantages in your code.
- Conditionals, `if` and `unless`, both as a statement and inline expressions `a if cond else b`
- Lambdas and closures (similar to functions in OCaml and arrow functions in Javascript)
- Variadic parameters (`...`) for lambdas, methods, and internal built-ins
- Classes with:
  - fields and methods
  - static fields and methods
  - constructors
  - access flags (`public`, `private`, `protected`, `readonly`)
  - operator overloading (`operator +`, etc.) with static binary dispatch
  - indexer methods (`get[...]`, `set[...]`)
- Generic type parameters with defaults and constraints
- Dynamic type declarations (types can have any runtime boolean expression to check assignability) and `is` checks
- `throw`, `try/catch/finally`, built-in exception hierarchy, and formatted runtime errors
- Built-in runtime classes:
  - `Array<T = any>`
  - `Map<U = string, V = any>`
  - `Standard` (`print`, `println`, `format` with formatting)

You may find the [full language documentation here](https://kateonbxsh.github.io/Cherry/docs).

## Quick Example

```typescript
type Optional<T> = T | null;
type short = int x when x <= 255 default 0;
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

- `.github/workflows/ci-cd.yml`

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

- Cherry is actively evolving; syntax and diagnostics are still being refined, and there are a lot edges cases not covered.
- The test suite is the current source of truth for expected behavior.

## Contributions

Contributions are welcome through issues and pull requests.

- Prefer small, focused PRs with clear commit history.
- For language/runtime changes, add or update tests in `test/cases` and `test/tests_manifest.json`.
- Keep behavior changes documented in the README and/or docs pages when relevant.

By submitting a contribution, you agree that your changes are licensed under this repository's license.

## License

This project is licensed under the **GNU AGPL v3.0 or later (AGPL-3.0-or-later)**.

- Use, modification, and redistribution are allowed under AGPL terms.
- Derivative works distributed to others must remain under AGPL-compatible licensing (copyleft).
- If you run a modified version as a network service, you must provide source code under AGPL terms.

See `LICENSE` for full terms.

