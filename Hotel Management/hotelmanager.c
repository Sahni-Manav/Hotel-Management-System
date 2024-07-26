wi#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>

#define MAX_TABLES 10
#define ITERATIONS_BEFORE_TERMINATION 1  // Change this value based on your application logic

int main() {
    int total_tables, total_earnings = 0, total_wages = 0;
    int termination_intimation = 0;  // Flag to indicate termination intimation

    // Ask for the total number of tables
    printf("Enter the Total Number of Tables at the Hotel: ");
    scanf("%d", &total_tables);

    // Create shared memory for communicating earnings with waiters
    int *earnings_to_manager = mmap(NULL, sizeof(int) * total_tables, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (earnings_to_manager == MAP_FAILED) {
        perror("Error creating shared memory");
        return 1;
    }

    // Open the output file for earnings
    FILE *earnings_file = fopen("earnings.txt", "r");
    if (earnings_file == NULL) {
        perror("Error opening earnings file");
        return 1;
    }

    // Read earnings from the file
    for (int i = 0; i < total_tables; i++) {
        fscanf(earnings_file, "%d", &earnings_to_manager[i]);
    }

    // Close the earnings file
    fclose(earnings_file);

    // Simulate waiter earnings for a few iterations
    for (int iteration = 1; iteration <= ITERATIONS_BEFORE_TERMINATION; iteration++) {
        // Print iteration information
        printf("Iteration %d:\n", iteration);
        printf("Waiter Earnings:\n");
        for (int i = 0; i < total_tables; i++) {
            printf("Table %d: %d INR\n", i + 1, earnings_to_manager[i]);
        }
        printf("\n");

        sleep(1);  // Simulate some time passing

        // Set termination intimation after a certain number of iterations
        if (iteration == ITERATIONS_BEFORE_TERMINATION) {
            termination_intimation = 1;
        }
    }

    // Rest of the code remains the same as in the previous example

    // Clean up shared memory
    munmap(earnings_to_manager, sizeof(int) * total_tables);

    // Display terminating message
    printf("Thank you for visiting the Hotel!\n");

    return 0;
}