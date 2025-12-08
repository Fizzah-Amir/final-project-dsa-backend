#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <fstream>
#include <string>
#include <vector>
#include <mutex>
#include <cstdint>

// Forward declare constants (defined in constants.h)
extern const int BLOCK_SIZE;

class StorageManager {
public:
    StorageManager(const std::string& filename);
    ~StorageManager();
    
    bool initialize();
    void shutdown();
    
    // Block management
    int allocate_block();
    void deallocate_block(int block_index);
    void read_block(int block_index, char* buffer);
    void write_block(int block_index, const char* buffer);
    
    // Superblock operations
    int get_root_block() const;
    void set_root_block(int root_block);
    
private:
    std::string filename_;
    mutable std::fstream file_;
    mutable std::mutex file_mutex_;
    
    // Bitmap management
    std::vector<uint64_t> bitmap_; // Using uint64_t for efficient bit operations
    static const int BITMAP_START = 8; // After root block index
    
    void load_bitmap();
    void save_bitmap();
    bool is_block_free(int block_index) const;
    void set_block_used(int block_index);
    void set_block_free(int block_index);
};

#endif