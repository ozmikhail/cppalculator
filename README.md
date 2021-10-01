# cppalculator

> A pocket-sized C++ expression REPL — types math the way you write it, with constants, functions, and a memory of what you just asked.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](CMakeLists.txt)

**cppalculator is a tiny calculator for the terminal.** It tokenizes and parses arithmetic expressions with proper precedence, supports parenthesized subexpressions, math constants, and built-in functions, and remembers your previous answer as `ans`. Built for people who reach for `bc` but want something slightly nicer.

## Features

- Recursive-descent parser with `+`, `-`, `*`, `/`, unary minus, and right-associative `^`
- Built-in constants `pi`, `e`, `tau` and one- and two-argument math functions
- Session-aware: `ans` for previous result, `hist` and `!N` recall
- Pointed error messages that show the offending column

## Quick Start

```bash
git clone https://github.com/ozmikhail/cppalculator.git
cd cppalculator

cmake -B build
cmake --build build

./build/calc
```

## Directory design conventions

**Philosophy:**
    * **Feature / Domain oriented** over technical layering
    * **High cohesion, low coupling** between modules
    * Clear separation between:
      - Lexing and tokenization
      - Parsing and evaluation
      - Session and REPL
    * Easy navigation and testability
    * Scalable from a single binary to richer tooling

```bash
.
├── CMakeLists.txt
├── CMakePresets.json
├── README.md
├── .gitattributes
├── main.cpp                    # REPL entry point
└── core/
    ├── CMakeLists.txt
    └── src/
        ├── token.hpp           # Token kinds and shape
        ├── lexer.hpp / .cpp    # Source → tokens
        ├── parser.hpp / .cpp   # Tokens → values, with ans context
        └── session.hpp / .cpp  # History + previous-answer context
```
