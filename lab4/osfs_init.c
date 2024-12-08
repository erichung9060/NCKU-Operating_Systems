#include <linux/init.h>
#include <linux/module.h>
#include "osfs.h"

/**
 * Function: osfs_mount
 * Description: Mounts the osfs filesystem.
 * Inputs:
 *   - fs_type: The file system type structure.
 *   - flags: Mount flags.
 *   - dev_name: Device name (not used in nodev filesystem).
 *   - data: Data passed during mount.
 * Returns:
 *   - A dentry pointer to the root of the mounted filesystem.
 */
static struct dentry *osfs_mount(struct file_system_type *fs_type,
                                 int flags,
                                 const char *dev_name,
                                 void *data);

/**
 * Function: osfs_kill_superblock
 * Description: Cleans up and releases the superblock of the filesystem.
 * Inputs:
 *   - sb: The superblock to be killed.
 * Returns:
 *   - None.
 */
static void osfs_kill_superblock(struct super_block *sb);

/**
 * Struct: osfs_type
 * Description: Defines the file system type for osfs.
 */
struct file_system_type osfs_type = {
    .owner = THIS_MODULE,
    .name = "osfs",
    .mount = osfs_mount,
    .kill_sb = osfs_kill_superblock,
    .fs_flags = FS_USERNS_MOUNT,
};

/**
 * Function: osfs_init
 * Description: Initializes the osfs module by registering the filesystem.
 * Inputs:
 *   - None.
 * Returns:
 *   - 0 on successful registration.
 *   - A negative error code on failure.
 */
static int __init osfs_init(void)
{
    int ret;

    ret = register_filesystem(&osfs_type);
    if (ret) {
        pr_err("Failed to register filesystem\n");
        return ret;
    }

    pr_info("osfs: Successfully registered\n");
    return 0;
}

/**
 * Function: osfs_exit
 * Description: Cleans up the osfs module by unregistering the filesystem.
 * Inputs:
 *   - None.
 * Returns:
 *   - None.
 */
static void __exit osfs_exit(void)
{
    int ret;

    ret = unregister_filesystem(&osfs_type);
    if (ret)
        pr_err("Failed to unregister filesystem\n");
    else
        pr_info("osfs: Successfully unregistered\n");
}

/**
 * Function: osfs_mount
 * Description: Handles the mounting of the osfs filesystem.
 * Inputs:
 *   - fs_type: The file system type structure.
 *   - flags: Mount flags.
 *   - dev_name: Device name (not used in nodev filesystem).
 *   - data: Data passed during mount.
 * Returns:
 *   - A dentry pointer to the root of the mounted filesystem.
 */
static struct dentry *osfs_mount(struct file_system_type *fs_type,
                                 int flags,
                                 const char *dev_name,
                                 void *data)
{
    return mount_nodev(fs_type, flags, data, osfs_fill_super);
}

/**
 * Function: osfs_kill_superblock
 * Description: Cleans up and releases the superblock of the filesystem.
 * Inputs:
 *   - sb: The superblock to be killed.
 * Returns:
 *   - None.
 */
static void osfs_kill_superblock(struct super_block *sb)
{
    struct osfs_sb_info *sb_info = sb->s_fs_info;

    pr_info("osfs_kill_superblock: Unmounting file system\n");

    if (sb_info) {
        pr_info("osfs_kill_superblock: free blcok \n");

        vfree(sb_info);
        sb->s_fs_info = NULL;
    }

    pr_info("osfs_kill_superblock: File system unmounted successfully\n");
}

module_init(osfs_init);
module_exit(osfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("OSLAB");
MODULE_DESCRIPTION("A simple memory-based file system kernel module");
