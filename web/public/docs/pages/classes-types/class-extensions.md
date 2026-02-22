## Class Extensions (expertimental)

`extends` currently supports single inheritance between classes.

### Basic inheritance

```chry
class Animal {
  public string name = "animal";
  public speak() {
    Standard.println("...");
  }
}

class Dog extends Animal {
  public speak() {
    Standard.println("woof");
  }
}
```

`Dog` gets the parent chain through its type parent link, so member lookup can walk up inherited types.

### What is inherited right now

- Instance fields from parent classes are collected when creating an instance.
- Instance methods are resolved through the parent chain.
- Static fields and static methods are also resolved through the parent chain.
- Type checks treat children as assignable to ancestor types.

### Constructor behavior (important)

Constructor chaining is currently minimal:

- `new X(...)` selects one constructor overload on `X`.
- The instance is created and fields are initialized.
- Only the selected constructor body runs.

There is currently:

- no `super(...)` call syntax
- no automatic parent-constructor call

So parent constructor logic does not run automatically. If parent initialization logic is required, it must be handled manually with the currently supported language features.

### Generic `extends` constraints

`extends` is also used in type-parameter constraints:

```chry
class Box<T extends string> {
  public T value;
}
```

At specialization time, provided type arguments are checked against the constraint. Default generic type values are checked too.
