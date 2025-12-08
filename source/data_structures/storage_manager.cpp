#include "storage_manager.h"
#include "constants.h"  // ADD THIS LINE
#include <iostream>
#include <cstring>

using namespace std;

StorageManager::StorageManager(const string& filename) 
    : filename_(filename) {
}

StorageManager::~StorageManager() {
    shutdown();
}

bool StorageManager::initialize() {
    file_.open(filename_, ios::binary | ios::in | ios::out);
    
    if (!file_.is_open()) {
        // Create new file
        file_.open(filename_, ios::binary | ios::out);
        file_.close();
        file_.open(filename_, ios::binary | ios::in | ios::out);
        
        if (!file_.is_open()) {
            cerr << "Failed to create file: " << filename_ << endl;
            return false;
        }
        
        // Initialize superblock
        int root_block = -1;
        file_.seekp(0);
        file_.write(reinterpret_cast<const char*>(&root_block), sizeof(int));
        
        // Initialize bitmap (block 0 is used for superblock)
        bitmap_.resize(1, 0);
        set_block_used(0);
        save_bitmap();
        
        cout << "Created new database file: " << filename_ << endl;
    } else {
        load_bitmap();
    }
    
    return true;
}

void StorageManager::shutdown() {
    if (file_.is_open()) {
        save_bitmap();
        file_.close();
    }
}

void StorageManager::load_bitmap() {
    lock_guard<mutex> lock(file_mutex_);
    
    // Read bitmap size (first 8 bytes after root block)
    file_.seekg(BITMAP_START);
    uint64_t bitmap_size;
    file_.read(reinterpret_cast<char*>(&bitmap_size), sizeof(bitmap_size));
    
    bitmap_.resize(bitmap_size);
    if (bitmap_size > 0) {
        file_.read(reinterpret_cast<char*>(bitmap_.data()), 
                   bitmap_size * sizeof(uint64_t));
    }
}

void StorageManager::save_bitmap() {
    lock_guard<mutex> lock(file_mutex_);
    
    file_.seekp(BITMAP_START);
    uint64_t bitmap_size = bitmap_.size();
    file_.write(reinterpret_cast<const char*>(&bitmap_size), sizeof(bitmap_size));
    file_.write(reinterpret_cast<const char*>(bitmap_.data()), 
                bitmap_size * sizeof(uint64_t));
    file_.flush();
}

bool StorageManager::is_block_free(int block_index) const {
    int word_index = block_index / 64;
    int bit_index = block_index % 64;
    
    if (word_index >= static_cast<int>(bitmap_.size())) {
        return true; // Beyond current bitmap, considered free
    }
    
    return (bitmap_[word_index] & (1ULL << bit_index)) == 0;
}

void StorageManager::set_block_used(int block_index) {
    int word_index = block_index / 64;
    int bit_index = block_index % 64;
    
    // Resize bitmap if necessary
    if (word_index >= static_cast<int>(bitmap_.size())) {
        bitmap_.resize(word_index + 1, 0);
    }
    
    bitmap_[word_index] |= (1ULL << bit_index);
}

void StorageManager::set_block_free(int block_index) {
    int word_index = block_index / 64;
    int bit_index = block_index % 64;
    
    if (word_index < static_cast<int>(bitmap_.size())) {
        bitmap_[word_index] &= ~(1ULL << bit_index);
    }
}

int StorageManager::allocate_block() {
    // Find first free block
    for (int i = 0; i < static_cast<int>(bitmap_.size() * 64); i++) {
        if (is_block_free(i)) {
            set_block_used(i);
            
            // Initialize block with zeros
            char buffer[BLOCK_SIZE] = {0};
            write_block(i, buffer);
            
            return i;
        }
    }
    
    // No free block found, extend file and bitmap
    int new_block = bitmap_.size() * 64;
    set_block_used(new_block);
    
    // Initialize new block
    char buffer[BLOCK_SIZE] = {0};
    write_block(new_block, buffer);
    
    return new_block;
}

void StorageManager::deallocate_block(int block_index) {
    if (block_index > 0) { // Don't deallocate superblock
        set_block_free(block_index);
    }
}

void StorageManager::read_block(int block_index, char* buffer) {
    lock_guard<mutex> lock(file_mutex_);
    
    file_.seekg(block_index * BLOCK_SIZE);
    file_.read(buffer, BLOCK_SIZE);
}

void StorageManager::write_block(int block_index, const char* buffer) {
    lock_guard<mutex> lock(file_mutex_);
    
    file_.seekp(block_index * BLOCK_SIZE);
    file_.write(buffer, BLOCK_SIZE);
    file_.flush();
}

int StorageManager::get_root_block() const {
    lock_guard<mutex> lock(file_mutex_);
    
    int root_block = -1;
    file_.seekg(0);
    file_.read(reinterpret_cast<char*>(&root_block), sizeof(int));
    return root_block;
}

void StorageManager::set_root_block(int root_block) {
    lock_guard<mutex> lock(file_mutex_);
    
    file_.seekp(0);
    file_.write(reinterpret_cast<const char*>(&root_block), sizeof(int));
    file_.flush();
}