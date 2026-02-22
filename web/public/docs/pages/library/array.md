## Array

`Array<T = any>` is a built-in generic class backed by a runtime vector.

---

### Construction styles

Cherry supports three common ways to construct arrays:

1. Constructor call:

```chry
Array<int> a = new Array<int>();
```

2. Inferred array literal:

```chry
infer a = [1, 2, 3];
```

3. Explicitly typed array literal:

```chry
infer names = Array<string>["aya", "noah"];
```

The typed literal form also works when the left side is an identifier or alias that resolves to `Array<...>`:

```chry
type Names = Array<string>;
infer names = Names["aya", "noah"];
```

### Type parameters

- `T`: element type
- Default: `any`

```chry
Array<int> numbers = new Array<int>();
Array mixed = new Array(); // Array<any>
```

### Array literal inference rules

For `[x, y, z]`, Cherry infers the element type by widening across elements:

- Start from the type of the first element.
- For each next element:
  - If current type accepts next element type, keep current type.
  - Else if next element type accepts current type, widen to next type.
  - Else fall back to `any`.

Examples:

```chry
typeof [1, 2, 3]      // Array<int>
typeof [1, 2.5, 3]    // Array<any> (with current built-in numeric rules)
typeof [1, "x"]       // Array<any>
```

### Explicit typed literals

`Array<T>[...]` enforces assignability of every element to `T`:

```chry
infer ok = Array<int>[1, 2, 3];      // OK
infer bad = Array<string>["x", 2];   // RuntimeException
```

### Methods

#### `push`

```chry
int push(T value);
```

- Appends one value.
- Returns the new size.
- Throws a runtime type error if `value` is not assignable to `T`.

---

#### `size`

```chry
int size();
```

- Returns current element count.

---

#### `get`

```chry
T get(int index);
```

- Returns element at `index`.
- Throws `IndexException` / runtime index error if out of range.

---

#### `set`

```chry
T set(int index, T value);
```

- Replaces element at `index`.
- Returns the assigned value.
- Throws on out-of-range index.
- Throws a runtime type error if `value` is not assignable to `T`.

---

#### `pop`

```chry
T pop();
```

- Removes and returns the last element.
- Throws on empty array.

---

#### `clear`

```chry
void clear();
```

- Removes all elements.

---

#### `empty`

```chry
boolean empty();
```

- Returns `true` when size is `0`.

---

#### `contains`

```chry
boolean contains(T value);
```

- Returns `true` when an equal value exists.

### Indexing syntax

`Array` integrates with index operators:

```chry
arr[0]       // equivalent to get(0)
arr[0] = 42  // equivalent to set(0, 42)
```

### Example

```chry
Array<string> names = new Array<string>();
names.push("Aya");
names.push("Noah");
Standard.println("size={}", names.size()); // 2
Standard.println("first={}", names[0]);    // Aya
```
