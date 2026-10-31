
## Problem Set 4 
CSCI 3271 -- Operating Systems \
2026.10.30 -- Tim Price


1. A FAT32 file system is configured with a disk-block size of 2 KiB on a 650-MiB disk partition.
The directory `C:\Documents\CSCI3271\` contains 2 files: Syllabus.doc of size 19 KiB and Textbook.pdf of size 5 MiB.

    1.A. How large is the FAT for this file system, in bytes?
    
    There would be `650 * 1024/2 = 332800` total clusters. Fat32 would use 4 bytes-per-entry so `332800 * 4 = 1331200` bytes.

    1.B. How many disk blocks are used by the directory and the two files? How long is each file’s linked list in the FAT?

    - directory: 1 block ~ estimate
    - syllabus.doc: `19/2` (rounded up) `= 10` blocks 
    - textbook.pdf: `5 Mib = 5120 Kib` then `5120 / 2 = 2560` blocks
    - total: 2571 blocks
    - linked list: each files linked-list is as long as its block count. So syllabus has a length of 10 and textbook has a length of 2560.

    1.C. Suppose a user process writes 5 KiB of data to the end of Syllabus.doc. Describe how the file system would change as a result of this operation. 
         Specify all the data structures and blocks that would change, and how many blocks the file uses afterwards.
    
    - Initially the syllabus file has 10 blocks, now it should grow to `24 / 2 = 12` blocks.
    - The syllabus block already had leftover room at its end because of rounding, so it would fill that space before adding an additioal 2 blocks.
    - The files linked list is physically seperated but the pointer in the last block should now be pointing to the head block in the newly allocated 2 block section.
    - The FAT entry will be updated to identify the end of the linked-list as the end of the second logical block.
    - The directory entry should be updated accordingly (file size updated).

2. A file system has a disk-block size of 2 KiB, and block addresses require 4 bytes.
The blocks that make up a file are pointed to by an i-node which contains 4 direct addresses, one single-indirect block entry, and one double-indirect block entry. 
What is the largest possible file size (in KiB) in this system?

    - Each block adress requires 4 bytes so 2KiB is 2048 bytes `2048 / 4 = 512` addresses for each block.
    - The i-node can adress using 4 direct pointers, 512 single-indirect pointers, or `512 * 512 = 262144` double-indirect pointers.
    - total: 262660 blocks which is `262660 * 2 KiB = 525320 KiB`.

3. A Linux ext2 file system is configured with a disk-block size of 4KiB and uses block addresses of 4 bytes.
The directory `/home/etremel/` contains 2 files: research-notes.txt of size 9.4 KiB, and desktop-wallpaper.png of size 10.5 MiB.

    3.A. Describe the set of blocks needed to store the directory and files on the disk. How many blocks in total do they occupy?

    - Directory: needs an i-node and at least 1 data block.
    - research-notes.txt: `9.4 KiB / 4 KiB = 3` (rounded up) data blocks.
    - desktop-wallpaper.png: if `10.5 MiB * 1024 = 10752 KiB` then `10752 KiB / 4 KiB = 2688` data blocks (rounded up).
    - total: `1 + 3 + 2688 = 2696` total data blocks.

    3.B. Suppose a user process writes 4.5KiB of data to the end of research-notes.txt.
         Describe how the file system would change as a result of this operation, compared to its state in part A.
         Specify all the data structures and blocks that would change, and how many blocks the file uses afterwards.

    - `9.4 KiB + 4.5 KiB = 13.9 KiB` which now will need `13.9 / 4 = 4` (rounded up) blocks.
    - The original blocks had some free space which is now full, and an additional block of space added.
    - The i-node will use another direct pointer, 
    - The file size will be updated but the directory entry probably stays the same as the i-node number remained the same. 

4. A computer has two memory-mapped I/O devices that it can use to display output:
    
    A text-based terminal and a graphical display adapter.
    
    The text-based terminal is controlled using a character buffer, which has one entry per character that can be displayed on the screen. 
    The terminal can display 25 lines of 80 characters each.
   
    The graphical display adapter has a video display buffer that contains the image currently displayed on the screen.
    The screen has a resolution of 1280 × 1024 and uses 16-bit color.

    4.A Assuming it takes the computer 4 nanoseconds to copy a byte of memory, how long does it take to completely rewrite each type of display?
    
    - text-based terminal: `25 * 80 = 2000` total bytes. `2000 * 4 ns = 8000 ns` of total time to refresh every byte ~ 8 microseconds
    - graphical display adapter: `1280 * 1024 = 1310720 pixels` which makes `1310720 * 2 bytes = 2621440 bytes` total. Then, `2621440 * 4 = 10485760` ns ~ 10.48.. milliseconds
    
    4.B. Imagine both of these display devices could be used to show the user a command shell, which needs to display the output from each command a user types by scrolling all the texton the screen up one line at a time.
    Which device would you prefer to use, and why?
    
    Well it takes `10485760 / 8000` seconds longer to rewrite the graphical display adapters screen.
    A shell even if just a single line was written on a full display would require the entire screen to be rewritten.
    So, it would be much faster to use the text-based terminal, especially since graphics are hardly a concern when working in a shell.
