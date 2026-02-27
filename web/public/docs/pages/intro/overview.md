## Overview

Cherry is an interpreted language designed for expressive syntax and robust runtime behavior. It combines classes,
    generics, functions, lambdas, operator overloading, indexers, dynamic type definitions, and detailed diagnostics.

### Background 

Like most of my projects, this started of as a curious dive into programming languages and how they work, I initially wanted to make a language that natively supported the use of `unless` as a keyword, then slowly evolved into this mix of ideas I could have in my own language. I really like TypeScript but TypeScript's types only lived up to its transpilation into JS, I wanted to have an interpreted language that supports those types even on runtime.

### Main capabilities

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

<div data-tip>Cherry source files use the `.chry` extension.</div>
