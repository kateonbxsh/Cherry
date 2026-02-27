## Generics

Generics let you define type families. Whether it is a dynamic type or class, generics let you have type parameters.

### Generic class basics

Take the following case
```chry
class Box<T> {
  public T value;

  public Box(T v) {
    this.value = v;
  }
}
```
`Box` is now a well defined class and type. However, when referred to or instantiated, it requires a type parameter `T` and that type parameter is used when defining the inner field `value` and as a parameter of its constructor.
```chry
class Box<T> {
  public T value;

  public Box(T v) {
    this.value = v;
  }
}

let boxedInt = new Box<int>();
```
We have defined a `Box` instance that uses `int`.

### Default type parameters

By default, type parameters are required, however you can set a default type parameter, making that type parameter optional.

```chry
class Pair<T, U = string> {
  public T left;
  public U right;
}

Pair<int> ok = new Pair<int>(); // same as Pair<int, string>
```

Default type parameters should always be at the end of the type parameter chain.

### Constraints with `extends`

You can use `extends` to require assignability to a base type.

```chry
type Binary = 0 | 1;
class Repo<T extends int> {
  public T last;
}
new Repo<int>(); // pass
new Repo<Binary>(); // pass
try {
  new Repo<string>(); // fail
} catch (Exception e) {
  Standard.println("expected: {}", e.message);
}
```

### Partial consumption of generics

As types are runtimes, they can be assigned, from an expression perspective the `A<T>` expression consumes the first type parameter as `T`, in other words, take the following example:

```chry
class Pair<T, U> {};

let IntPair = Pair<int>;
```
Now, `IntPair` is a class that takes only one type parameter.
```chry
class Pair<T, U> {};
let IntPair = Pair<int>;

let IntAndStringPair = IntPair<string>;
// equivalent to Pair<int, string>
```

### Dynamic type patterns

Dynamic types also support generics.

```chry
type Optional<T> = T | null;
```
The type `Optional` requires a type parameter to resolve its expression.

### `is` keyword

As you previously have seen, we use the `is` keyword to check assignability to a certain type.

Taking the previous example, `is` also checks assignability of type parameters.

```chry
type Binary = 0 | 1;
type Optional<T> = T | null;

Optional<Binary> a = 0;
a is Optional<Binary>; // true
a is Optional<int>; // true
a is Optional; // true
a is Optional<string>; // false
```
