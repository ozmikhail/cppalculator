#include "parser.hpp"
#include <stdexcept>

Parser::Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

double Parser::parse() {
    double result = parseExpr();
    if (current().kind != TokenKind::End)
        throw std::runtime_error("unexpected token after expression");
    return result;
}

double Parser::parseExpr() {
    return parseAddition();
}

double Parser::parseAddition() {
    double left = parseMultiply();
    while (current().kind == TokenKind::Plus || current().kind == TokenKind::Minus) {
        bool isPlus = current().kind == TokenKind::Plus;
        advance();
        double right = parseMultiply();
        left = isPlus ? left + right : left - right;
    }
    return left;
}

double Parser::parseMultiply() {
    double left = parsePrimary();
    while (current().kind == TokenKind::Star || current().kind == TokenKind::Slash) {
        bool isMul = current().kind == TokenKind::Star;
        advance();
        double right = parsePrimary();
        if (!isMul && right == 0.0)
            throw std::runtime_error("division by zero");
        left = isMul ? left * right : left / right;
    }
    return left;
}

double Parser::parsePrimary() {
    if (current().kind == TokenKind::Number) {
        double val = current().value;
        advance();
        return val;
    }
    throw std::runtime_error("expected a number");
}

const Token& Parser::current() const { return m_tokens[m_pos]; }
const Token& Parser::advance() { return m_tokens[m_pos++]; }
