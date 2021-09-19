#pragma once
#include <string>

enum class TokenKind {
    Number,
    Plus, Minus,
    Star, Slash,
    Caret,
    LParen, RParen,
    Ident,
    End
};

struct Token {
    TokenKind kind;
    double value{};
    std::string name{};
};
