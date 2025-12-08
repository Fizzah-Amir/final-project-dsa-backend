#include "Btree.h"
#include "constants.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <queue>

using namespace std;

BTree::BTree(const string& filename) 
    : storage_(new StorageManager(filename)), 
      filename_(filename), 
      root_(nullptr), 
      flight_count_(0),
      shutdown_flag_(false) {
}

BTree::~BTree() {
    shutdown();
    delete storage_;
}

bool BTree::initialize() {
    if (!storage_->initialize()) {
        return false;
    }
    
    // Start background worker thread
    worker_thread_ = thread(&BTree::worker_function, this);
    
    // Load root node if it exists
    int root_block = storage_->get_root_block();
    if (root_block != -1) {
        root_ = load_node(root_block);
        
        // Count flights by traversing
        auto all_flights = get_all();
        flight_count_ = all_flights.size();
        
        cout << "Loaded B-Tree with " << flight_count_ << " flights" << endl;
    } else {
        // Create new root
        root_ = new Node(storage_->allocate_block(), true);
        storage_->set_root_block(root_->block_index);
        save_node(root_, false);
        flight_count_ = 0;
        cout << "Created new B-Tree" << endl;
    }
    
    return true;
}

void BTree::shutdown() {
    shutdown_flag_ = true;
    queue_cv_.notify_all();
    
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
    
    // Sync all remaining dirty nodes
    lock_guard<mutex> lock(queue_mutex_);
    while (!dirty_queue_.empty()) {
        Node* node = dirty_queue_.front();
        dirty_queue_.pop();
        
        char buffer[BLOCK_SIZE];
        node->serialize(buffer);
        storage_->write_block(node->block_index, buffer);
        
        delete node;
    }
    
    if (root_) {
        delete root_;
        root_ = nullptr;
    }
    
    storage_->shutdown();
    cout << "B-Tree shutdown complete" << endl;
}

Node* BTree::load_node(int block_index) {
    char buffer[BLOCK_SIZE];
    storage_->read_block(block_index, buffer);
    
    Node* node = new Node(block_index);
    node->deserialize(buffer);
    return node;
}
  int BTree::get_flight_count() const { return flight_count_; }
void BTree::save_node(Node* node, bool async) {
    if (async) {
        lock_guard<mutex> lock(queue_mutex_);
        dirty_queue_.push(node);
        queue_cv_.notify_one();
    } else {
        char buffer[BLOCK_SIZE];
        node->serialize(buffer);
        storage_->write_block(node->block_index, buffer);
        node->is_dirty = false;
    }
}

void BTree::worker_function() {
    while (!shutdown_flag_) {
        unique_lock<mutex> lock(queue_mutex_);
        queue_cv_.wait(lock, [this]() { 
            return !dirty_queue_.empty() || shutdown_flag_; 
        });
        
        while (!dirty_queue_.empty()) {
            Node* node = dirty_queue_.front();
            dirty_queue_.pop();
            lock.unlock();
            
            // Write to disk
            char buffer[BLOCK_SIZE];
            node->serialize(buffer);
            storage_->write_block(node->block_index, buffer);
            node->is_dirty = false;
            
            lock.lock();
        }
    }
}

// Search with value return
bool BTree::search(int key, string& value, vector<int>& path) {
    if (!root_) return false;
    
    Node* current = root_;
    path.push_back(current->block_index);
    
    while (current) {
        int idx = current->find_key(key);
        
        if (idx < current->key_count && current->keys[idx] == key) {
            value = current->values[idx];
            return true;
        }
        
        if (current->is_leaf) {
            break;
        }
        
        if (!current->memory_pointers[idx]) {
            current->memory_pointers[idx] = load_node(current->disk_pointers[idx]);
        }
        
        current = current->memory_pointers[idx];
        if (current) {
            path.push_back(current->block_index);
        }
    }
    
    return false;
}

