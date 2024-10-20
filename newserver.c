#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#define LOANS_FILE "loans.txt"
#define PORT 8086
#define MAX_BUFFER 1024
#define MAX_SESSIONS 100
#define MAX_BUFFER 1024
#define employee_file "employees.txt"
#define BUFFER_SIZE 1024
#define MAX_LINE_LENGTH 256
#define FILENAME "customers.txt"
#define LOANS_FILE "loans.txt"
#define CUSTOMERS_FILE "customers.txt"
#define MAX_LINE_LENGTH 256
typedef struct {
    int custid;   // Customer ID
    int loan;     // Loan amount
    int empid;    // Employee ID (who is handling the loan)
} Loan;

typedef struct {
    int id;
    char username[50];
    char password[20];
    char role[10];  // Admin, Manager, Employee
} Employee;
typedef struct {
    int account_number;
    int socket;
} ActiveSession;

ActiveSession active_sessions[MAX_SESSIONS];
int session_count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int is_account_active(int account_number) {
    for (int i = 0; i < session_count; i++) {
        if (active_sessions[i].account_number == account_number) {
            return 1; // Account is already logged in
        }
    }
    return 0; // Account is not active
}

void add_active_session(int account_number, int socket) {
    active_sessions[session_count].account_number = account_number;
    active_sessions[session_count].socket = socket;
    session_count++;
}

void remove_active_session(int account_number) {
    for (int i = 0; i < session_count; i++) {
        if (active_sessions[i].account_number == account_number) {
            // Shift all sessions down
            for (int j = i; j < session_count - 1; j++) {
                active_sessions[j] = active_sessions[j + 1];
            }
            session_count--;
            break;
        }
    }
}
typedef struct {
    int account_number;
    char name[50];
    char password[20];
    double balance;
    int loan_status;     // 0 = No loan, 1 = Active loan
    int active_status;   // 1 = Active, 0 = Deactivated
    char feedback[100];
    int loan_application; // 0 = None, 1 = Applied
    int transaction_count;
} Customer;

pthread_mutex_t lock;

void *handle_customer_operations(void *socket_desc);
int validate_login(int account_number, const char *password, Customer *customer);
void update_customer_file(Customer *customer);
void view_balance(Customer *customer, int client_socket);
void deposit_money(Customer *customer, int client_socket);
void withdraw_money(Customer *customer, int client_socket);
void transfer_funds(Customer *customer, int client_socket);
void apply_for_loan(Customer *customer, int client_socket);
void change_password(Customer *customer, int client_socket);
void add_feedback(Customer *customer, int client_socket);
void view_transaction_history(Customer *customer, int client_socket);
void print_customer_data(Customer *customer);

void add_employee(Employee *emp,int client_socket);
void modify_employee(Employee *emp,int client_socket);
void modify_manager(Employee *emp,int client_socket);
void manage_user_roles(Employee *emp,int client_socket);
void change_password1(Employee *emp,int client_socket);
int validate_login1(int account_number, const char *password, Employee *emp);


void deactivate(int client_socket);
void activate(int client_socket);
void assign(int client_socket);
void review(int client_socket);


void add_cust(int client_socket);

// Prototype for modifying a customer's username in customers.txt
void modify_cust(int client_socket);

// Prototype for viewing loan applications from loans.txt
void view_loan(int client_socket);

// Prototype for sanctioning loans and updating loan_status in customers.txt
void sanction_loan(int client_socket);

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Banking server started...\n");

    // Accept clients
    while ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))) {
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_customer_operations, (void *)&client_socket);
    }

    return 0;
}

