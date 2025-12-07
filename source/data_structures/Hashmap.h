#ifndef HASHMAP_H
#define HASHMAP_H

#include "linked_list.h"
#include <iostream>
#include <vector>
#include <string>
#include <functional>
using namespace std;

// HashNode structure
template <typename K, typename V>
struct HashNode {
    K key;
    V value;
    
    HashNode(K k, V v) : key(k), value(v) {}
    
    // For comparison in LinkedList
    bool operator==(const HashNode& other) const {
        return key == other.key;
    }
    
    // For printing
    friend ostream& operator<<(ostream& os, const HashNode& node) {
        os << node.key << ":" << node.value;
        return os;
    }
};

// HashMap with chaining using LinkedList
template <typename K, typename V>
class HashMap {
private:
    vector<LinkedList<HashNode<K, V>>> table;
    int capacity;
    int size;
    
    // Hash function
    int hashFunction(const K& key) const {
        hash<K> hashFunc;
        return hashFunc(key) % capacity;
    }
    
public:
    HashMap(int cap = 100) : capacity(cap), size(0) {
        table.resize(capacity);
    }
    
    // Insert or update
    void insert(const K& key, const V& value) {
        int index = hashFunction(key);
        
        // Check if key exists
        HashNode<K, V> temp(key, value);
        HashNode<K, V>* existing = table[index].get(temp);
        
        if (existing != nullptr) {
            existing->value = value;  // Update
        } else {
            table[index].add(temp);   // Insert new
            size++;
        }
    }
    
    // Get value (returns pointer)
    V* get(const K& key)  {
        int index = hashFunction(key);
        
        // Create dummy for search
        HashNode<K, V> dummy(key, V());
        HashNode<K, V>* node = table[index].get(dummy);
        
        if (node != nullptr) {
            return &(node->value);
        }
        return nullptr;
    }
    
    // Check if key exists
    bool contains(const K& key) const {
        int index = hashFunction(key);
        HashNode<K, V> dummy(key, V());
        return table[index].contains(dummy);
    }
    
    // Remove key
    bool remove(const K& key) {
        int index = hashFunction(key);
        HashNode<K, V> dummy(key, V());
        
        if (table[index].remove(dummy)) {
            size--;
            return true;
        }
        return false;
    }
    
    // Get size
    int getSize() const {
        return size;
    }
    
    // Check if empty
    bool isEmpty() const {
        return size == 0;
    }
    
    // Clear all
    void clear() {
        for (int i = 0; i < capacity; i++) {
            table[i].clear();
        }
        size = 0;
    }
    
    // Get all key-value pairs
    vector<pair<K, V>> getAll() const {
        vector<pair<K, V>> result;
        
        for (int i = 0; i < capacity; i++) {
            ListNode<HashNode<K, V>>* current = table[i].begin();
            while (current != nullptr) {
                result.push_back({current->data.key, current->data.value});
                current = current->next;
            }
        }
        
        return result;
    }
    
    // Print for debugging
    void print() const {
        cout << "\n=== Hash Map (Size: " << size << ") ===" << endl;
        for (int i = 0; i < capacity; i++) {
            if (!table[i].isEmpty()) {
                cout << "Bucket " << i << ": ";
                ListNode<HashNode<K, V>>* current = table[i].begin();
                while (current != nullptr) {
                    cout << current->data << " | ";
                    current = current->next;
                }
                cout << endl;
            }
        }
    }
    
    // Operator []
    V& operator[](const K& key) {
        V* value = get(key);
        if (value == nullptr) {
            insert(key, V());
            value = get(key);
        }
        return *value;
    }
};

#endif