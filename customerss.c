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
    int flag=2;
    char password[20];
    char buffer[MAX_BUFFER] = {0};
    send(sock,&flag,sizeof(flag),0);
    printf("Enter Account Number: ");
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
        printf("1. View Account Balance\n");
        printf("2. Deposit Money\n");
        printf("3. Withdraw Money\n");
        printf("4. Transfer Funds\n");
        printf("5. Apply for Loan\n");
        printf("6. Change Password\n");
        printf("7. Add Feedback\n");
        printf("8. View Transaction History\n");
        printf("9. Logout\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        send(sock, &choice, sizeof(choice), 0);

        switch (choice) {
            case 1:
                // Just receive and print the balance
                break;
            case 2: {
                double amount;
                printf("Enter amount to deposit: ");
                scanf("%lf", &amount);
                send(sock, &amount, sizeof(amount), 0);
                break;
            }
            case 3: {
                double amount;
                printf("Enter amount to withdraw: ");
                scanf("%lf", &amount);
                send(sock, &amount, sizeof(amount), 0);
                break;
            }
            case 4: {
                double amount;
                int recipient_account;
                printf("Enter recipient account number: ");
                scanf("%d", &recipient_account);
                printf("Enter amount to transfer: ");
                scanf("%lf", &amount);
                send(sock, &recipient_account, sizeof(recipient_account), 0);
                send(sock, &amount, sizeof(amount), 0);
                break;
            }
            case 5:
                // Loan application handling
                int loan_amount;
                printf("Enter the amount you want to apply for a loan: ");
                scanf("%d", &loan_amount);
                send(sock, &loan_amount, sizeof(loan_amount), 0);
                break;
            case 6: {
                char new_password[20];
                printf("Enter new password: ");
                scanf("%s", new_password);
                send(sock, new_password, sizeof(new_password), 0);
                break;
            }
            case 7: {
                char feedback[100];
                printf("Enter your feedback: ");
                getchar();  // Clear the newline
                fgets(feedback, sizeof(feedback), stdin);
                send(sock, feedback, sizeof(feedback), 0);
                break;
            }
            case 8:
                // View transaction history handling
                break;
            case 9:
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
