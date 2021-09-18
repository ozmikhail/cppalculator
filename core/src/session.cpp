#include "session.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include <iomanip>
#include <iostream>
#include <sstream>

static std::string fmtNum(double v) {
    std::ostringstream oss;
    oss << std::setprecision(10) << v;
    return oss.str();
}

double Session::evaluate(const std::string& expr) {
    auto tokens = Lexer(expr).tokenize();
    double result = Parser(std::move(tokens), m_ans).parse();
    m_history.push_back({expr, result});
    m_ans = result;
    m_hasAns = true;
    return result;
}

const std::vector<HistoryEntry>& Session::history() const {
    return m_history;
}

double Session::ans() const { return m_ans; }
bool Session::hasAns() const { return m_hasAns; }

void Session::clear() {
    m_history.clear();
    m_ans = 0.0;
    m_hasAns = false;
}

void Session::printHistory() const {
    if (m_history.empty()) {
        std::cout << "(no history)\n";
        return;
    }
    for (std::size_t i = 0; i < m_history.size(); ++i)
        std::cout << "  [" << (i + 1) << "]  "
                  << m_history[i].expression << "  =  "
                  << fmtNum(m_history[i].result) << '\n';
}
