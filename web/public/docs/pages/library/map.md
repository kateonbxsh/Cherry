## Map

`Map<U = string, V = any>` is a built-in generic dictionary class.

### Type parameters

- `U`: key type (default `string`)
- `V`: value type (default `any`)

```chry
Map<string, int> scores = new Map<string, int>();
Map m = new Map(); // Map<string, any>
```

<div data-note>Current runtime storage supports string keys. Non-string key types are rejected at runtime.</div>

### Methods

#### `set`

```text
V set(U key, V value);
```

- Inserts or updates an entry.
- Returns the assigned value.
- Throws if key/value are not assignable to `U` / `V`.
- Throws if runtime key is not string-compatible.

---

#### `get`

```text
V get(U key);
```

- Returns the value for `key`.
- Throws when key does not exist.

---

#### `has`

```text
boolean has(U key);
```

- Returns whether `key` exists.

---

#### `remove`

```text
boolean remove(U key);
```

- Removes key if present.
- Returns `true` if removed, else `false`.

---

#### `size`

```text
int size();
```

- Returns number of entries.

---

#### `clear`

```text
void clear();
```

- Removes all entries.

---

#### `empty`

```text
boolean empty();
```

- Returns `true` when size is `0`.

---

#### `keys`

```text
Array<U> keys();
```

- Returns all keys as an `Array`.

---

#### `values`

```text
Array<V> values();
```

- Returns all values as an `Array`.

---

### Indexing syntax

`Map` integrates with index operators:

```text
m["k"]       // equivalent to get("k")
m["k"] = 7   // equivalent to set("k", 7)
```

### Example

```chry
Map<string, int> scores = new Map<string, int>();
scores["alice"] = 12;
scores.set("bob", 20);

if (scores.has("alice")) {
  Standard.println("alice={}", scores["alice"]);
}

Standard.println("keys={}", scores.keys());
Standard.println("values={}", scores.values());
```
