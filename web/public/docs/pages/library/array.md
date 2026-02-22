## Array

`Array<T = any>` is a built-in generic class backed by a runtime vector.

### Type parameters

- `T`: element type
- Default: `any`

```chry
Array<int> numbers = new Array<int>();
Array mixed = new Array(); // Array<any>
```

### Methods

#### `push`

```chry
int push(T value);
```

- Appends one value.
- Returns the new size.
- Throws a runtime type error if `value` is not assignable to `T`.

#### `size`

```chry
int size();
```

- Returns current element count.

#### `get`

```chry
T get(int index);
```

- Returns element at `index`.
- Throws `IndexException` / runtime index error if out of range.

#### `set`

```chry
T set(int index, T value);
```

- Replaces element at `index`.
- Returns the assigned value.
- Throws on out-of-range index.
- Throws a runtime type error if `value` is not assignable to `T`.

#### `pop`

```chry
T pop();
```

- Removes and returns the last element.
- Throws on empty array.

#### `clear`

```chry
void clear();
```

- Removes all elements.

#### `empty`

```chry
boolean empty();
```

- Returns `true` when size is `0`.

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
