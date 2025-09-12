# trees

C project building a compact [LISP](https://en.wikipedia.org/wiki/Lisp_(programming_language))-inspired functional-declarative language, using Daniel Holden’s [mpc parser combinator library](https://github.com/orangeduck/mpc).

## Build & Run
```bash
make
./trees
```
## Shell Demo
```
TREES 0.8
Press Ctrl+C to Exit

trees : add 3 (mod 12 8)
7
trees : joi {1 2 3} {10 11 12}
{1 2 3 10 11 12}
trees : def {square} (one {x} {exp x 2})
{}
trees : square 5
25
trees : 
```
## Status
Complete with a parser combinator supplied with [mpc](https://github.com/orangeduck/mpc), a low-level evaluation engine, and a robust error-handling scheme, <i> trees </i> now completely supports lambda definition, variable setting, and all arithmetic. There are six conditional, one ternary and ten list-manipulation symbols. 
 
