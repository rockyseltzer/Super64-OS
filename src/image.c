#include "image.h"

#include <libdragon.h>

#include "types.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_THREAD_LOCALS
#include "stb_image.h"

sprite_t *loadImage32(u8 *png, int size)
{
    int x, y, n;
    u8 *tbuf;
    u32 *ibuf;
    sprite_t *sbuf;

    tbuf = malloc(size);
    if (!tbuf)
        return 0;
    memcpy(tbuf, png, size);

    ibuf = (u32 *) stbi_load_from_memory(tbuf, size, &x, &y, &n, 4);
    free(tbuf);
    if (!ibuf)
        return 0;

    sbuf = (sprite_t *) malloc(sizeof(sprite_t) + x * y * 4);
    if (!sbuf)
    {
        stbi_image_free(ibuf);
        return 0;
    }

    sbuf->width = x;
    sbuf->height = y;
    sbuf->bitdepth = 4;
    sbuf->format = 0;
    sbuf->hslices = x / 32;
    sbuf->vslices = y / 32;

    u32 *dst = (u32 *) ((u32) sbuf + sizeof(sprite_t));

    for (int j = 0; j < y; j++)
        for (int i = 0; i < x; i++)
            dst[i + j * x] = ibuf[i + j * x];

    data_cache_hit_writeback_invalidate(sbuf->data, sbuf->width * sbuf->height * sbuf->bitdepth);

    stbi_image_free(ibuf);
    return sbuf;
}
