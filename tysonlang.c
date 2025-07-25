#include <stdio.h>
#include <stdlib.h>

#include "mpc.h"

/* If we are compiling on Windows compile these functions */
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

/* Fake readline function */
char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

/* Fake add_history function */
void add_history(char* unused) {}

/* Otherwise include the editline headers */
#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

;

struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

/* Lisp Value */

enum { LVAL_ERR, LVAL_NUM,   LVAL_SYM,
       LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR };


typedef lval*(*lbuiltin)(lenv*, lval*);

struct lval {
    int type;
    /* Basic */
    long num;
    char* err;
    char* sym;
    /* Function */
    lbuiltin builtin;  /* NULL if it's not a builtin */
    lenv* env;
    lval* formals;
    lval* body;

    /* Expression */
    int count;
    lval** cell;
};

struct lenv {
    /* Parent environment. Ex global symbols and functions */
    lenv* parent;
    int count;
    /* List of strings : variable / function names */
    char** syms;
    /* Their values. corresponding 1 to 1*/
    lval** vals;
};


#define LASSERT(args, cond, fmt, ...) \
    if (!(cond)) { \
        lval* err = lval_err(fmt, ##__VA_ARGS__); \
        lval_del(args); \
        return err; \
    }

#define TOO_MANY_ARGUMENTS_EXCEPTION(function_name, got, expected) \
    "Function '%s' passed too many arguments! " \
    "Got %i, expected %i", \
    function_name, got, expected

#define WRONG_TYPE_EXCEPTION(function_name, got, arg_index, expected) \
    "Function '%s' passed incorrect type for argument %d! " \
    "Got %s, expected %s", \
    function_name, arg_index, got, expected

#define EMPTY_LIST_EXCEPTION(function_name) \
    "Function '%s' passed {}!", \
    function_name

#define LASSERT_TYPE(func, args, index, expect) \
  LASSERT(args, args->cell[index]->type == expect, \
    "Function '%s' passed incorrect type for argument %i. Got %s, Expected %s.", \
    func, index, ltype_name(args->cell[index]->type), ltype_name(expect))

#define LASSERT_ARG_NUM(func, args, num) \
  LASSERT(args, args->count == num, \
    "Function '%s' passed incorrect number of arguments. Got %i, Expected %i.", \
    func, args->count, num)

#define LASSERT_NOT_EMPTY(func, args, index) \
  LASSERT(args, args->cell[index]->count != 0, \
    "Function '%s' passed {} for argument %i.", func, index);

char* ltype_name(int t) {
    switch(t) {
        case LVAL_ERR: return "Error";
        case LVAL_NUM: return "Number";
        case LVAL_SYM: return "Symbol";
        case LVAL_FUN: return "Function";
        case LVAL_SEXPR: return "S-Expression";
        case LVAL_QEXPR: return "Q-Expression";
        default: return "Unknown";
    }
}

lval* lval_num(long x) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

/* Construct a pointer to a new Error lval */
lval* lval_err(char* fmt, ...) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;

    va_list va;
    va_start(va, fmt);

    /* Allocate 512 bytes of space for all of args */
    v->err = malloc(512);

    vsnprintf(v->err, 511, fmt, va);

    v->err = realloc(v->err, strlen(v->err)+1);

    va_end(va);

    return v;
}


lval* lval_sym(char* s) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s) + 1);
    strcpy(v->sym, s);
    return v;
}

lval* lval_sexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell =   NULL;
    return v;
}

lval* lval_qexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_QEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

lval* lval_fun(lbuiltin func) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;
    v->builtin = func;
    return v;
}

lenv* lenv_new(void) {
    lenv* e = malloc(sizeof(lenv));

    e->count = 0;
    e->parent = NULL;
    e->syms = NULL;
    e->vals = NULL;

    return e;
}

lval* lval_lambda(lval* formals, lval* body) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;
    v->builtin = NULL;

    /* Inner env within func */
    v->env = lenv_new();

    v->formals = formals;
    v->body = body;
    return v;
}

