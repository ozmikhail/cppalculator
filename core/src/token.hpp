#pragma once
#include <stdexcept>
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
    std::size_t col{};
};

class CalcError : public std::runtime_error {
public:
    CalcError(std::string msg, std::size_t col)
        : std::runtime_error(std::move(msg)), m_col(col) {}
    std::size_t col() const noexcept { return m_col; }
private:
    std::size_t m_col;
};
