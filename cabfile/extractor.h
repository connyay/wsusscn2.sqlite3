#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <stdlib.h>

typedef void (*FileCallback)(char *filename, void *data, int data_len);
void SetFileCallback(FileCallback func);

int extract(char *cab_bytes, size_t data_len);

#endif /* EXTRACTOR_H */