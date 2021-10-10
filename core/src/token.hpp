#pragma once
#include <string>

enum class TokenKind {
    Number,
    Plus, Minus,
    Star, Slash, Percent,
    Caret,
    Bang,
    LParen, RParen,
    Comma,
    Assign,
    Ident,
    End
};

struct Token {
    TokenKind kind;
    double value{};
    std::string name{};
};
