#pragma once

enum class TokenKind {
    Number,
    Plus, Minus,
    Star, Slash,
    Caret,
    LParen, RParen,
    End
};

struct Token {
    TokenKind kind;
    double value{};
};
