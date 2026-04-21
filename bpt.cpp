#include "bpt.hpp"
#include <iostream>
#include <cstring>

BPTree::BPTree(const std::string& filename) : filename(filename), root_page(-1), next_page_num(0) {
    file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        // File doesn't exist, create it
        file.open(filename, std::ios::out | std::ios::binary);
        file.close();
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);

        // Create root node
        BPTNode root;
        root.is_leaf = true;
        root.page_num = allocatePage();
        root.parent_page = -1;
        root_page = root.page_num;
        writeNode(root_page, root);
    } else {
        // Read header
        file.seekg(0);
        file.read(reinterpret_cast<char*>(&root_page), sizeof(root_page));
        file.read(reinterpret_cast<char*>(&next_page_num), sizeof(next_page_num));
    }
}

BPTree::~BPTree() {
    if (file.is_open()) {
        // Write header
        file.seekp(0);
        file.write(reinterpret_cast<const char*>(&root_page), sizeof(root_page));
        file.write(reinterpret_cast<const char*>(&next_page_num), sizeof(next_page_num));
        file.close();
    }
}

void BPTree::writeNode(int page_num, const BPTNode& node) {
    file.seekp(sizeof(int) * 2 + page_num * sizeof(BPTNode));
    file.write(reinterpret_cast<const char*>(&node), sizeof(BPTNode));
}

BPTNode BPTree::readNode(int page_num) {
    BPTNode node;
    file.seekg(sizeof(int) * 2 + page_num * sizeof(BPTNode));
    file.read(reinterpret_cast<char*>(&node), sizeof(BPTNode));
    return node;
}

int BPTree::allocatePage() {
    return next_page_num++;
}

void BPTree::insert(const std::string& key, int value) {
    // Find the appropriate leaf
    BPTNode node = readNode(root_page);

    while (!node.is_leaf) {
        // Binary search to find the correct child
        int left = 0, right = node.key_count - 1;
        int child_pos = node.key_count; // Default to last child

        while (left <= right) {
            int mid = (left + right) / 2;
            if (std::string(node.keys[mid]) < key) {
                left = mid + 1;
            } else {
                right = mid - 1;
                child_pos = mid;
            }
        }
        int child_page = node.children[child_pos];
        if (child_page == -1) {
            // This shouldn't happen in a properly constructed tree
            return;
        }
        node = readNode(child_page);
    }

    // Binary search to check if key-value pair already exists
    int left = 0, right = node.key_count - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        int cmp = std::string(node.keys[mid]).compare(key);
        if (cmp < 0) {
            left = mid + 1;
        } else if (cmp > 0) {
            right = mid - 1;
        } else {
            // Found the key, check if value exists
            int pos = mid;
            while (pos >= 0 && std::string(node.keys[pos]) == key) {
                if (node.values[pos] == value) {
                    return; // Duplicate found
                }
                pos--;
            }
            pos = mid + 1;
            while (pos < node.key_count && std::string(node.keys[pos]) == key) {
                if (node.values[pos] == value) {
                    return; // Duplicate found
                }
                pos++;
            }
            break;
        }
    }

    // Insert in leaf
    insertInLeaf(node.page_num, key, value);
}

void BPTree::insertInLeaf(int page_num, const std::string& key, int value) {
    BPTNode leaf = readNode(page_num);

    // Find position to insert
    int pos = 0;
    while (pos < leaf.key_count && (std::string(leaf.keys[pos]) < key ||
           (std::string(leaf.keys[pos]) == key && leaf.values[pos] < value))) {
        pos++;
    }

    // Check if we need to split
    if (leaf.key_count == MAX_KEYS) {
        splitLeaf(page_num);
        // After split, we need to find the correct leaf again
        insert(key, value);  // Recursive call to handle the split
        return;
    }

    // Make space for new entry
    for (int i = leaf.key_count; i > pos; i--) {
        strcpy(leaf.keys[i], leaf.keys[i-1]);
        leaf.values[i] = leaf.values[i-1];
    }

    // Insert new entry
    strcpy(leaf.keys[pos], key.c_str());
    leaf.values[pos] = value;
    leaf.key_count++;

    writeNode(page_num, leaf);
}

