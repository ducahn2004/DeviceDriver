#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

int running = 1;  // Flag to control program loop

// Signal handler for Ctrl + C (SIGINT)
void handle_sigint(int sig) {
    printf("\nProgram is terminated by user\n");
    running = 0;   // Set flag to stop the loop
}

// Function to list files in a directory
void list_files(const char *path) {
    DIR *dir;
    struct dirent *entry;
    struct stat fileStat;
    char fullPath[512];

    dir = opendir(path); //Open the directory
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    printf("\n=== Listing files in directory: %s ===\n", path);

    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".." directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Build full path for each file
        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);

        // Get file info using stat()
        if (stat(fullPath, &fileStat) == -1) {
            perror("stat");
            continue;
        }

        // Print file details
        if (S_ISDIR(fileStat.st_mode))
            printf("[DIR ] %s\n", entry->d_name);
        else
            printf("[FILE] %s (%ld bytes)\n", entry->d_name, fileStat.st_size);
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    const char *folder;

    // Default folder is current directory if not specified
    if (argc > 1)
        folder = argv[1];
    else
        folder = ".";

    // Register signal handler for Ctrl+C
    signal(SIGINT, handle_sigint);

    // Continuously check every 1 minute
    while (running) {
        list_files(folder);
        printf("\nWaiting 1 minute before next check... (Press Ctrl+C to stop)\n");
        sleep(60);  // Sleep for 60 seconds
    }

    printf("Goodbye!\n");
    return 0;
}
