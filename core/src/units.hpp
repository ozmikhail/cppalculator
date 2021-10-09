#pragma once
#include <map>
#include <string>

inline const std::map<std::string, double>& builtinUnits() {
    static const std::map<std::string, double> units = {
        // Length (SI base: 1 metre)
        {"km", 1000.0},
        {"cm", 0.01},
        {"mm", 0.001},
        {"ft", 0.3048},
        {"inch", 0.0254},
        {"yd", 0.9144},
        {"miles", 1609.344},
        {"nmi", 1852.0},
        // Mass (SI base: 1 kilogram)
        {"gram", 0.001},
        {"mg", 1.0e-6},
        {"lb", 0.453592},
        {"oz", 0.0283495},
        {"tonne", 1000.0},
        // Time (SI base: 1 second)
        {"sec", 1.0},
        {"hr", 3600.0},
        {"day", 86400.0},
        {"week", 604800.0},
    };
    return units;
}
