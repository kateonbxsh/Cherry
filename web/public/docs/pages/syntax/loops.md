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
for (int k = 0; k < 5; k += 1) {
  n += k;
}
```

### repeat-until loop

```chry
repeat {
  n *= 2;
} until (n > 1000)
```

### repeat-times loop

```chry
int n = 1;
int k = 4;  
repeat (k) times {
  n *= 2;
}
```
