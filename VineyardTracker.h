#ifndef VINEYARDTRACKER_H
#define VINEYARDTRACKER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STR 64
#define MAX_RECORDS 200
#define FILENAME "vineyard_records.txt"

typedef enum {
    Planted = 0,
    Grafted = 1,
    OwnRooted = 2
} VineyardType;


static const char *TYPE_NAMES[] = { "Planted", "Grafted", "Own-rooted" };

typedef struct {
    char location[MAX_STR];
    char site[MAX_STR];
    char plot[MAX_STR];
    char variety[MAX_STR];
    int area;
    VineyardType vineyard_type;
    float damage;
} Record;

extern Record records[MAX_RECORDS];
extern int num_records;

void load_records();
void save_records();
void print_record(int idx);
void list_all();
void list_by_location(char *location);
void list_by_variety(char *variety);
void add_record();
void modify_record();
void delete_record();

#endif