void lval_del(lval* v);

void lenv_del(lenv* e) {
    for (int i = 0; i < e->count; i++) {
        free(e->syms[i]);
        lval_del(e->vals[i]);
    }

    free(e->syms);
    free(e->vals);
    free(e);
}

void lval_del(lval* v) {

    switch (v->type) {
        case LVAL_NUM: break;
        case LVAL_ERR: free(v->err); break;
        case LVAL_SYM: free(v->sym); break;
        case LVAL_FUN:
            if (!v->builtin) {
                lenv_del(v->env);
                lval_del(v->formals);
                lval_del(v->body);
            }
            break;

        /* If it's a sexpr or Qexpr, delete all elements inside. */
        case LVAL_QEXPR:
        case LVAL_SEXPR:
            for (int i = 0; i < v->count; i++) {
                lval_del(v->cell[i]);
            }
            free(v->cell);
            break;
    }
    free(v);
}

lval* lval_read_num(mpc_ast_t* t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ?
    lval_num(x) : lval_err("invalid number");
}

lval* lval_add(lval* v, lval* x) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  v->cell[v->count-1] = x;
  return v;
}

lval* lval_copy(lval* v);

lenv* lenv_copy(lenv* e) {
    lenv* n = malloc(sizeof(lenv));
    n->parent = e->parent;
    n->count = e->count;
    n->syms = malloc(sizeof(char*) * n->count);
    n->vals = malloc(sizeof(lval*) * n->count);
    for (int i = 0; i < e->count; i++) {
        n->syms[i] = malloc(strlen(e->syms[i] + 1));
        strcpy(n->syms[i], e->syms[i]);
        n->vals[i] = lval_copy(e->vals[i]);
    }
    return n;
}

lval* lval_copy(lval* v) {

    lval* x = malloc(sizeof(lval));
    x->type = v->type;
    switch (v->type) {
        case LVAL_NUM: x->num = v->num; break;

        case LVAL_FUN:
            if (v->builtin) {
                x->builtin = v->builtin;
            }
            else {
                x->builtin = NULL;
                x->env = lenv_copy(v->env);
                x->body = lval_copy(v->body);
                x->formals = lval_copy(v->formals);
            }
            break;


        case LVAL_ERR:
            x->err = malloc(strlen(v->err) + 1);
            strcpy(x->err, v->err); break;

        case LVAL_SYM:
            x->sym = malloc(strlen(v->sym) + 1);
            strcpy(x->sym, v->sym); break;

            case LVAL_QEXPR:
            case LVAL_SEXPR:
                x->count = v->count;
                x->cell = malloc(sizeof(lval*) * x->count);
                for (int i = 0; i < x->count; i++) {
                    x->cell[i] = lval_copy(v->cell[i]);
                }
                break;
    }
    return x;
}

lval* lval_read(mpc_ast_t* t) {

    /* If Symbol or Number return conversion to that type */
    if (strstr(t->tag, "number")) { return lval_read_num(t); }
    if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

    /* If root (>) or sexpr then create empty list */
    lval* x = NULL;
    if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
    if (strstr(t->tag, "sexpr"))  { x = lval_sexpr(); }
    if (strstr(t->tag, "qexpr"))  { x = lval_qexpr(); }

    /* Fill this list with any valid expression contained within */
    for (int i = 0; i < t->children_num; i++) {
        if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
        if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
        if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
        if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
        if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
        x = lval_add(x, lval_read(t->children[i]));
    }

    return x;
}

lval* lval_eval(lenv* e, lval* v);
void lval_print(lval* v);

void lval_expr_print(lval* v, char open, char close) {
    putchar(open);
    for (int i = 0; i < v->count; i++) {

        lval_print(v->cell[i]);

        if( i != v->count-1) {
            putchar(' ');
        }
    }
    putchar(close);
}

