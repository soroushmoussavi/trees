# trees

Early-stage C project building a compact [LISP](https://en.wikipedia.org/wiki/Lisp_(programming_language))-inspired functional-declarative language, using Daniel Holden’s [mpc parser combinator library](https://github.com/orangeduck/mpc).

## Build & Run
```bash
make
./trees
```
## Shell Demo
```
TREES 0.0.0.0.1
Press Ctrl+C to Exit

trees : add 1 (exp 12 0)
2
trees : joi {1 2 3} {0 9 8} {4 5 6}
{1 2 3 0 9 8 4 5 6}
trees : div 4 0
Error: Division by Zero
trees : 
```
## Status
Complete with a parser combinator supplied with [mpc](https://github.com/orangeduck/mpc), a low-level evaluation engine, and a robust error-handling scheme, <i> trees </i> now supports seven arithmetic and nine list-manipulation symbols. More coming soon ...
 
