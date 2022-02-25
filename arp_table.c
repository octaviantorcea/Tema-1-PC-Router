#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_SIZE 512

struct arp_table {
    int size;
    struct arp_table_entry *arp_entries;
};

struct arp_table_entry {
    uint32_t ip_add;
    uint8_t mac[6];
};

struct arp_table *new_arp_table() {
    struct arp_table* arp_table = (struct arp_table*)malloc(sizeof(struct arp_table));
    struct arp_table_entry* entries = (struct arp_table_entry*)malloc(MAX_SIZE * sizeof(struct arp_table_entry));

    arp_table->size = 0;
    arp_table->arp_entries = entries;
    return arp_table;
}

void add_arp_entry(struct arp_table *arp_table, uint32_t ip_add, uint8_t mac[6]) {
    arp_table->arp_entries[arp_table->size].ip_add = ip_add;
    
    for (int i = 0; i < 6; i++) {
        arp_table->arp_entries[arp_table->size].mac[i] = mac[i];
    }

    arp_table->size++;
}

struct arp_table_entry *get_arp_entry(struct arp_table *arp_table, uint32_t ip_add) {
    for (int i = 0; i < arp_table->size; i++) {
        if (arp_table->arp_entries[i].ip_add == ip_add) {
            return &arp_table->arp_entries[i];
        }
    }

    return NULL;
}
