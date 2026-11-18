#include <iostream>
#include <string>
#include <cstdint>
#include <vector>

#include "ext4_parser.hpp"

int main()
{

    std::string imagePath = "test.img"; // relative to the working directory

    ext4_parser parser;
    parser.open_ext4_image(imagePath);
    parser.seek_to_superblock();
    if(!parser.read_superblock())
        std::cout << "Superblock read failed\n";

    uint16_t s_magic = parser.read_s_magic();
    uint32_t s_inodes_count = parser.read_s_inodes_count();
    uint32_t s_blocks_count_lo = parser.read_s_blocks_count_lo();
    uint32_t s_inodes_per_group = parser.read_s_inodes_per_group();
    uint32_t s_blocks_per_group = parser.read_s_blocks_per_group();
    uint32_t block_size = parser.read_s_log_block_size();
    uint16_t s_inode_size = parser.read_s_inode_size();
    uint32_t block_group_count = parser.get_block_group_count();

    std::cout << "Magic value is: " << std::hex << s_magic << std::dec << std::endl;
    std::cout << "Block size: " << block_size << std::endl;
    std::cout << "Inode size: " << s_inode_size << std::endl;
    std::cout << "Total Blocks count: " << s_blocks_count_lo << std::endl;
    std::cout << "Total Inodes count: " << s_inodes_count << std::endl;
    std::cout << "Blocks per group: " << s_blocks_per_group << std::endl;
    std::cout << "Inodes per group: " << s_inodes_per_group << std::endl;
    std::cout << "Blocks group count: " << block_group_count << std::endl;

    /* Block Group Descriptor */
    parser.seek_to_blockGroupDescriptor();
    if(!parser.read_blockGroupDescriptor())
        std::cout << "Block Group Descriptor read failed\n";

    uint32_t bg_block_bitmap_lo = parser.bgd.bg_block_bitmap_lo;
    uint32_t bg_inode_bitmap_lo = parser.bgd.bg_inode_bitmap_lo;
    uint32_t bg_inode_table_lo  = parser.bgd.bg_inode_table_lo;

    std::cout << "Location of block bitmap = " << std::hex << bg_block_bitmap_lo << std::dec << std::endl;
    std::cout << "Location of inode bitmap = " << std::hex << bg_inode_bitmap_lo << std::dec << std::endl;
    std::cout << "Location of inode table = " << std::hex  << bg_inode_table_lo  << std::dec << std::endl;

    parser.read_inode2();
    uint16_t i_mode = parser.inode.i_mode;
    uint32_t i_flags = parser.inode.i_flags;
    uint32_t i_size_lo = parser.inode.i_size_lo;
    uint8_t*  i_block = parser.inode.i_block;
    std::cout << "i_mode = " << std::hex << i_mode << std::dec << std::endl;
    std::cout << "i_flags = " << std::hex << i_flags << std::dec << std::endl;
    std::cout << "i_size_lo = " << std::hex << i_size_lo << std::dec << std::endl;
    std::cout << "i_block[0] = " << std::hex << static_cast<unsigned>(i_block[0]) << std::dec << std::endl;

    //Read Extent Header
    // TODO: Replace raw extent reinterpret_casts with memcpy/explicit little-endian decoding; packing does not address object lifetime or byte order.
    const auto* extent_header = reinterpret_cast<ext4::ext4_extent_header *>(parser.inode.i_block);
    std::cout << "eh_magic: 0x" << std::hex << extent_header->eh_magic << std::dec << std::endl;
    std::cout << "eh_entries: 0x" << std::hex << extent_header->eh_entries << std::dec << std::endl;
    std::cout << "eh_max: 0x" << std::hex << extent_header->eh_max << std::dec << std::endl;
    std::cout << "eh_depth: 0x" << std::hex << extent_header->eh_depth << std::dec << std::endl;
    std::cout << "eh_generation: 0x" << std::hex << extent_header->eh_generation << std::dec << std::endl;

    if(extent_header->eh_magic != ext4::EXTENT_MAGIC)
    {
        std::cout << "Invalid extent magic" << std::endl;
        return 1;
    }

    if(extent_header->eh_depth == 0)
    {
        /* Following entries are Leaf nodes, which directly map to disk blocks*/

        //Read First Extent Entry. Seek past 12 bytes of Extent Header.
        const auto* extent = reinterpret_cast<ext4::ext4_extent*>(parser.inode.i_block + sizeof(ext4::ext4_extent_header));

        uint64_t physical_block = static_cast<uint64_t> (static_cast<uint64_t>(extent->ee_start_hi) << 32) | extent->ee_start_lo;
        uint64_t dir_block_offset = physical_block * block_size;

        std::cout << "ee_block: 0x" << std::hex << extent->ee_block << std::dec << std::endl;
        std::cout << "ee_len: 0x" << std::hex << extent->ee_len << std::dec << std::endl;
        std::cout << "Physical Block: 0x" << std::hex << physical_block << std::dec << std::endl;
        std::cout << "Directory Block Offset: 0x" << std::hex << dir_block_offset << std::dec << std::endl;

        // Create buffer to hold data of size block_size(block_size)
        std::vector<uint8_t> buffer(block_size);

        //Read into buffer
        if(!parser.read_block(buffer, dir_block_offset, block_size))
            return 1;

        // Read directory entries
        parser.parse_directory_entries(buffer, block_size);

    }

    return 0;

}