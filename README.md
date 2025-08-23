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
        The standard library is written in tysonLang itself.
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

## Try it out in the webassembly REPL!
https://tysonlang-cd8a7.web.app

## Local usage
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

## Basic Syntax

### Defining a variable

```sh
  def {x} 10
  def {y} (+ 1 1)
  def {z} {1 2 3 4 5}
```

### Defining a function

```sh
  fun {timesTen y} {* 10 y}
```
A function definition consists of the fun keyword followed by 2 Q-expressions. 

- First Q-expr: function name + parameters
- Second Q-expr: function body

The last expression evaluated in the function body is returned upon invocation.

### Calling a function
```
timesTen 20
; -> 200
timesTen "a string"
; -> Error: Cannot operate on non-number!
+ 1 2 3 4 5 6
; -> 21
* 10 20
== 5 5
; -> 1
== 5 4
; -> 0 
```

### binding returned value to a variable
```sh
def {res} (timesTen 20)
```

arithmetic operators such as +, -, * and ==  are functions like any other in tysonLang and are therefore called using polish notation.


| infix notation (Normal) | Polish notation (TysonLang) |
|-------------------------|-----------------------------|
| 1 + 2 + 3               | + 1 2 3                     |
| 5 * 5                   | * 5 5                       |
| 1 == 2                  | == 1 2                      |

## Advanced syntax

Everything in a LISP is a list - both source code and data (except functions and symbols). 
This allows for writing functional code where we pass functions as arguments into other functions.
TysonLang has no for or while loops and relies on recursion for iteration.

A line starting with a semicolon is ignored by the interpreter; it's a comment!

### Conditionals
TysonLang uses if-expressions instead of if-statements, similar to the ternary operator in Java or C.

```sh
if (== 1 1) { "happens if true" } { "this is the else" }
```
we can then print the string that the if expression returns:
```sh
  print (if (== 1 1) { "happens if true" } { "this is the else" })
  ; -> "happens if true"
```

select, which is used below, works like a quirky switch statement.

### Recursion example 
```sh
fun {fib n} {
    ; if item 0 evaluates to true, item 1 gets returned when using select
    select
        {(== n 0) 0} 
        {(== n 1) 1}
        {otherwise (+ (fib (- n 1)) (fib (- n 2)))}
}
```

For a deeper understanding, including control flow and conditionals, consider reading std.tyson.

Most LISPs, including TysonLang, have 2 types of lists listed below:

### S-Expressions (+ 1 2)
A Symbolic expression, commonly shortened to sexpr, is a list where the first element is most often some sort of function or operator that operates on the rest of the list.
In tysonLang S-Expressions are enclosed within parentheses: ()

The file containing the standard tyson library is a list of S-Expressions that are all evaluated upon loading the file into the interpreter.

### Q-Expressions {}
Quoted expressions are a simple type of list that is largely untouched by the interpreter itself.
Useful for storing data that we can then manipulate ourselves with functions such as <em>fst</em> that extracts the first element in said list.

## Factoids
LISP stands for LISt Processor.
Python is a lisp.


## Credits
The core of this project is based on the book [Build Your Own Lisp](https://www.buildyourownlisp.com/)

Uses [mpc](https://github.com/orangeduck/mpc) to model the abstract syntax tree.
