#include <iostream>
#include <string>
#include <cstdint>
#include <vector>

#include "ext4_parser.hpp"

std::string ext4_parser::file_type_name(ext4::ext4_file_type type)
{
    switch(type)
    {
        case ext4::ext4_file_type::unknown: return "UNKNOWN";
        case ext4::ext4_file_type::regular: return "REG";
        case ext4::ext4_file_type::directory: return "DIR";
        case ext4::ext4_file_type::character_device: return "CHAR_DEV";
        case ext4::ext4_file_type::block_device: return "BLOCK_DEV";
        case ext4::ext4_file_type::fifo: return "FIFO";
        case ext4::ext4_file_type::socket: return "SOCKET";
        case ext4::ext4_file_type::symlink: return "SYM_LINK";
        default: return "INVALID";
    }
}

bool ext4_parser::open_ext4_image(std::string& imgPath)
{
    if(imgPath.empty()) //if no path was given
        return false;

    if(binFile.is_open()) //if it is already open
        return true;

    binFile.open(imgPath, std::ios::in | std::ios::binary); //open for read in binary
    if(!binFile.is_open())
        return false;
    else
    {
        std::cout << "EXT4 image successfully opened." << std::endl;
        return true;
    }
}

bool ext4_parser::seek_to_superblock()
{
    if(!binFile.is_open())
        return false;
    binFile.seekg(ext4::SUPERBLOCK_OFFSET, std::ios::beg);
    std::cout << "Seeked to Superblock position.\n";
    return true;
}

bool ext4_parser::seek_to_blockGroupDescriptor()
{
    /* If block size == 1024 bytes
     *   then reserved space is block 0.
     *   superblock is in block 1.
     *   block group descriptor is in block 2.
     * Else
     *   then reserved space + superblock is in block 0 (eg. 4096 bytes)
     *   Block group descriptor is in block 1.
     * End */
    if(!binFile.is_open())
        return false;

    if((1024 << sb.s_log_block_size) == ext4::DEFAULT_BLOCK_SIZE)
    {
        binFile.seekg(ext4::BLOCKGROUPDESCRIPTOR_POSITION, std::ios::beg);
    }
    else
    {
        binFile.seekg((1024 << sb.s_log_block_size), std::ios::beg);
    }
    std::cout << "Seeked to Block Group Descriptor position\n";
    return true;
}

bool ext4_parser::seek_to_inode_table()
{
    /* Inode table is at block number bg_inode_table_lo
     * location is bg_inode_table_lo * block_size
     */
    if(!binFile.is_open())
        return false;

    uint32_t block_size = (1024 << sb.s_log_block_size);

    binFile.seekg(bgd.bg_inode_table_lo*block_size, std::ios::beg);

    return static_cast<bool>(binFile);
}

bool ext4_parser::seek_to_offset(uint64_t offset)
{
    if(!binFile.is_open())
        return false;

    binFile.seekg(static_cast<std::streamoff>(offset), std::ios::beg);

    return static_cast<bool>(binFile);
}

bool ext4_parser::read_superblock()
{
    if(!binFile.is_open())
        return false;

    binFile.read(reinterpret_cast<char*>(&sb), sizeof(sb)); //static_cast doesnt work.

    //Check if read was successful and check if 1024 bytes were read
    if(binFile && binFile.gcount() == sizeof(sb))
    {
        std::cout << "Superblock read into memory.\n";
        return true;
    }
    else
    {
        std::cout << "Failed to read Superblock\n";
        return false;
    }
}

bool ext4_parser::read_blockGroupDescriptor()
{
    if(!binFile.is_open())
        return false;

    binFile.read(reinterpret_cast<char*>(&bgd), sizeof(bgd));

    //Check if read was successful and check if 32 bytes were read
    if(binFile && binFile.gcount() == sizeof(bgd))
    {
        std::cout << "Block Group Descriptor read into memory\n";
        return true;
    }
    else
    {
        std::cout << "Failed to read Block Group Descriptor\n";
        return false;
    }
}

bool ext4_parser::read_inode2()
{
    /* Go to offset of inode2, which is index 1.*/
    if(!binFile.is_open())
        return false;
    uint32_t block_size = (1024 << sb.s_log_block_size);
    uint32_t inode_table_position = bgd.bg_inode_table_lo * block_size;
    uint32_t offset = sb.s_inode_size * (2-1);

    /* Seek to inode2 */
    binFile.seekg(inode_table_position+offset, std::ios::beg);

    /* Read into memory */
    binFile.read(reinterpret_cast<char*>(&inode), sizeof(inode));

    if(binFile && binFile.gcount() == sizeof(inode))
    {
        std::cout << "Inode2 read into memory\n";
        return true;
    }
    else
    {
        std::cout << "Failed to read inode2\n";
        return false;
    }
}

bool ext4_parser::read_block(std::vector<uint8_t>& buffer, uint64_t offset, uint32_t block_size)
{
    if(!binFile.is_open())
        return false;

    binFile.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    binFile.read(reinterpret_cast<char*>(buffer.data()), block_size);
    if(binFile && binFile.gcount() == (block_size))
    {
        std::cout << "Directory block read into memory\n";
        return true;
    }
    else
    {
        std::cout << "Failed to read Directory block\n";
        return false;
    }
}

uint16_t ext4_parser::read_s_magic()
{
    return sb.s_magic;
}

uint32_t ext4_parser::read_s_inodes_count()
{
    return sb.s_inodes_count;
}

uint32_t ext4_parser::read_s_blocks_count_lo()
{
    return sb.s_blocks_count_lo;
}

uint32_t ext4_parser::read_s_inodes_per_group()
{
    return sb.s_inodes_per_group;
}

uint32_t ext4_parser::read_s_blocks_per_group()
{
    return sb.s_blocks_per_group;
}

uint32_t ext4_parser::read_s_log_block_size()
{
    return (1024 << sb.s_log_block_size);
}

uint16_t ext4_parser::read_s_inode_size()
{
    return sb.s_inode_size;
}

uint32_t ext4_parser::get_block_group_count()
{
    uint32_t blocks_count = sb.s_blocks_count_lo - sb.s_first_data_block; //exclude first block

    if(blocks_count == 0 || sb.s_blocks_per_group == 0)
        return 0;

    uint32_t ret;
    ret = (blocks_count / sb.s_blocks_per_group) + (blocks_count % sb.s_blocks_per_group != 0);
    return ret;
}

void ext4_parser::parse_directory_entries(std::vector<uint8_t>& buffer, uint32_t block_size)
{
    /* Read until the end of block. Note: Last entry will span entire block */
    size_t offset = 0;
    std::cout << "Directory entries: \n";
    while(offset < block_size)
    {
        /* Read inode, rec_len, name_len, file_type and name[] into memory */
        const auto* entry = reinterpret_cast<ext4::ext4_dir_entry_2*>(buffer.data()+offset);

        /* rec_len must not be 0 */
        if(entry->rec_len == 0)
            break;


        /* Inode 0 does not exist. */
        if(entry->inode != 0)
        {
            const char* name = reinterpret_cast<const char*>(entry) + sizeof(entry);
            std::string filename(name, entry->name_len);
            std::string file_type = file_type_name(static_cast<ext4::ext4_file_type>(entry->file_type));
            uint16_t rec_len = entry->rec_len;
            uint32_t ino = entry->inode;

            /* Print Directory entries */
            std::cout << file_type << " " << filename << " " << "Inode: " << ino << " " << "rec_len: " << rec_len << std::endl;
        }
        offset = offset + entry->rec_len;
    }
}
