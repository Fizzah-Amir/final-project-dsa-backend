#ifndef NODE_H
#define NODE_H

#include <vector>
#include <cstdint>
#include <string>
#include <memory>
#include <functional>

// Include constants
#include "constants.h"

class Node {
public:
    // Disk representation
    std::vector<int> keys;          // Departure timestamps
    std::vector<std::string> values; // Flight numbers for lookup
    std::vector<int> disk_pointers; // Block indices for children
    int block_index;                // This node's position in file
    bool is_leaf;
    int key_count;
    
    // Memory representation
    std::vector<Node*> memory_pointers;
    bool is_dirty; // Track if node needs to be written to disk
    
    Node(int block_idx, bool leaf = false);
    ~Node();
    
    // Serialization/Deserialization
    void serialize(char* buffer) const;
    void deserialize(const char* buffer);
    
    // Memory management
    void clear_memory_pointers();
    
    // Utility
    int find_key(int key) const;
    void insert_key_value(int key, const std::string& value, int disk_ptr = -1, Node* mem_ptr = nullptr);
    void remove_key(int index);
    
    // Range query - NEW for flight searches
    void range_query(int low, int high, std::vector<std::pair<int, std::string>>& result, 
                     std::function<Node*(int)> load_node_func);
    
private:
    void initialize();
};

#endif