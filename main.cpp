#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include "lexer.hpp"
#include "parser.hpp"
#include "token.hpp"
#include "units.hpp"

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

static const std::vector<std::string> COMPLETIONS = {
    // REPL commands
    "quit", "exit", "history", "vars", "funcs", "units", "unset", "clear", "help",
    "hex", "bin", "oct", "save", "load",
    // constants
    "pi", "e", "tau", "inf", "ans",
    // 1-arg functions
    "sin(", "cos(", "tan(", "asin(", "acos(", "atan(",
    "sinh(", "cosh(", "tanh(",
    "sqrt(", "cbrt(", "exp(", "ln(", "log(", "log10(", "log2(",
    "abs(", "ceil(", "floor(", "round(", "deg(", "rad(", "sign(",
    "c_to_f(", "f_to_c(", "c_to_k(", "k_to_c(", "f_to_k(", "k_to_f(",
    "bnot(",
    "min(", "max(", "pow(", "atan2(", "hypot(",
    "band(", "bor(", "bxor(", "shl(", "shr(",
    // 3-arg unit conversion
    "convert(",
    // unit constants
    "km", "cm", "mm", "ft", "inch", "yd", "miles", "nmi",
    "gram", "mg", "lb", "oz", "tonne",
    "sec", "hr", "day", "week",
    "sqkm", "sqcm", "sqmm", "sqft", "sqin", "sqyd", "sqmi", "acre", "hectare",
    "litre", "ml", "cuft", "cuin", "gal", "gal_uk", "pint", "cup", "floz",
    "kph", "mph", "knot",
    "KB", "MB", "GB", "TB",
};

static std::map<std::string, double>* g_vars = nullptr;

#ifdef HAVE_READLINE
static char* completionGen(const char* text, int state) {
    static std::vector<std::string> matches;
    static int idx;
    if (state == 0) {
        matches.clear();
        idx = 0;
        std::string pfx(text);
        for (const auto& s : COMPLETIONS)
            if (s.compare(0, pfx.size(), pfx) == 0)
                matches.push_back(s);
        if (g_vars)
            for (const auto& [name, val] : *g_vars)
                if (name.compare(0, pfx.size(), pfx) == 0)
                    matches.push_back(name);
        // deduplicate (user var might overlap a completion entry)
        std::sort(matches.begin(), matches.end());
        matches.erase(std::unique(matches.begin(), matches.end()), matches.end());
    }
    if (idx < static_cast<int>(matches.size()))
        return strdup(matches[idx++].c_str());
    return nullptr;
}

static char** completionCb(const char* text, int /*start*/, int /*end*/) {
    rl_attempted_completion_over = 1;
    rl_completion_append_character = '\0';
    return rl_completion_matches(text, completionGen);
}
#endif

static std::string fmtNum(double v) {
    if (std::isfinite(v) && v == std::floor(v) && std::abs(v) < 1e15) {
        std::ostringstream oss;
        oss << static_cast<long long>(v);
        return oss.str();
    }
    std::ostringstream oss;
    oss << std::setprecision(10) << v;
    return oss.str();
}

static std::string fmtBase(double v, int base) {
    if (!std::isfinite(v) || v != std::floor(v) || std::abs(v) >= 9.2e18)
        throw std::runtime_error("base output requires a finite integer in range");
    auto n = static_cast<long long>(v);
    std::ostringstream oss;
    if (n < 0) { oss << '-'; n = -n; }
    auto u = static_cast<unsigned long long>(n);
    if (base == 16) oss << "0x" << std::hex << std::uppercase << u;
    else if (base == 8) oss << "0o" << std::oct << u;
    else /* base == 2 */ {
        std::string bits;
        if (u == 0) bits = "0";
        else while (u) { bits.push_back('0' + (u & 1)); u >>= 1; }
        std::reverse(bits.begin(), bits.end());
        oss << "0b" << bits;
    }
    return oss.str();
}