void lval_print(lval* v) {
    switch (v->type) {
        case LVAL_NUM:   printf("%li", v->num); break;
        case LVAL_ERR:   printf("Error: %s", v->err); break;
        case LVAL_SYM:   printf("%s", v->sym); break;
        case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
        case LVAL_QEXPR: lval_expr_print(v, '{', '}'); break;
        case LVAL_FUN:
            if (v->builtin) {
                printf("<BUILTIN>");
            } else {
                printf("(\\ )");
                lval_print(v->formals);
                putchar(' ');
                lval_print(v->body);
                putchar(')');
            }
            break;
    }
}

void lval_println(lval* v) {
    lval_print(v);
    putchar('\n');
}

lval* lval_pop(lval* v, int i) {
    /* Find item at i*/
    lval* x = v->cell[i];

    /* Shift memory (All other lvals) after the item at index i */
    memmove(&v->cell[i], &v->cell[i+1],
        sizeof(lval*) * (v->count-i-1));

    v->count--;

    v->cell = realloc(v->cell, sizeof(lval*) * v->count);

    return x;
}

lval* lval_take(lval* v, int i) {
    lval* x = lval_pop(v, i);
    lval_del(v);
    return x;
}

lval* lval_join(lval* x, lval* y) {
    /* Adds all cells in y to x, deletes y, returns x */
    while (y->count) {
        x = lval_add(x, lval_pop(y, 0));
    }
    lval_del(y);
    return x;
}

int lval_eq(lval* x, lval* y) {

    if (x->type != y->type) { return 0; }

    switch (x->type) {
        case LVAL_NUM: return (x->num == y->num);
        case LVAL_ERR: return (strcmp(x->err, y->err) == 0);
        case LVAL_SYM: return (strcmp(x->sym, y->sym) == 0);
        case LVAL_FUN:
            if (x->builtin || y->builtin) {
                return x->builtin == y->builtin;
            } else {
                return lval_eq(x->formals, y->formals) &&
                    lval_eq(x->body, y->body);
            }
        /* If it's a list, compare every element within. */
        case LVAL_QEXPR:
        case LVAL_SEXPR:
            if (x->count != y->count) { return 0; }
            for (int i = 0; i < x->count; i++) {
                if (!lval_eq(x->cell[i], y->cell[i])) { return 0; }
            }
            return 1;
        default:
            return 0;
    }
}

/* Only works on number types! */
#define ORDERING(op, a, comp) \
    LASSERT_ARG_NUM(op, a, 2); \
    LASSERT_TYPE(op, a, 0, LVAL_NUM); \
    LASSERT_TYPE(op, a, 1, LVAL_NUM); \
    int r = comp; \
    lval_del(a); \
    return lval_num(r)

lval* builtin_leq(lenv* e, lval* a) {
    ORDERING("<=", a, (a->cell[0]->num <= a->cell[1]->num));
}

lval* builtin_geq(lenv* e, lval* a) {
    ORDERING(">=", a, (a->cell[0]->num >= a->cell[1]->num));
}

lval* builtin_lt(lenv* e, lval* a) {
    ORDERING("<", a, (a->cell[0]->num < a->cell[1]->num));
}

lval* builtin_gt(lenv* e, lval* a) {
    ORDERING(">", a, (a->cell[0]->num > a->cell[1]->num));
}

lval* builtin_eq(lenv* e, lval* a) {
    LASSERT_ARG_NUM("==", a, 2);
    int r = lval_eq(a->cell[0], a->cell[1]);
    lval_del(a);
    return lval_num(r);
}

lval* builtin_neq(lenv* e, lval* a) {
    LASSERT_ARG_NUM("!=", a, 2);
    int r = !lval_eq(a->cell[0], a->cell[1]);
    lval_del(a);
    return lval_num(r);
}

