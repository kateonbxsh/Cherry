## Functions

Functions are first-class values in Cherry. You can store them, pass them, return them, and call them dynamically. Think of a function declaration as creating a value whose type is `function`.

When you bind a function to a variable, that value can be invoked and passed around.

```chry
function add = (int a, int b) => {
  return a + b;
};

int result = add(2, 3);
Standard.println("{}", result); // prints: 5
```

Functions take arguments, which have types, when invoking a function, the passed arguments should necessarily be assignable to the specified types in the function's signature.

```chry
function f = (string a) => {
  return a;
};

f("hello"); // passes
f(5); // fails
```

If you know the concepts of [lambdas (anonymous functions)](https://en.wikipedia.org/wiki/Anonymous_function), in Cherry, there is no distinction between functions and lambdas, you can think of as basically defining a function as an arrow expression and store it in a variable.
Functions capture outer variables by scope chain. This lets you build stateful behavior without classes.

```chry
int seed = 0;
function next = () => {
  seed += 1;
  return seed;
};

Standard.println("{} {}", next(), next()); // prints: 1 2
```
In this example, when the `next` function is defined, it captures the outer scope it's defined it, and remembers the reference of the outer `seed`, so everytime it's invoked, it's mutating the `seed` variable.

### Variadic parameters

Variadic parameters are parameters that can very in count, for example if you have a function `sum` that sums a certain amount of numbers, but that amount can vary. You can use a variadic parameter for that:

```chry
function sum = (int ...numbers) => { ... };
```

### Return

To return a value from a function, you use the `return` keyword, but if you use this keyword outside of a function, that will actually exit the program you're running, and if you pass a number as the returned value, that will be the `exit code` provided by the program.