#pragma once
#include "token.hpp"
#include <string>
#include <vector>

class Parser {
public:
    explicit Parser(std::vector<Token> tokens, double ans = 0.0);
    double parse();

private:
    std::vector<Token> m_tokens;
    std::size_t m_pos{};
    double m_ans;

    double parseExpr();
    double parseAddition();
    double parseMultiply();
    double parseUnary();
    double parsePower();
    double parsePrimary();
    double parseCall(const std::string& name);

    const Token& current() const;
    const Token& advance();
};
