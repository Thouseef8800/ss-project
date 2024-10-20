#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <termios.h> 
#define PORT 8086
#define MAX_BUFFER 1024

void login(int sock);
void menu();
void perform_operations(int sock);

int main() {
    struct sockaddr_in serv_addr;
    int sock = 0;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed\n");
        return -1;
    }

    login(sock);
    perform_operations(sock);

    close(sock);
    return 0;
}



    void login(int sock) {
    int account_number;
    int flag=1;
    char password[20];
    char buffer[MAX_BUFFER] = {0};
    send(sock,&flag,sizeof(flag),0);
    printf("Enter id: ");
    scanf("%d", &account_number);
    printf("Enter Password: ");
    scanf("%s",password);
    
    // Send login details to server
    send(sock, &account_number, sizeof(account_number), 0);
    send(sock, password, sizeof(password), 0);

    // Receive response from server
    recv(sock, buffer, MAX_BUFFER, 0);
    printf("%s", buffer);  // Print response from the server

    // Check if the login was successful or if the user is already logged in
    if (strstr(buffer, "Login successful")) {
        // Proceed to operations if login is successful
        perform_operations(sock);
    } else {
        // If login failed, exit the client
        printf("Exiting...\n");
        close(sock);
        exit(0);
    }
}


void perform_operations(int sock) {
    int choice;

    while (1) {
        printf("\nMenu:\n");
        printf("1. deactivate customer accounts\n");
        printf("2. activate customer accounts\n");
        printf("3. Assign loan applications to employees\n");
        printf("4. Review customer feedback\n");
        printf("5. Change password\n");
        printf("6. Logout\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        send(sock, &choice, sizeof(choice), 0);

        switch (choice) {
            case 1:
                int account_number;
                printf("Enter customer account_number to which account to be deactivated\n");
                scanf("%d",&account_number);
                send(sock,&account_number,sizeof(account_number),0);

                break;
            case 2: {
                int account_number;
                printf("Enter account_number to which account to be activated\n");
                scanf("%d",&account_number);
                send(sock,&account_number,sizeof(account_number),0);
                break;
            }
            case 3: {
                printf("Assigning loans to the employees\n");
                break;
            }
            case 4: {
                int account_number;
                printf("Enter account_number to see the review\n");
                scanf("%d",&account_number);
                send(sock,&account_number,sizeof(account_number),0);
                break;
            }
            case 5:{
            char passwd[100];
            printf("Enter the new password\n");
            scanf("%s",passwd);
            send(sock,passwd,sizeof(passwd),0);
            break;
            }             
            case 6:
                printf("Logging out...\n");
                return; // Logout
            default:
                printf("Invalid choice. Please try again.\n");
        }
        // Receive any response from server after an operation
        char buffer[MAX_BUFFER] = {0};
        recv(sock, buffer, MAX_BUFFER, 0);
        printf("%s", buffer);
    }
}
