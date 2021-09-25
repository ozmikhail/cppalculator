#include "parser.hpp"
#include <cmath>
#include <limits>
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
    double left = parseUnary();
    while (current().kind == TokenKind::Star || current().kind == TokenKind::Slash) {
        bool isMul = current().kind == TokenKind::Star;
        advance();
        double right = parseUnary();
        if (!isMul && right == 0.0)
            throw std::runtime_error("division by zero");
        left = isMul ? left * right : left / right;
    }
    return left;
}

double Parser::parseUnary() {
    if (current().kind == TokenKind::Minus) {
        advance();
        return -parseUnary();
    }
    return parsePower();
}

double Parser::parsePower() {
    double base = parsePrimary();
    if (current().kind == TokenKind::Caret) {
        advance();
        // recurse into parseUnary (not parsePower) to stay right-associative
        double exp = parseUnary();
        return std::pow(base, exp);
    }
    return base;
}

double Parser::parsePrimary() {
    if (current().kind == TokenKind::LParen) {
        advance();
        double val = parseExpr();
        if (current().kind != TokenKind::RParen)
            throw std::runtime_error("expected ')'");
        advance();
        return val;
    }
    if (current().kind == TokenKind::Number) {
        double val = current().value;
        advance();
        return val;
    }
    if (current().kind == TokenKind::Ident) {
        std::string name = current().name;
        advance();
        if (name == "pi") return std::acos(-1.0);
        if (name == "e") return std::exp(1.0);
        if (name == "tau") return 2.0 * std::acos(-1.0);
        if (name == "inf") return std::numeric_limits<double>::infinity();
        return parseCall(name);
    }
    throw std::runtime_error("expected a number or '('");
}

double Parser::parseCall(const std::string& name) {
    if (current().kind != TokenKind::LParen)
        throw std::runtime_error("unknown identifier: " + name);
    advance();
    double arg = parseExpr();
    if (current().kind != TokenKind::RParen)
        throw std::runtime_error("expected ')'");
    advance();
    if (name == "sin") return std::sin(arg);
    if (name == "cos") return std::cos(arg);
    if (name == "tan") return std::tan(arg);
    if (name == "asin") return std::asin(arg);
    if (name == "acos") return std::acos(arg);
    if (name == "atan") return std::atan(arg);
    if (name == "sinh") return std::sinh(arg);
    if (name == "cosh") return std::cosh(arg);
    if (name == "tanh") return std::tanh(arg);
    if (name == "sqrt") return std::sqrt(arg);
    if (name == "cbrt") return std::cbrt(arg);
    if (name == "exp") return std::exp(arg);
    if (name == "ln") return std::log(arg);
    if (name == "log") return std::log10(arg);
    if (name == "log10") return std::log10(arg);
    if (name == "log2") return std::log2(arg);
    if (name == "abs") return std::abs(arg);
    if (name == "ceil") return std::ceil(arg);
    if (name == "floor") return std::floor(arg);
    if (name == "round") return std::round(arg);
    if (name == "deg") return arg * (180.0 / std::acos(-1.0));
    if (name == "rad") return arg * (std::acos(-1.0) / 180.0);
    if (name == "sign") return static_cast<double>((arg > 0.0) - (arg < 0.0));
    throw std::runtime_error("unknown function: " + name);
}

const Token& Parser::current() const { return m_tokens[m_pos]; }
const Token& Parser::advance() { return m_tokens[m_pos++]; }
