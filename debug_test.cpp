#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "bpt.hpp"

int main() {
    BPTree tree("debug.bpt");

    // Test insert
    tree.insert("CppPrimer", 2012);
    tree.insert("CppPrimer", 2001);

    // Test find
    std::vector<int> values = tree.find("CppPrimer");
    std::cout << "Found " << values.size() << " values for CppPrimer:";
    for (int v : values) {
        std::cout << " " << v;
    }
    std::cout << std::endl;

    // Test delete
    tree.remove("CppPrimer", 2012);

    // Test find again
    values = tree.find("CppPrimer");
    std::cout << "After deletion, found " << values.size() << " values for CppPrimer:";
    for (int v : values) {
        std::cout << " " << v;
    }
    std::cout << std::endl;

    return 0;
}