lval* builtin_if(lenv* e, lval* a) {
    /* Acts like a ternary operator */
    LASSERT_ARG_NUM("if", a, 3);
    LASSERT_TYPE("if", a, 0, LVAL_NUM);
    LASSERT_TYPE("if", a, 1, LVAL_QEXPR);
    LASSERT_TYPE("if", a, 2, LVAL_QEXPR);

    lval* x;
    /* Mark them as evaluatable */
    a->cell[1]->type = LVAL_SEXPR;
    a->cell[2]->type = LVAL_SEXPR;

    if (a->cell[0]->num) {
        x = lval_eval(e, lval_pop(a, 1));
    } else {
        x = lval_eval(e, lval_pop(a, 2));
    }

    lval_del(a);

    return x;

}

lval* builtin_op(lenv* e, lval* a, char* op) {

    for (int i = 0; i < a->count; i++) {
        if (a->cell[i]->type != LVAL_NUM) {
            lval_del(a);
            return lval_err("Cannot operate on non-number!");
        }
    }

    /* Pop first element */
    lval* x = lval_pop(a, 0);

    /* if no arguments and subtraction then perform unary negation */
    if ((strcmp(op, "-")) && a->count == 0) {
        x->num = x->num;
    }

    /* */
    while (a->count > 0) {
        lval* y = lval_pop(a, 0);

        if (strcmp(op, "+") == 0) { x->num += y->num; }
        if (strcmp(op, "-") == 0) { x->num -= y->num; }
        if (strcmp(op, "*") == 0) { x->num *= y->num; }
        if (strcmp(op, "/") == 0) {
        if (y->num == 0) {
            lval_del(x); lval_del(y);
            x = lval_err("Division By Zero!"); break;
            }
            x->num /= y->num;
        }

        lval_del(y);
    }
    lval_del(a);

    return x;
}

lval* builtin_head(lenv* e, lval* a) {
    /* Check error conditions */
    LASSERT(a, a->count == 1,
        TOO_MANY_ARGUMENTS_EXCEPTION("head", a->count, 1));
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
        WRONG_TYPE_EXCEPTION("head", a->cell[0]->type, 0, ltype_name(LVAL_QEXPR)));
    LASSERT(a, a->cell[0]->count != 0,
        EMPTY_LIST_EXCEPTION("head"));

    /* Otherwise take first argument */
    lval* v = lval_take(a, 0);
    /* Delete all others */
    while (v->count > 1) { lval_del(lval_pop(v, 1)); }

    return v;
}

lval* builtin_tail(lenv* e, lval* a) {
    /* Check Error Conditions */
    LASSERT(a, a->count == 1,
        TOO_MANY_ARGUMENTS_EXCEPTION("tail", a->count, 1));
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
        WRONG_TYPE_EXCEPTION("tail", a->cell[0]->type, 0, ltype_name(LVAL_QEXPR)));
    LASSERT(a, a->cell[0]->count != 0,
        EMPTY_LIST_EXCEPTION("tail"));

    /* Take first element, delete pointer to it, return it. */
    lval* v = lval_take(a, 0);
    lval_del(lval_pop(v, 0));

    return v;
}

lval* builtin_list(lenv* e, lval* a) {
    a->type = LVAL_QEXPR;
    return a;
}

lval* builtin_eval(lenv* e, lval* a) {
    LASSERT(a, a->count == 1,
        TOO_MANY_ARGUMENTS_EXCEPTION("eval", a->count, 1));
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
        WRONG_TYPE_EXCEPTION("eval", a->cell[0]->type, 0, ltype_name(LVAL_QEXPR)));

    lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(e, x);
}

void lenv_put(lenv* e, lval* k, lval* v);

