
a basic writeup on the very, very simple moonchild "fastfile" format.

First 4 bytes are dedicated to the "dwFHCnt", but i have taken to calling it the "File Heading Count" which probably is what it stands for in the first place.

Next for dwFHCnt number of files, they are listed off linearly with this structure
```
typedef struct {
    int        offset;
    char        name[16];
} FILEENTRY, *LPFILEENTRY;
```
Then, the data is lined up back to back, with a compression applied up on them. To calculate the length of a file, compare it with the next file in the list.

THE FINAL ENTRY IS A DUMMY ENTRY. (anyone with a hex editor can tell this, though...)
