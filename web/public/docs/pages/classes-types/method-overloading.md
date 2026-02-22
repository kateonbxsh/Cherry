## Method overloading

In Cherry, methods differ slightly are functions, beside in the traditional sense where methods are linked to classes, methods allow for overloading.

Overloading is defining multiple methods with the same name, but with different signature (argument count and types).

When an overloaded method is invoked, the right overload is resolved dispatched using the argument types and count.

```chry
class Person {
    public sayHello() {
        Standard.println("Hello!");
    }
    public sayHello(string to) {
        Standard.println("Hello, {}!", to);
    }
}

let p = new Person();
p.sayHello(); // Hello!
p.sayHello("Mark"); // Hello, Mark!
```

An example of method overloading is `Standard.println`, this standard built-in method is defined using two signatures:

```chry
class Standard {
    public static println(any x);
    public static println(string format, any ...args);
}
```

### Ambiguity
Since overloaded methods use argument types to resolve the correct function, there should be no ambiguity in the types. In other words, there should'nt be two functions compatible with a certain signature.

```chry
class Person {
    public sayHello(any a) {
        Standard.println("any");
    }
    public sayHello(string to) {
        Standard.println("string");
    }
}
// throws an error
```
Here, a `string` is assignable to `any`, so there is ambiguity.

### Indirect ambiguity

Sometimes the ambiguity is not directly apparent, take the following generic example:

```chry
class Person<T> {
    public sayHello(T a) {
        Standard.println("any");
    }
    public sayHello(string to) {
        Standard.println("string");
    }
}
```
This does not throw an error right away, but if you instantiate `Person` with a type parameter compatible with `string`, `sayHello` will have ambiguity, and will throw an error

