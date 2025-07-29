<table>
  <tr>
    <td>
      <img src=".github/images/lisp_user.png" alt="Image of average lisp user" width="150">
    </td>
    <td>
      <h3>TysonLang</h3>
      <p>
        A functional, list-based programming language.<br>
        The interpreter for this <a href="https://sv.wikipedia.org/wiki/Lisp">LISP</a> is written in 100% C.
      </p>
      <p>
        <img src="https://img.shields.io/badge/platform-linux--x86%20%7C%20macOS%20%7C%20windows-lightgrey"><br>
        <img src="https://img.shields.io/badge/language-C-blue"><br>
        <a href="https://creativecommons.org/licenses/by-nc-sa/3.0/">
          <img src="https://img.shields.io/badge/license-CC%20BY--NC--SA%203.0-lightgrey.svg" alt="License: CC BY-NC-SA 3.0">
        </a>
      </p>
    </td>
  </tr>
</table>


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
