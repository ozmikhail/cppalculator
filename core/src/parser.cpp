#include "parser.hpp"
#include "units.hpp"
#include <cmath>
#include <cstdint>
#include <limits>
#include <set>

static const std::set<std::string> RESERVED = {"pi", "e", "tau", "inf", "ans"};

static std::int64_t toInt(double v, const char* fn, std::size_t col) {
    if (!std::isfinite(v) || v != std::floor(v) || std::abs(v) >= 9.2e18)
        throw CalcError(std::string(fn) + ": argument must be a finite integer", col);
    return static_cast<std::int64_t>(v);
}

Parser::Parser(std::vector<Token> tokens, double ans, std::map<std::string, double>& vars)
    : m_tokens(std::move(tokens)), m_ans(ans), m_vars(vars) {}

double Parser::parse() {
    if (m_tokens.size() >= 3 &&
            m_tokens[0].kind == TokenKind::Ident &&
            m_tokens[1].kind == TokenKind::Assign) {
        const std::string& name = m_tokens[0].name;
        if (RESERVED.count(name))
            throw CalcError("cannot assign to reserved name '" + name + "'", m_tokens[0].col);
        m_pos = 2;
        double val = parseExpr();
        if (current().kind != TokenKind::End) errHere("unexpected token after expression");
        m_vars[name] = val;
        return val;
    }
    double result = parseExpr();
    if (current().kind != TokenKind::End) errHere("unexpected token after expression");
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

static bool startsPrimary(TokenKind k) {
    return k == TokenKind::Number || k == TokenKind::LParen || k == TokenKind::Ident;
}

double Parser::parseMultiply() {
    double left = parseUnary();
    while (true) {
        TokenKind k = current().kind;
        TokenKind op;
        if (k == TokenKind::Star || k == TokenKind::Slash || k == TokenKind::Percent) {
            op = k;
            advance();
        } else if (startsPrimary(k)) {
            op = TokenKind::Star;
        } else {
            break;
        }
        std::size_t opCol = current().col;
        double right = parseUnary();
        if (op != TokenKind::Star && right == 0.0)
            throw CalcError(op == TokenKind::Slash ? "division by zero" : "modulo by zero", opCol);
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
    double base = parsePostfix();
    if (current().kind == TokenKind::Caret) {
        advance();
        // right-associative: recurse into parseUnary
        return std::pow(base, parseUnary());
    }
    return base;
}

double Parser::parsePostfix() {
    double val = parsePrimary();
    while (current().kind == TokenKind::Bang) {
        std::size_t bangCol = current().col;
        advance();
        if (!std::isfinite(val) || val < 0.0)
            throw CalcError("factorial requires a non-negative finite number", bangCol);
        if (val > 170.0)
            throw CalcError("factorial overflow (argument > 170)", bangCol);
        val = std::tgamma(val + 1.0);
    }
    return val;
}

double Parser::parsePrimary() {
    if (current().kind == TokenKind::LParen) {
        advance();
        double val = parseExpr();
        if (current().kind != TokenKind::RParen) errHere("expected ')'");
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
        std::size_t identCol = current().col;
        advance();
        if (name == "pi") return std::acos(-1.0);
        if (name == "e") return std::exp(1.0);
        if (name == "tau") return 2.0 * std::acos(-1.0);
        if (name == "inf") return std::numeric_limits<double>::infinity();
        if (name == "ans") return m_ans;
        const auto& units = builtinUnits();
        auto uit = units.find(name);
        if (uit != units.end()) return uit->second;
        auto vit = m_vars.find(name);
        if (vit != m_vars.end()) return vit->second;
        if (current().kind != TokenKind::LParen)
            throw CalcError("unknown identifier: " + name, identCol);
        return parseCall(name, identCol);
    }
    errHere("expected a number or '('");
}

double Parser::parseCall(const std::string& name, std::size_t nameCol) {
    std::size_t callCol = nameCol;
    advance();
    double a = parseExpr();

    if (current().kind == TokenKind::Comma) {
        advance();
        double b = parseExpr();

        if (current().kind == TokenKind::Comma) {
            advance();
            double c = parseExpr();
            if (current().kind != TokenKind::RParen) errHere("expected ')'");
            advance();
            if (name == "convert") {
                if (c == 0.0) throw CalcError("convert: destination unit is zero", callCol);
                return a * b / c;
            }
            throw CalcError("'" + name + "' does not take three arguments", callCol);
        }

        if (current().kind != TokenKind::RParen) errHere("expected ')'");
        advance();
        if (name == "min") return std::min(a, b);
        if (name == "max") return std::max(a, b);
        if (name == "pow") return std::pow(a, b);
        if (name == "atan2") return std::atan2(a, b);
        if (name == "hypot") return std::hypot(a, b);
        if (name == "log") {
            if (b <= 0.0 || b == 1.0)
                throw CalcError("log base must be positive and not 1", callCol);
            return std::log(a) / std::log(b);
        }
        if (name == "band") return static_cast<double>(toInt(a,"band",callCol) & toInt(b,"band",callCol));
        if (name == "bor") return static_cast<double>(toInt(a,"bor", callCol) | toInt(b,"bor", callCol));
        if (name == "bxor") return static_cast<double>(toInt(a,"bxor",callCol) ^ toInt(b,"bxor",callCol));
        if (name == "shl") {
            std::int64_t x = toInt(a, "shl", callCol), n = toInt(b, "shl", callCol);
            if (n < 0 || n > 62) throw CalcError("shl: shift out of range [0,62]", callCol);
            return static_cast<double>(x << n);
        }
        if (name == "shr") {
            std::int64_t x = toInt(a, "shr", callCol), n = toInt(b, "shr", callCol);
            if (n < 0 || n > 62) throw CalcError("shr: shift out of range [0,62]", callCol);
            return static_cast<double>(x >> n);
        }
        throw CalcError("'" + name + "' does not take two arguments", callCol);
    }

    if (current().kind != TokenKind::RParen) errHere("expected ')'");
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
    if (name == "bnot") return static_cast<double>(~toInt(a, "bnot", callCol));
    if (name == "c_to_f") return a * 9.0/5.0 + 32.0;
    if (name == "f_to_c") return (a - 32.0) * 5.0/9.0;
    if (name == "c_to_k") return a + 273.15;
    if (name == "k_to_c") return a - 273.15;
    if (name == "f_to_k") return (a - 32.0) * 5.0/9.0 + 273.15;
    if (name == "k_to_f") return (a - 273.15) * 9.0/5.0 + 32.0;
    throw CalcError("unknown function: " + name, callCol);
}

const Token& Parser::current() const { return m_tokens[m_pos]; }
const Token& Parser::advance() { return m_tokens[m_pos++]; }
void Parser::errHere(const std::string& msg) const { throw CalcError(msg, current().col); }
