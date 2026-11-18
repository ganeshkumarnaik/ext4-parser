#pragma once

#include <cstdint>

namespace ext4
{
    /* Using pragma pack with alignment capped at 1 byte. */
    #pragma pack(push, 1)
    struct ext4_superblock {
        uint32_t s_inodes_count;          // 0x00: Total inode count
        uint32_t s_blocks_count_lo;       // 0x04: Total block count
        uint32_t s_r_blocks_count_lo;     // 0x08: Reserved block count
        uint32_t s_free_blocks_count_lo;  // 0x0C: Free block count
        uint32_t s_free_inodes_count;     // 0x10: Free inode count
        uint32_t s_first_data_block;      // 0x14: First data block (0 for 4KB blocks, 1 for 1KB)
        uint32_t s_log_block_size;        // 0x18: Block size is 2 ^ (10 + s_log_block_size).
        uint32_t s_log_cluster_size;      // 0x1C: Cluster size
        uint32_t s_blocks_per_group;      // 0x20: Blocks per group
        uint32_t s_clusters_per_group;    // 0x24: Clusters per group
        uint32_t s_inodes_per_group;      // 0x28: Inodes per group
        uint32_t s_mtime;                 // 0x2C: Mount time
        uint32_t s_wtime;                 // 0x30: Write time
        uint16_t s_mnt_count;             // 0x34: Mount count
        uint16_t s_max_mnt_count;         // 0x36: Maximal mount count
        uint16_t s_magic;                 // 0x38: Magic signature (0xEF53)
        uint16_t s_state;                 // 0x3A: File system state
        uint16_t s_errors;                // 0x3C: Behaviour when detecting errors
        uint16_t s_minor_rev_level;       // 0x3E: Minor revision level
        uint32_t s_lastcheck;             // 0x40: Time of last check
        uint32_t s_checkinterval;         // 0x44: Max time between checks
        uint32_t s_creator_os;            // 0x48: OS creator
        uint32_t s_rev_level;             // 0x4C: Revision level
        uint16_t s_def_resuid;            // 0x50: Default uid for reserved blocks
        uint16_t s_def_resgid;            // 0x52: Default gid for reserved blocks
        uint32_t s_first_ino;             // 0x54: First non-reserved inode
        uint16_t s_inode_size;            // 0x58: Size of inode structure
        uint16_t s_block_group_nr;        // 0x5A: Block group # of this superblock
        uint32_t s_feature_compat;        // 0x5C: Compatible feature set
        uint32_t s_feature_incompat;      // 0x60: Incompatible feature set
        uint32_t s_feature_ro_compat;     // 0x64: Readonly-compatible feature set
        uint8_t  s_uuid[16];              // 0x68: 128-bit UUID for volume
        char     s_volume_name[16];       // 0x78: Volume name
        char     s_last_mounted[64];      // 0x88: Directory path where last mounted
        uint32_t s_algorithm_usage_bitmap;// 0xC8: For compression

        /* Pad remaining for future updates */
        uint8_t  s_padding[820];
    };
    #pragma pack(pop)
    static_assert(sizeof(ext4_superblock) == 1024, "Ext4 Super block must be 1024 bytes long");

    #pragma pack(push, 1)
    struct ext4_blockGroupDescriptors{
        uint32_t bg_block_bitmap_lo;      // 0x00: Block bitmap
        uint32_t bg_inode_bitmap_lo;      // 0x04: Inodes bitmap
        uint32_t bg_inode_table_lo;       // 0x08: Inodes table block number
        uint16_t bg_free_blocks_count_lo; // 0x0C: Free blocks
        uint16_t bg_free_inodes_count_lo; // 0x0E: Free inodes
        uint16_t bg_used_dirs_count_lo;   // 0x10: Directories count
        uint16_t bg_flags;                // 0x12: Block group flags
        uint32_t bg_exclude_bitmap_lo;    // 0x14: Snapshot exclusion bitmap
        uint16_t bg_block_bitmap_csum_lo; // 0x18: Block bitmap checksum
        uint16_t bg_inode_bitmap_csum_lo; // 0x1A: Inode bitmap checksum
        uint16_t bg_itable_unused_lo;     // 0x1C: Unused inodes count
        uint16_t bg_checksum;             // 0x1E: Group descriptor checksum
    };
    #pragma pack(pop)
    static_assert(sizeof(ext4_blockGroupDescriptors) == 32, "Ext4 block group descriptor for 32bit FS must be 32 bytes");