lval* lval_call(lenv* e, lval* f, lval* a) {

    /* If it's builtin, simply call it */
    if (f->builtin) { return f->builtin(e, a); }

    int given = a->count;
    int total = f->formals->count;

    while (a->count) {

        if (f->formals->count == 0) {
            lval_del(a);
            return lval_err(TOO_MANY_ARGUMENTS_EXCEPTION("<>", given, total));
        }

        lval* sym = lval_pop(f->formals, 0);

        /* Special case: list after & symbol. Same as ... in c kinda */
        if (strcmp(sym->sym, "&") == 0) {
            if (f->formals->count != 1) {
                lval_del(a);
                return lval_err("Function format invalid. "
                "Symbol '&' Not followed by single symbol.");
            }
            /* Next symbol is a builtin list of all extra arguments */
            lval* nsym = lval_pop(f->formals, 0);
            lenv_put(f->env, nsym, builtin_list(e, a));
            lval_del(sym);
            lval_del(nsym);
            break;
        }

        lval* val = lval_pop(a, 0);
        lenv_put(f->env, sym, val);

        lval_del(sym);
        lval_del(val);
    }

    lval_del(a);

    /* If '&' remains in formal list, bind to empty list {} */
    if (f->formals->count > 0 && strcmp(f->formals->cell[0]->sym, "&") == 0) {

        if (f->formals->count != 2) {
            return lval_err("Function format invalid. "
                "Symbol '&' not followed by single symbol.");
        }

        /* Pop and delete & */
        lval_del(lval_pop(f->formals, 0));

        lval* sym = lval_pop(f->formals, 0);
        lval* val = lval_qexpr();

        lenv_put(f->env, sym, val);
        lval_del(sym);
        lval_del(val);
    }

    /* If all formals have been bound, evaluate */
    if (f->formals->count == 0) {
        f->env->parent = e;
        return builtin_eval(
            f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
    }
    /* Else return a copy of itself with some argumets filled */
    return lval_copy(f);
}

lval* builtin_join(lenv* e, lval* a) {
    for (int i = 0; i < a->count; i++) {
        LASSERT(a, a->cell[i]->type == LVAL_QEXPR,
            WRONG_TYPE_EXCEPTION(
                "join", a->cell[i]->type, i, ltype_name(LVAL_QEXPR)));
    }

    lval* x = lval_pop(a, 0);

    while (a->count) {
        x = lval_join(x, lval_pop(a, 0));
    }
    lval_del(a);
    return x;
}

lval* builtin_len(lenv* e, lval* a) {
    LASSERT(a, a->count == 1,
        TOO_MANY_ARGUMENTS_EXCEPTION("len", a->count, 1));
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
        WRONG_TYPE_EXCEPTION("len", a->cell[0]->type, 0, ltype_name(LVAL_QEXPR)));

    lval* x = lval_take(a, 0);

    return lval_num(x->count);
}

lval* builtin_add(lenv* e, lval* a) {
  return builtin_op(e, a, "+");
}

lval* builtin_sub(lenv* e, lval* a) {
  return builtin_op(e, a, "-");
}

lval* builtin_mul(lenv* e, lval* a) {
  return builtin_op(e, a, "*");
}

lval* builtin_div(lenv* e, lval* a) {
  return builtin_op(e, a, "/");
}

lval* builtin_lambda(lenv* e, lval* a) {

    LASSERT_ARG_NUM("lambda", a, 2);
    LASSERT_TYPE("lambda", a, 0, LVAL_QEXPR);
    LASSERT_TYPE("lambda", a, 1, LVAL_QEXPR);

    /* Ensure first QEXPR only contains symbols */
    for (int i = 0; i < a->cell[0]->count; i++) {
        LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
            "Cannot define non-symbol. Got %s, expected %s",
            ltype_name(a->cell[0]->cell[i]->type), ltype_name(LVAL_SYM));
    }

    lval* formals = lval_pop(a, 0);
    lval* body = lval_pop(a, 0);
    lval_del(a);

    return lval_lambda(formals, body);
}

lval* lenv_get(lenv* e, lval* k) {

    /* Iterate over stuff in environment e */
    for (int i = 0; i < e->count; i++) {
        /* Check if the stored string matches the symbol string.
        If it does, return a copy of that value */
        if (strcmp(e->syms[i], k->sym) == 0) {
            return lval_copy(e->vals[i]);
        }
    }
    /* Recursively searches parents until a match is found */
    if (e->parent) {
        return lenv_get(e->parent, k);
    }
    /* If no matching symbol is found, throw error. */
    return lval_err("Unbound symbol! '%s'", k->sym);
}