// Insert with key-value pair
bool BTree::insert(int key, const string& value) {
    string dummy;
    vector<int> path;
    
    if (search(key, dummy, path)) {
        return false;
    }
    
    if (root_->key_count == M - 1) {
        Node* new_root = new Node(storage_->allocate_block(), false);
        new_root->disk_pointers[0] = root_->block_index;
        new_root->memory_pointers[0] = root_;
        
        split_child(new_root, 0, root_);
        
        storage_->set_root_block(new_root->block_index);
        root_ = new_root;
        save_node(root_, false);
    }
    
    insert_non_full(root_, key, value);
    flight_count_++;
    return true;
}

void BTree::insert_non_full(Node* node, int key, const string& value) {
    if (node->is_leaf) {
        node->insert_key_value(key, value);
        save_node(node);
    } else {
        int idx = node->find_key(key);
        
        if (!node->memory_pointers[idx]) {
            node->memory_pointers[idx] = load_node(node->disk_pointers[idx]);
        }
        
        Node* child = node->memory_pointers[idx];
        if (child->key_count == M - 1) {
            split_child(node, idx, child);
            if (key > node->keys[idx]) {
                idx++;
            }
        }
        
        insert_non_full(node->memory_pointers[idx], key, value);
    }
}

void BTree::split_child(Node* parent, int index, Node* child) {
    Node* new_child = new Node(storage_->allocate_block(), child->is_leaf);
    new_child->key_count = M / 2 - 1;
    
    int middle_idx = M / 2 - 1;
    
    for (int i = 0; i < new_child->key_count; i++) {
        new_child->keys[i] = child->keys[i + M / 2];
        new_child->values[i] = child->values[i + M / 2];
    }
    
    if (!child->is_leaf) {
        for (int i = 0; i <= new_child->key_count; i++) {
            new_child->disk_pointers[i] = child->disk_pointers[i + M / 2];
            new_child->memory_pointers[i] = nullptr;
        }
    }
    
    child->key_count = M / 2 - 1;
    
    parent->insert_key_value(child->keys[middle_idx], child->values[middle_idx], 
                            new_child->block_index, new_child);
    
    save_node(child);
    save_node(new_child);
    save_node(parent);
}

// Range query for flights between time ranges
vector<pair<int, string>> BTree::range_query(int low, int high) {
    vector<pair<int, string>> result;
    if (root_) {
        auto load_func = [this](int block_index) -> Node* {
            return this->load_node(block_index);
        };
        root_->range_query(low, high, result, load_func);
    }
    return result;
}

// Get all flights (in-order traversal)
vector<pair<int, string>> BTree::get_all()  {
    vector<pair<int, string>> result;
    inorder_traversal(root_, result);
    return result;
}

void BTree::inorder_traversal(Node* node, vector<pair<int, string>>& result)  {
    if (!node) return;
    
    int i;
    for (i = 0; i < node->key_count; i++) {
        if (!node->is_leaf) {
            if (!node->memory_pointers[i]) {
                node->memory_pointers[i] = load_node(node->disk_pointers[i]);
            }
            inorder_traversal(node->memory_pointers[i], result);
        }
        result.push_back({node->keys[i], node->values[i]});
    }
    
    if (!node->is_leaf) {
        if (!node->memory_pointers[i]) {
            node->memory_pointers[i] = load_node(node->disk_pointers[i]);
        }
        inorder_traversal(node->memory_pointers[i], result);
    }
}

// Remove operation
bool BTree::remove(int key) {
    string dummy;
    vector<int> path;
    if (!search(key, dummy, path)) {
        return false;
    }
    
    remove_key(root_, key);
    flight_count_--;
    
    if (root_->key_count == 0 && !root_->is_leaf) {
        Node* old_root = root_;
        root_ = load_node(root_->disk_pointers[0]);
        storage_->set_root_block(root_->block_index);
        storage_->deallocate_block(old_root->block_index);
        delete old_root;
    }
    
    return true;
}

