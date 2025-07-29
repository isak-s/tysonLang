# TysonLang

| ![Image of average lisp user](.github/images/lisp_user.png) | **TysonLang** <br> is a functional  list based programming language.<br>The interpreter for this [LISP](https://sv.wikipedia.org/wiki/Lisp) is written in 100% C. |
|--------------------------|-------------------------------|

## Running a program
1. Compile the interpreter
```sh
    make
```
2. Run your program
```sh
    make tyson fileName.tyson
```

## REPL
```sh
    make tyson repl
```
The standard library is included by default.
definitions from additional files can also be included.
```sh
    make tyson fileName.tyson repl
```

## Syntax
placeholder
## Factoids
placeholder
## Credits
The core of this project is based on the book [Build Your Own Lisp](https://www.buildyourownlisp.com/)

Uses [mpc](https://github.com/orangeduck/mpc) to model the abstract syntax tree.
