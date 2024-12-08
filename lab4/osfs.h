#ifndef _OSFS_H
#define _OSFS_H

#include <linux/types.h>      // Include basic type definitions
#include <linux/fs.h>
#include <linux/bitmap.h>    // For bitmap operations
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/module.h>

#define OSFS_MAGIC 0x051AB520
//#define BLOCK_SIZE 4096       // Each data block size is 4KB
#define INODE_COUNT 20         // Maximum of 20 inodes in the filesystem
#define DATA_BLOCK_COUNT 20    // Assume there are 20 data blocks
#define MAX_FILENAME_LEN 255
#define MAX_DIR_ENTRIES (BLOCK_SIZE / sizeof(struct osfs_dir_entry))

#define BITMAP_SIZE(bits) (((bits) + BITS_PER_LONG - 1) / BITS_PER_LONG)

// Calculate the size of the bitmap (in units of unsigned long)
#define INODE_BITMAP_SIZE BITMAP_SIZE(INODE_COUNT)
#define BLOCK_BITMAP_SIZE BITMAP_SIZE(DATA_BLOCK_COUNT)

#define ROOT_INODE 1            // Define the root inode as 1

/**
 * Struct: osfs_sb_info
 * Description: Superblock information for the osfs filesystem.
 */
struct osfs_sb_info {
    uint32_t magic;              // Magic number to identify the filesystem
    uint32_t block_size;         // Size of each data block
    uint32_t inode_count;        // Total number of inodes
    uint32_t block_count;        // Total number of data blocks
    uint32_t nr_free_inodes;     // Number of free inodes
    uint32_t nr_free_blocks;     // Number of free data blocks
    unsigned long *inode_bitmap; // Pointer to the inode bitmap
    unsigned long *block_bitmap; // Pointer to the data block bitmap
    void *inode_table;           // Pointer to the inode table
    void *data_blocks;           // Pointer to the data blocks area
};

/**
 * Struct: osfs_dir_entry
 * Description: Directory entry structure.
 */
struct osfs_dir_entry {
    char filename[MAX_FILENAME_LEN]; // File name
    uint32_t inode_no;               // Corresponding inode number
};

/**
 * Struct: osfs_inode
 * Description: Filesystem-specific inode structure.
 */
struct osfs_inode {
    uint32_t i_ino;                     // Inode number
    uint32_t i_size;                    // File size in bytes
    uint32_t i_blocks;                  // Number of blocks occupied by the file
    uint16_t i_mode;                    // File mode (permissions and type)
    uint16_t i_links_count;             // Number of hard links
    uint32_t i_uid;                     // User ID of owner
    uint32_t i_gid;                     // Group ID of owner
    struct timespec64 __i_atime;        // Last access time
    struct timespec64 __i_mtime;        // Last modification time
    struct timespec64 __i_ctime;        // Creation time
    uint32_t i_block;                   // Simplified handling, single data block pointer
};

struct inode *osfs_iget(struct super_block *sb, unsigned long ino);
struct osfs_inode *osfs_get_osfs_inode(struct super_block *sb, uint32_t ino);
int osfs_get_free_inode(struct osfs_sb_info *sb_info);
int osfs_alloc_data_block(struct osfs_sb_info *sb_info, uint32_t *block_no);
int osfs_fill_super(struct super_block *sb, void *data, int silent);
struct inode *osfs_new_inode(const struct inode *dir, umode_t mode);
void osfs_free_data_block(struct osfs_sb_info *sb_info, uint32_t block_no);
void osfs_destroy_inode(struct inode *inode);
// External Operations Structures

extern const struct inode_operations osfs_file_inode_operations;
extern const struct file_operations osfs_file_operations;
extern const struct inode_operations osfs_dir_inode_operations;
extern const struct file_operations osfs_dir_operations;
extern const struct super_operations osfs_super_ops;

#endif /* _osfs_H */
