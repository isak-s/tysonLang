<table width="100%">
  <tr>
    <td style="width: 250px; vertical-align: top;">
      <img src=".github/images/lisp_user.png" alt="Image of average lisp user" width="250">
    </td>
    <td style="width: 100%; vertical-align: top;">
      <h3>TysonLang</h3>
      <p>
        A functional, dynamically typed, list-based programming language.<br>
        The interpreter for this <a href="https://sv.wikipedia.org/wiki/Lisp">LISP</a> dialect is written in 100% C.<br>
        The standard library is written in tysonlang itself.
      </p>
      <p>
        <img src="https://img.shields.io/badge/platform-linux--x86%20%7C%20macOS%20%7C%20windows-lightgrey"><br>
        <a href="https://creativecommons.org/licenses/by-nc-sa/3.0/">
          <img src="https://img.shields.io/badge/license-CC%20BY--NC--SA%203.0-lightgrey.svg" alt="License: CC BY-NC-SA 3.0"> <br>
        <img src="https://img.shields.io/badge/language-C-blue">
        </a>
      </p>
    </td>
  </tr>
</table>


## Usage
1. Compile the interpreter
```sh
    make
```
2. Run your program
```sh
    make tyson fileName.tyson
```

### REPL
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
