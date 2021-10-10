#include "parser.hpp"
#include "units.hpp"
#include <cmath>
#include <limits>
#include <set>
#include <stdexcept>

static const std::set<std::string> RESERVED = {"pi", "e", "tau", "inf", "ans"};

Parser::Parser(std::vector<Token> tokens, double ans, std::map<std::string, double>& vars)
    : m_tokens(std::move(tokens)), m_ans(ans), m_vars(vars) {}

double Parser::parse() {
    // Detect assignment: Ident '=' Expr
    if (m_tokens.size() >= 3 &&
            m_tokens[0].kind == TokenKind::Ident &&
            m_tokens[1].kind == TokenKind::Assign) {
        const std::string& name = m_tokens[0].name;
        if (RESERVED.count(name))
            throw std::runtime_error("cannot assign to reserved name '" + name + "'");
        m_pos = 2;
        double val = parseExpr();
        if (current().kind != TokenKind::End)
            throw std::runtime_error("unexpected token after expression");
        m_vars[name] = val;
        return val;
    }
    double result = parseExpr();
    if (current().kind != TokenKind::End)
        throw std::runtime_error("unexpected token after expression");
    return result;
}

double Parser::parseExpr() { return parseAddition(); }

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
    while (current().kind == TokenKind::Star ||
           current().kind == TokenKind::Slash ||
           current().kind == TokenKind::Percent) {
        TokenKind op = current().kind;
        advance();
        double right = parseUnary();
        if (op != TokenKind::Star && right == 0.0)
            throw std::runtime_error(op == TokenKind::Slash
                ? "division by zero" : "modulo by zero");
        switch (op) {
            case TokenKind::Star: left = left * right; break;
            case TokenKind::Slash: left = left / right; break;
            case TokenKind::Percent: left = std::fmod(left, right); break;
            default: break;
        }
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
        // right-associative: recurse into parseUnary
        return std::pow(base, parseUnary());
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
        // built-in constants
        if (name == "pi") return std::acos(-1.0);
        if (name == "e") return std::exp(1.0);
        if (name == "tau") return 2.0 * std::acos(-1.0);
        if (name == "inf") return std::numeric_limits<double>::infinity();
        if (name == "ans") return m_ans;
        // built-in unit constants
        const auto& units = builtinUnits();
        auto uit = units.find(name);
        if (uit != units.end()) return uit->second;
        // user variables
        auto vit = m_vars.find(name);
        if (vit != m_vars.end()) return vit->second;
        // function call
        return parseCall(name);
    }
    throw std::runtime_error("expected a number or '('");
}

double Parser::parseCall(const std::string& name) {
    if (current().kind != TokenKind::LParen)
        throw std::runtime_error("unknown identifier: " + name);
    advance();
    double a = parseExpr();

    if (current().kind == TokenKind::Comma) {
        advance();
        double b = parseExpr();

        // 3-arg branch
        if (current().kind == TokenKind::Comma) {
            advance();
            double c = parseExpr();
            if (current().kind != TokenKind::RParen)
                throw std::runtime_error("expected ')'");
            advance();
            if (name == "convert") {
                if (c == 0.0) throw std::runtime_error("convert: destination unit is zero");
                return a * b / c;
            }
            throw std::runtime_error("'" + name + "' does not take three arguments");
        }

        if (current().kind != TokenKind::RParen)
            throw std::runtime_error("expected ')'");
        advance();
        if (name == "min") return std::min(a, b);
        if (name == "max") return std::max(a, b);
        if (name == "pow") return std::pow(a, b);
        if (name == "atan2") return std::atan2(a, b);
        if (name == "hypot") return std::hypot(a, b);
        if (name == "log") {
            if (b <= 0.0 || b == 1.0)
                throw std::runtime_error("log base must be positive and not 1");
            return std::log(a) / std::log(b);
        }
        throw std::runtime_error("'" + name + "' does not take two arguments");
    }

    if (current().kind != TokenKind::RParen)
        throw std::runtime_error("expected ')'");
    advance();
    if (name == "sin") return std::sin(a);
    if (name == "cos") return std::cos(a);
    if (name == "tan") return std::tan(a);
    if (name == "asin") return std::asin(a);
    if (name == "acos") return std::acos(a);
    if (name == "atan") return std::atan(a);
    if (name == "sinh") return std::sinh(a);
    if (name == "cosh") return std::cosh(a);
    if (name == "tanh") return std::tanh(a);
    if (name == "sqrt") return std::sqrt(a);
    if (name == "cbrt") return std::cbrt(a);
    if (name == "exp") return std::exp(a);
    if (name == "ln") return std::log(a);
    if (name == "log") return std::log10(a);
    if (name == "log10") return std::log10(a);
    if (name == "log2") return std::log2(a);
    if (name == "abs") return std::abs(a);
    if (name == "ceil") return std::ceil(a);
    if (name == "floor") return std::floor(a);
    if (name == "round") return std::round(a);
    if (name == "deg") return a * (180.0 / std::acos(-1.0));
    if (name == "rad") return a * (std::acos(-1.0) / 180.0);
    if (name == "sign") return static_cast<double>((a > 0.0) - (a < 0.0));
    // temperature conversion
    if (name == "c_to_f") return a * 9.0/5.0 + 32.0;
    if (name == "f_to_c") return (a - 32.0) * 5.0/9.0;
    if (name == "c_to_k") return a + 273.15;
    if (name == "k_to_c") return a - 273.15;
    if (name == "f_to_k") return (a - 32.0) * 5.0/9.0 + 273.15;
    if (name == "k_to_f") return (a - 273.15) * 9.0/5.0 + 32.0;
    throw std::runtime_error("unknown function: " + name);
}

const Token& Parser::current() const { return m_tokens[m_pos]; }
const Token& Parser::advance() { return m_tokens[m_pos++]; }