static void printHelp() {
    std::cout <<
        "\ncppalculator commands:\n"
        "  history            list expression history\n"
        "  vars               list user-defined variables\n"
        "  funcs              list user-defined functions\n"
        "  units              list built-in unit constants\n"
        "  unset <name>       remove a user variable or function\n"
        "  save <file>        write all variables and functions to file\n"
        "  load <file>        evaluate every line of file (variables, functions, etc.)\n"
        "  hex <expr>         evaluate and print result in hexadecimal\n"
        "  bin <expr>         evaluate and print result in binary\n"
        "  oct <expr>         evaluate and print result in octal\n"
        "  !N                 re-evaluate history entry N (1-based)\n"
        "  !!                 re-evaluate the most recent history entry\n"
        "  clear              clear expression history\n"
        "  help               show this message\n"
        "  quit / exit        exit\n"
        "\nOperators:  + - * / % ^ ! ( )\n"
        "  ! is postfix factorial (e.g. 5! = 120, fractional via gamma)\n"
        "Assignment: name = expression   (e.g. r = 3, area = pi * r^2)\n"
        "  'ans' holds the last result\n"
        "Function definition: name(p1, p2, ...) = expression\n"
        "  e.g. f(x) = x^2 + 1     hyp(a, b) = sqrt(a^2 + b^2)\n"
        "\nFunctions (1-arg):\n"
        "  sin cos tan asin acos atan sinh cosh tanh\n"
        "  sqrt cbrt exp ln log log10 log2\n"
        "  abs ceil floor round deg rad sign\n"
        "  c_to_f  f_to_c  c_to_k  k_to_c  f_to_k  k_to_f\n"
        "  bnot(x)  bitwise NOT (integers only)\n"
        "\nFunctions (2-arg):  min(x,y)  max(x,y)  pow(x,y)  atan2(y,x)  hypot(x,y)  log(x,base)\n"
        "  band(x,y)  bor(x,y)  bxor(x,y)  shl(x,n)  shr(x,n)\n"
        "\nUnit conversion:  convert(value, from_unit, to_unit)\n"
        "  e.g. convert(5, km, miles)  →  3.10686\n"
        "  Type 'units' to list all unit constants.\n"
        "\nConstants: pi  e  tau  inf  ans\n"
        "\nNumeric literals: decimals (3.14), hex (0xFF), binary (0b1010), octal (0o17)\n\n";
}

static void printUnits() {
    const auto& u = builtinUnits();
    auto row = [&](const char* name) {
        auto it = u.find(name);
        if (it != u.end())
            std::cout << "  " << std::left << std::setw(8) << name
                      << " = " << it->second << '\n';
    };
    std::cout << "Length (SI base = 1 metre):\n";
    for (auto n : {"km","cm","mm","ft","inch","yd","miles","nmi"}) row(n);
    std::cout << "Mass (SI base = 1 kilogram):\n";
    for (auto n : {"gram","mg","lb","oz","tonne"}) row(n);
    std::cout << "Time (SI base = 1 second):\n";
    for (auto n : {"sec","hr","day","week"}) row(n);
    std::cout << "Area (SI base = 1 square metre):\n";
    for (auto n : {"sqkm","sqcm","sqmm","sqft","sqin","sqyd","sqmi","acre","hectare"}) row(n);
    std::cout << "Volume (SI base = 1 cubic metre):\n";
    for (auto n : {"litre","ml","cuft","cuin","gal","gal_uk","pint","cup","floz"}) row(n);
    std::cout << "Speed (SI base = 1 m/s):\n";
    for (auto n : {"kph","mph","knot"}) row(n);
    std::cout << "Data (SI base = 1 byte, binary):\n";
    for (auto n : {"KB","MB","GB","TB"}) row(n);
    std::cout << "Temperature (non-linear — use conversion functions):\n"
                 "  c_to_f(x)  f_to_c(x)  c_to_k(x)  k_to_c(x)  f_to_k(x)  k_to_f(x)\n"
                 "Usage: convert(value, from_unit, to_unit)\n"
                 "  e.g. convert(5, km, miles)   convert(70, lb, kg)\n\n";
}

