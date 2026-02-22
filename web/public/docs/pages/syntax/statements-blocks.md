## Statements and blocks

Cherry executes statements top-to-bottom. This page is a syntax catalog of statement forms with short guidance between examples.

### Block syntax and scope

Blocks use `{` and `}`. Each block introduces a nested scope where inner declarations can shadow outer names.

```chry
int x = 10;
if (true) {
  int x = 99;
  Standard.println('inner={}', x);
}
Standard.println('outer={}', x);
```
The inner `x` only exists inside the `if` block.

> **NOTE:** All statements that don't involve a block end in a semicolon `;`

Any expression can be used as a statement when you care about side effects.

```chry
expression;
```
```chry
Standard.println('hello');
makeLogger()('ready');
new User('Mina');
```
