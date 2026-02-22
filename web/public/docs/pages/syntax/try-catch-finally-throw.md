## try-catch block

Cherry errors are values of type `Exception`. Use `throw` to raise, `try/catch` to recover, and `finally` for cleanup that must always run.

```chry
throw "something went wrong";
```
Throwing a string is normalized to an `Exception` instance whose message is that string.

You can also construct a `Exception` instance and throw it

```chry
try {
  throw new Exception("bad input");
} catch (Exception e) {
  Standard.println("type error: {}", e.message);
}
```

Sometimes you want the catch code to depend on what's being thrown, given a function `dangerous` that can throw multiple children of `Exception`, you can write multiple `catch` blocks, going from specific exception class to general cases:

```chry
try {
  dangerous();
} catch (DivisionByZeroException e) {
  Standard.println("divide by zero");
} catch (OperationException e) {
  Standard.println("operation failure");
} catch (Exception e) {
  Standard.println("fallback: {}", e.message);
} finally {
  Standard.println("cleanup done");
}
```

You can also handle two different types in the same case, thanks to type union:

```chry
try {
  dangerous();
} catch (DivisionByZeroException | TypeException e) {
  Standard.println("division or type error");
}
```
