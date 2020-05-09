#include "block_buf.h"
#include "module.h"
#include "mutex.h"
#include "panic.h"
#include "semaphore.h"

#define BLOCK_512_COUNT 1

char blocks_512[BLOCK_512_COUNT][512];
semaphore_t available_512;
mutex_t free_512[BLOCK_512_COUNT];

int block_buf_init(void);
void block_buf_exit(void);

struct module block_buf_module = {
    block_buf_init,
    block_buf_exit
};

int block_buf_init(void) {
    semaphore_init(&available_512, BLOCK_512_COUNT);
    for (unsigned i = 0; i < BLOCK_512_COUNT; ++i) {
        mutex_init(&free_512[i]);
    }
    return 0;
}

void block_buf_exit(void) {

}

char *block_512_alloc() {
    semaphore_wait(&available_512);

    for (unsigned i = 0; i < BLOCK_512_COUNT; ++i) {
        if (mutex_trylock(&free_512[i])) {
            return blocks_512[i];
        }
    }

    panic();
    return NULL;
}

char *block_512_tryalloc() {
    if (!semaphore_trywait(&available_512)) {
        return NULL;
    }

    for (unsigned i = 0; i < BLOCK_512_COUNT; ++i) {
        if (mutex_trylock(&free_512[i])) {
            return blocks_512[i];
        }
    }

    panic();
    return NULL;
}

void block_512_free(char *block) {
    unsigned i = (block - blocks_512) / sizeof(blocks_512[0]);
    mutex_unlock(&free_512[i]);
    semaphore_signal(&available_512);
}
