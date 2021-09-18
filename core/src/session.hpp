#pragma once
#include <string>
#include <vector>

struct HistoryEntry {
    std::string expression;
    double result;
};

class Session {
public:
    double evaluate(const std::string& expr);

    const std::vector<HistoryEntry>& history() const;
    double ans() const;
    bool hasAns() const;
    void clear();

    void printHistory() const;

private:
    std::vector<HistoryEntry> m_history;
    double m_ans{0.0};
    bool m_hasAns{false};
};
