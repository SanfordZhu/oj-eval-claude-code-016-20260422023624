#ifndef BPT_HPP
#define BPT_HPP

#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

const int MAX_KEYS = 32;   // Maximum keys per node (reduced for testing)
const int MIN_KEYS = 16;   // Minimum keys per node (except root)

struct BPTNode {
    bool is_leaf;
    int key_count;
    int page_num;
    int parent_page;
    int next_leaf;  // For leaf nodes only

    // Storage for keys and values/children
    char keys[MAX_KEYS][65];  // Keys (max 64 chars + null terminator)
    int values[MAX_KEYS];     // Values for leaf nodes
    int children[MAX_KEYS + 1]; // Child page numbers for internal nodes

    BPTNode() : is_leaf(true), key_count(0), page_num(-1), parent_page(-1), next_leaf(-1) {
        for (int i = 0; i <= MAX_KEYS; i++) {
            children[i] = -1;
            if (i < MAX_KEYS) {
                values[i] = 0;
                keys[i][0] = '\0';
            }
        }
    }
};

class BPTree {
private:
    std::string filename;
    std::fstream file;
    int root_page;
    int next_page_num;

    // File operations
    void writeNode(int page_num, const BPTNode& node);
    BPTNode readNode(int page_num);
    int allocatePage();

    // B+ tree operations
    void insertInLeaf(int page_num, const std::string& key, int value);
    void insertInParent(int left_page, const std::string& key, int right_page);
    void splitLeaf(int leaf_page);
    void splitInternal(int internal_page);
    bool deleteFromLeaf(int leaf_page, const std::string& key, int value);
    void mergeNodes(int parent_page, int merge_idx);

public:
    BPTree(const std::string& filename);
    ~BPTree();

    void insert(const std::string& key, int value);
    void remove(const std::string& key, int value);
    std::vector<int> find(const std::string& key);
};

#endif // BPT_HPP