void *handle_customer_operations(void *socket_desc) {
    int client_socket = *(int *)socket_desc;
    int flag;
    

    // Receive login details
    
    recv(client_socket,&flag,sizeof(flag),0);
    
    // Validate login
    if(flag==3){
    Employee emp;
    int account_number;
    char password[20];
    char buffer[MAX_BUFFER];
    recv(client_socket, &account_number, sizeof(account_number), 0);
    recv(client_socket, password, sizeof(password), 0);
    if (validate_login1(account_number, password, &emp)) {
    if (is_account_active(account_number)) {
        send(client_socket, "Another session is already active for this account. Logout first.\n", 67, 0);
        close(client_socket);
        return NULL;
    }
    add_active_session(account_number, client_socket); // Add session
    sprintf(buffer, "Login successful. Welcome, %s!\n", emp.username);
    send(client_socket, buffer, strlen(buffer), 0);
} else {
    send(client_socket, "Login failed. Please check your account number and password.\n", 62, 0);
    close(client_socket);
    return NULL;
}
    // Handle customer operations
    int choice;
    while (1) {
        recv(client_socket, &choice, sizeof(choice), 0);
        switch (choice) {
            case 1: add_cust(client_socket); break;
            case 2: modify_cust(client_socket); break;
            case 3: view_loan(client_socket); break;
            case 4: sanction_loan(client_socket); break;
            case 5: change_password1(&emp,client_socket); break;
            case 6: close(client_socket); return NULL; // Logout
            default: break;
        }
    }
    close(client_socket);
    return NULL;
    }
    if(flag==1){
    Employee emp;
    int account_number;
    char password[20];
    char buffer[MAX_BUFFER];
    recv(client_socket, &account_number, sizeof(account_number), 0);
    recv(client_socket, password, sizeof(password), 0);
    if (validate_login1(account_number, password, &emp)) {
    if (is_account_active(account_number)) {
        send(client_socket, "Another session is already active for this account. Logout first.\n", 67, 0);
        close(client_socket);
        return NULL;
    }
    add_active_session(account_number, client_socket); // Add session
    sprintf(buffer, "Login successful. Welcome, %s!\n", emp.username);
    send(client_socket, buffer, strlen(buffer), 0);
} else {
    send(client_socket, "Login failed. Please check your account number and password.\n", 62, 0);
    close(client_socket);
    return NULL;
}
    // Handle customer operations
    int choice;
    while (1) {
        recv(client_socket, &choice, sizeof(choice), 0);
        switch (choice) {
            case 1: deactivate(client_socket); break;
            case 2: activate(client_socket); break;
            case 3: assign(client_socket); break;
            case 4: review(client_socket); break;
            case 5: change_password1(&emp,client_socket); break;
            case 6: close(client_socket); return NULL; // Logout
            default: break;
        }
    }

    close(client_socket);
    return NULL;
    }
    if(flag==2){
    Customer customer;
    int account_number;
    char password[20];
    char buffer[MAX_BUFFER];
    recv(client_socket, &account_number, sizeof(account_number), 0);
    recv(client_socket, password, sizeof(password), 0);
    if (validate_login(account_number, password, &customer)) {
    if (is_account_active(account_number)) {
        send(client_socket, "Another session is already active for this account. Logout first.\n", 67, 0);
        close(client_socket);
        return NULL;
    }
    add_active_session(account_number, client_socket); // Add session
    sprintf(buffer, "Login successful. Welcome, %s!\n", customer.name);
    send(client_socket, buffer, strlen(buffer), 0);
} else {
    send(client_socket, "Login failed. Please check your account number and password.\n", 62, 0);
    close(client_socket);
    return NULL;
}
    // Handle customer operations
    int choice;
    while (1) {
        recv(client_socket, &choice, sizeof(choice), 0);
        switch (choice) {
            case 1: view_balance(&customer, client_socket); break;
            case 2: deposit_money(&customer, client_socket); break;
            case 3: withdraw_money(&customer, client_socket); break;
            case 4: transfer_funds(&customer, client_socket); break;
            case 5: apply_for_loan(&customer, client_socket); break;
            case 6: change_password(&customer, client_socket); break;
            case 7: add_feedback(&customer, client_socket); break;
            case 8: view_transaction_history(&customer, client_socket); break;
            case 9: close(client_socket); return NULL; // Logout
            default: break;
        }
    }

    close(client_socket);
    return NULL;}
    if(flag==0){
    Employee emp;
    int account_number;
    char password[20];
    char buffer[MAX_BUFFER];
    recv(client_socket, &account_number, sizeof(account_number), 0);
    recv(client_socket, password, sizeof(password), 0);
    if (validate_login1(account_number, password, &emp)) {
    if (is_account_active(account_number)) {
        send(client_socket, "Another session is already active for this account. Logout first.\n", 67, 0);
        close(client_socket);
        return NULL;
    }
    add_active_session(account_number, client_socket); // Add session
    sprintf(buffer, "Login successful. Welcome, %s!\n", emp.username);
    send(client_socket, buffer, strlen(buffer), 0);
} else {
    send(client_socket, "Login failed. Please check your account number and password.\n", 62, 0);
    close(client_socket);
    return NULL;
}
    // Handle customer operations
    int choice;
    while (1) {
        recv(client_socket, &choice, sizeof(choice), 0);
        switch (choice) {
            case 1: add_employee(&emp, client_socket); break;
            case 2: modify_employee(&emp, client_socket); break;
            case 3: modify_manager(&emp, client_socket); break;
            case 4: change_password1(&emp, client_socket); break;
            case 5: manage_user_roles(&emp, client_socket); break;
            case 6: close(client_socket); return NULL; // Logout
            default: break;
        }
    }

    close(client_socket);
    return NULL;
    }    
}


// Admin Function: Add new employee
void add_employee(Employee *emp, int client_socket) {
    // Receive data from the client
    recv(client_socket, &emp->id, sizeof(emp->id), 0);
    recv(client_socket, emp->username, sizeof(emp->username), 0);
    recv(client_socket, emp->password, sizeof(emp->password), 0);
    recv(client_socket, emp->role, sizeof(emp->role), 0);

    // Lock the mutex before accessing the file to prevent race conditions
    pthread_mutex_lock(&mutex);

    // Open the file in append mode to add the new employee
    FILE *file = fopen("employees.txt", "a");
    if (file == NULL) {
        perror("Unable to open employees.txt");
        pthread_mutex_unlock(&mutex);  // Unlock the mutex before returning
        return;
    }

    // Write the employee details to the file
    fprintf(file, "\n%d,%s,%s,%s\n", emp->id, emp->username, emp->password, emp->role);

    // Close the file after writing
    fclose(file);

    // Unlock the mutex after file operations are complete
    char response[MAX_BUFFER] = "Employee added successfully\n";
    send(client_socket, response, strlen(response), 0);
    pthread_mutex_unlock(&mutex);

}

