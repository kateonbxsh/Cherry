## Loops
Cherry supports classic loop patterns.

### while loop

```chry
int i = 0;
while (i < 3) {
  Standard.println("i={}", i);
  i += 1;
}
```

### do-while loop

Use `do ... while` when the body must execute at least once.

```chry
int j = 0;
do {
  Standard.println("j={}", j);
  j += 1;
} while (j < 2);
```

### for loop

```chry
int n = 0;
for (int k = 0; k < 5; k += 1) {
  Standard.println("n={}", n);
  n += k;
}
```

### repeat-until loop

```chry
int n = 1;
repeat {
  Standard.println("n={}", n);
  n *= 2;
} until (n > 10)
```

### repeat-times loop

```chry
int n = 1;
int k = 10;  
repeat (k) times {
  n *= 2;
  Standard.println("n={}", n);
}
```
