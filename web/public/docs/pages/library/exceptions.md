## Exceptions

Cherry exceptions are runtime objects.  
The base class is `Exception`, and all built-in runtime errors extend it.

### Base class

`Exception` has these standard fields:

- `message`
- `stacktrace`
- `line`
- `col`
- `file`

These are used by runtime diagnostics and printed error reports.

### Built-in hierarchy

Built-in classes include:

- `Exception`
- `RuntimeException`
- `OperationException`
- `TypeException`
- `NameException`
- `ArgumentException`
- `IndexException`
- `DivisionByZeroException`

`DivisionByZeroException` is part of operation/runtime failures and is thrown on invalid division operations.

### Throwing

Use `throw` with an exception instance:

```chry
throw new Exception("something went wrong");
```

Throwing a `string` is also accepted by runtime conversion: it is wrapped into an `Exception` and used as `message`.

```chry
throw "bad state";
```

### Try / catch / finally

Cherry supports:

- `try { ... }`
- one or more `catch (Type e) { ... }`
- optional `finally { ... }`

```chry
try {
  throw new NameException("x is undefined");
} catch (NameException e) {
  Standard.println("name error: {}", e.message);
} catch (Exception e) {
  Standard.println("generic error: {}", e.message);
} finally {
  Standard.println("cleanup");
}
```

### Catch compatibility

Catch blocks should be in order of generality.
```chry
try {
  throw new NameException("x is undefined");
} catch (Exception e) {
  Standard.println("generic error: {}", e.message);
} catch (NameException e) {
  Standard.println("name error: {}", e.message);
}
```
Fails because `NameException` extends `Exception` and comes after it.