void modify_employee(Employee *emp,int client_socket) {
    int id;
    char new_username[100];
    

    // Receive the employee ID and new username from the client
    recv(client_socket, &id, sizeof(id), 0);
    recv(client_socket, new_username, sizeof(new_username), 0);

    // Lock the mutex before accessing the files
    pthread_mutex_lock(&mutex);

    // Open the employee file in read mode and a temporary file in write mode
    FILE *file = fopen("employees.txt", "r");
    FILE *temp_file = fopen("temp.txt", "w");
    if (file == NULL || temp_file == NULL) {
        perror("Unable to open files.");
        pthread_mutex_unlock(&mutex);
        return;
    }

    int found = 0;
    char line[MAX_BUFFER];

    // Read through the file line by line, updating the employee if found
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d,%49[^,],%19[^,],%9[^\n]", &emp->id, emp->username, emp->password, emp->role);

        // If the employee ID matches, modify the username
        if (emp->id == id) {
            strcpy(emp->username, new_username);
            found = 1;
        }

        // Write the updated or unchanged employee details to the temp file
        fprintf(temp_file, "%d,%s,%s,%s\n", emp->id, emp->username, emp->password, emp->role);
    }

    // Close the files
    fclose(file);
    fclose(temp_file);

    // If employee was found, replace the old file with the modified file
    if (found) {
        remove("employees.txt");
        rename("temp.txt", "employees.txt");
        send(client_socket, "Employee username updated successfully.\n", 40, 0);
    } else {
        // If not found, delete the temp file and notify the client
        remove("temp.txt");
        send(client_socket, "Employee with the given ID not found.\n", 38, 0);
    }

    // Unlock the mutex after file operations are complete
    pthread_mutex_unlock(&mutex);
}
// Admin Function: Modify employee
void modify_manager(Employee *emp, int client_socket) {
    int id;
    char new_username[100];
    

    // Receive the employee ID and new username from the client
    recv(client_socket, &id, sizeof(id), 0);
    recv(client_socket, new_username, sizeof(new_username), 0);

    // Lock the mutex before accessing the files
    pthread_mutex_lock(&mutex);

    // Open the employee file in read mode and a temporary file in write mode
    FILE *file = fopen("employees.txt", "r");
    FILE *temp_file = fopen("temp.txt", "w");
    if (file == NULL || temp_file == NULL) {
        perror("Unable to open files.");
        pthread_mutex_unlock(&mutex);
        return;
    }

    int found = 0;
    char line[MAX_BUFFER];

    // Read through the file line by line, updating the employee if found
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d,%49[^,],%19[^,],%9[^\n]", &emp->id, emp->username, emp->password, emp->role);

        // If the employee ID matches, modify the username
        if (emp->id == id) {
            strcpy(emp->username, new_username);
            found = 1;
        }

        // Write the updated or unchanged employee details to the temp file
        fprintf(temp_file, "%d,%s,%s,%s\n", emp->id, emp->username, emp->password, emp->role);
    }

    // Close the files
    fclose(file);
    fclose(temp_file);

    // If employee was found, replace the old file with the modified file
    if (found) {
        remove("employees.txt");
        rename("temp.txt", "employees.txt");
        send(client_socket, "manager username updated successfully.\n", 40, 0);
    } else {
        // If not found, delete the temp file and notify the client
        remove("temp.txt");
        send(client_socket, "manager with the given ID not found.\n", 38, 0);
    }

    // Unlock the mutex after file operations are complete
    pthread_mutex_unlock(&mutex);
}


// Admin Function: Manage user roles
void manage_user_roles(Employee *emp, int client_socket) {
    int id;
    char new_role[10];

    // Receive the employee ID and new role from the client
    recv(client_socket, &id, sizeof(id), 0);
    recv(client_socket, new_role, sizeof(new_role), 0);

    // Lock the mutex before accessing the files
    pthread_mutex_lock(&mutex);

    // Open the file in read mode to find the employee and in write mode to rewrite the file
    FILE *file = fopen("employees.txt", "r");
    FILE *temp_file = fopen("temp.txt", "w");
    if (file == NULL || temp_file == NULL) {
        perror("Unable to open files.");
        pthread_mutex_unlock(&mutex);  // Unlock the mutex before returning
        return;
    }

    // Temporary variables for reading employee data from the file
    int file_id;
    char file_username[50];
    char file_password[20];
    char file_role[10];

    // Flag to indicate if the employee role was changed
    int found = 0;

    // Read through the employees.txt file and copy data to temp.txt with modifications
    while (fscanf(file, "%d,%49[^,],%19[^,],%9[^,\n]\n",
                  &file_id, file_username, file_password, file_role) != EOF) {
        if (file_id == id) {
            // Employee with the given ID found, change the role
            strcpy(file_role, new_role);
            found = 1;
        }
        // Write the (possibly modified) employee data to the temporary file
        fprintf(temp_file, "%d,%s,%s,%s\n", file_id, file_username, file_password, file_role);
    }

    // Close both files
    fclose(file);
    fclose(temp_file);

    // Replace the original file with the modified temp file
    if (found) {
        remove("employees.txt");
        rename("temp.txt", "employees.txt");
        printf("Employee role updated successfully.\n");
    } else {
        // If the employee was not found, remove the temp file
        remove("temp.txt");
        printf("Employee with ID %d not found.\n", id);
    }

    // Unlock the mutex after file operations are complete
    pthread_mutex_unlock(&mutex);
}


// Admin Function: Change password
void change_password1(Employee *emp, int client_socket) {
    char new_password[100];
    
    // Receive the new password from the client
    recv(client_socket, new_password, sizeof(new_password), 0);

    // Open the file in read mode to find the employee and in write mode to rewrite the file
    FILE *file = fopen("employees.txt", "r");
    FILE *temp_file = fopen("temp.txt", "w");
    if (file == NULL || temp_file == NULL) {
        perror("Unable to open files.");
        return;
    }

    // Temporary variables for reading employee data from the file
    int file_id;
    char file_username[50];
    char file_password[20];
    char file_role[10];

    // Flag to indicate if the current user is the one whose password is being changed
    int found = 0;

    // Read through the employees.txt file and copy data to temp.txt with modifications
    while (fscanf(file, "%d,%49[^,],%19[^,],%9[^,\n]\n",
                  &file_id, file_username, file_password, file_role) != EOF) {
        if (strcmp(file_username, emp->username) == 0) {
            // If username matches, update the password
            strcpy(file_password, new_password);
            found = 1;
        }
        // Write the (possibly modified) employee data to the temporary file
        fprintf(temp_file, "%d,%s,%s,%s\n", file_id, file_username, file_password, file_role);
    }

    // Close both files
    fclose(file);
    fclose(temp_file);

    // Replace the original file with the modified temp file
    if (found) {
        remove("employees.txt");
        rename("temp.txt", "employees.txt");
        printf("Password updated successfully.\n");
    } else {
        // If the employee was not found, remove the temp file
        remove("temp.txt");
        printf("Employee not found.\n");
    }
}