void BTree::remove_key(Node* node, int key) {
    int idx = node->find_key(key);
    
    if (idx < node->key_count && node->keys[idx] == key) {
        if (node->is_leaf) {
            remove_from_leaf(node, idx);
        } else {
            remove_from_non_leaf(node, idx);
        }
    } else {
        if (node->is_leaf) {
            return;
        }
        
        if (!node->memory_pointers[idx]) {
            node->memory_pointers[idx] = load_node(node->disk_pointers[idx]);
        }
        
        Node* child = node->memory_pointers[idx];
        
        if (child->key_count < M / 2) {
            if (idx > 0) {
                if (!node->memory_pointers[idx - 1]) {
                    node->memory_pointers[idx - 1] = load_node(node->disk_pointers[idx - 1]);
                }
                Node* left_sibling = node->memory_pointers[idx - 1];
                
                if (left_sibling->key_count >= M / 2) {
                    borrow_from_left(node, idx);
                } else {
                    merge_children(node, idx - 1);
                    idx = idx - 1;
                }
            } else if (idx < node->key_count) {
                if (!node->memory_pointers[idx + 1]) {
                    node->memory_pointers[idx + 1] = load_node(node->disk_pointers[idx + 1]);
                }
                Node* right_sibling = node->memory_pointers[idx + 1];
                
                if (right_sibling->key_count >= M / 2) {
                    borrow_from_right(node, idx);
                } else {
                    merge_children(node, idx);
                }
            }
        }
        
        remove_key(node->memory_pointers[idx], key);
    }
}

void BTree::remove_from_leaf(Node* node, int index) {
    node->remove_key(index);
    save_node(node);
}

void BTree::remove_from_non_leaf(Node* node, int index) {
    int key = node->keys[index];
    
    if (!node->memory_pointers[index]) {
        node->memory_pointers[index] = load_node(node->disk_pointers[index]);
    }
    Node* left_child = node->memory_pointers[index];
    
    if (left_child->key_count >= M / 2) {
        int pred = find_predecessor(node, index);
        node->keys[index] = pred;
        remove_key(left_child, pred);
    } else {
        if (!node->memory_pointers[index + 1]) {
            node->memory_pointers[index + 1] = load_node(node->disk_pointers[index + 1]);
        }
        Node* right_child = node->memory_pointers[index + 1];
        
        if (right_child->key_count >= M / 2) {
            int succ = find_successor(node, index);
            node->keys[index] = succ;
            remove_key(right_child, succ);
        } else {
            merge_children(node, index);
            remove_key(left_child, key);
        }
    }
}

int BTree::find_predecessor(Node* node, int index) {
    Node* current = node->memory_pointers[index];
    while (!current->is_leaf) {
        int last_idx = current->key_count;
        if (!current->memory_pointers[last_idx]) {
            current->memory_pointers[last_idx] = load_node(current->disk_pointers[last_idx]);
        }
        current = current->memory_pointers[last_idx];
    }
    return current->keys[current->key_count - 1];
}

int BTree::find_successor(Node* node, int index) {
    Node* current = node->memory_pointers[index + 1];
    while (!current->is_leaf) {
        if (!current->memory_pointers[0]) {
            current->memory_pointers[0] = load_node(current->disk_pointers[0]);
        }
        current = current->memory_pointers[0];
    }
    return current->keys[0];
}

void BTree::merge_children(Node* parent, int index) {
    Node* left_child = parent->memory_pointers[index];
    Node* right_child = parent->memory_pointers[index + 1];
    
    left_child->keys[left_child->key_count] = parent->keys[index];
    left_child->values[left_child->key_count] = parent->values[index];
    left_child->key_count++;
    
    for (int i = 0; i < right_child->key_count; i++) {
        left_child->keys[left_child->key_count + i] = right_child->keys[i];
        left_child->values[left_child->key_count + i] = right_child->values[i];
    }
    
    if (!left_child->is_leaf) {
        for (int i = 0; i <= right_child->key_count; i++) {
            left_child->disk_pointers[left_child->key_count + i] = right_child->disk_pointers[i];
            left_child->memory_pointers[left_child->key_count + i] = right_child->memory_pointers[i];
        }
    }
    
    left_child->key_count += right_child->key_count;
    
    parent->remove_key(index);
    
    parent->disk_pointers[index + 1] = -1;
    parent->memory_pointers[index + 1] = nullptr;
    
    storage_->deallocate_block(right_child->block_index);
    delete right_child;
    
    save_node(left_child);
    save_node(parent);
}

