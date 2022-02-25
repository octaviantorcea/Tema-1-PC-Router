struct arp_table {
    int size;
    struct arp_table_entry *arp_entries;
};

struct arp_table_entry {
    uint32_t ip_add;
    uint8_t mac[6];
};

struct arp_table* new_arp_table();

void add_arp_entry(struct arp_table *arp_table, uint32_t ip_add, uint8_t mac[6]);

struct arp_table_entry *get_arp_entry(struct arp_table *arp_table, uint32_t ip_add);
