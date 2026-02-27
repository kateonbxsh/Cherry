## Dynamic types

Cherry supports the concept of dynamic types, where types can defined as operations or constraints of other types.

### Union

A simple example is the union of two types:
```chry
type Comparable = int | string | real;
```
Here we define the type Comparable as the union of `int`, `string` and `real`. We can use `Comparable` as a type in our code to match one of those three types.
```chry
type Comparable = int | string | real;

0 is Comparable; // true
"hello" is Comparable; // true

// define a function
function less = (Comparable a, Comparable b) => a < b;
```

### Constraint types

A more advanced use of types are constraints, in Cherry, you may define constraint types, which are types that enforce a certain boolean condition on their value.
```chry
type Username = string x when x.length() > 5 default "sample";
```
Now `Username` is a string, which enforces the constraint of being longer than 5 characters.
```chry
type Username = string x when x.length() > 5 default "sample";

"John" is Username; // false
"Jessica" is Username; // true
```
A constraint type systematically requires a default value.

### Values as types

Values can be used as types.
```chry
type Binary = 0 | 1;
```
This is a type that can only takes the two `int` values `0` and `1`.
This is technically equivalent to the following constraint type.
```chry
type Binary = int x when x == 0 or x == 1 default 0;
```
