/*
 * Copyright (C) 2024 Grenoble INP - ESISAR.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

/* The runtime pre-opens /lfs, which is where the littlefs volume is mounted */
#define CWD "/lfs"
#define FOLDER_PATH CWD "/folder"
#define FILE_PATH FOLDER_PATH "/test.txt"

static const char DATA[] = "Hello, World!";

/* Write DATA to FILE_PATH, truncating whatever was there before. */
static int
write_file(void)
{
    FILE *file = fopen(FILE_PATH, "w");
    if (!file) {
        printf("fopen(w) failed with error %d\n", errno);
        return -1;
    }

    size_t written = fwrite(DATA, 1, strlen(DATA), file);
    printf("wrote %d bytes\n", (int)written);

    /* fclose flushes, so the data is in the file system when it returns */
    if (fclose(file) != 0) {
        printf("fclose failed with error %d\n", errno);
        return -1;
    }

    return written == strlen(DATA) ? 0 : -1;
}

/* Re-open FILE_PATH from scratch and check that DATA comes back. */
static int
read_file(void)
{
    char buffer[32] = { 0 };

    FILE *file = fopen(FILE_PATH, "r");
    if (!file) {
        printf("fopen(r) failed with error %d\n", errno);
        return -1;
    }

    size_t read = fread(buffer, 1, sizeof(buffer) - 1, file);
    fclose(file);

    printf("read %d bytes: %s\n", (int)read, buffer);

    if (strcmp(buffer, DATA) != 0) {
        printf("content mismatch, expected: %s\n", DATA);
        return -1;
    }

    return 0;
}

int
main(int argc, char **argv)
{
    struct stat info;

    printf("Hello WebAssembly Module !\n");

    /* The directory survives a warm reboot, so an existing one is not an error */
    if (mkdir(FOLDER_PATH, 0777) != 0 && errno != EEXIST) {
        printf("mkdir failed with error %d\n", errno);
        return -1;
    }
    printf("directory " FOLDER_PATH " ready\n");

    if (write_file() != 0 || read_file() != 0)
        return -1;

    if (stat(FILE_PATH, &info) != 0) {
        printf("stat failed with error %d\n", errno);
        return -1;
    }
    printf("file size on disk: %d bytes\n", (int)info.st_size);

    if (remove(FILE_PATH) != 0) {
        printf("remove failed with error %d\n", errno);
        return -1;
    }

    if (stat(FILE_PATH, &info) == 0 || errno != ENOENT) {
        printf("file still there after remove\n");
        return -1;
    }
    printf("file removed\n");

    return 0;
}
