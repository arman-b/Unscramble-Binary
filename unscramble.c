#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef enum {ADD, SUB, XOR, WRITE} operation_t;
typedef enum {BYTE = 1, WORD = 2, DWORD = 4} oper_size_t;

void *mmap_files(char *infilename, char *outfilename, size_t *out_size) {
  /*
   * Use mmap() to map a file into our memory space, rather than read the whole file into a buffer.
   * For large files this is a performance win, and it avoids dealing with explicitly reading and
   * writing the file.
   *
   * Returns a pointer to the mapped file and updates the size variable on success.  Returns NULL
   * on failure.
   */
  int in_fd, out_fd, success;
  struct stat sbuf;
  void *in_mapped, *out_mapped;

  in_fd = open(infilename, O_RDONLY);
  if (in_fd < 0) {
    perror("Error opening input file");
    return NULL;
  }
  // fstat() fetches statistics about a file.  In this case, we need the size so we can map the
  // whole thing.
  success = fstat(in_fd, &sbuf);
  if (success < 0) {
    perror("Error finding file size");
    return NULL;
  }
  *out_size = sbuf.st_size;
  in_mapped = mmap(0, *out_size, PROT_READ,
                               MAP_SHARED, in_fd, 0);
  if (in_mapped == MAP_FAILED) {
    perror("Error mmapping input file");
    return NULL;
  }

  // Copy input file to output file
  // Start by opening the output file
  out_fd = open(outfilename, O_RDWR|O_CREAT, 0666);
  if (out_fd < 0) {
    perror("Error opening output file");
    return NULL;
  }
  // Resize the output file to be the same size as the input file
  success = ftruncate(out_fd, *out_size);
  if (success != 0) {
    perror("Error resizing output file");
    return NULL;
  }
  // mmap the output file
  out_mapped = mmap(0, *out_size, PROT_READ|PROT_WRITE,
                    MAP_SHARED, out_fd, 0);
  if (out_mapped == MAP_FAILED) {
    perror("Error mmapping output file");
    return NULL;
  }
  // Copy from the input file to the output file
  memcpy(out_mapped, in_mapped, *out_size);

  // Now that mmap has succeeded, we don't need to keep the fds open.
  munmap(in_mapped, *out_size);
  close(in_fd);
  close(out_fd);
  return out_mapped;
}

int main(int argc, char **argv) {
  void *mapped;
  size_t map_size;
  FILE *script;

  if (argc < 4) {
    fprintf(stderr, "Usage: %s <mediafile> <outputfile> <scriptfile>\n", argv[0]);
    exit(1);
  }
  mapped = mmap_files(argv[1], argv[2], &map_size);
  if (! mapped) {
    exit(2);
  }
  script = fopen(argv[3], "r");
  if (! script) {
    perror("Error opening script file");
    exit(3);
  }
  
  char opcode[5];
  char size[5];
  unsigned int offset;
  unsigned int constant;

  while (EOF != fscanf(script, "%s%s%x%x", opcode, size, &offset, &constant)) {
    if (!strcmp(size, "BYTE")) {
      char *currMapped = (char*)(mapped + offset);
      if (!strcmp(opcode, "ADD"))
        *currMapped += constant;
      else if (!strcmp(opcode, "SUB"))
        *currMapped -= constant;
      else if (!strcmp(opcode, "XOR"))
        *currMapped ^= constant;
      else
        *currMapped = constant;
    }
    else if (!strcmp(size, "WORD")) {
      short *currMapped = (short*)(mapped + offset);
      if (!strcmp(opcode, "ADD"))
        *currMapped += constant;
      else if (!strcmp(opcode, "SUB"))
        *currMapped -= constant;
      else if (!strcmp(opcode, "XOR"))
        *currMapped ^= constant;
      else
        *currMapped = constant;
    }
    else {
      int *currMapped = (int*)(mapped + offset);
      if (!strcmp(opcode, "ADD"))
        *currMapped += constant;
      else if (!strcmp(opcode, "SUB"))
        *currMapped -= constant;
      else if (!strcmp(opcode, "XOR"))
        *currMapped ^= constant;
      else
        *currMapped = constant;
    }
  }

  fclose(script);
  munmap(mapped, map_size);
}
