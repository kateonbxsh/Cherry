## Standard library

The standard library in Cherry is implemented in the class `Standard`.

### `println`

```chry
Standard.println(any x);
```
Prints stringified `x` and a line break

```chry
Standard.println(string format, any... args);
```
Formats `args` in `format` and prints them, and a line break.
`format` follows these rules:

- `"{i}"` is replaced by `args[i]`
- `"{}"` is replaced by `args[c]` with `c` being an internal counter that increments for each `{}`

### `print`

```chry
Standard.println(any x);
Standard.println(string format, any... args);
```
Follows the same principles as `println` without a line break.

### `format`

```chry
Standard.format(string format, any... args);
```
Follows the same principles as `print` and `println` but returns the formatted string.