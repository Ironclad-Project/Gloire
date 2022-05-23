#include <bmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct bmp_header {
    uint16_t bf_signature;
    uint32_t bf_size;
    uint32_t reserved;
    uint32_t bf_offset;
    uint32_t bi_size;
    uint32_t bi_width;
    uint32_t bi_height;
    uint16_t bi_planes;
    uint16_t bi_bpp;
    uint32_t bi_compression;
    uint32_t bi_image_size;
    uint32_t bi_xcount;
    uint32_t bi_ycount;
    uint32_t bi_clr_used;
    uint32_t bi_clr_important;
    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
} __attribute__((packed));

uint32_t *open_bmp_to_array(const char *path, size_t *size, size_t *width,
                            size_t *height) {
    // Open the file.
    FILE *bmp = fopen(path, "r");
    if (bmp == NULL) {
        puts("death 1");
        return NULL;
    }

    struct bmp_header header;
    if (fread(&header, sizeof(struct bmp_header), 1, bmp) != 1) {
        return NULL;
    }
    if (memcmp(&header.bf_signature, "BM", 2) != 0) {
        puts("death 2");
        return NULL;
    }

    //  Get file size.
    fseek(bmp, 0, SEEK_END);
    size_t file_size = ftell(bmp);

    uint32_t bf_size;
    if (header.bf_offset + header.bf_size > file_size) {
        bf_size = file_size - header.bf_offset;
    } else {
        bf_size = header.bf_size;
    }

    uint32_t *ret = malloc(header.bf_size);
    *size  = header.bf_size;
    *width = header.bi_width;
    *height = header.bi_height;
    fseek (bmp, header.bf_offset, SEEK_SET);
    if (fread(ret, bf_size, 1, bmp) != 1) {
        puts("death 3");
        return NULL;
    }
    fclose(bmp);
    return ret;
}
