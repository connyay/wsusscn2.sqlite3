#include <extractor.h>
#include <mspack.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct mem_buf {
  void *data;
  size_t length;
};

struct mem_file {
  char *data;
  size_t length, posn;
};

static void *mem_alloc(struct mspack_system *self, size_t bytes) {
  return malloc(bytes);
}

static void mem_free(void *buffer) { free(buffer); }

static void mem_copy(void *src, void *dest, size_t bytes) {
  memcpy(dest, src, bytes);
}

static void mem_msg(struct mem_file *file, const char *format, ...) {}

static struct mem_file *mem_open(struct mspack_system *self, struct mem_buf *fn,
                                 int mode) {
  struct mem_file *fh;
  if (!fn || !fn->data || !fn->length)
    return NULL;
  if ((fh = (struct mem_file *)mem_alloc(self, sizeof(struct mem_file)))) {
    fh->data = (char *)fn->data;
    fh->length = fn->length;
    fh->posn = (mode == MSPACK_SYS_OPEN_APPEND) ? fn->length : 0;
  }
  return fh;
}

static void mem_close(struct mem_file *fh) {
  if (fh)
    mem_free(fh);
}

static int mem_read(struct mem_file *fh, void *buffer, int bytes) {
  int todo;
  if (!fh || !buffer || bytes < 0)
    return -1;
  todo = fh->length - fh->posn;
  if (todo > bytes)
    todo = bytes;
  if (todo > 0)
    mem_copy(&fh->data[fh->posn], buffer, (size_t)todo);
  fh->posn += todo;
  return todo;
}

static int mem_write(struct mem_file *fh, void *buffer, int bytes) {
  int todo;
  if (!fh || !buffer || bytes < 0)
    return -1;
  todo = fh->length - fh->posn;
  if (todo > bytes)
    todo = bytes;
  if (todo > 0)
    mem_copy(buffer, &fh->data[fh->posn], (size_t)todo);
  fh->posn += todo;
  return todo;
}

static int mem_seek(struct mem_file *fh, off_t offset, int mode) {
  if (!fh)
    return 1;
  switch (mode) {
  case MSPACK_SYS_SEEK_START:
    break;
  case MSPACK_SYS_SEEK_CUR:
    offset += (off_t)fh->posn;
    break;
  case MSPACK_SYS_SEEK_END:
    offset += (off_t)fh->length;
    break;
  default:
    return 1;
  }
  if ((offset < 0) || (offset > (off_t)fh->length))
    return 1;
  fh->posn = (size_t)offset;
  return 0;
}

static off_t mem_tell(struct mem_file *fh) {
  return (fh) ? (off_t)fh->posn : -1;
}

static struct mspack_system mem_system = {
    (struct mspack_file * (*)(struct mspack_system *, const char *, int)) &
        mem_open,
    (void (*)(struct mspack_file *)) & mem_close,
    (int (*)(struct mspack_file *, void *, int)) & mem_read,
    (int (*)(struct mspack_file *, void *, int)) & mem_write,
    (int (*)(struct mspack_file *, off_t, int)) & mem_seek,
    (off_t(*)(struct mspack_file *)) & mem_tell,
    (void (*)(struct mspack_file *, const char *, ...)) & mem_msg,
    &mem_alloc,
    &mem_free,
    &mem_copy,
    NULL};

FileCallback FileFunc;
void SetFileCallback(FileCallback func) { FileFunc = func; }

int extract(char *cab_bytes, size_t data_len) {
  struct mscab_decompressor *cabd;
  struct mscabd_cabinet *cab;
  struct mscabd_file *file;
  struct mem_buf source = {&cab_bytes[0], data_len};
  struct mem_buf output;
  int err;

  MSPACK_SYS_SELFTEST(err);
  if (err)
    return 1;

  if ((cabd = mspack_create_cab_decompressor(&mem_system))) {

    if ((cab = cabd->open(cabd, (char *)&source))) {

      for (file = cab->files; file; file = file->next) {
        output.data = (char *)malloc(file->length);
        output.length = file->length;

        if (output.data && cabd->extract(cabd, file, (char *)&output)) {
          fprintf(stderr, "can't open cabinet (%d)\n", cabd->last_error(cabd));
          mspack_destroy_cab_decompressor(cabd);
          return 1;
        }

        free(output.data);
      }
      cabd->close(cabd, cab);
    } else {
      fprintf(stderr, "can't open cabinet (%d)\n", cabd->last_error(cabd));
      mspack_destroy_cab_decompressor(cabd);
      return 1;
    }
    mspack_destroy_cab_decompressor(cabd);
  } else {
    fprintf(stderr, "can't make decompressor\n");
  }
  return 0;
}