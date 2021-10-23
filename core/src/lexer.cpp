#include "lexer.hpp"
#include <cctype>

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
    std::size_t col = m_pos;
    if (atEnd()) return {TokenKind::End, 0.0, {}, col};

    char c = current();
    if (std::isdigit(c) || c == '.') {
        Token t = readNumber();
        t.col = col;
        return t;
    }
    if (std::isalpha(c) || c == '_') {
        Token t = readIdent();
        t.col = col;
        return t;
    }

    advance();
    switch (c) {
        case '+': return {TokenKind::Plus, 0.0, {}, col};
        case '-': return {TokenKind::Minus, 0.0, {}, col};
        case '*': return {TokenKind::Star, 0.0, {}, col};
        case '/': return {TokenKind::Slash, 0.0, {}, col};
        case '%': return {TokenKind::Percent, 0.0, {}, col};
        case '^': return {TokenKind::Caret, 0.0, {}, col};
        case '!': return {TokenKind::Bang, 0.0, {}, col};
        case '(': return {TokenKind::LParen, 0.0, {}, col};
        case ')': return {TokenKind::RParen, 0.0, {}, col};
        case ',': return {TokenKind::Comma, 0.0, {}, col};
        case '=': return {TokenKind::Assign, 0.0, {}, col};
        default:
            throw CalcError(std::string("unexpected character: ") + c, col);
    }
}

Token Lexer::readNumber() {
    std::size_t litStart = m_pos;
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
                throw CalcError("invalid numeric literal", litStart);
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
