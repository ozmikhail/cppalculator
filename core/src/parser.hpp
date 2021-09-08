#pragma once
#include "token.hpp"
#include <vector>

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    double parse();

private:
    std::vector<Token> m_tokens;
    std::size_t m_pos{};

    double parseExpr();
    double parseAddition();
    double parseMultiply();
    double parsePrimary();

    const Token& current() const;
    const Token& advance();
};
