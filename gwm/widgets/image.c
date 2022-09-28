#include <widgets/image.h>
#include <bmp.h>
#include <stdlib.h>

struct image *init_image(const char *path) {
    size_t size, width, height;
    uint32_t *bmp_arr = open_bmp_to_array (path, &size, &width, &height);
    if (bmp_arr == NULL) {
        return NULL;
    }

    struct image *ret = malloc(sizeof(struct image));
    ret->image_data = bmp_arr;
    ret->image_size = size;
    ret->image_width = width;
    ret->image_height = height;
    return ret;
}

void draw_image(struct image *im, struct framebuffer *fb, int start_x,
    int start_y, int width_x, int width_y, int center)
{
    size_t image_x_start, image_y_start;
    if (center) {
        image_x_start = (width_x / 2) - (im->image_width  / 2);
        image_y_start = (width_y / 2) - (im->image_height / 2);
    } else {
        image_x_start = 0;
        image_y_start = 0;
    }

    size_t image_x_length = im->image_width  > width_x ? width_x : im->image_width;
    size_t image_y_length = im->image_height > width_y ? width_y : im->image_height;

    for (int j = 0; j < image_y_length; j++) {
        for (int i = 0; i < image_x_length; i++) {
            uint32_t color = im->image_data[i + im->image_width * (im->image_height - j - 1)];
            draw_pixel(fb, image_x_start + start_x + i,
               image_y_start + start_y + j, color);
        }
    }
}

void destroy_image(struct image *im) {
    free(im->image_data);
    free(im);
}
