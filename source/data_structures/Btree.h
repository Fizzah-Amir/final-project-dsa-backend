#ifndef BTREE_H
#define BTREE_H

#include "node.h"
#include "storage_manager.h"
#include <vector>
#include <memory>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <string>
#include <functional>
#include <utility>

using namespace std;

class BTree {
public:
    BTree(const string& filename);
    ~BTree();
    
    bool initialize();
    void shutdown();
    
    // Core operations with values
    bool search(int key, string& value, vector<int>& path);
    bool insert(int key, const string& value);
    bool remove(int key);
    
    // Range query for flight time searches
    vector<pair<int, string>> range_query(int low, int high);
    
    // Get all flights (in-order traversal)
    vector<pair<int, string>> get_all()  ;
    
    // For debugging
    void print_tree();
    
    // Statistics
    
    
private:
    StorageManager* storage_;  // Changed from unique_ptr to raw pointer
    string filename_;
    Node* root_;
    int flight_count_;
    
    // Asynchronous writer
    thread worker_thread_;
    queue<Node*> dirty_queue_;
    mutex queue_mutex_;
    condition_variable queue_cv_;
    bool shutdown_flag_;
    
    // Internal methods
    Node* load_node(int block_index);
    void save_node(Node* node, bool async = true);
    void worker_function();
     int get_flight_count() const;
    // B-tree operations
    void split_child(Node* parent, int index, Node* child);
    void insert_non_full(Node* node, int key, const string& value);
    void merge_children(Node* parent, int index);
    void borrow_from_left(Node* parent, int index);
    void borrow_from_right(Node* parent, int index);
    void remove_from_leaf(Node* node, int index);
    void remove_from_non_leaf(Node* node, int index);
    int find_predecessor(Node* node, int index);
    int find_successor(Node* node, int index);
    void remove_key(Node* node, int key);
    
    // Helper for in-order traversal
    void inorder_traversal(Node* node, vector<pair<int, string>>& result);
};

#endif