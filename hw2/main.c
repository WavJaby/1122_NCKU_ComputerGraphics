#define MEM_TRACK
#define WJCL_HASH_MAP_IMPLEMENTATION
#define WJCL_LINKED_LIST_IMPLEMENTATION
#include "../../WJCL/memory/wjcl_mem_track.h"

int main(int argc, char* argv[]) {
    char* str;
    str = malloc(100);
    str = malloc(50);

    memTrackResult();

    free(str);
    memTrackResult();
    return 0;
}