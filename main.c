#include <stdio.h>
#include "VineyardTracker.h"

int main() {
    load_records();
    int choice;
    char str[MAX_STR];

    while (1) {
        printf("\nVINEYARD TRACKER\n");
        printf("1. Add new record\n");
        printf("2. Modify record\n");
        printf("3. Delete record\n");
        printf("4. List all records\n");
        printf("5. List by location\n");
        printf("6. List by grape variety\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                add_record();
                break;
            case 2:
                modify_record();
                break;
            case 3:
                delete_record();
                break;
            case 4:
                list_all();
                break;
            case 5:
                printf("Enter location: ");
                scanf("%63s", str);
                list_by_location(str);
                break;
            case 6:
                printf("Enter variety: ");
                scanf("%63s", str);
                list_by_variety(str);
                break;
            case 0:
                printf("Data saved. Goodbye!\n");
                return 0;
            default:
                printf("Invalid choice! Please try again.\n");
        }
    }
    return 0;
}