#include "common.hpp"
// This is for real time task
int Common::initialize_memory_allocation(void)
{
    // Allocate some memory
    int i, page_size;
    char *buffer;
    // Now lock all current and future pages from preventing of being paged
    if (mlockall(MCL_CURRENT | MCL_FUTURE))
    {
            perror("mlockall failed:");
            return -1;
    }
    // Turn off malloc trimming.
    mallopt(M_TRIM_THRESHOLD, -1);
    // Turn off mmap usage.
    mallopt(M_MMAP_MAX, 0);
    page_size = sysconf(_SC_PAGESIZE);
    buffer = (char *)malloc(POOLSIZE);
    // Touch page to prove there will be no page fault later
    for (i = 0; i < POOLSIZE; i += page_size)
    {
        buffer[i] = 0;
    }
    free(buffer);
    return 0;
}
void Common::stack_prefault(void)
{

    unsigned char dummy[MAX_SAFE_STACK];

    memset(dummy, 0, MAX_SAFE_STACK);
    return;
}


void Common::tsnorm(struct timespec *ts)
{
    while (ts->tv_nsec >= NSEC_PER_SEC)
    {
        ts->tv_nsec -= NSEC_PER_SEC;
        ts->tv_sec++;
    }
}
Common::Common(/* args */)
{
}

Common::~Common()
{
}