#include "VineyardTracker.h"

Record records[MAX_RECORDS];
int num_records = 0;

void load_records() {
    FILE *f = fopen(FILENAME, "r");
    if (!f) {
        num_records = 0;
        return;
    }
    num_records = 0;
    while (num_records < MAX_RECORDS) {
        Record *r = &records[num_records];
        if (fscanf(f, "%63s %63s %63s %63s %d %d %f",r->location, r->site, r->plot, r->variety, &r->area, (int*)&r->vineyard_type, &r->damage) != 7) {
            break;
        }
        num_records++;
    }
    fclose(f);
}

void save_records() {
    FILE *f = fopen(FILENAME, "w");
    if (!f) {
        printf("Error: Could not save to file!\n");
        return;
    }
    for (int i = 0; i < num_records; i++) {
        fprintf(f, "%s %s %s %s %d %d %.1f\n", records[i].location, records[i].site, records[i].plot, records[i].variety, records[i].area, records[i].vineyard_type, records[i].damage);
    }
    fclose(f);
}

void print_record(int idx) {
    if (idx < 1 || idx > num_records) {
        printf("Record #%d not found.\n", idx);
        return;
    }
    Record *r = &records[idx - 1];
    printf("\nRecord #%d\n", idx);
    printf("Location : %s\n", r->location);
    printf("Site : %s\n", r->site);
    printf("Plot : %s\n", r->plot);
    printf("Variety : %s\n", r->variety);
    printf("Area : %d sq. fathoms\n", r->area);
    printf("Type : %s\n", TYPE_NAMES[r->vineyard_type]);
    printf("Damage : %.1f%%\n", r->damage);
}

void list_all() {
    if (num_records == 0) {
        printf("\nNo records found.\n");
        return;
    }
    printf("\n%-5s %-20s %-16s %-12s %-16s %10s %-12s %8s\n",
           "#", "Location", "Site", "Plot", "Variety", "Area", "Type", "Damage");
    
    for (int i = 0; i < num_records; i++) {
        printf("%-5d %-20s %-16s %-12s %-16s %10d %-12s %7.1f%%\n", i + 1, records[i].location, records[i].site, records[i].plot, records[i].variety, records[i].area,
               TYPE_NAMES[records[i].vineyard_type], records[i].damage);
    }
}

void list_by_location(char *location) {
    int found = 0;
    printf("\nRecords matching location \"%s\":\n", location);
    for (int i = 0; i < num_records; i++) {
        if (strstr(records[i].location, location) != NULL) {
            print_record(i + 1);
            found++;
        }
    }
    printf("%d record(s) found.\n", found);
}

void list_by_variety(char *variety) {
    int found = 0;
    printf("\nRecords matching variety \"%s\":\n", variety);
    for (int i = 0; i < num_records; i++) {
        if (strstr(records[i].variety, variety) != NULL) {
            print_record(i + 1);
            found++;
        }
    }
    printf("%d record(s) found.\n", found);
}

void add_record() {
    if (num_records >= MAX_RECORDS) {
        printf("Database is full! (Maximum %d records)\n", MAX_RECORDS);
        return;
    }
    Record *r = &records[num_records];
    printf("Enter: location site plot variety area type(0=Planted,1=Grafted,2=Own-rooted) damage\n");
    scanf("%63s %63s %63s %63s %d %d %f", r->location, r->site, r->plot, r->variety, &r->area, (int*)&r->vineyard_type, &r->damage);
    num_records++;
    save_records();
    printf("Record added successfully!\n");
}

void modify_record() {
    int idx;
    printf("Enter record number to modify: ");
    scanf("%d", &idx);
    
    if (idx < 1 || idx > num_records) {
        printf("Invalid record number!\n");
        return;
    }
    
    Record *r = &records[idx - 1];
    printf("Enter new values: location site plot variety area type damage\n");
    scanf("%63s %63s %63s %63s %d %d %f", r->location, r->site, r->plot, r->variety, &r->area, (int*)&r->vineyard_type, &r->damage);
    save_records();
    printf("Record modified successfully!\n");
}

void delete_record() {
    int idx;
    printf("Enter record number to delete: ");
    scanf("%d", &idx);
    if (idx < 1 || idx > num_records) {
        printf("Invalid record number!\n");
        return;
    }
    for (int i = idx - 1; i < num_records - 1; i++) {
        records[i] = records[i + 1];
    }
    num_records--;
    save_records();
    printf("Record deleted successfully!\n");
}