    #pragma pack(push, 1)
    struct ext4_inode{
        uint16_t i_mode;
        uint16_t i_uid;
        uint32_t i_size_lo;
        uint32_t i_atime;
        uint32_t i_ctime;
        uint32_t i_mtime;
        uint32_t i_dtime;
        uint16_t i_gid;
        uint16_t i_links_count;
        uint32_t i_blocks_lo;
        uint32_t i_flags;
        uint32_t i_osd1;
        uint8_t  i_block[60];
        uint32_t i_generation;
        uint32_t i_file_acl_lo;
        uint32_t i_size_high;
        uint32_t i_obso_faddr;
        uint8_t  i_osd2[12];
        uint16_t i_extra_isize;
        uint16_t i_checksum_hi;
        uint32_t i_ctime_extra;
        uint32_t i_mtime_extra;
        uint32_t i_atime_extra;
        uint32_t i_crtime;
        uint32_t i_crtime_extra;
        uint32_t i_version_hi;
        uint32_t i_projid;
        uint8_t  i_padding[96];
    };
    #pragma pack(pop)
    static_assert(sizeof(ext4_inode) == 256, "Size of Inode must be 256 bytes");

    #pragma pack(push,1)
    struct ext4_extent_header{
        uint16_t eh_magic;
        uint16_t eh_entries;
        uint16_t eh_max;
        uint16_t eh_depth;
        uint32_t eh_generation;
    };
    #pragma pack(pop)
    static_assert(sizeof(ext4_extent_header) == 12, "Extent Header must be 12 bytes");

    #pragma pack(push,1)
    struct ext4_extent_idx{
        uint32_t ei_block;
        uint32_t ei_leaf_lo;
        uint16_t ei_leaf_hi;
        uint16_t ei_unused;
    };
    #pragma pack(pop)
    static_assert(sizeof(ext4_extent_idx) == 12, "Extent Index must be 12 bytes");

    #pragma pack(push,1)
    struct ext4_extent{
        uint32_t ee_block;
        uint16_t ee_len;
        uint16_t ee_start_hi;
        uint32_t ee_start_lo;
    };
    #pragma pack(pop)
    static_assert(sizeof(ext4_extent) == 12, "Extent must be 12 bytes");

    #pragma pack(push, 1)
    struct ext4_dir_entry_2{
        uint32_t inode;
        uint16_t rec_len;
        uint8_t name_len;
        uint8_t file_type;
        /* No need to add the name[] here */
    };
    #pragma pack(pop)
    static_assert(sizeof(ext4_dir_entry_2) == 8, "Directory entry header must be 8 bytes");

    enum class ext4_file_type : uint8_t {
        unknown = 0,
        regular = 1,
        directory = 2,
        character_device = 3,
        block_device = 4,
        fifo = 5,
        socket = 6,
        symlink = 7
    };

    inline constexpr uint16_t SUPERBLOCK_MAGIC = 0xEF53;
    inline constexpr uint16_t EXTENT_MAGIC = 0xF30A;
    inline constexpr uint16_t SUPERBLOCK_OFFSET = 1024;
    inline constexpr uint16_t DEFAULT_BLOCK_SIZE = 1024;
    inline constexpr int  BLOCKGROUPDESCRIPTOR_POSITION = 2048;

}