int validate_login(int account_number, const char *password, Customer *customer) {
    FILE *file = fopen("customers.txt", "r");
    if (file == NULL) {
        perror("Unable to open customers.txt");
        return 0;
    }

    while (fscanf(file, "%d,%49[^,],%19[^,],%lf,%d,%d,%99[^,],%d,%d\n",
                  &customer->account_number, customer->name, customer->password,
                  &customer->balance, &customer->loan_status, &customer->active_status,
                  customer->feedback, &customer->loan_application, &customer->transaction_count) != EOF) {
        if (customer->account_number == account_number && strcmp(customer->password, password) == 0) {
            fclose(file);
            return 1; // Login successful
        }
    }

    fclose(file);
    return 0; // Login failed
}
int validate_login1(int account_number, const char *password, Employee *emp) {
    FILE *file = fopen("employees.txt", "r");
    if (file == NULL) {
        perror("Unable to open employees.txt");
        return 0;
    }

    while (fscanf(file, "%d,%49[^,],%19[^,],%9[^,\n]\n",
                  &emp->id, emp->username, emp->password, emp->role) != EOF) {
        if (emp->id == account_number && strcmp(emp->password, password) == 0) {
            fclose(file);
            return 1; // Login successful
        }
    }

    fclose(file);
    return 0; // Login failed
}


void log_transaction(int from_account, int to_account, double amount, const char *status) {
    FILE *file = fopen("transactions.txt", "a");
    if (file == NULL) {
        perror("Unable to open transactions.txt");
        return;
    }

    fprintf(file, "%d,%d,%.2f,%s\n", from_account, to_account, amount, status);
    fclose(file);
}




void view_balance(Customer *customer, int client_socket) {
    char buffer[MAX_BUFFER];
    sprintf(buffer, "Your account balance is: %.2f\n", customer->balance);
    send(client_socket, buffer, strlen(buffer), 0);
}

void deposit_money(Customer *customer, int client_socket) {
    double amount;
    char buffer[MAX_BUFFER];
    
    recv(client_socket, &amount, sizeof(amount), 0);
    if (amount <= 0) {
        send(client_socket, "Invalid amount. Please enter a positive number.\n", 48, 0);
        return;
    }

    customer->balance += amount;
    customer->transaction_count++;
    update_customer_file(customer);
    sprintf(buffer, "You have successfully deposited %.2f. New balance: %.2f\n", amount, customer->balance);
    send(client_socket, buffer, strlen(buffer), 0);
	log_transaction(customer->account_number, customer->account_number, amount, "credit");
}

void withdraw_money(Customer *customer, int client_socket) {
    double amount;
    char buffer[MAX_BUFFER];

    recv(client_socket, &amount, sizeof(amount), 0);
    if (amount <= 0 || amount > customer->balance) {
        send(client_socket, "Invalid amount. Please enter a positive number that does not exceed your balance.\n", 80, 0);
        return;
    }

    customer->balance -= amount;
    customer->transaction_count++;
    update_customer_file(customer);
    sprintf(buffer, "You have successfully withdrawn %.2f. New balance: %.2f\n", amount, customer->balance);
    send(client_socket, buffer, strlen(buffer), 0);
	log_transaction(customer->account_number, customer->account_number, amount, "debit");
}

void transfer_funds(Customer *customer, int client_socket) {
    double amount;
    int recipient_account;
    char buffer[MAX_BUFFER];

    // Receive recipient account and transfer amount
    recv(client_socket, &recipient_account, sizeof(recipient_account), 0);
    recv(client_socket, &amount, sizeof(amount), 0);

    // Validate the transfer amount
    if (amount <= 0 || amount > customer->balance) {
        send(client_socket, "Invalid amount. Please enter a positive number that does not exceed your balance.\n", 80, 0);
        return;
    }

    // Open the customer file to search for the recipient account
    FILE *file = fopen("customers.txt", "r+");
    if (file == NULL) {
        perror("Unable to open customers.txt");
        send(client_socket, "Internal error. Please try again later.\n", 40, 0);
        return;
    }

    Customer recipient;
    int recipient_found = 0;
    long recipient_pos;

    // Find the recipient account
    while ((recipient_pos = ftell(file)) >= 0 && fscanf(file, "%d,%49[^,],%19[^,],%lf,%d,%d,%99[^,],%d,%d\n",
                  &recipient.account_number, recipient.name, recipient.password,
                  &recipient.balance, &recipient.loan_status, &recipient.active_status,
                  recipient.feedback, &recipient.loan_application, &recipient.transaction_count) != EOF) {
        if (recipient.account_number == recipient_account) {
            recipient_found = 1;
            break;
        }
    }

    // If recipient not found
    if (!recipient_found) {
        send(client_socket, "Recipient account not found.\n", 30, 0);
        fclose(file);
        return;
    }

    // Deduct amount from sender's balance
    customer->balance -= amount;
    customer->transaction_count++;

    // Update recipient's balance
    recipient.balance += amount;
    recipient.transaction_count++;

    // Update customer files (sender and recipient)
    update_customer_file(customer);  // Update sender's details
    fseek(file, recipient_pos, SEEK_SET); // Go back to recipient's position
    fprintf(file, "%d,%s,%s,%.2f,%d,%d,%s,%d,%d\n",
            recipient.account_number, recipient.name, recipient.password,
            recipient.balance, recipient.loan_status, recipient.active_status,
            recipient.feedback, recipient.loan_application, recipient.transaction_count);
    fflush(file); // Flush changes to the file
    fclose(file);

    // Log the transaction
    log_transaction(customer->account_number, recipient_account, amount, "transfer");

    // Inform the client that the transfer was successful
    sprintf(buffer, "You have successfully transferred %.2f to account number %d. New balance: %.2f\n", amount, recipient_account, customer->balance);
    send(client_socket, buffer, strlen(buffer), 0);
}



