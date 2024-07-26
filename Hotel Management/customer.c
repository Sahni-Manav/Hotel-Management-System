#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_ORDER_LEN 100

int main() {
    int tableNumber, pipeWriteEnd;
    char order[MAX_ORDER_LEN];

    // Read table number and pipe write end from table process
    scanf("%d %d", &tableNumber, &pipeWriteEnd);

    // Close stdin to prevent reading from table process
    fclose(stdin);

    // Redirect stdout to the pipe
    dup2(pipeWriteEnd, STDOUT_FILENO);
    close(pipeWriteEnd);

    // Take orders from the user
    printf("Enter the serial number(s) of the item(s) to order from the menu. Enter -1 when done:\n");
    fgets(order, MAX_ORDER_LEN, stdin); // Read input from user
    printf("%s", order); // Send orders to table process via pipe

    return 0;
}