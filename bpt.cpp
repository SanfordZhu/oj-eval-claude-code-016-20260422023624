#include "bpt.hpp"
#include <iostream>
#include <fstream>

BPTree::BPTree(const std::string& filename) : filename(filename), modified(false) {
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

            std::set<int> values;
            for (size_t j = 0; j < num_values; j++) {
                int value;
                infile.read(reinterpret_cast<char*>(&value), sizeof(int));
                values.insert(value);
            }

            data[key] = values;
        }
        infile.close();
    }
}

BPTree::~BPTree() {
    if (!modified) return;

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
    if (values.find(value) != values.end()) {
        return;  // Value already exists, don't insert duplicate
    }

    values.insert(value);
    modified = true;
}

void BPTree::remove(const std::string& key, int value) {
    auto it = data.find(key);
    if (it != data.end()) {
        it->second.erase(value);
        if (it->second.empty()) {
            data.erase(it);
        }
        modified = true;
    }
}

std::vector<int> BPTree::find(const std::string& key) {
    auto it = data.find(key);
    if (it != data.end()) {
        return std::vector<int>(it->second.begin(), it->second.end());
    }
    return std::vector<int>();
}