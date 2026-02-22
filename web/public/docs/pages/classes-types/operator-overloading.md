## Operator overloading

Operator overriding is an advanced use of classes, where you define the behavior that should occur when a classic binary operator is used with the class.

We all know the binary `+` operator, which performs addition between two integers, or concatenation between two strings, what if we want this operator to work between two classes we previously defined, that's where operator overloading comes in, it lets us define tha behavior.

For example:
```chry
class Point {
    int x;
    int y;
}
```
We have a `Point` that has two values, we want to be able to add two instances of `Point` using their coordinates, we use the following formula:
```chry
class Point {
    int x;
    int y;
    public static operator+(Point a, Point b) {
        return new Point(a.x + b.x, a.y + b.y);
    }
} 
```
`operator` followed by the overloaded operator, this method is static and always takes two arguments.

We can now do:
```chry
Point a; Point b;
Point c = a + b;
```

This works for all binary operators (`-`, `*`, `or`, `mod`, etc...)

### Indexing overload

Another classic behavior we can overload is the bracket indexing `[]`, by default, this can only be used for `Array`s, `string`s and `Map`s.

We can overload this behavior using the following syntax

```chry
class Array3D {

    public get[int x, int y, int z] {
        return 0;
    }

    public set[int x, int y, int z] {
        Standard.println(value * x * y * z);
    }

}
let a = new Array3D();
a[0, 4, 4]; // 0
a[1, 2, 1] = 5; // prints 10
```
`get[...]` is used when value is retrieved, and `set[...]` is used when value is set, both these can take any amount of arguments. `set` definition has an implictly defined `value` variable, which corresponds to the passed `value`.