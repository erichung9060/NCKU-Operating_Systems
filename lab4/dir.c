#include <linux/fs.h>
#include <linux/string.h>
#include <linux/slab.h>
#include "osfs.h"

/**
 * Function: osfs_lookup
 * Description: Looks up a file within a directory.
 * Inputs:
 *   - dir: The inode of the directory to search in.
 *   - dentry: The dentry representing the file to look up.
 *   - flags: Flags for the lookup operation.
 * Returns:
 *   - A pointer to the dentry if the file is found.
 *   - NULL if the file is not found, allowing the VFS to handle it.
 */
static struct dentry *osfs_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags)
{
    struct osfs_sb_info *sb_info = dir->i_sb->s_fs_info;
    struct osfs_inode *parent_inode = dir->i_private;
    void *dir_data_block;
    struct osfs_dir_entry *dir_entries;
    int dir_entry_count;
    int i;
    struct inode *inode = NULL;

    pr_info("osfs_lookup: Looking up '%.*s' in inode %lu\n",
            (int)dentry->d_name.len, dentry->d_name.name, dir->i_ino);

    // Read the parent directory's data block
    dir_data_block = sb_info->data_blocks + parent_inode->i_block * BLOCK_SIZE;

    // Calculate the number of directory entries
    dir_entry_count = parent_inode->i_size / sizeof(struct osfs_dir_entry);
    dir_entries = (struct osfs_dir_entry *)dir_data_block;

    // Traverse the directory entries to find a matching filename
    for (i = 0; i < dir_entry_count; i++) {
        if (strlen(dir_entries[i].filename) == dentry->d_name.len &&
            strncmp(dir_entries[i].filename, dentry->d_name.name, dentry->d_name.len) == 0) {
            // File found, get inode
            inode = osfs_iget(dir->i_sb, dir_entries[i].inode_no);
            if (IS_ERR(inode)) {
                pr_err("osfs_lookup: Error getting inode %u\n", dir_entries[i].inode_no);
                return ERR_CAST(inode);
            }
            return d_splice_alias(inode, dentry);
        }
    }

    return NULL;
}

/**
 * Function: osfs_iterate
 * Description: Iterates over the entries in a directory.
 * Inputs:
 *   - filp: The file pointer representing the directory.
 *   - ctx: The directory context used for iteration.
 * Returns:
 *   - 0 on successful iteration.
 *   - A negative error code on failure.
 */
static int osfs_iterate(struct file *filp, struct dir_context *ctx)
{
    struct inode *inode = file_inode(filp);
    struct osfs_sb_info *sb_info = inode->i_sb->s_fs_info;
    struct osfs_inode *osfs_inode = inode->i_private;
    void *dir_data_block;
    struct osfs_dir_entry *dir_entries;
    int dir_entry_count;
    int i;

    if (ctx->pos == 0) {
        if (!dir_emit_dots(filp, ctx))
            return 0;
    }

    dir_data_block = sb_info->data_blocks + osfs_inode->i_block * BLOCK_SIZE;
    dir_entry_count = osfs_inode->i_size / sizeof(struct osfs_dir_entry);
    dir_entries = (struct osfs_dir_entry *)dir_data_block;

    /* Adjust the index based on ctx->pos */
    i = ctx->pos - 2;

    for (; i < dir_entry_count; i++) {
        struct osfs_dir_entry *entry = &dir_entries[i];
        unsigned int type = DT_UNKNOWN;

        if (!dir_emit(ctx, entry->filename, strlen(entry->filename), entry->inode_no, type)) {
            pr_err("osfs_iterate: dir_emit failed for entry '%s'\n", entry->filename);
            return -EINVAL;
        }

        ctx->pos++;
    }

    return 0;
}

/**
 * Function: osfs_new_inode
 * Description: Creates a new inode within the filesystem.
 * Inputs:
 *   - dir: The inode of the directory where the new inode will be created.
 *   - mode: The mode (permissions and type) for the new inode.
 * Returns:
 *   - A pointer to the newly created inode on success.
 *   - ERR_PTR(-EINVAL) if the file type is not supported.
 *   - ERR_PTR(-ENOSPC) if there are no free inodes or blocks.
 *   - ERR_PTR(-ENOMEM) if memory allocation fails.
 *   - ERR_PTR(-EIO) if an I/O error occurs.
 */
struct inode *osfs_new_inode(const struct inode *dir, umode_t mode)
{
    struct super_block *sb = dir->i_sb;
    struct osfs_sb_info *sb_info = sb->s_fs_info;
    struct inode *inode;
    struct osfs_inode *osfs_inode;
    int ino, ret;

    /* Check if the mode is supported */
    if (!S_ISDIR(mode) && !S_ISREG(mode) && !S_ISLNK(mode)) {
        pr_err("File type not supported (only directory, regular file and symlink supported)\n");
        return ERR_PTR(-EINVAL);
    }

    /* Check if there are free inodes and blocks */
    if (sb_info->nr_free_inodes == 0 || sb_info->nr_free_blocks == 0)
        return ERR_PTR(-ENOSPC);

    /* Allocate a new inode number */
    ino = osfs_get_free_inode(sb_info);
    if (ino < 0 || ino >= sb_info->inode_count)
        return ERR_PTR(-ENOSPC);

    /* Allocate a new VFS inode */
    inode = new_inode(sb);
    if (!inode)
        return ERR_PTR(-ENOMEM);

    /* Initialize inode owner and permissions */
    inode_init_owner(&nop_mnt_idmap, inode, dir, mode);
    inode->i_ino = ino;
    inode->i_sb = sb;
    inode->i_blocks = 0;
    simple_inode_init_ts(inode);

