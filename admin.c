#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

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
    int flag=0;
    char password[20];
    char buffer[MAX_BUFFER] = {0};
    send(sock,&flag,sizeof(flag),0);
    printf("Enter id: ");
    scanf("%d", &account_number);
    printf("Enter Password: ");
    scanf("%s", password);

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
        printf("1. Add employee\n");
        printf("2. Modify employee details\n");
        printf("3. Modify manager details\n");
        printf("4. Change password\n");
        printf("5. Manage user roles\n");
        printf("6. Logout\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        send(sock, &choice, sizeof(choice), 0);

        switch (choice) {
            case 1:
                int id;
                char username[50];
                char password[20];
                char role[10];
                printf("Enter id\n");
                scanf("%d",&id);
                printf("Enter username\n");
                scanf("%s",username);
                printf("Enter password\n");
                scanf("%s",password);
                printf("Enter the role\n");
                scanf("%s",role);
                send(sock,&id,sizeof(id),0);
                send(sock,username,sizeof(username),0);
                send(sock,password,sizeof(password),0);
                send(sock,role,sizeof(role),0);
                printf("Record added successfully\n");
                break;
            case 2: {
                int id;
                char name[100];
                printf("Enter employee to be changed");
                scanf("%d", &id);
                printf("Enter new username");
                scanf("%s",name);
                send(sock,&id,sizeof(id),0);
                send(sock, name, sizeof(name), 0);
                break;
            }
            case 3: {
                int id;
                char name[100];
                printf("Enter manager to be changed");
                scanf("%d", &id);
                printf("Enter new username");
                scanf("%s",name);
                send(sock,&id,sizeof(id),0);
                send(sock, name, sizeof(name), 0);
                break;
            }
            case 4: {
                char pass[100];
                printf("Enter the new password\n");
                scanf("%s",pass);
                send(sock,pass,sizeof(pass),0);
                break;
            }
            case 5:{
            int id;
            char role[10];
            printf("Enter the employee id to be changed to manager\n");
            scanf("%d",&id);
            printf("Enter Manager\n");
            scanf("%s",role);
            send(sock,&id,sizeof(id),0);
            send(sock,role,sizeof(role),0);
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