void BPTree::splitLeaf(int leaf_page) {
    BPTNode leaf = readNode(leaf_page);

    // Create new leaf
    BPTNode new_leaf;
    new_leaf.is_leaf = true;
    new_leaf.page_num = allocatePage();
    new_leaf.parent_page = leaf.parent_page;
    new_leaf.next_leaf = leaf.next_leaf;

    // Split keys and values
    int split_pos = MIN_KEYS;
    new_leaf.key_count = leaf.key_count - split_pos;

    for (int i = 0; i < new_leaf.key_count; i++) {
        strcpy(new_leaf.keys[i], leaf.keys[i + split_pos]);
        new_leaf.values[i] = leaf.values[i + split_pos];
    }

    leaf.key_count = split_pos;
    leaf.next_leaf = new_leaf.page_num;

    // Write both leaves
    writeNode(leaf_page, leaf);
    writeNode(new_leaf.page_num, new_leaf);

    // Insert new_leaf's first key into parent
    insertInParent(leaf_page, new_leaf.keys[0], new_leaf.page_num);
}

void BPTree::insertInParent(int left_page, const std::string& key, int right_page) {
    BPTNode left = readNode(left_page);

    if (left.page_num == root_page) {
        // Create new root
        BPTNode new_root;
        new_root.is_leaf = false;
        new_root.page_num = allocatePage();
        new_root.key_count = 1;
        new_root.parent_page = -1;
        strcpy(new_root.keys[0], key.c_str());
        new_root.children[0] = left_page;
        new_root.children[1] = right_page;

        // Update children's parent
        left.parent_page = new_root.page_num;
        BPTNode right = readNode(right_page);
        right.parent_page = new_root.page_num;

        writeNode(left_page, left);
        writeNode(right_page, right);
        writeNode(new_root.page_num, new_root);

        root_page = new_root.page_num;
        return;
    }

    // Find parent
    int parent_page = left.parent_page;
    BPTNode parent = readNode(parent_page);

    // Check if parent has space
    if (parent.key_count < MAX_KEYS) {
        // Find position in parent
        int pos = 0;
        while (pos < parent.key_count && std::string(parent.keys[pos]) < key) {
            pos++;
        }

        // Make space
        for (int i = parent.key_count; i > pos; i--) {
            strcpy(parent.keys[i], parent.keys[i-1]);
            parent.children[i+1] = parent.children[i];
        }

        // Insert key and child
        strcpy(parent.keys[pos], key.c_str());
        parent.children[pos+1] = right_page;
        parent.key_count++;

        // Update right child's parent
        BPTNode right = readNode(right_page);
        right.parent_page = parent_page;
        writeNode(right_page, right);

        writeNode(parent_page, parent);
    } else {
        // Parent is full, need to split
        splitInternal(parent_page);
        // After split, recursively insert
        insertInParent(left_page, key, right_page);
    }
}

void BPTree::splitInternal(int internal_page) {
    BPTNode internal = readNode(internal_page);

    // Create new internal node
    BPTNode new_internal;
    new_internal.is_leaf = false;
    new_internal.page_num = allocatePage();
    new_internal.parent_page = internal.parent_page;

    // Split keys and children
    int split_pos = MIN_KEYS;
    new_internal.key_count = internal.key_count - split_pos - 1;

    // The middle key goes up to parent
    std::string middle_key = internal.keys[split_pos];

    for (int i = 0; i < new_internal.key_count; i++) {
        strcpy(new_internal.keys[i], internal.keys[i + split_pos + 1]);
        new_internal.children[i] = internal.children[i + split_pos + 1];

        // Update children's parent
        BPTNode child = readNode(new_internal.children[i]);
        child.parent_page = new_internal.page_num;
        writeNode(child.page_num, child);
    }
    new_internal.children[new_internal.key_count] = internal.children[internal.key_count];
    BPTNode last_child = readNode(new_internal.children[new_internal.key_count]);
    last_child.parent_page = new_internal.page_num;
    writeNode(last_child.page_num, last_child);

    internal.key_count = split_pos;

    // Write both nodes
    writeNode(internal_page, internal);
    writeNode(new_internal.page_num, new_internal);

    // Insert middle key into parent
    insertInParent(internal_page, middle_key, new_internal.page_num);
}

