#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <stdio.h>

/**
 * Save received data to a file
 * @param data Buffer containing filename and file content
 * @param data_len Total length of received data
 * 
 * Data format:
 *   - First line: filename (terminated by newline)
 *   - Remaining data: file content
 */
void save_to_file(const unsigned char *data, int data_len);

#endif // FILE_HANDLER_H