void apply_for_loan(Customer *customer, int client_socket) {
    if (customer->active_status==0){
        send(client_socket, "You are not  active.\n", 20, 0);
        return;
    }
    else{
    if (customer->loan_application == 1) {
        send(client_socket, "You have already applied for a loan.\n", 37, 0);
        return;
    }


    // Set loan_application flag
    customer->loan_application = 1;
    
    // Receive loan amount from the client
    int loan_amount;
    
    recv(client_socket, &loan_amount, sizeof(loan_amount), 0);
    if (customer->balance>=(loan_amount/10)){
    // Update customer file (if necessary)
    FILE *loan_file = fopen("loans.txt", "a");
    if (loan_file == NULL) {
        perror("Unable to open loan file");
        return;
    }
    fprintf(loan_file, "%d,%d,%d\n",
            customer->account_number, loan_amount,0);
    
    // Send response back to the client
    fclose(loan_file);
    send(client_socket, "Your loan application has been submitted.\n", 42, 0);
    }else{
        customer->loan_application =0;
        send(client_socket, "Your loan application not been submitted.\n", 42, 0);
    }
    }
}
void change_password(Customer *customer, int client_socket) {
    char new_password[20];
    recv(client_socket, new_password, sizeof(new_password), 0);
    
    strcpy(customer->password, new_password);
    update_customer_file(customer);
    send(client_socket, "Your password has been changed successfully.\n", 45, 0);
}

void add_feedback(Customer *customer, int client_socket) {
    char feedback[100];
    recv(client_socket, feedback, sizeof(feedback), 0);
    
    strcpy(customer->feedback, feedback);
    update_customer_file(customer);
    send(client_socket, "Your feedback has been recorded.\n", 34, 0);
}

void view_transaction_history(Customer *customer, int client_socket) {
    char buffer[MAX_BUFFER];
    sprintf(buffer, "Transaction History for Account %d:\n", customer->account_number);
    send(client_socket, buffer, strlen(buffer), 0);

    FILE *file = fopen("transactions.txt", "r");
    if (file == NULL) {
        perror("Unable to open transactions.txt");
        send(client_socket, "No transaction history available.\n", 34, 0);
        return;
    }

    char line[MAX_BUFFER];
    int from_account, to_account;
    double amount;
    char status[10];

    // Read through the transactions and filter by the account number
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d,%d,%lf,%s", &from_account, &to_account, &amount, status);

        // Check if the transaction involves this account
        if (from_account == customer->account_number || to_account == customer->account_number) {
            sprintf(buffer, "From: %d, To: %d, Amount: %.2f, Status: %s\n", from_account, to_account, amount, status);
            send(client_socket, buffer, strlen(buffer), 0);
        }
    }

    fclose(file);
}


void assign(int client_socket) {
    FILE *loan_file, *emp_file;
    Loan loan;
    Employee employee;
    int emp_index = 0;
    int total_employees = 0;
    char role_needed[] = "employee"; 

    printf("Banking server started...\n");
    pthread_mutex_lock(&mutex);
    printf("Locking mutex for loan assignment...\n");

    // Open the employees file for reading
    emp_file = fopen("employees.txt", "r");
    if (!emp_file) {
        perror("Failed to open employees.txt");
        pthread_mutex_unlock(&mutex);
        return;
    }
    printf("Successfully opened employees.txt\n");

    // Read employee data
    Employee employees[100];  
    while (fscanf(emp_file, "%d,%49[^,],%19[^,],%9[^,\n]\n", 
                  &employee.id, employee.username, employee.password, employee.role) == 4) {
        if (strcmp(employee.role, role_needed) == 0) {
            printf("Found employee: ID=%d, Username=%s, Role=%s\n", 
                   employee.id, employee.username, employee.role);
            employees[total_employees++] = employee;  
        }
    }
    fclose(emp_file);

    if (total_employees == 0) {
        printf("No employees available with the role '%s'\n", role_needed);
        pthread_mutex_unlock(&mutex);
        return;
    }

    // Open the loans file for reading and updating
    loan_file = fopen("loans.txt", "r+b");
    if (!loan_file) {
        perror("Failed to open loans.txt");
        pthread_mutex_unlock(&mutex);
        return;
    }
    printf("Successfully opened loans.txt for reading and updating\n");

    // Loop to read and assign loans
    while (fread(&loan, sizeof(Loan), 1, loan_file) == 1) {
        printf("Reading loan: custid: %d, loan: %d, empid: %d\n", 
               loan.custid, loan.loan, loan.empid);

        // Check if the loan is not already assigned
        if (loan.empid == 0) {
            // Assign the employee ID from the array
            loan.empid = employees[emp_index].id;

            // Move the file pointer back to the position of this loan record
            fseek(loan_file, -sizeof(Loan), SEEK_CUR);

            // Write the modified loan back to the file
            if (fwrite(&loan, sizeof(Loan), 1, loan_file) != 1) {
                perror("Failed to write updated loan back to loans.txt");
            } else {
                printf("Assigned Loan (custid: %d, loan: %d) to Employee (empid: %d)\n",
                       loan.custid, loan.loan, loan.empid);
            }
            emp_index = (emp_index + 1) % total_employees; // Circular assignment of employees
        } else {
            printf("Loan already assigned to Employee ID: %d\n", loan.empid);
        }
    }

    fclose(loan_file); 
    printf("Closed loans.txt after processing\n");

    // Send response to client
    send(client_socket, "Loan application has been assigned.\n", 37, 0); 
    pthread_mutex_unlock(&mutex);
    printf("Mutex unlocked, loan assignment complete.\n");
}


