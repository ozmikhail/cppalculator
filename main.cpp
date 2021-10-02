#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include "lexer.hpp"
#include "parser.hpp"

int main() {
    std::cout << "cppalculator — type an expression or 'quit'\n";
    std::cout << "  functions: sin cos tan asin acos atan sinh cosh tanh\n";
    std::cout << "             sqrt cbrt exp ln log log10 log2\n";
    std::cout << "             abs ceil floor round deg rad sign\n";
    std::cout << "             min(x,y)  max(x,y)  pow(x,y)  atan2(y,x)\n";
    std::cout << "             hypot(x,y)  log(x,base)\n";
    std::cout << "  constants: pi  e  tau  inf\n";
    std::cout << "  'ans' recalls the last result; 'history' lists prior results\n";

    std::vector<std::pair<std::string, double>> history;
    double ans = 0.0;
    std::string line;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "quit" || line == "exit") break;
        if (line.empty()) continue;

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

        try {
            auto tokens = Lexer(line).tokenize();
            double result = Parser(std::move(tokens), ans).parse();
            std::cout << "= " << result << '\n';
            ans = result;
            history.emplace_back(line, result);
        } catch (const std::exception& e) {
            std::cerr << "error: " << e.what() << '\n';
        }
    }
}
