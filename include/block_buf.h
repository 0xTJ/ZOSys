#ifndef INCLUDE_BLOCK_BUF
#define INCLUDE_BLOCK_BUF

char *block_512_alloc();
char *block_512_tryalloc();
void block_512_free(char *block);

#endif