void update_customer_file(Customer *customer) {
    pthread_mutex_lock(&lock); // Lock the access to the file

    // Open the file in read mode to load all customers into memory
    FILE *file = fopen("customers.txt", "r");
    if (file == NULL) {
        perror("Unable to open customers.txt");
        pthread_mutex_unlock(&lock); // Unlock the mutex
        return;
    }

    // Temporary array to store all customer data
    Customer customers[100];
    int count = 0;

    // Load all customers from the file
    while (fscanf(file, "%d,%49[^,],%19[^,],%lf,%d,%d,%99[^,],%d,%d\n",
                  &customers[count].account_number, customers[count].name,
                  customers[count].password, &customers[count].balance,
                  &customers[count].loan_status, &customers[count].active_status,
                  customers[count].feedback, &customers[count].loan_application,
                  &customers[count].transaction_count) != EOF) {
        count++;
    }
    
    fclose(file); // Close the file after reading

    // Open the file again in write mode to rewrite the entire file
    file = fopen("customers.txt", "w");
    if (file == NULL) {
        perror("Unable to open customers.txt for writing");
        pthread_mutex_unlock(&lock); // Unlock the mutex
        return;
    }

    // Rewrite each customer's data
    for (int i = 0; i < count; i++) {
        if (customers[i].account_number == customer->account_number) {
            // Update the relevant customer record
            customers[i] = *customer;
        }

        // Write the customer back to the file
        fprintf(file, "%d,%s,%s,%.2f,%d,%d,%s,%d,%d\n",
                customers[i].account_number, customers[i].name,
                customers[i].password, customers[i].balance,
                customers[i].loan_status, customers[i].active_status,
                customers[i].feedback, customers[i].loan_application,
                customers[i].transaction_count);
    }

    fclose(file); // Close the file after writing
    pthread_mutex_unlock(&lock); // Unlock the mutex after updating
}



void deactivate(int client_socket) {
    int account_number;
    char buffer[MAX_LINE_LENGTH];
    // Receive account number from client
    recv(client_socket, &account_number, sizeof(account_number), 0);
    
    // Lock the mutex before accessing shared resource
    pthread_mutex_lock(&mutex);

    FILE *file = fopen("customers.txt", "r");
    if (!file) {
        perror("Failed to open customers.txt");
        pthread_mutex_unlock(&mutex);
        return;
    }

    FILE *temp_file = fopen("temp.txt", "w");
    if (!temp_file) {
        perror("Failed to open temp file");
        fclose(file);
        pthread_mutex_unlock(&mutex);
        return;
    }

    Customer customer;
    int found = 0;

    // Read records to find the customer
    while (fgets(buffer, MAX_LINE_LENGTH, file)) {
        sscanf(buffer, "%d,%49[^,],%19[^,],%lf,%d,%d,%99[^,],%d,%d",
               &customer.account_number, customer.name, customer.password,
               &customer.balance, &customer.loan_status, &customer.active_status,
               customer.feedback, &customer.loan_application, &customer.transaction_count);

        if (customer.account_number == account_number) {
            found = 1;
            if (customer.active_status == 1) {
                // Deactivate the customer
                customer.active_status = 0;
                printf("Account %d has been deactivated.\n", account_number);
            } else {
                printf("Account %d is already deactivated.\n", account_number);
            }
        }

        // Write the record to the temporary file
        fprintf(temp_file, "%d,%s,%s,%.2f,%d,%d,%s,%d,%d\n",
                customer.account_number, customer.name, customer.password,
                customer.balance, customer.loan_status, customer.active_status,
                customer.feedback, customer.loan_application, customer.transaction_count);
    }

    if (!found) {
        char response[MAX_BUFFER] = " record not found\n";
        send(client_socket, response, strlen(response), 0);
        fclose(file);
        fclose(temp_file);
        remove("customers.txt");
        rename("temp.txt", "customers.txt");

    } else{
    fclose(file);
    fclose(temp_file);
    
    // Replace the original file with the modified temporary file
    
    remove("customers.txt");
    rename("temp.txt", "customers.txt");
    char response[MAX_BUFFER] = " deactivated successfully\n";
    send(client_socket, response, strlen(response), 0);
    }
    pthread_mutex_unlock(&mutex); // Unlock the mutex
}

