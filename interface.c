#include <stdio.h>
#include <stdlib.h>

int main() {
    int choice;

    while (1) {
        printf("\nBanking Management System\n");
        printf("1. Customer\n");
        printf("2. Bank Employee\n");
        printf("3. Manager\n");
        printf("4. Administrator\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                // Execute Customer Client Code
                system("./customers");
                break;
            case 2:
                // Execute Bank Employee Client Code
                system("./bank_emp");
                break;
            case 3:
                // Execute Manager Client Code
                system("./manager");
                break;
            case 4:
                // Execute Administrator Client Code
                system("./admin");
                break;
            case 5:
                printf("Exiting...\n");
                exit(0);
            default:
                printf("Invalid choice! Please try again.\n");
        }
    }

    return 0;
}