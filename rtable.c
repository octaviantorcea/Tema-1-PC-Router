#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#define MAX_LINE_LENGTH 55
#define DELIM " "

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

int get_rtable_line_nr(char* file_name) {
    FILE *rtable = fopen(file_name, "r");
    int line_count = 0;
    char *line = (char *)malloc(MAX_LINE_LENGTH * sizeof(char));
    size_t max_length = MAX_LINE_LENGTH;

    while (getline(&line, &max_length, rtable) > 0) {
        line_count++;
    }

    free(line);
    fclose(rtable);
    return line_count;
}

int cmpfunc(const void *a, const void *b) {
    if ((*(struct rtable_entry*) a).prefix == (*(struct rtable_entry*) b).prefix) {
        return (*(struct rtable_entry*) a).mask - (*(struct rtable_entry*) b).mask;
    } else {
        return (*(struct rtable_entry*) a).prefix - (*(struct rtable_entry*) b).prefix;
    }
}

struct rtable* parse_rtable(char *file_name) {
    struct rtable *rtable = NULL;
    struct rtable_entry *rtable_entries = NULL;
    char delim[2] = DELIM;
    size_t max_length = MAX_LINE_LENGTH;

    int rtable_entries_nr = get_rtable_line_nr(file_name);
    char *line = (char *)malloc(MAX_LINE_LENGTH * sizeof(char));
    

    FILE *rtable_text = fopen(file_name,"r");

    rtable = (struct rtable *)malloc(sizeof(struct rtable));
    rtable_entries = (struct rtable_entry *)
                        malloc(rtable_entries_nr * sizeof(struct rtable_entry));

    rtable->size = rtable_entries_nr;

    for (int i = 0; i < rtable_entries_nr; i++) {
        getline(&line, &max_length, rtable_text);

        // get prefix
        char *token = strtok(line, delim);
        rtable_entries[i].prefix = ntohl(inet_addr(token));

        // get next hop
        token = strtok(NULL, delim);
        rtable_entries[i].next_hop = ntohl(inet_addr(token));

        // get mask
        token = strtok(NULL, delim);
        rtable_entries[i].mask = ntohl(inet_addr(token));

        // get interface
        token = strtok(NULL, delim);
        rtable_entries[i].interface = atoi(token);
    }

    qsort(rtable_entries, rtable_entries_nr, sizeof(struct rtable_entry), cmpfunc);

    rtable->entries = rtable_entries;

    free(line);
    fclose(rtable_text);
    return rtable;
}

struct rtable_entry *get_best_route(struct rtable *rtable, uint32_t dest_ip) {
    int best_index = -1;
    int start = 0;
    int stop = rtable->size - 1;
    int middle = 0;

    while (start <= stop) {
        middle = (start + stop) / 2;

        if ((rtable->entries[middle].mask & dest_ip) == rtable->entries[middle].prefix) {
            if (best_index == -1 || rtable->entries[middle].mask > rtable->entries[best_index].mask) {
                best_index = middle;
            }

            start = middle + 1;
        } else if ((rtable->entries[middle].mask & dest_ip) < rtable->entries[middle].prefix) {
            stop = middle - 1;
        } else {
            start = middle + 1;
        }
    }

    if (best_index == -1) {
        return NULL;
    } else {
        return &rtable->entries[best_index];
    }
}
