#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <iostream>
#include <string>
using namespace std;

// Node for linked list
template <typename T>
struct ListNode {
    T data;
    ListNode* next;
    
    ListNode(T val) : data(val), next(nullptr) {}
};

// Singly Linked List
template <typename T>
class LinkedList {
private:
    ListNode<T>* head;
    int size;
    
public:
    LinkedList() : head(nullptr), size(0) {}
    
    ~LinkedList() {
        clear();
    }
    
    // Add to front
    void add(T value) {
        ListNode<T>* newNode = new ListNode<T>(value);
        newNode->next = head;
        head = newNode;
        size++;
    }
    
    // Remove by value
    bool remove(T value) {
        if (head == nullptr) return false;
        
        // If head needs to be removed
        if (head->data == value) {
            ListNode<T>* temp = head;
            head = head->next;
            delete temp;
            size--;
            return true;
        }
        
        // Search for node to remove
        ListNode<T>* current = head;
        while (current->next != nullptr) {
            if (current->next->data == value) {
                ListNode<T>* temp = current->next;
                current->next = current->next->next;
                delete temp;
                size--;
                return true;
            }
            current = current->next;
        }
        
        return false;
    }
    
    // Search for value
    bool contains(T value) const {
        ListNode<T>* current = head;
        while (current != nullptr) {
            if (current->data == value) {
                return true;
            }
            current = current->next;
        }
        return false;
    }
    
    // Get pointer to value
    T* get(T value) {
        ListNode<T>* current = head;
        while (current != nullptr) {
            if (current->data == value) {
                return &(current->data);
            }
            current = current->next;
        }
        return nullptr;
    }
    
    // Get size
    int getSize() const {
        return size;
    }
    
    // Check if empty
    bool isEmpty() const {
        return size == 0;
    }
    
    // Clear list
    void clear() {
        while (head != nullptr) {
            ListNode<T>* temp = head;
            head = head->next;
            delete temp;
        }
        size = 0;
    }
    
    // Print list
    void print() const {
        ListNode<T>* current = head;
        cout << "List: ";
        while (current != nullptr) {
            cout << current->data << " -> ";
            current = current->next;
        }
        cout << "NULL" << endl;
    }
    
    // For iteration (begin/end)
    ListNode<T>* begin() const {
        return head;
    }
    
    ListNode<T>* end() const {
        return nullptr;
    }
};

#endif