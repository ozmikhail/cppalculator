#include <iostream>
#include <stdexcept>
#include <string>
#include "session.hpp"

static void printHelp() {
    std::cout << "  hist      — show history\n"
              << "  !N        — recall and re-evaluate entry N\n"
              << "  clear     — clear history and ans\n"
              << "  ans       — last result (usable inside expressions)\n"
              << "  quit/exit — exit\n";
}

int main() {
    std::cout << "cppalculator  (type 'help' for commands)\n";

    Session session;
    std::string line;

    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        if (line == "quit" || line == "exit") break;
        if (line == "help") { printHelp(); continue; }
        if (line == "clear") { session.clear(); std::cout << "cleared\n"; continue; }
        if (line == "hist" || line == "history") { session.printHistory(); continue; }

        if (line.size() >= 2 && line[0] == '!') {
            std::size_t n = 0;
            try {
                n = static_cast<std::size_t>(std::stoul(line.substr(1)));
            } catch (...) {
                std::cerr << "error: invalid recall syntax — use !N\n";
                continue;
            }
            const auto& hist = session.history();
            if (n < 1 || n > hist.size()) {
                std::cerr << "error: no history entry " << n << '\n';
                continue;
            }
            line = hist[n - 1].expression;
            std::cout << "  >> " << line << '\n';
        }

        try {
            double result = session.evaluate(line);
            std::cout << "= " << result << '\n';
        } catch (const std::exception& e) {
            std::cerr << "error: " << e.what() << '\n';
        }
    }
}
