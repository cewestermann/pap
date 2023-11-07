typedef struct {
  size_t size;
  u8* data;
} Buffer;

#define STRING(x) {(sizeof(x) - 1), (u8*)(x)}

static Buffer allocate_buffer(size_t size) {
  Buffer result = {0};
  result.data = malloc(size);

  if (result.data)
    result.size = size;
  else
    fprintf(stderr, "ERROR: Unable to allocate %llu bytes.\n", size);

  return result;
}

bool buffer_in_bounds(Buffer source,  u64 pos) {
  bool result = (pos < source.count);
  return result;
}

static void free_buffer(Buffer* buffer) {
  if (buffer->data) 
    free(buffer->data);
  // Reset to zero such that we get a proper error opposed to Segmentation Fault.
  memset(buffer, 0, sizeof(Buffer));
}