void activate(int client_socket) {
    int account_number;
    char buffer[MAX_LINE_LENGTH];
    // Receive account number from client
    recv(client_socket, &account_number, sizeof(account_number), 0);
    
    // Lock the mutex before accessing shared resource
    pthread_mutex_lock(&mutex);

    FILE *file = fopen("customers.txt", "r");
    if (!file) {
        perror("Failed to open customers.txt");
        pthread_mutex_unlock(&mutex);
        return;
    }

    FILE *temp_file = fopen("temp.txt", "w");
    if (!temp_file) {
        perror("Failed to open temp file");
        fclose(file);
        pthread_mutex_unlock(&mutex);
        return;
    }

    Customer customer;
    int found = 0;

    // Read records to find the customer
    while (fgets(buffer, MAX_LINE_LENGTH, file)) {
        sscanf(buffer, "%d,%49[^,],%19[^,],%lf,%d,%d,%99[^,],%d,%d",
               &customer.account_number, customer.name, customer.password,
               &customer.balance, &customer.loan_status, &customer.active_status,
               customer.feedback, &customer.loan_application, &customer.transaction_count);

        if (customer.account_number == account_number) {
            found = 1;
            if (customer.active_status == 0) {
                // Activate the customer
                customer.active_status = 1;
                printf("Account %d has been activated.\n", account_number);
            } else {
                printf("Account %d is already active.\n", account_number);
            }
        }

        // Write the record to the temporary file
        fprintf(temp_file, "%d,%s,%s,%.2f,%d,%d,%s,%d,%d\n",
                customer.account_number, customer.name, customer.password,
                customer.balance, customer.loan_status, customer.active_status,
                customer.feedback, customer.loan_application, customer.transaction_count);
    }

    if (!found) {
        char response[MAX_BUFFER] = " record not found\n";
        send(client_socket, response, strlen(response), 0);
        fclose(file);
        fclose(temp_file);
        remove("customers.txt");
        rename("temp.txt", "customers.txt");

    } else{
    fclose(file);
    fclose(temp_file);
    
    // Replace the original file with the modified temporary file
    
    remove("customers.txt");
    rename("temp.txt", "customers.txt");
    char response[MAX_BUFFER] = " activated successfully\n";
    send(client_socket, response, strlen(response), 0);
    }
    pthread_mutex_unlock(&mutex); // Unlock the mutex
}

void review(int client_socket) {
    int account_number;
    char buffer[MAX_LINE_LENGTH];
    
    // Receive account number from client
    recv(client_socket, &account_number, sizeof(account_number), 0);
    
    // Lock the mutex before accessing shared resource
    pthread_mutex_lock(&mutex);

    FILE *file = fopen("customers.txt", "r");
    if (!file) {
        perror("Failed to open customers.txt");
        pthread_mutex_unlock(&mutex);
        return;
    }

    Customer customer;
    int found = 0;

    // Read records to find the customer
    while (fgets(buffer, MAX_LINE_LENGTH, file)) {
        sscanf(buffer, "%d,%49[^,],%19[^,],%lf,%d,%d,%99[^,],%d,%d",
               &customer.account_number, customer.name, customer.password,
               &customer.balance, &customer.loan_status, &customer.active_status,
               customer.feedback, &customer.loan_application, &customer.transaction_count);

        if (customer.account_number == account_number) {
            found = 1;
            // Print the feedback
            printf("Feedback for Account %d: %s\n", account_number, customer.feedback);
            break;
        }
    }

    if (!found) {
        printf("Account %d not found.\n", account_number);
    }

    fclose(file); // Close the file after operations
    pthread_mutex_unlock(&mutex); // Unlock the mutex
}

int get_next_account_number() {
    FILE *file = fopen(FILENAME, "r");
    if (!file) {
        // If the file doesn't exist, return the starting account number
        return 5;
    }

    char buffer[MAX_LINE_LENGTH];
    Customer customer;
    int max_account_number = 4; // Initialize below the starting account number

    // Read through the file to find the highest existing account number
    while (fgets(buffer, MAX_LINE_LENGTH, file)) {
        sscanf(buffer, "%d %s %s %lf %d %d %s %d %d",
               &customer.account_number,
               customer.name,
               customer.password,
               &customer.balance,
               &customer.loan_status,
               &customer.active_status,
               customer.feedback,
               &customer.loan_application,
               &customer.transaction_count);

        if (customer.account_number > max_account_number) {
            max_account_number = customer.account_number;
        }
    }

    fclose(file);
    return max_account_number + 1; // Next account number
}

void add_cust(int client_socket) {
    char name[100], password[100];
    
    // Receive name and password from client
    recv(client_socket, name, sizeof(name), 0);
    recv(client_socket, password, sizeof(password), 0);

    // Lock mutex before accessing the file
    pthread_mutex_lock(&mutex);

    // Get the next available account number
    int account_number = get_next_account_number();

    // Open the file in append mode
    FILE *file = fopen(FILENAME, "a");
    if (!file) {
        perror("Failed to open customers.txt");
        pthread_mutex_unlock(&mutex); // Unlock mutex if file opening fails
        return;
    }

    // Initialize the customer record
    Customer new_customer;
    new_customer.account_number = account_number;
    strncpy(new_customer.name, name, sizeof(new_customer.name) - 1);
    strncpy(new_customer.password, password, sizeof(new_customer.password) - 1);
    new_customer.balance = 0.0; // Default balance
    new_customer.loan_status = 0; // No loan
    new_customer.active_status = 1; // Active account
    strcpy(new_customer.feedback, ""); // No feedback yet
    new_customer.loan_application = 0; // No loan application
    new_customer.transaction_count = 0; // No transactions yet

    // Write the new customer record to the file
    fprintf(file, "%d %s %s %.2lf %d %d %s %d %d\n",
            new_customer.account_number,
            new_customer.name,
            new_customer.password,
            new_customer.balance,
            new_customer.loan_status,
            new_customer.active_status,
            new_customer.feedback,
            new_customer.loan_application,
            new_customer.transaction_count);

    fclose(file); // Close the file

    printf("Added customer: Account Number %d, Name: %s\n", account_number, name);

    // Unlock the mutex after finishing the file operations
    pthread_mutex_unlock(&mutex);
}


