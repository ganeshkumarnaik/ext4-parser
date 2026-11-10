# ext4-parser

A C++20 tool that reads an ext4 filesystem image directly from its on-disk format and lists the root directory. It doesn't mount the image or use a kernel driver or filesystem library, only the C++ standard library.

The current version builds and runs on Windows. Cross-compiling for embedded Linux targets is planned.

## How it works

The parser follows the same path a filesystem driver takes to find the root directory:

1. Open the image as a binary file.
2. Read the superblock and verify the magic number (`0xEF53`).
3. Read the first block group descriptor to find the inode table.
4. Read inode 2, the root directory, and its `i_block[]` field.
5. Read the extent header from `i_block[]` and validate `eh_magic` (`0xF30A`).
6. Use the first extent to find the root directory's physical block.
7. Walk the directory entries and print each file and folder.

With the sample image below, which uses 4 KiB blocks, those steps land at these offsets:

```text
0x00400    superblock           block size, inode size, group layout
   |
0x01000    group descriptor     inode table starts at block 0x25
   |
0x25100    inode 2              second 256-byte slot in the inode table
   |
i_block[]  extent header        depth 0, so entries point straight at data
   |
0x06000    directory block      ., .., lost+found, sub, hello.txt
```

## Build

You need CMake 3.16 or later and a C++20 compiler. These commands use Ninja with GCC:

```sh
cmake -S . -B build -G Ninja
cmake --build build --target build
```

The executable is written to `build/bin/`. Without `-G Ninja`, CMake uses your platform's default generator. With Visual Studio, the executable is in `build/bin/Debug/`.

To remove build output:

```sh
cmake --build build --target clean
```

## Create a test image

The image isn't stored in the repository. Run this from the repository root in WSL or any Linux shell:

```sh
mkdir -p /tmp/rootfs/sub
echo "Hello from Ext4!" > /tmp/rootfs/hello.txt
echo "Nested test content" > /tmp/rootfs/sub/test.txt
mkfs.ext4 -O ^64bit -b 4096 -d /tmp/rootfs test.img 64M
```

`-d` copies the folder into the new filesystem.
`-O ^64bit` turns off the 64-bit feature, which keeps group descriptors at the 32-byte layout the parser expects.

## Run

The parser opens `test.img` in the current directory, so run it from the repository root:

```sh
./build/bin/ext4-parser
```

Excerpt of the output:

```text
Magic value is: ef53
Block size: 4096
Inode size: 256
Total Blocks count: 16384
Total Inodes count: 16384
Blocks group count: 1
Location of inode table = 25
eh_magic: 0xf30a
eh_depth: 0x0
Physical Block: 0x6
Directory Block Offset: 0x6000
Directory entries:
DIR . Inode: 2 rec_len: 12
DIR .. Inode: 2 rec_len: 12
DIR lost+found Inode: 11 rec_len: 20
DIR sub Inode: 12 rec_len: 24
REG hello.txt Inode: 13 rec_len: 4016
```

Values printed by the location and extent lines are in hexadecimal. Inode numbers depend on the order files were added to the image, so yours may differ.

## Current limitations

- It reads only the first block group, and expects 32-byte group descriptors, so the image must be created without the `64bit` feature.
- It reads only the root directory, using its first extent and first directory block.
- It handles only depth-0 extent trees, where the extents are stored inside the inode.
- It decodes on-disk structures by casting them directly, so it assumes a little-endian host.
- The image name is fixed at `test.img`.

## Roadmap

- Take the image path as a command-line argument.
- Look up files by path and read their contents.
- Support deeper extent trees and directories that span several blocks.
- Decode on-disk fields explicitly as little-endian.
- Cross-compile for embedded Linux targets.
