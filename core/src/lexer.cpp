#include "lexer.hpp"
#include <cctype>
#include <stdexcept>

Lexer::Lexer(std::string input) : m_input(std::move(input)) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    Token tok;
    do {
        tok = nextToken();
        tokens.push_back(tok);
    } while (tok.kind != TokenKind::End);
    return tokens;
}

Token Lexer::nextToken() {
    skipWhitespace();
    if (atEnd()) return {TokenKind::End};

    char c = current();
    if (std::isalpha(static_cast<unsigned char>(c))) return readIdentifier();
    if (std::isdigit(c) || c == '.') return readNumber();

    advance();
    switch (c) {
        case '+': return {TokenKind::Plus};
        case '-': return {TokenKind::Minus};
        case '*': return {TokenKind::Star};
        case '/': return {TokenKind::Slash};
        case '^': return {TokenKind::Caret};
        case '(': return {TokenKind::LParen};
        case ')': return {TokenKind::RParen};
        default:
            throw std::runtime_error(std::string("unexpected character: ") + c);
    }
}

Token Lexer::readIdentifier() {
    std::size_t start = m_pos;
    while (!atEnd() && std::isalnum(static_cast<unsigned char>(current())))
        advance();
    return {TokenKind::Identifier, 0.0, m_input.substr(start, m_pos - start)};
}

Token Lexer::readNumber() {
    std::size_t start = m_pos;
    bool hasDot = false;
    while (!atEnd() && (std::isdigit(current()) || (current() == '.' && !hasDot))) {
        if (current() == '.') hasDot = true;
        advance();
    }
    double val = std::stod(m_input.substr(start, m_pos - start));
    return {TokenKind::Number, val};
}

void Lexer::skipWhitespace() {
    while (!atEnd() && std::isspace(current())) advance();
}

char Lexer::current() const { return m_input[m_pos]; }
char Lexer::advance() { return m_input[m_pos++]; }
bool Lexer::atEnd() const { return m_pos >= m_input.size(); }
