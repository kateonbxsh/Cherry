## Nature of types

In Cherry, types are runtime values. This is the mental model that unlocks the rest of the language. You are not only writing values of type `int` or `string`; you can also manipulate type objects themselves.

### Types as values

The identifier `int` is itself a value. Its runtime type is the type `type` (it only gets confusing from now), and its value is the integer type.

```chry
Standard.println(int);         // [type int]
Standard.println(string);      // [type string]
```

### The special case: `type`

`type` is also a runtime value of type `type` and of value its own type. In other words, `type` metadata is self-describing.

```chry
Standard.println(int); // [type int]
Standard.println(typeof int); // [type type] 
Standard.println(type); // [type type]
Standard.println(typeof type); // [type type]
```

The type of `int` is the same as the value of `type`

### Classes are types

When you define a class, Cherry registers a new runtime type. Instantiation with `new` creates object values whose runtime type is that class type.

```chry
class Person {
  public string name;
}

Standard.println(typeof Person);      // [type type]
Person p = new Person();              // instance value
Standard.println(p);                  // [Person instance]
Standard.println(typeof p);           // [type Person]
```

### `typeof` keyword

The `typeof` keyword is used to grab the runtime type of a certain value.
```chry
typeof 10; // [type int]
typeof typeof 10; // [type type]
```