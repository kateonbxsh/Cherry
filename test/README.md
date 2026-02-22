# Cherry Test Suite

Test case files use the `.chry` extension.

Run locally:

```bash
cmake -S . -B build
cmake --build build
python scripts/run_cherry_tests.py
```

Or provide a custom executable path:

```bash
python scripts/run_cherry_tests.py --exe build/Cherry.exe
```
