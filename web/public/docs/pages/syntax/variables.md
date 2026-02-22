## Variables

Variables in Cherry hold runtime values. A value always carries a type, and assignment checks assignability at runtime.

### Manipulating variables

Declaring a variable in Cherry is quite similar to a lot of other languages:

```chry
TypeName name = expression;
```
```chry
// Example
int count = 0;
string label = 'ok';
Array<int> xs = new Array<int>();
```

You may also assign a value to a pre-defined variable.

```chry
target = expression;
```
```chry
count = count + 1;
user.name = 'Ari';
matrix[1, 2] = 42;
```

You can also perform an operation + assignment in one step by putting the operator before the `=` sign

```chry
target += expression;
target -= expression;
target *= expression;
target /= expression;
```
```chry
count += 1;
vec -= otherVec;
```

If a variable has been previously defined with a type, you can only re-assign values compatible with that type to it.

```chry
// fail: incompatible assignment
int age = 20;
age = "twenty";
```

---

### Blocks create new variable worlds

Variables declared inside a block are not visible outside it. Shadowing creates a new variable with the same name.

```chry
int x = 10;
{
  int x = 99;
  Standard.println('inner={}', x);
}
Standard.println('outer={}', x);
```

---

### Basic types

These are the basic primitive types in Cherry.

```chry
int integer = 10; // underlying 64-bit signed integer
real value = 1.2; // underlying double
string name = "John";
boolean isTrue = true;
function print = Standard.println;
any x = 1; // this variable can be re-assigned to any type
```

When declaring a variable, you can omit the value and have the default value of the type.

```chry
int integer; //0
string value; //""
```

You can also infer the value from the assigned value, using `let` or `infer`, in this case, a value is required.

```chry
let a = 0; // int
let s = null; // any
```
