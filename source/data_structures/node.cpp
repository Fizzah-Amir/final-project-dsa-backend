#include "node.h"
#include "storage_manager.h"
#include <cstring>
#include <algorithm>
#include <iostream>
#include <functional>

using namespace std;

// Constructor
Node::Node(int block_idx, bool leaf) 
    : block_index(block_idx), is_leaf(leaf), key_count(0), is_dirty(false) {
    initialize();
}

Node::~Node() {
    clear_memory_pointers();
}

void Node::initialize() {
    keys.resize(M - 1);
    values.resize(M - 1);
    disk_pointers.resize(M, -1);
    memory_pointers.resize(M, nullptr);
    key_count = 0;
}

void Node::serialize(char* buffer) const {
    memset(buffer, 0, BLOCK_SIZE);
    
    // Header: is_leaf (1 byte) + key_count (4 bytes)
    buffer[0] = is_leaf ? 1 : 0;
    memcpy(buffer + 1, &key_count, sizeof(int));
    
    int offset = 5; // 1 + 4
    
    // Keys
    for (int i = 0; i < key_count; i++) {
        memcpy(buffer + offset, &keys[i], sizeof(int));
        offset += sizeof(int);
    }
    
    // Values (Flight numbers) - store with length prefix
    for (int i = 0; i < key_count; i++) {
        int value_len = values[i].size();
        memcpy(buffer + offset, &value_len, sizeof(int));
        offset += sizeof(int);
        
        if (value_len > 0) {
            memcpy(buffer + offset, values[i].c_str(), value_len);
            offset += value_len;
        }
    }
    
    // Disk pointers
    for (int i = 0; i <= key_count; i++) {
        memcpy(buffer + offset, &disk_pointers[i], sizeof(int));
        offset += sizeof(int);
    }
    
    // Fill remaining space with zeros
    while (offset < BLOCK_SIZE) {
        buffer[offset++] = 0;
    }
}

void Node::deserialize(const char* buffer) {
    // Header
    is_leaf = (buffer[0] == 1);
    memcpy(&key_count, buffer + 1, sizeof(int));
    
    int offset = 5;
    
    // Keys
    for (int i = 0; i < key_count; i++) {
        memcpy(&keys[i], buffer + offset, sizeof(int));
        offset += sizeof(int);
    }
    
    // Values
    values.clear();
    for (int i = 0; i < key_count; i++) {
        int value_len;
        memcpy(&value_len, buffer + offset, sizeof(int));
        offset += sizeof(int);
        
        if (value_len > 0) {
            values.push_back(string(buffer + offset, value_len));
            offset += value_len;
        } else {
            values.push_back("");
        }
    }
    
    // Disk pointers
    disk_pointers.resize(M, -1);
    for (int i = 0; i <= key_count; i++) {
        memcpy(&disk_pointers[i], buffer + offset, sizeof(int));
        offset += sizeof(int);
    }
    
    // Clear memory pointers
    clear_memory_pointers();
}

void Node::clear_memory_pointers() {
    for (auto& ptr : memory_pointers) {
        ptr = nullptr;
    }
}

int Node::find_key(int key) const {
    int idx = 0;
    while (idx < key_count && keys[idx] < key) {
        idx++;
    }
    return idx;
}

void Node::insert_key_value(int key, const string& value, int disk_ptr, Node* mem_ptr) {
    int idx = find_key(key);
    
    // Shift keys and values to the right
    for (int i = key_count; i > idx; i--) {
        keys[i] = keys[i - 1];
        values[i] = values[i - 1];
    }
    for (int i = key_count + 1; i > idx + 1; i--) {
        disk_pointers[i] = disk_pointers[i - 1];
        memory_pointers[i] = memory_pointers[i - 1];
    }
    
    // Insert new key, value and pointer
    keys[idx] = key;
    values[idx] = value;
    if (disk_ptr != -1) {
        disk_pointers[idx + 1] = disk_ptr;
    }
    if (mem_ptr != nullptr) {
        memory_pointers[idx + 1] = mem_ptr;
    }
    
    key_count++;
    is_dirty = true;
}

void Node::remove_key(int index) {
    for (int i = index; i < key_count - 1; i++) {
        keys[i] = keys[i + 1];
        values[i] = values[i + 1];
    }
    for (int i = index + 1; i < key_count; i++) {
        disk_pointers[i] = disk_pointers[i + 1];
        memory_pointers[i] = memory_pointers[i + 1];
    }
    key_count--;
    is_dirty = true;
}

// Range query for flight searches
void Node::range_query(int low, int high, vector<pair<int, string>>& result, 
                       function<Node*(int)> load_node_func) {
    int i = 0;
    while (i < key_count) {
        if (!is_leaf) {
            // Load child if not in memory
            if (!memory_pointers[i]) {
                memory_pointers[i] = load_node_func(disk_pointers[i]);
            }
            memory_pointers[i]->range_query(low, high, result, load_node_func);
        }
        
        // Check if current key is in range
        if (keys[i] >= low && keys[i] <= high) {
            result.push_back({keys[i], values[i]});
        }
        i++;
    }
    
    // Check last child
    if (!is_leaf) {
        if (!memory_pointers[i]) {
            memory_pointers[i] = load_node_func(disk_pointers[i]);
        }
        memory_pointers[i]->range_query(low, high, result, load_node_func);
    }
}