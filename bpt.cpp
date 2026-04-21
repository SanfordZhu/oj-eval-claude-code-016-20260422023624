#include "bpt.hpp"
#include <iostream>
#include <fstream>

BPTree::BPTree(const std::string& filename) : filename(filename) {
    // Try to load existing data from file
    std::ifstream infile(filename, std::ios::binary);
    if (infile.is_open()) {
        size_t num_keys;
        infile.read(reinterpret_cast<char*>(&num_keys), sizeof(num_keys));

        for (size_t i = 0; i < num_keys; i++) {
            size_t key_len;
            infile.read(reinterpret_cast<char*>(&key_len), sizeof(key_len));

            std::string key(key_len, '\0');
            infile.read(&key[0], key_len);

            size_t num_values;
            infile.read(reinterpret_cast<char*>(&num_values), sizeof(num_values));

            std::vector<int> values(num_values);
            for (size_t j = 0; j < num_values; j++) {
                infile.read(reinterpret_cast<char*>(&values[j]), sizeof(int));
            }

            data[key] = values;
        }
        infile.close();
    }
}

BPTree::~BPTree() {
    // Save data to file
    std::ofstream outfile(filename, std::ios::binary);
    if (outfile.is_open()) {
        size_t num_keys = data.size();
        outfile.write(reinterpret_cast<const char*>(&num_keys), sizeof(num_keys));

        for (const auto& pair : data) {
            size_t key_len = pair.first.length();
            outfile.write(reinterpret_cast<const char*>(&key_len), sizeof(key_len));
            outfile.write(pair.first.c_str(), key_len);

            size_t num_values = pair.second.size();
            outfile.write(reinterpret_cast<const char*>(&num_values), sizeof(num_values));

            for (int value : pair.second) {
                outfile.write(reinterpret_cast<const char*>(&value), sizeof(int));
            }
        }
        outfile.close();
    }
}

void BPTree::insert(const std::string& key, int value) {
    auto& values = data[key];

    // Check if value already exists
    auto it = std::lower_bound(values.begin(), values.end(), value);
    if (it != values.end() && *it == value) {
        return;  // Value already exists, don't insert duplicate
    }

    // Insert in sorted order
    values.insert(it, value);
}

void BPTree::remove(const std::string& key, int value) {
    auto it = data.find(key);
    if (it != data.end()) {
        auto& values = it->second;
        auto vit = std::lower_bound(values.begin(), values.end(), value);
        if (vit != values.end() && *vit == value) {
            values.erase(vit);
            if (values.empty()) {
                data.erase(it);
            }
        }
    }
}

std::vector<int> BPTree::find(const std::string& key) {
    auto it = data.find(key);
    if (it != data.end()) {
        return it->second;
    }
    return std::vector<int>();
}