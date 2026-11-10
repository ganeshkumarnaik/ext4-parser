#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <string_view>
#include <cstdint>
#include <map>
#include <vector>

class ext4_parser{
public:
    std::ifstream binFile;
    int superblock_position = 1024;
    int blockGroupDescriptor_position1 = 2048;
    int s_magic_offset = 0x38;

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
    }sb;
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
    }bgd;
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
    }inode;
    #pragma pack(pop)
    static_assert(sizeof(ext4_inode) == 256, "Size of Inode must be 256 bytes");

    #pragma pack(push,1)
    struct ext4_extent_header{
        uint16_t eh_magic;
        uint16_t eh_entries;
        uint16_t eh_max;
        uint16_t eh_depth;
        uint32_t eh_generation;
    }eh;
    #pragma pack(pop)
    static_assert(sizeof(ext4_extent_header) == 12, "Extent Header must be 12 bytes");

    #pragma pack(push,1)
    struct ext4_extent_idx{
        uint32_t ei_block;
        uint32_t ei_leaf_lo;
        uint16_t ei_leaf_hi;
        uint16_t ei_unused;
    }ei;
    #pragma pack(pop)
    static_assert(sizeof(ext4_extent_idx) == 12, "Extent Index must be 12 bytes");

    #pragma pack(push,1)
    struct ext4_extent{
        uint32_t ee_block;
        uint16_t ee_len;
        uint16_t ee_start_hi;
        uint32_t ee_start_lo;
    }ext;
    #pragma pack(pop)
    static_assert(sizeof(ext4_extent) == 12, "Extent must be 12 bytes");

    #pragma pack(push, 1)
    struct ext4_dir_entry_2{
        uint32_t inode;
        uint16_t rec_len;
        uint8_t name_len;
        uint8_t file_type;
        /* No need to add the name[] here */
    }dir2;
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

    std::string file_type_name(ext4_file_type type)
    {
        switch(type)
        {
            case ext4_file_type::unknown: return "UNKNOWN";
            case ext4_file_type::regular: return "REG";
            case ext4_file_type::directory: return "DIR";
            case ext4_file_type::character_device: return "CHAR_DEV";
            case ext4_file_type::block_device: return "BLOCK_DEV";
            case ext4_file_type::fifo: return "FIFO";
            case ext4_file_type::socket: return "SOCKET";
            case ext4_file_type::symlink: return "SYM_LINK";
            default: return "INVALID";
        }
    }

    bool open_ext4_image(std::string& imgPath)
    {
        if(imgPath.empty()) //if files does not exist
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

    bool seek_to_superblock()
    {
        if(!binFile.is_open())
            return false;
        binFile.seekg(superblock_position, std::ios::beg);
        std::cout << "Seeked to Superblock position.\n";
        return true;
    }

    bool seek_to_blockGroupDescriptor()
    {
        /* If block size == 1024 bytes
         *   then reserved space is block 0.
         *   superblock is in block 0.
         *   block group descriptor is in block 2.
         * Else
         *   then reserved space + superblock is in block 0 (eg. 4096 bytes)
         *   Block group descriptor is in block 1.
         * End */
        if(!binFile.is_open())
            return false;
        
        if((1024 << sb.s_log_block_size) == 0x400)
        {
            binFile.seekg(blockGroupDescriptor_position1, std::ios::beg);
        }
        else
        {
            binFile.seekg((1024 << sb.s_log_block_size), std::ios::beg);
        }
        std::cout << "Seeked to Block Group Descriptor position\n";
        return true;
    }

    bool seek_to_inode_table()
    {
        /* Inode table is at bg_inode_table_loth block
         * location is bg_inode_table_lo * block_size
        */
        if(!binFile.is_open())
            return false;

        uint32_t block_size = (1024 << sb.s_log_block_size);

        binFile.seekg(bgd.bg_inode_table_lo*block_size, std::ios::beg);

        return static_cast<bool>(binFile);
    }

    bool seek_to_offset(uint64_t offset)
    {
        if(!binFile.is_open())
            return false;
        
        binFile.seekg(static_cast<std::streamoff>(offset), std::ios::beg);

        return static_cast<bool>(binFile);
    }

    bool read_superblock()
    {
        if(!binFile.is_open())
            return false;
            
        binFile.read(reinterpret_cast<char*>(&sb), sizeof(sb)); //static_cast doesnt work. 

        //Check if read was successfully and check if 1024 bytes were read
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

    bool read_blockGroupDescriptor()
    {
        if(!binFile.is_open())
            return false;
        
        binFile.read(reinterpret_cast<char*>(&bgd), sizeof(bgd));

        //Check if read was successfully and check if 32 bytes were read
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

    bool read_inode2()
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
            std::cout << "Faield to read inode2\n";
            return false;
        }
    }

    bool read_block(std::vector<uint8_t>& buffer, uint64_t offset, uint32_t block_size)
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
            std::cout << "Faield to read Directory block\n";
            return false;
        }
    }

    uint16_t read_s_magic()
    {
        return sb.s_magic;
    }

    uint32_t read_s_inodes_count()
    {
        return sb.s_inodes_count;
    }

    uint32_t read_s_blocks_count_lo()
    {
        return sb.s_blocks_count_lo;
    }

    uint32_t read_s_inodes_per_group()
    {
        return sb.s_inodes_per_group;
    }

    uint32_t read_s_blocks_per_group()
    {
        return sb.s_blocks_per_group;
    }

    uint32_t read_s_log_block_size()
    {
        return (1024 << sb.s_log_block_size);
    }

    uint16_t read_s_inode_size()
    {
        return sb.s_inode_size;
    }

    uint32_t get_block_group_count()
    {
        uint32_t blocks_count = sb.s_blocks_count_lo - sb.s_first_data_block; //exclude first block

        if(blocks_count == 0 || sb.s_blocks_per_group == 0)
            return 0;

        uint32_t ret;
        ret = (blocks_count / sb.s_blocks_per_group) + (blocks_count % sb.s_blocks_per_group != 0);
        return ret;
    }

    void parse_directory_entries(std::vector<uint8_t>& buffer, uint32_t block_size)
    {
        /* Read until the end of block. Note: Last entry will span entire block */
        size_t offset = 0;
        std::cout << "Directory entries: \n";
        while(offset < block_size)
        {
            /* Read inode, rec_len, name_len, file_type and name[] into memory */
            const auto* entry = reinterpret_cast<ext4_parser::ext4_dir_entry_2*>(buffer.data()+offset);

            /* rec_len must not be 0 */
            if(entry->rec_len == 0)
                break;


            /* Inode 0 does not exist. */
            if(entry->inode != 0)
            {
                const char* name = reinterpret_cast<const char*>(entry) + sizeof(entry);
                std::string filename(name, entry->name_len);
                std::string file_type = file_type_name(static_cast<ext4_parser::ext4_file_type>(entry->file_type));
                uint16_t rec_len = entry->rec_len; 
                uint32_t ino = entry->inode;

                /* Print Directory entries */
                std::cout << file_type << " " << filename << " " << "Inode: " << ino << " " << "rec_len: " << rec_len << std::endl;
            }
            offset = offset + entry->rec_len;
        }
    }
};

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
    const auto* extent_header = reinterpret_cast<ext4_parser::ext4_extent_header *>(parser.inode.i_block);
    std::cout << "eh_magic: 0x" << std::hex << extent_header->eh_magic << std::dec << std::endl;
    std::cout << "eh_entries: 0x" << std::hex << extent_header->eh_entries << std::dec << std::endl;
    std::cout << "eh_max: 0x" << std::hex << extent_header->eh_max << std::dec << std::endl;
    std::cout << "eh_depth: 0x" << std::hex << extent_header->eh_depth << std::dec << std::endl;
    std::cout << "eh_generation: 0x" << std::hex << extent_header->eh_generation << std::dec << std::endl;

    if(extent_header->eh_magic != 0xF30A)
    {
        std::cout << "Invalid extent magic" << std::endl;
        return 1;
    }

    if(extent_header->eh_depth == 0)
    {
        /* Following entries are Leaf nodes, which directly map to disk blocks*/

        //Read First Extent Entry. Seek past 12 bytes of Extent Header.
        const auto* extent = reinterpret_cast<ext4_parser::ext4_extent*>(parser.inode.i_block + sizeof(ext4_parser::ext4_extent_header));

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