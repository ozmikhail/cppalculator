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
        // Area (SI base: 1 square metre)
        {"sqkm", 1.0e6},
        {"sqcm", 1.0e-4},
        {"sqmm", 1.0e-6},
        {"sqft", 0.09290304},
        {"sqin", 6.4516e-4},
        {"sqyd", 0.83612736},
        {"sqmi", 2589988.110336},
        {"acre", 4046.8564224},
        {"hectare",10000.0},
        // Volume (SI base: 1 cubic metre)
        {"litre", 0.001},
        {"ml", 1.0e-6},
        {"cuft", 0.028316846592},
        {"cuin", 1.6387064e-5},
        {"gal", 0.003785411784},
        {"gal_uk", 0.00454609},
        {"pint", 4.73176473e-4},
        {"cup", 2.365882365e-4},
        {"floz", 2.95735295625e-5},
        // Speed (SI base: 1 metre per second)
        {"kph", 0.2777777777777778},
        {"mph", 0.44704},
        {"knot", 0.5144444444444445},
        // Data (SI base: 1 byte; binary IEC prefixes)
        {"KB", 1024.0},
        {"MB", 1048576.0},
        {"GB", 1073741824.0},
        {"TB", 1099511627776.0},
    };
    return units;
}