void modify_cust(int client_socket) {
    int account_number;
    char new_name[100];

    // Receive the account number and new name from the client
    recv(client_socket, &account_number, sizeof(account_number), 0);
    recv(client_socket, new_name, sizeof(new_name), 0);

    // Lock mutex before accessing the file
    pthread_mutex_lock(&mutex);

    FILE *file = fopen(FILENAME, "r");
    if (!file) {
        perror("Failed to open customers.txt");
        pthread_mutex_unlock(&mutex); // Unlock mutex if file opening fails
        return;
    }

    // Temporary file to write updated data
    FILE *temp_file = fopen("temp_customers.txt", "w");
    if (!temp_file) {
        perror("Failed to open temporary file");
        fclose(file);
        pthread_mutex_unlock(&mutex);
        return;
    }

    char buffer[MAX_LINE_LENGTH];
    Customer customer;
    int found = 0; // Flag to check if account is found

    // Read each line from the original file
    while (fgets(buffer, MAX_LINE_LENGTH, file)) {
        sscanf(buffer, "%d %s %s %lf %d %d %s %d %d",
               &customer.account_number,
               customer.name,
               customer.password,
               &customer.balance,
               &customer.loan_status,
               &customer.active_status,
               customer.feedback,
               &customer.loan_application,
               &customer.transaction_count);

        // If account number matches, modify the name
        if (customer.account_number == account_number) {
            strncpy(customer.name, new_name, sizeof(customer.name) - 1);
            customer.name[sizeof(customer.name) - 1] = '\0'; // Ensure null termination
            found = 1;
        }

        // Write the (possibly modified) customer data to the temp file
        fprintf(temp_file, "%d %s %s %.2lf %d %d %s %d %d\n",
                customer.account_number,
                customer.name,
                customer.password,
                customer.balance,
                customer.loan_status,
                customer.active_status,
                customer.feedback,
                customer.loan_application,
                customer.transaction_count);
    }

    fclose(file);
    fclose(temp_file);

    // If the account was found and modified, replace the original file with the temp file
    if (found) {
        remove(FILENAME);              // Remove original file
        rename("temp_customers.txt", FILENAME); // Rename temp file to original file name
        printf("Username for account number %d updated to %s\n", account_number, new_name);
    } else {
        remove("temp_customers.txt"); // Clean up temp file if no account was modified
        printf("Account number %d not found\n", account_number);
    }
  }
  
  void view_loan(int client_socket) {
    int empid;
    
    // Receive empid from the client
    recv(client_socket, &empid, sizeof(empid), 0);

    // Lock the file to ensure thread-safe access
    pthread_mutex_lock(&mutex);

    FILE *file = fopen(LOANS_FILE, "r");
    if (!file) {
        perror("Failed to open loans.txt");
        pthread_mutex_unlock(&mutex); // Unlock mutex if file opening fails
        return;
    }

    char buffer[MAX_LINE_LENGTH];
    int custid, file_empid;
    int found = 0; // Flag to check if any loan applications match
    Loan loan;
    printf("Loan applications assigned to employee ID %d:\n", empid);
    printf("Customer ID | Loan Status\n");

    // Read each line from loans.txt and check if empid matches
    while (fscanf(file, "%d,%d,%d\n", &loan.custid, &loan.loan, &loan.empid) == 3) {
        if (loan.empid == empid) {
            found = 1;
            printf("%d          | %s\n", loan.custid, loan.loan ? "Active Loan" : "No Loan");
        }
    }

    if (!found) {
        printf("No loan applications found for employee ID %d.\n", empid);
    }

    fclose(file);
    send(client_socket, "successfully viewed.\n", 21, 0); 
    pthread_mutex_unlock(&mutex); // Unlock the mutex after file operations
}

void sanction_loan(int client_socket) {
    int empid;
    
    // Receive empid from the client
    recv(client_socket, &empid, sizeof(empid), 0);

    // Lock the mutex for thread safety
    pthread_mutex_lock(&mutex);

    // Open the loans.txt file to find customers associated with the empid
    FILE *loans_file = fopen(LOANS_FILE, "r");
    if (!loans_file) {
        perror("Failed to open loans.txt");
        pthread_mutex_unlock(&mutex); // Unlock mutex if file opening fails
        return;
    }

    // Read loans.txt to find all customer IDs assigned to the employee
    int custid, loan, file_empid;
    int found = 0;  // To track if any matching loan applications are found

    // Loop through the loans file to find customers associated with empid
    while (fscanf(loans_file, "%d,%d,%d", &custid, &loan, &file_empid) == 3) {
        // If the empid matches, we need to sanction the loan for this customer
        if (file_empid == empid) {
            found = 1;

            // Now open the customers.txt file to update the loan_status for this custid
            FILE *customers_file = fopen(CUSTOMERS_FILE, "r+b");
            if (!customers_file) {
                perror("Failed to open customers.txt");
                fclose(loans_file);
                pthread_mutex_unlock(&mutex); // Unlock mutex if file opening fails
                return;
            }

            Customer customer;
            // Read customer records and find the matching account number
            while (fread(&customer, sizeof(Customer), 1, customers_file)) {
                if (customer.account_number == custid) {
                    // Sanction the loan by updating the loan_status to 1
                    customer.loan_status = 1;

                    // Move the file pointer back to the start of the record
                    fseek(customers_file, -sizeof(Customer), SEEK_CUR);

                    // Write the updated record back to the file
                    fwrite(&customer, sizeof(Customer), 1, customers_file);
                    printf("Loan for customer account number %d has been sanctioned.\n", customer.account_number);
                    break;
                }
            }

            fclose(customers_file);
        }
    }

    if (!found) {
        printf("No loan applications found for employee ID %d.\n", empid);
    }

    fclose(loans_file);
    send(client_socket, "Loan successfully sanctioned.\n", 31, 0); 
    pthread_mutex_unlock(&mutex); // Unlock the mutex after all operations
}
