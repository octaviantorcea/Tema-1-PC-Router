#include <stdio.h>
#include <stdint.h>

struct rtable {
    struct rtable_entry *entries;
    int size;
};

struct rtable_entry {
    uint32_t prefix;
    uint32_t next_hop;
    uint32_t mask;
    int interface;
};

struct rtable *parse_rtable(char *file_name);

struct rtable_entry *get_best_route(struct rtable *rtable, uint32_t dest_ip);