    /* Set inode operations based on file type */
    if (S_ISDIR(mode)) {
        inode->i_op = &osfs_dir_inode_operations;
        inode->i_fop = &osfs_dir_operations;
        set_nlink(inode, 2); /* . and .. */
        inode->i_size = 0;
    } else if (S_ISREG(mode)) {
        inode->i_op = &osfs_file_inode_operations;
        inode->i_fop = &osfs_file_operations;
        set_nlink(inode, 1);
        inode->i_size = 0;
    } else if (S_ISLNK(mode)) {
        // inode->i_op = &osfs_symlink_inode_operations;
        set_nlink(inode, 1);
        inode->i_size = 0;
    }

    /* Get osfs_inode */
    osfs_inode = osfs_get_osfs_inode(sb, ino);
    if (!osfs_inode) {
        pr_err("osfs_new_inode: Failed to get osfs_inode for inode %d\n", ino);
        iput(inode);
        return ERR_PTR(-EIO);
    }
    memset(osfs_inode, 0, sizeof(*osfs_inode));

    /* Initialize osfs_inode */
    osfs_inode->i_ino = ino;
    osfs_inode->i_mode = inode->i_mode;
    osfs_inode->i_uid = i_uid_read(inode);
    osfs_inode->i_gid = i_gid_read(inode);
    osfs_inode->i_size = inode->i_size;
    osfs_inode->i_blocks = 1; // Simplified handling
    osfs_inode->__i_atime = osfs_inode->__i_mtime = osfs_inode->__i_ctime = current_time(inode);
    inode->i_private = osfs_inode;

    /* Allocate data block */
    ret = osfs_alloc_data_block(sb_info, &osfs_inode->i_block);
    if (ret) {
        pr_err("osfs_new_inode: Failed to allocate data block\n");
        iput(inode);
        return ERR_PTR(ret);
    }

    /* Update superblock information */
    sb_info->nr_free_inodes--;

    /* Mark inode as dirty */
    mark_inode_dirty(inode);

    return inode;
}

static int osfs_add_dir_entry(struct inode *dir, uint32_t inode_no, const char *name, size_t name_len)
{
    struct osfs_sb_info *sb_info = dir->i_sb->s_fs_info;
    struct osfs_inode *parent_inode = dir->i_private;
    void *dir_data_block;
    struct osfs_dir_entry *dir_entries;
    int dir_entry_count;
    int i;

    // Read the parent directory's data block
    dir_data_block = sb_info->data_blocks + parent_inode->i_block * BLOCK_SIZE;

    // Calculate the existing number of directory entries
    dir_entry_count = parent_inode->i_size / sizeof(struct osfs_dir_entry);
    if (dir_entry_count >= MAX_DIR_ENTRIES) {
        pr_err("osfs_add_dir_entry: Parent directory is full\n");
        return -ENOSPC;
    }

    dir_entries = (struct osfs_dir_entry *)dir_data_block;

    // Check if a file with the same name exists
    for (i = 0; i < dir_entry_count; i++) {
        if (strlen(dir_entries[i].filename) == name_len &&
            strncmp(dir_entries[i].filename, name, name_len) == 0) {
            pr_warn("osfs_add_dir_entry: File '%.*s' already exists\n", (int)name_len, name);
            return -EEXIST;
        }
    }

    // Add a new directory entry
    strncpy(dir_entries[dir_entry_count].filename, name, name_len);
    dir_entries[dir_entry_count].filename[name_len] = '\0';
    dir_entries[dir_entry_count].inode_no = inode_no;

    // Update the size of the parent directory
    parent_inode->i_size += sizeof(struct osfs_dir_entry);

    return 0;
}


/**
 * Function: osfs_create
 * Description: Creates a new file within a directory.
 * Inputs:
 *   - idmap: The mount namespace ID map.
 *   - dir: The inode of the parent directory.
 *   - dentry: The dentry representing the new file.
 *   - mode: The mode (permissions and type) for the new file.
 *   - excl: Whether the creation should be exclusive.
 * Returns:
 *   - 0 on successful creation.
 *   - -EEXIST if the file already exists.
 *   - -ENAMETOOLONG if the file name is too long.
 *   - -ENOSPC if the parent directory is full.
 *   - A negative error code from osfs_new_inode on failure.
 */
static int osfs_create(struct mnt_idmap *idmap, struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{   
    // Step1: Parse the parent directory passed by the VFS 
    struct osfs_inode *parent_inode = dir->i_private;
    struct osfs_inode *osfs_inode;
    struct inode *inode;
    int ret;

    // Step2: Validate the file name length


    // Step3: Allocate and initialize VFS & osfs inode
    

    osfs_inode = inode->i_private;
    if (!osfs_inode) {
        pr_err("osfs_create: Failed to get osfs_inode for inode %lu\n", inode->i_ino);
        iput(inode);
        return -EIO;
    }
    // init osfs_inode attribute
    osfs_inode->i_block = 0; 
    osfs_inode->i_size = 0;
    osfs_inode->i_blocks = 0;

    // Step4: Parent directory entry update for the new file

    if (ret) {
        pr_err("osfs_create: Failed to add directory entry\n");
        iput(inode);
        return ret;
    }

    // Step 5: Update the parent directory's metadata 
    
    
    // Step 6: Bind the inode to the VFS dentry

    pr_info("osfs_create: File '%.*s' created with inode %lu\n",
            (int)dentry->d_name.len, dentry->d_name.name, inode->i_ino);

    return 0;
}



const struct inode_operations osfs_dir_inode_operations = {
    .lookup = osfs_lookup,
    .create = osfs_create,
    // Add other operations as needed
};

const struct file_operations osfs_dir_operations = {
    .iterate_shared = osfs_iterate,
    .llseek = generic_file_llseek,
    // Add other operations as needed
};