void lenv_put(lenv* e, lval* k, lval* v) {
    /* Defining in local environment */

    /* Iterate over all vals in the envronment.
    If it already exists, overwrite it. */
    for (int i = 0; i < e->count; i++) {
        if (strcmp(e->syms[i], k->sym) == 0) {
            lval_del(e->vals[i]);
            e->vals[i] = lval_copy(v);
            return;
        }
    }

    /* If no existing entry is found. Allocate for a new one */
    e->count++;
    e->vals = realloc(e->vals, sizeof(lval*) * e->count);
    e->syms = realloc(e->syms, sizeof(char*) * e->count);

    /* Copy args to new location */
    e->vals[e->count-1] = lval_copy(v);
    e->syms[e->count-1] = malloc(strlen(k->sym) + 1);
    strcpy(e->syms[e->count-1], k->sym);
}

void lenv_def(lenv* e, lval* k, lval* v) {
    /* Define in global environ */
    while (e->parent) {
        e = e->parent;
    }
    lenv_put(e, k, v);
}

lval* builtin_get_env(lenv* e, lval* a) {
    lval* env_list = lval_sexpr();

    for (int i = 0; i < e->count-1; i++) {
        lval_add(env_list, lval_sym(e->syms[i]));
    }

    return env_list; /* {head list tail etc} */
}

lval* builtin_var(lenv* e, lval* a, char* func) {
    LASSERT_TYPE(func, a, 0, LVAL_QEXPR);

    lval* syms = a->cell[0];

    for (int i = 0; i < syms->count; i++) {
        LASSERT(a, (syms->cell[i]->type == LVAL_SYM),
            "Function '%s' cannot define non-symbol. "
            "Got %s, Expected %s.", func,
            ltype_name(syms->cell[i]->type),
            ltype_name(LVAL_SYM));
    }
    LASSERT(a, (syms->count == a->count-1),
        "Function '%s' passed too many arguments for symbols. "
        "Got %i, Expected %i.", func, syms->count, a->count-1);

    /* DO THE CHECK ONCE INSTEAD!!! */
    for (int i = 0; i < syms->count; i++) {
    /* If 'def' define in globally. If 'put' define in locally */
        if (strcmp(func, "def") == 0) {
            lenv_def(e, syms->cell[i], a->cell[i+1]);
        }

        if (strcmp(func, "=") == 0) {
            lenv_put(e, syms->cell[i], a->cell[i+1]);
        }
    }

    lval_del(a);
    return lval_sexpr();
}

/* Global environ */
lval* builtin_def(lenv* e, lval* a) {
  return builtin_var(e, a, "def");
}

/* Local environ*/
lval* builtin_put(lenv* e, lval* a) {
  return builtin_var(e, a, "=");
}

void lenv_add_builtin(lenv* e, char* name, lbuiltin func) {
    lval* k = lval_sym(name);
    lval* v = lval_fun(func);
    lenv_put(e, k, v);
    lval_del(k); lval_del(v);
}

void lenv_add_builtins(lenv* e) {
    /* List functions */
    lenv_add_builtin(e, "list", builtin_list);
    lenv_add_builtin(e, "head", builtin_head);
    lenv_add_builtin(e, "tail", builtin_tail);
    lenv_add_builtin(e, "eval", builtin_eval);
    lenv_add_builtin(e, "join", builtin_join);
    lenv_add_builtin(e, "len", builtin_len);

    /* Conditionals */
    lenv_add_builtin(e, "if", builtin_if);
    lenv_add_builtin(e, "==", builtin_eq);
    lenv_add_builtin(e, "!=", builtin_neq);
    lenv_add_builtin(e, ">",  builtin_gt);
    lenv_add_builtin(e, "<",  builtin_lt);
    lenv_add_builtin(e, ">=", builtin_geq);
    lenv_add_builtin(e, "<=", builtin_leq);

    /* Mathematical functions */
    lenv_add_builtin(e, "+", builtin_add);
    lenv_add_builtin(e, "-", builtin_sub);
    lenv_add_builtin(e, "*", builtin_mul);
    lenv_add_builtin(e, "/", builtin_div);

    /* User defined functions */
    lenv_add_builtin(e, "def", builtin_def);
    lenv_add_builtin(e, "=", builtin_put);
    lenv_add_builtin(e, "\\", builtin_lambda);

    /* Utils */
    lenv_add_builtin(e, "get_env", builtin_get_env);
}