void BTree::borrow_from_left(Node* parent, int index) {
    Node* child = parent->memory_pointers[index];
    Node* left_sibling = parent->memory_pointers[index - 1];
    
    for (int i = child->key_count; i > 0; i--) {
        child->keys[i] = child->keys[i - 1];
        child->values[i] = child->values[i - 1];
    }
    if (!child->is_leaf) {
        for (int i = child->key_count + 1; i > 0; i--) {
            child->disk_pointers[i] = child->disk_pointers[i - 1];
            child->memory_pointers[i] = child->memory_pointers[i - 1];
        }
    }
    
    child->keys[0] = parent->keys[index - 1];
    child->values[0] = parent->values[index - 1];
    if (!child->is_leaf) {
        child->disk_pointers[0] = left_sibling->disk_pointers[left_sibling->key_count];
        child->memory_pointers[0] = left_sibling->memory_pointers[left_sibling->key_count];
    }
    
    child->key_count++;
    
    parent->keys[index - 1] = left_sibling->keys[left_sibling->key_count - 1];
    parent->values[index - 1] = left_sibling->values[left_sibling->key_count - 1];
    left_sibling->key_count--;
    
    save_node(child);
    save_node(left_sibling);
    save_node(parent);
}

void BTree::borrow_from_right(Node* parent, int index) {
    Node* child = parent->memory_pointers[index];
    Node* right_sibling = parent->memory_pointers[index + 1];
    
    child->keys[child->key_count] = parent->keys[index];
    child->values[child->key_count] = parent->values[index];
    if (!child->is_leaf) {
        child->disk_pointers[child->key_count + 1] = right_sibling->disk_pointers[0];
        child->memory_pointers[child->key_count + 1] = right_sibling->memory_pointers[0];
    }
    
    child->key_count++;
    
    parent->keys[index] = right_sibling->keys[0];
    parent->values[index] = right_sibling->values[0];
    
    for (int i = 0; i < right_sibling->key_count - 1; i++) {
        right_sibling->keys[i] = right_sibling->keys[i + 1];
        right_sibling->values[i] = right_sibling->values[i + 1];
    }
    if (!right_sibling->is_leaf) {
        for (int i = 0; i < right_sibling->key_count; i++) {
            right_sibling->disk_pointers[i] = right_sibling->disk_pointers[i + 1];
            right_sibling->memory_pointers[i] = right_sibling->memory_pointers[i + 1];
        }
    }
    
    right_sibling->key_count--;
    
    save_node(child);
    save_node(right_sibling);
    save_node(parent);
}

void BTree::print_tree() {
    if (!root_) {
        cout << "Tree is empty" << endl;
        return;
    }
    
    queue<Node*> q;
    q.push(root_);
    int level = 0;
    
    while (!q.empty()) {
        int level_size = q.size();
        cout << "Level " << level << ": ";
        
        for (int i = 0; i < level_size; i++) {
            Node* current = q.front();
            q.pop();
            
            cout << "[Block " << current->block_index << ": ";
            for (int j = 0; j < current->key_count; j++) {
                cout << current->keys[j] << "(" << current->values[j] << ")";
                if (j < current->key_count - 1) cout << ",";
            }
            cout << "] ";
            
            if (!current->is_leaf) {
                for (int j = 0; j <= current->key_count; j++) {
                    if (current->disk_pointers[j] != -1) {
                        if (!current->memory_pointers[j]) {
                            current->memory_pointers[j] = load_node(current->disk_pointers[j]);
                        }
                        q.push(current->memory_pointers[j]);
                    }
                }
            }
        }
        
        cout << endl;
        level++;
    }
}