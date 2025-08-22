#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

// ------------------------------------------------------------
// Signal handler function
// This function is called automatically when the process
// receives a specific signal (SIGHUP or SIGINT).
// ------------------------------------------------------------
void handle_signal(int signal) {
    if (signal == SIGHUP) {
        printf("Ouch!\n");
        fflush(stdout);  
    } else if (signal == SIGINT) {
        printf("Yeah!\n");
        fflush(stdout);  
    }
}

// ------------------------------------------------------------
// Entry function (main function)
// ------------------------------------------------------------
int main(int argc, char *args[]) {
    // Input validation, check if the program received exactly 1 argument
    if (argc != 2) {
        fprintf(stderr, "log: %s <n>\n", args[0]);
        return 1;  // Exit with error code
    }

    // Boundary check, convert the arguments to integer
    int count = atoi(args[1]);
    if (count <= 0) {
        fprintf(stderr, "Enter a positive integer.\n");
        return 1;  // Exit with error code
    }

    // --------------------------------------------------------
    // Register handlers
    // --------------------------------------------------------
    signal(SIGHUP, handle_signal);
    signal(SIGINT, handle_signal);

    // --------------------------------------------------------
    // Main loop: print the first 'count' even numbers
    // Each number is followed by a 5-second sleep
    // --------------------------------------------------------
    for (int i = 0; i < count; i++) {
        printf("%d\n", 2 * i);
        fflush(stdout);  
        sleep(5);
    }

    return 0; //program stop successful
}