lval* builtin(lenv* e, lval* a, char* func) {
    if (strcmp("list", func) == 0) { return builtin_list(e, a); }
    if (strcmp("head", func) == 0) { return builtin_head(e, a); }
    if (strcmp("tail", func) == 0) { return builtin_tail(e, a); }
    if (strcmp("join", func) == 0) { return builtin_join(e, a); }
    if (strcmp("eval", func) == 0) { return builtin_eval(e, a); }
    if (strcmp("len", func) == 0) { return builtin_len(e, a); }
    if (strstr("+-/*", func)) { return builtin_op(e, a, func); }
    lval_del(a);
    return lval_err("Unknown function!");
}

lval* lval_eval_sexpr(lenv* e, lval* v) {

    /* Evaluate children first */
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = lval_eval(e, v->cell[i]);
    }

    /* Erorr checking */
    for (int i = 0; i < v->count; i++) {
        if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
    }

    /* Empty Expression */
    if (v->count == 0) { return v; }

    /* Single Expression */
    if (v->count == 1) { return lval_take(v, 0); }

    /* Ensure First Element a function after evaluation */
    lval* f = lval_pop(v, 0);
    if (f->type != LVAL_FUN) {
        lval_del(f); lval_del(v);
        return lval_err("first element is not a function!");
    }
    /* Call the function that f points to, with the given environment */
    lval* result = lval_call(e, f, v);
    lval_del(f);
    return result;
}

lval* lval_eval(lenv* e, lval* v) {
    /* Get value of symbol from the given environment */
    if (v->type == LVAL_SYM) {
        lval* x = lenv_get(e, v);
        lval_del(v);
        return x;
    }
    /* Evaluate Sexpressions */
    if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(e, v); }
    /* All other lval types remain the same */
    return v;
}

int main(int argc, char** argv) {

    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr  = mpc_new("sexpr");
    mpc_parser_t* Qexpr  = mpc_new("qexpr");
    mpc_parser_t* Expr   = mpc_new("expr");
    mpc_parser_t* Lispy  = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
    "                                                      \
        number : /-?[0-9]+/ ;                              \
        symbol : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;        \
        sexpr  : '(' <expr>* ')' ;                         \
        qexpr  : '{' <expr>* '}' ;                         \
        expr   : <number> | <symbol> | <sexpr> | <qexpr> ; \
        lispy  : /^/ <expr>* /$/ ;                         \
    ",
    Number, Symbol, Sexpr, Qexpr, Expr, Lispy);


  /* Print Version and Exit Information */
  puts("TysonLang Version 0.0.0.2.1");
  puts("Press Ctrl+c to Exit\n");

  lenv* e = lenv_new();
  lenv_add_builtins(e);
  /* In a never ending loop */
  while (1) {

    /* Output our prompt and get input */
    char* input = readline("TysonLang> ");

    /* Add input to history */
    add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
        lval* x = lval_eval(e, lval_read(r.output));
        lval_println(x);
        lval_del(x);

        mpc_ast_delete(r.output);
    }
    else {
        mpc_err_print(r.error);
        mpc_err_delete(r.error);
    }

    /* Free retrieved input */
    free(input);

  }
  lenv_del(e);
  mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Lispy);
  return 0;
}