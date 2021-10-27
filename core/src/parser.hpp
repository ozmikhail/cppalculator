#pragma once
#include "token.hpp"
#include <map>
#include <string>
#include <vector>

struct UserFunc {
    std::vector<std::string> params;
    std::vector<Token> body;
};

using FuncMap = std::map<std::string, UserFunc>;

class Parser {
public:
    Parser(std::vector<Token> tokens, double ans,
           std::map<std::string, double>& vars,
           FuncMap& funcs,
           int depth = 0);
    double parse();

    static constexpr int MAX_DEPTH = 256;

    enum class DefineResult { Defined };
    static DefineResult defineFunction(std::vector<Token> tokens, FuncMap& funcs);

private:
    std::vector<Token> m_tokens;
    std::size_t m_pos{};
    double m_ans;
    std::map<std::string, double>& m_vars;
    FuncMap& m_funcs;
    int m_depth;

    double parseExpr();
    double parseAddition();
    double parseMultiply();
    double parseUnary();
    double parsePower();
    double parsePostfix();
    double parsePrimary();
    double parseCall(const std::string& name, std::size_t nameCol);
    double callUserFunc(const UserFunc& fn, const std::string& name,
                        const std::vector<double>& args, std::size_t col);

    const Token& current() const;
    const Token& advance();
    [[noreturn]] void errHere(const std::string& msg) const;
};
