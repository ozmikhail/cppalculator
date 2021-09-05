#pragma once
#include "token.hpp"
#include <string>
#include <vector>

class Lexer {
public:
    explicit Lexer(std::string input);
    std::vector<Token> tokenize();

private:
    std::string m_input;
    std::size_t m_pos{};

    Token nextToken();
    Token readNumber();
    void skipWhitespace();
    char current() const;
    char advance();
    bool atEnd() const;
};
