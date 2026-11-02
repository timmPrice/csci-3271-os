## Programming Project 4 - Implementation plans

### vsfs_link:
Operations Requried

1. Ensure source path exist
2. Find the correct inode for source path
3. check that the file is not a link
4. look into the directory holding desination path and find which data block it is in
5. find a empty directory entry in said data block 
6. check that there is no name conflict 
7. update new directory entry
8. increment link-count
9. write changse to disk

### vsfs_unlink:
Operations Requried

1. Ensure source and destination paths exist
2. Find the correct inode for source path
3. find the directory the link is in 
4. Verify that the file is actually a link 
5. remove directory entry 
6. reduce inode link-count

7. -- if link-count > 0 --
change inode 
    

7. -- if link-count = 0 -- 
read the inode
free blocks that are referenced
free indirect block if it exist
re-write the inode 
clear the allocation bit
mark inode as free - reset metadata 

8. write changes to disk 
