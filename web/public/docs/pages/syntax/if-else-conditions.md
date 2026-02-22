## Conditions

Cherry has multiple uses for condition statements.

The most common is a `if, else if, else` statement

```chry
int score = 72;
if (score >= 60) {
  Standard.println("pass");
} else if (score >= 40) {
  Standard.println("retake");
} else {
  Standard.println("fail");
}
```

Another use is a guard-style statement

```chry
Standard.println("b is greater than 5") if b > 5;
```
This code only runs if `b > 5` condition passes

This syntax can also be used as ternary in expressions

```chry
int a = (b if b > 5 else 5);
```
This code assigns a conditional value to `a`, as a matter of fact, it assigns b clamped at 5.

Cherry also introduces a `unless` keyword, `unless` is the inverse of `if`. It reads naturally for guard-style conditions.

```chry
boolean maintenance = false;
Standard.println("service online") unless maintenance;
```

It can also be used in a normal if-else statement, replacing `if`
