## Standard library

The standard library in Cherry is implemented in the class `Standard`.

---

### `println`

```text
Standard.println(any x);
```
Prints stringified `x` and a line break

```text
Standard.println(string format, any... args);
```
Formats `args` in `format` and prints them, and a line break.
`format` follows these rules:

- `"{i}"` is replaced by `args[i]`
- `"{}"` is replaced by `args[c]` with `c` being an internal counter that increments for each `{}`

---

### `print`

```text
Standard.println(any x);
Standard.println(string format, any... args);
```
Follows the same principles as `println` without a line break.

---

### `format`

```text
Standard.format(string format, any... args);
```
Follows the same principles as `print` and `println` but returns the formatted string.

---

### `display` and stringification

When `Standard.print`, `Standard.println`, or `Standard.format` needs a string for a value, Cherry stringifies that value.

For class instances, if the class defines:

```text
display {
  return "some string";
}
```

that method is used as the string representation.

```chry
class User {
  public string name;

  public User(string name) {
    this.name = name;
  }

  display {
    return "User<" + this.name + ">";
  }
}

let u = new User("aya");
Standard.println(u);                         // User<aya>
Standard.println("value={}", u);             // value=User<aya>
let s = Standard.format("as text: {}", u);   // as text: User<aya>
```

If a class does not define `display`, the default runtime representation is used.