static bool looksLikeFuncDef(const std::vector<Token>& t) {
    if (t.size() < 6) return false;
    if (t[0].kind != TokenKind::Ident || t[1].kind != TokenKind::LParen) return false;
    std::size_t i = 2;
    if (t[i].kind != TokenKind::RParen) {
        while (true) {
            if (t[i].kind != TokenKind::Ident) return false;
            ++i;
            if (i >= t.size()) return false;
            if (t[i].kind == TokenKind::Comma) { ++i; continue; }
            break;
        }
    }
    if (t[i].kind != TokenKind::RParen) return false;
    ++i;
    return i < t.size() && t[i].kind == TokenKind::Assign;
}

int main() {
    std::map<std::string, double> vars;
    FuncMap funcs;
    g_vars = &vars;

#ifdef HAVE_READLINE
    rl_attempted_completion_function = completionCb;
    using_history();
#endif

    std::cout << "cppalculator — type 'help' for commands or 'quit' to exit\n";

    std::vector<std::pair<std::string, double>> history;
    double ans = 0.0;

    while (true) {
        std::string line;

#ifdef HAVE_READLINE
        char* raw = readline("> ");
        if (!raw) break;
        line = raw;
        if (!line.empty()) add_history(raw);
        free(raw);
#else
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) break;
#endif

        if (line.empty()) continue;
        if (line == "quit" || line == "exit") break;

        if (line.size() >= 2 && line[0] == '!' && line != "!") {
            std::size_t idx = 0;
            bool valid = false;
            if (line == "!!") {
                if (history.empty()) {
                    std::cerr << "no history yet\n";
                    continue;
                }
                idx = history.size();
                valid = true;
            } else {
                try {
                    std::size_t consumed = 0;
                    long n = std::stol(line.substr(1), &consumed);
                    if (consumed + 1 == line.size() && n >= 1) {
                        idx = static_cast<std::size_t>(n);
                        valid = true;
                    }
                } catch (...) {}
            }
            if (!valid) {
                std::cerr << "usage: !N or !!  (N is a 1-based history index)\n";
                continue;
            }
            if (idx > history.size()) {
                std::cerr << "no history entry " << idx << " (have " << history.size() << ")\n";
                continue;
            }
            line = history[idx - 1].first;
            std::cout << line << '\n';
        }

        if (line == "help") { printHelp(); continue; }
        if (line == "units") { printUnits(); continue; }

        if (line == "history") {
            if (history.empty()) {
                std::cout << "(no history)\n";
            } else {
                for (std::size_t i = 0; i < history.size(); ++i)
                    std::cout << "[" << i + 1 << "] "
                              << history[i].first << " = " << history[i].second << '\n';
            }
            continue;
        }

        if (line == "vars") {
            if (vars.empty()) {
                std::cout << "(no user variables)\n";
            } else {
                std::size_t w = 0;
                for (const auto& [name, val] : vars) w = std::max(w, name.size());
                for (const auto& [name, val] : vars)
                    std::cout << "  " << std::left << std::setw(static_cast<int>(w))
                              << name << " = " << fmtNum(val) << '\n';
            }
            continue;
        }

        if (line == "funcs") {
            if (funcs.empty()) {
                std::cout << "(no user functions)\n";
            } else {
                for (const auto& [name, fn] : funcs) {
                    std::cout << "  " << name << "(";
                    for (std::size_t i = 0; i < fn.params.size(); ++i)
                        std::cout << (i ? ", " : "") << fn.params[i];
                    std::cout << ")\n";
                }
            }
            continue;
        }

        if (line == "clear") {
            history.clear();
            std::cout << "history cleared\n";
            continue;
        }

        auto trimmedArg = [](const std::string& s) {
            auto a = s.find_first_not_of(" \t");
            auto b = s.find_last_not_of(" \t");
            return (a == std::string::npos) ? std::string{} : s.substr(a, b - a + 1);
        };

        if (line.rfind("save ", 0) == 0) {
            std::string path = trimmedArg(line.substr(5));
            if (path.empty()) { std::cerr << "usage: save <file>\n"; continue; }
            std::ofstream out(path);
            if (!out) { std::cerr << "cannot open '" << path << "' for writing\n"; continue; }
            for (const auto& [name, val] : vars)
                out << name << " = " << fmtNum(val) << '\n';
            for (const auto& [name, fn] : funcs)
                out << fn.source << '\n';
            std::cout << "saved " << vars.size() << " variable(s) and "
                      << funcs.size() << " function(s) to " << path << '\n';
            continue;
        }

        if (line.rfind("load ", 0) == 0) {
            std::string path = trimmedArg(line.substr(5));
            if (path.empty()) { std::cerr << "usage: load <file>\n"; continue; }
            std::ifstream in(path);
            if (!in) { std::cerr << "cannot open '" << path << "'\n"; continue; }
            std::string fline;
            std::size_t loaded = 0, errors = 0;
            while (std::getline(in, fline)) {
                auto trimStart = fline.find_first_not_of(" \t");
                if (trimStart == std::string::npos) continue;
                if (fline[trimStart] == '#') continue;
                try {
                    auto tokens = Lexer(fline).tokenize();
                    if (looksLikeFuncDef(tokens)) {
                        Parser::defineFunction(std::move(tokens), fline, funcs);
                    } else {
                        ans = Parser(std::move(tokens), ans, vars, funcs).parse();
                    }
                    ++loaded;
                } catch (const std::exception& ex) {
                    std::cerr << "  " << path << ": " << ex.what() << " (line: " << fline << ")\n";
                    ++errors;
                }
            }
            std::cout << "loaded " << loaded << " line(s) from " << path;
            if (errors) std::cout << " (" << errors << " error(s))";
            std::cout << '\n';
            continue;
        }

        if (line.rfind("unset ", 0) == 0) {
            std::string name = line.substr(6);
            auto a = name.find_first_not_of(" \t");
            auto b = name.find_last_not_of(" \t");
            name = (a == std::string::npos) ? "" : name.substr(a, b - a + 1);
            if (name.empty())
                std::cerr << "usage: unset <name>\n";
            else if (vars.erase(name) || funcs.erase(name))
                std::cout << "unset " << name << '\n';
            else
                std::cerr << "no variable or function '" << name << "'\n";
            continue;
        }

        int displayBase = 0;
        std::size_t prefixLen = 0;
        std::string expr = line;
        if (line.rfind("hex ", 0) == 0) { displayBase = 16; prefixLen = 4; expr = line.substr(4); }
        else if (line.rfind("bin ", 0) == 0) { displayBase = 2; prefixLen = 4; expr = line.substr(4); }
        else if (line.rfind("oct ", 0) == 0) { displayBase = 8; prefixLen = 4; expr = line.substr(4); }

        try {
            auto tokens = Lexer(expr).tokenize();
            if (displayBase == 0 && looksLikeFuncDef(tokens)) {
                std::string fname = tokens[0].name;
                Parser::defineFunction(std::move(tokens), line, funcs);
                std::cout << "defined " << fname << "\n";
                history.emplace_back(line, 0.0);
                continue;
            }
            double result = Parser(std::move(tokens), ans, vars, funcs).parse();
            std::string formatted = (displayBase != 0)
                ? fmtBase(result, displayBase)
                : fmtNum(result);
            std::cout << "= " << formatted << '\n';
            ans = result;
            history.emplace_back(line, result);
        } catch (const CalcError& ex) {
            std::cerr << "  " << std::string(prefixLen + ex.col(), ' ') << "^\n";
            std::cerr << "error: " << ex.what() << '\n';
        } catch (const std::exception& ex) {
            std::cerr << "error: " << ex.what() << '\n';
        }
    }
}
