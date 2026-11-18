#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <fstream>

#include "on-disk-format.hpp"

class ext4_parser{
public:
    std::ifstream binFile;

    ext4::ext4_superblock sb{};
    ext4::ext4_blockGroupDescriptors bgd{};
    ext4::ext4_inode inode{};
    ext4::ext4_extent_header eh{};
    ext4::ext4_extent_idx ei{};
    ext4::ext4_extent ext{};
    ext4::ext4_dir_entry_2 dir2{};

    std::string file_type_name(ext4::ext4_file_type type);
    bool open_ext4_image(std::string&);
    bool seek_to_superblock();
    bool seek_to_blockGroupDescriptor();
    bool seek_to_inode_table();
    bool seek_to_offset(uint64_t offset);
    bool read_superblock();
    bool read_blockGroupDescriptor();
    bool read_inode2();
    bool read_block(std::vector<uint8_t>&, uint64_t, uint32_t);
    uint16_t read_s_magic();
    uint32_t read_s_inodes_count();
    uint32_t read_s_blocks_count_lo();
    uint32_t read_s_inodes_per_group();
    uint32_t read_s_blocks_per_group();
    uint32_t read_s_log_block_size();
    uint16_t read_s_inode_size();
    uint32_t get_block_group_count();
    void parse_directory_entries(std::vector<uint8_t>&, uint32_t);
};
