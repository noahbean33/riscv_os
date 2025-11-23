#include "string.h"
#include "tar-parser.h"
#include "uart.h"

void *tarfs_lookup(const char *filename, size_t *filesize, int output_flag) {
    uint8_t *ptr = _binary_initramfs_tar_start;

    while (ptr < _binary_initramfs_tar_end) {
        struct tar_header *hdr = (struct tar_header *)ptr;

        if (hdr->name[0] == '\0') break; // End of archive

        if (output_flag) {
            uart_printf("[tarfs] found: %s\n" , hdr->name);
        }

        if (strcmp(hdr->name, filename) == 0) {
            size_t size = 0;
            for (int i = 0; i < 11; ++i) {
                size = (size << 3) + (hdr->size[i] - '0');
            }
            if (filesize) *filesize = size;
            return ptr + TAR_BLOCK_SIZE;
        }

        size_t size = 0;
        for (int i = 0; i < 11; ++i) {
            size = (size << 3) + (hdr->size[i] - '0');
        }
        size_t blocks = (size + TAR_BLOCK_SIZE - 1) / TAR_BLOCK_SIZE;
        ptr += (1 + blocks) * TAR_BLOCK_SIZE;
    }

    return NULL;
}