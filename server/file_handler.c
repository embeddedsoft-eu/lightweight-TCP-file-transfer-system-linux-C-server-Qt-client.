/**
 * File handling module for TCP file server
 */

#include "file_handler.h"
#include <string.h>
#include <stdio.h>

#define MAX_FILENAME_LEN 256
#define MAX_FILEPATH_LEN 512
#define STORAGE_PATH "/var/www/embeddedsoft/test/"

/**
 * Extract filename from received data
 */
static int extract_filename(const unsigned char *data, int data_len,
                           char *filename, int max_len) {
    int i;
    
    if (!data || !filename || data_len <= 0 || max_len <= 0) {
        return -1;
    }
    
    for (i = 0; i < data_len && i < max_len - 1; i++) {
        if (data[i] == '\n' || data[i] == '\r') {
            filename[i] = '\0';
            return i + 1;
        }
        filename[i] = data[i];
    }
    
    filename[i] = '\0';
    return i;
}

/**
 * Validate filename for security
 */
static int is_filename_safe(const char *filename) {
    if (strstr(filename, "..") != NULL) return 0;
    if (filename[0] == '/') return 0;
    if (strchr(filename, '/') != NULL || strchr(filename, '\\') != NULL) return 0;
    if (strlen(filename) >= MAX_FILENAME_LEN) return 0;
    return 1;
}

void save_to_file(const unsigned char *data, int data_len) {
    char filename[MAX_FILENAME_LEN] = {0};
    char filepath[MAX_FILEPATH_LEN] = {0};
    FILE *output_file = NULL;
    int content_offset;

 	printf("[DEBUG] save_to_file called with %d bytes\n", data_len);
       
    // Extract filename
    content_offset = extract_filename(data, data_len, filename, sizeof(filename));
    if (content_offset < 0) {
        printf("[ERROR] Failed to extract filename\n");
        return;
    }
    
    // Show what we received
    printf("[FILE] Filename: '%s'\n", filename);
    printf("[FILE] Total data: %d bytes (filename + content)\n", data_len);
    printf("[FILE] Content offset: %d bytes\n", content_offset);
    
    // Validate
    if (!is_filename_safe(filename)) {
        printf("[SECURITY] Unsafe filename rejected: '%s'\n", filename);
        return;
    }
    
    // Create full path
    snprintf(filepath, sizeof(filepath), "%s%s", STORAGE_PATH, filename);
    
    // Save file
    output_file = fopen(filepath, "wb");
    if (!output_file) {
        printf("[ERROR] Cannot create file: %s\n", filepath);
        return;
    }
    
    int content_length = data_len - content_offset;
    printf("[FILE] Content length: %d bytes\n", content_length);
    
    if (content_length > 0) {
        size_t written = fwrite(data + content_offset, 1, content_length, output_file);
        if (written == content_length) {
            printf("[SAVED] %s (%d bytes)\n", filename, content_length);
        } else {
            printf("[ERROR] Partial write: %zu of %d bytes\n", written, content_length);
        }
    } else {
        printf("[WARNING] Empty file\n");
    }
    
    fclose(output_file);
    
    // Verify file was created
    FILE *check = fopen(filepath, "rb");
    if (check) {
        fseek(check, 0, SEEK_END);
        long file_size = ftell(check);
        fclose(check);
        printf("[VERIFY] File on disk: %ld bytes\n", file_size);
    }
}