std::vector<int> BPTree::find(const std::string& key) {
    std::vector<int> result;

    // Find the leaf
    BPTNode node = readNode(root_page);

    while (!node.is_leaf) {
        // Binary search to find the correct child
        int left = 0, right = node.key_count - 1;
        int child_pos = node.key_count; // Default to last child

        while (left <= right) {
            int mid = (left + right) / 2;
            if (std::string(node.keys[mid]) < key) {
                left = mid + 1;
            } else {
                right = mid - 1;
                child_pos = mid;
            }
        }
        node = readNode(node.children[child_pos]);
    }

    // Binary search to find the key in the leaf
    int left = 0, right = node.key_count - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        int cmp = std::string(node.keys[mid]).compare(key);
        if (cmp < 0) {
            left = mid + 1;
        } else if (cmp > 0) {
            right = mid - 1;
        } else {
            // Found the key, collect all values
            // Go left to find the first occurrence
            int first = mid;
            while (first > 0 && std::string(node.keys[first - 1]) == key) {
                first--;
            }

            // Collect all values for this key
            int pos = first;
            while (pos < node.key_count && std::string(node.keys[pos]) == key) {
                result.push_back(node.values[pos]);
                pos++;
            }

            // Check next leaves if they have the same key
            int next_page = node.next_leaf;
            while (next_page != -1) {
                BPTNode next_leaf = readNode(next_page);
                bool found_more = false;
                for (int i = 0; i < next_leaf.key_count; i++) {
                    if (std::string(next_leaf.keys[i]) == key) {
                        result.push_back(next_leaf.values[i]);
                        found_more = true;
                    } else if (found_more) {
                        break;
                    }
                }
                if (!found_more) break;
                next_page = next_leaf.next_leaf;
            }

            // Sort the result (though it should already be sorted)
            std::sort(result.begin(), result.end());
            return result;
        }
    }

    return result; // Key not found
}

void BPTree::remove(const std::string& key, int value) {
    // Find the leaf containing the key-value pair
    BPTNode node = readNode(root_page);

    while (!node.is_leaf) {
        int pos = 0;
        while (pos < node.key_count && std::string(node.keys[pos]) < key) {
            pos++;
        }
        node = readNode(node.children[pos]);
    }

    // Find the key in the leaf using binary search
    int left = 0, right = node.key_count - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        int cmp = std::string(node.keys[mid]).compare(key);
        if (cmp < 0) {
            left = mid + 1;
        } else if (cmp > 0) {
            right = mid - 1;
        } else {
            // Found the key, now find the exact value
            // Since values are sorted, search around this position
            int pos = mid;
            while (pos >= 0 && std::string(node.keys[pos]) == key) {
                if (node.values[pos] == value) {
                    // Found it, remove
                    for (int i = pos; i < node.key_count - 1; i++) {
                        strcpy(node.keys[i], node.keys[i + 1]);
                        node.values[i] = node.values[i + 1];
                    }
                    node.key_count--;
                    writeNode(node.page_num, node);
                    return;
                }
                pos--;
            }

            pos = mid + 1;
            while (pos < node.key_count && std::string(node.keys[pos]) == key) {
                if (node.values[pos] == value) {
                    // Found it, remove
                    for (int i = pos; i < node.key_count - 1; i++) {
                        strcpy(node.keys[i], node.keys[i + 1]);
                        node.values[i] = node.values[i + 1];
                    }
                    node.key_count--;
                    writeNode(node.page_num, node);
                    return;
                }
                pos++;
            }
            return; // Value not found
        }
    }
}