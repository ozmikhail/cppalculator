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
    if (std::isdigit(c) || c == '.') return readNumber();
    if (std::isalpha(c) || c == '_') return readIdent();

    advance();
    switch (c) {
        case '+': return {TokenKind::Plus};
        case '-': return {TokenKind::Minus};
        case '*': return {TokenKind::Star};
        case '/': return {TokenKind::Slash};
        case '%': return {TokenKind::Percent};
        case '^': return {TokenKind::Caret};
        case '!': return {TokenKind::Bang};
        case '(': return {TokenKind::LParen};
        case ')': return {TokenKind::RParen};
        case ',': return {TokenKind::Comma};
        case '=': return {TokenKind::Assign};
        default:
            throw std::runtime_error(std::string("unexpected character: ") + c);
    }
}

Token Lexer::readNumber() {
    // 0x..., 0b..., 0o... prefixed integer literals
    if (current() == '0' && m_pos + 1 < m_input.size()) {
        char prefix = m_input[m_pos + 1];
        int base = 0;
        if (prefix == 'x' || prefix == 'X') base = 16;
        else if (prefix == 'b' || prefix == 'B') base = 2;
        else if (prefix == 'o' || prefix == 'O') base = 8;
        if (base != 0) {
            m_pos += 2;
            std::size_t start = m_pos;
            auto isDigit = [base](char c) {
                if (base == 16) return static_cast<bool>(std::isxdigit(static_cast<unsigned char>(c)));
                if (base == 8) return c >= '0' && c <= '7';
                return c == '0' || c == '1';
            };
            while (!atEnd() && isDigit(current())) advance();
            if (m_pos == start)
                throw std::runtime_error("invalid numeric literal");
            std::string digits = m_input.substr(start, m_pos - start);
            return {TokenKind::Number, static_cast<double>(std::stoll(digits, nullptr, base))};
        }
    }
    std::size_t start = m_pos;
    bool hasDot = false;
    while (!atEnd() && (std::isdigit(current()) || (current() == '.' && !hasDot))) {
        if (current() == '.') hasDot = true;
        advance();
    }
    double val = std::stod(m_input.substr(start, m_pos - start));
    return {TokenKind::Number, val};
}

Token Lexer::readIdent() {
    std::size_t start = m_pos;
    while (!atEnd() && (std::isalnum(current()) || current() == '_'))
        advance();
    Token tok;
    tok.kind = TokenKind::Ident;
    tok.name = m_input.substr(start, m_pos - start);
    return tok;
}

void Lexer::skipWhitespace() {
    while (!atEnd() && std::isspace(current())) advance();
}

char Lexer::current() const { return m_input[m_pos]; }
char Lexer::advance() { return m_input[m_pos++]; }
bool Lexer::atEnd() const { return m_pos >= m_input.size(); }
