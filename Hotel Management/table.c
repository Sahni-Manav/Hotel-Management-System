#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

#define SHM_SIZE 1024

typedef struct {
    int table_number;
    char order[256]; // Assuming orders are strings with a maximum length of 255 characters
    int terminate;   // Flag to indicate termination
} Order;

typedef struct {
    int table_number;
    int bill_amount;
} BillInfo;

typedef struct {
    int serial_number;
    char item_name[256];
    int price;
} MenuItem;

// Function to check if a given serial number is valid in the menu
int isValidItem(int serial_number, MenuItem menu[], int menu_size) {
    for (int i = 0; i < menu_size; i++) {
        if (menu[i].serial_number == serial_number) {
            return 1; // Valid item
        }
    }
    return 0; // Invalid item
}

// Function to calculate total bill amount for the table
int calculateBillAmount(char order[], MenuItem menu[], int menu_size) {
    int total_amount = 0;
    char *token = strtok(order, ",");
    while (token != NULL) {
        int serial_number = atoi(token);
        for (int i = 0; i < menu_size; i++) {
            if (menu[i].serial_number == serial_number) {
                total_amount += menu[i].price;
                break;
            }
        }
        token = strtok(NULL, ",");
    }
    return total_amount;
}

int main() {
    int waiter_id;
    pid_t pid = fork(); // Create a child process

    if (pid < 0) {
        // Fork failed
        fprintf(stderr, "Fork failed\n");
        return 1;
    } else if (pid == 0) {
        // Child process
        printf("Enter Waiter ID (1 to 10): ");
        scanf("%d", &waiter_id);
        printf("Waiter process created with PID: %d, Waiter ID: %d\n", getpid(), waiter_id);

        // Define menu
        MenuItem menu[] = {
            {1, "Burger", 20},
            {2, "Pizza", 30},
            {3, "Salad", 15},
            // Add more items as needed
        };
        int menu_size = sizeof(menu) / sizeof(menu[0]);

        // Create or open shared memory segment with table process
        char table_shm_name[20];
        sprintf(table_shm_name, "/waiter_table_%d", waiter_id);
        int table_shm_fd = shm_open(table_shm_name, O_RDWR | O_CREAT, 0666);
        if (table_shm_fd == -1) {
            perror("shm_open");
            return 1;
        }

        // Set the size of the shared memory segment
        ftruncate(table_shm_fd, SHM_SIZE);

        // Map the shared memory segment to memory
        Order *shared_table_data = (Order *)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, table_shm_fd, 0);
        if (shared_table_data == MAP_FAILED) {
            perror("mmap");
            return 1;
        }

        // Create or open shared memory segment with hotel manager process
        char manager_shm_name[20];
        sprintf(manager_shm_name, "/waiter_manager_%d", waiter_id);
        int manager_shm_fd = shm_open(manager_shm_name, O_RDWR | O_CREAT, 0666);
        if (manager_shm_fd == -1) {
            perror("shm_open");
            return 1;
        }

        // Set the size of the shared memory segment
        ftruncate(manager_shm_fd, sizeof(BillInfo));

        // Map the shared memory segment to memory
        BillInfo *shared_manager_data = (BillInfo *)mmap(NULL, sizeof(BillInfo), PROT_READ | PROT_WRITE, MAP_SHARED, manager_shm_fd, 0);
        if (shared_manager_data == MAP_FAILED) {
            perror("mmap");
            return 1;
        }

        printf("Waiting for orders from table %d\n", waiter_id);

        // Wait for orders from the corresponding table process
        while (1) {
            if (shared_table_data->table_number == waiter_id && strlen(shared_table_data->order) > 0) {
                printf("Received order from table %d: %s\n", shared_table_data->table_number, shared_table_data->order);
                
                // Check if order items are valid
                char *token = strtok(shared_table_data->order, ",");
                int isValid = 1;
                while (token != NULL) {
                    int serial_number = atoi(token);
                    if (!isValidItem(serial_number, menu, menu_size)) {
                        printf("Invalid item in the order: %d\n", serial_number);
                        isValid = 0;
                        break;
                    }
                    token = strtok(NULL, ",");
                }

                // Calculate bill amount
                int bill_amount = isValid ? calculateBillAmount(shared_table_data->order, menu, menu_size) : 0;

                // Communicate the bill amount to the hotel manager process
                shared_manager_data->table_number = waiter_id;
                shared_manager_data->bill_amount = bill_amount;

                // Reset table number to indicate order has been processed
                shared_table_data->table_number = 0;
            }

            // Check for termination signal from table process
            if (shared_table_data->terminate) {
                printf("Received termination signal from table %d\n", waiter_id);
                break;
            }

            sleep(10); // Sleep for 100ms before checking again
        }

        // Unmap and close shared memory segments
        munmap(shared_table_data, SHM_SIZE);
        close(table_shm_fd);
        munmap(shared_manager_data, sizeof(BillInfo));
        close(manager_shm_fd);

        printf("Waiter process with PID %d and Waiter ID %d has finished\n", getpid(), waiter_id);
    } else {
        // Parent process
        // Wait for the child process to finish
        wait(NULL);
    }

    return 0;
}