#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#define SHARED_MEM_SIZE 2048  // Size of the shared memory

struct SharedData {
    int close_flag;  // Flag to signal hotel manager process to close
};

int main(void) {
    int shared_memory_id;
    void *mem_ptr = (void *)0;
    struct SharedData *shared_info;
    key_t mem_key;
    char user_input;

    // Use a fixed key for simplicity; in a real application, it might be generated with ftok()
    mem_key = 5678;  

    // Create the shared memory
    shared_memory_id = shmget(mem_key, sizeof(struct SharedData), IPC_CREAT | 0666);
    if (shared_memory_id == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach the shared memory
    mem_ptr = shmat(shared_memory_id, NULL, 0);
    if (mem_ptr == (void *)(-1)) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    printf("Shared memory attached at address %p\n", mem_ptr);

    shared_info = (struct SharedData *)mem_ptr;
    shared_info->close_flag = 0; // Initialize the flag to 0 (keep hotel open)

    // Admin process loop
    while (1) {
        printf("Do you want to close the hotel? Enter Y for Yes and N for No\n");
        user_input = getc(stdin);
        while (getc(stdin) != '\n'); // Clear the input buffer

        if (user_input == 'Y' || user_input == 'y') {
            shared_info->close_flag = 1;  // Set the flag to close the hotel
            break;  // Exit the loop and terminate the admin process
        } else if (user_input == 'N' || user_input == 'n') {
            continue;
        } else {
            printf("Invalid input! Please enter Y for Yes or N for No.\n");
        }

	//sleep before checking again
	sleep(5);
    }

    // Detach the shared memory
    if (shmdt(mem_ptr) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    return 0;
}