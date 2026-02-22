## Operators

Operators are the symbols and keywords Cherry uses to build expressions:
arithmetic, comparison, boolean logic, type checks, and assignment updates.

### Operator summary

- **Arithmetic**: `+`, `-`, `*`, `/`, `mod`, `div`, `**`, `^`
- **Assignment**: `=`, `+=`, `-=`, `*=`, `/=`, `mod=`, `div=`
- **Comparison**: `==`, `!=`, `>`, `>=`, `<`, `<=`
- **Boolean logic**: `not` / `!`, `and` / `&&`, `or` / `||`, `xor`
- **Bitwise**: `&`, `|`, `!|`
- **Type check**: `is`

<div data-warn>Division by zero throws a <code>DivisionByZeroException</code>.</div>

---

### Arithmetic operators

#### Addition (`+`)

```chry
int a = 7 + 4;
Standard.println('{}', a); // 11
```

#### Subtraction (`-`)

```chry
int a = 10 - 3;
Standard.println('{}', a); // 7
```

#### Multiplication (`*`)

```chry
int a = 6 * 5;
Standard.println('{}', a); // 30
```

#### Division (`/`)

```chry
real a = 7 / 2;
Standard.println('{}', a); // 3.5
```

#### Remainder (`mod`)

```chry
int a = 10 mod 3;
Standard.println('{}', a); // 1
```

#### Integer division (`div`)

```chry
int a = 10 div 3;
Standard.println('{}', a); // 3
```

#### Exponentiation (`**`, alias `^`)

```chry
int a = 2 ** 8;
int b = 2 ^ 8;
Standard.println('{} {}', a, b); // 256 256
```

---

### Comparison operators

Comparison operators return a `boolean`.

```chry
Standard.println(10 == 10); // true
Standard.println(10 != 11); // true
Standard.println(10 > 5);   // true
Standard.println(10 >= 10); // true
Standard.println(3 < 8);    // true
Standard.println(3 <= 3);   // true
```

---

### Boolean and bitwise operators

```chry
boolean a = true and false;
boolean b = true or false;
boolean c = not false;
boolean d = true xor false;
```

Bitwise operators are available as `&&`, `||`, and `^||` (xor).

---

### Type test operator

#### `is`

`is` checks whether a runtime value is assignable to a target type.

```chry
any v = 123;

if (v is int) {
  Standard.println('int');
}

if (v is string | int) {
  Standard.println('string or int');
}
```

---

### Operator precedence and grouping

Parentheses always override precedence.

```chry
int x = 1 + 2 * 3;
int y = (1 + 2) * 3;
Standard.println('{} {}', x, y); // 7 9
```

Precedence order (high to low):

- Exponentiation
- Multiplication / Division / Modulo / Integer division
- Addition / Subtraction
- Comparisons
- Logical NOT
- Logical AND
- Logical OR / XOR
- Assignment
