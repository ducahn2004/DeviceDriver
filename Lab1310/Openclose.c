#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    FILE *fp;
    char buffer[1024];

    // Check if the user provided exactly one argument (the filename)
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // Try to open the file in read mode
    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        perror("Cannot open file");
        return 1;
    }

    printf("Contents of file %s:\n", argv[1]);
    // Read and print each line from the file
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
    }

    // Close the file
    fclose(fp);
    return 0;
}