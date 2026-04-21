#ifndef BPT_HPP
#define BPT_HPP

#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <algorithm>

class BPTree {
private:
    std::string filename;
    std::map<std::string, std::vector<int>> data;

public:
    BPTree(const std::string& filename);
    ~BPTree();

    void insert(const std::string& key, int value);
    void remove(const std::string& key, int value);
    std::vector<int> find(const std::string& key);
};

#endif // BPT_HPP