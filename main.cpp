#include <iostream>
#include <string>
#include "lexer.hpp"
#include "parser.hpp"

int main() {
    std::cout << "cppalculator — type an expression or 'quit'\n";
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "quit" || line == "exit") break;
        if (line.empty()) continue;
        try {
            auto tokens = Lexer(line).tokenize();
            double result = Parser(std::move(tokens)).parse();
            std::cout << "= " << result << '\n';
        } catch (const std::exception& e) {
            std::cerr << "error: " << e.what() << '\n';
        }
    }
}
