#include <stdio.h>
#include <stdlib.h>

typedef struct _raw_info_s {
    unsigned char pix_width;
    unsigned char bus_width;
    unsigned short img_width;
    unsigned short img_height;
} raw_info_t;

typedef struct _unpack_ctx_s {
    unsigned short* dst_pix_buf;
    unsigned short pix_num_unpacked;
    unsigned short left_bits_of_last_blk;
    unsigned short last_pix_val;
} unpack_ctx_t;

void unpack_blk(
#ifndef ISA_64
unsigned int blk,
#else
unsigned long long blk,
#endif
raw_info_t* info, unpack_ctx_t* ctx)
{
    int tmp = 0;
    int miss_bits_of_last_blk = 0;
#ifndef ISA_64
    int bits_left = 32;
#else
    int bits_left = 64;
#endif
    if (ctx->left_bits_of_last_blk > 0) {
        miss_bits_of_last_blk = info->pix_width - ctx->left_bits_of_last_blk;
        tmp = blk & ((1 << miss_bits_of_last_blk) - 1);
        tmp <<= ctx->left_bits_of_last_blk;
        tmp |= ctx->last_pix_val;
        ctx->dst_pix_buf[ctx->pix_num_unpacked++] = tmp;
        blk >>= miss_bits_of_last_blk;
        bits_left -= miss_bits_of_last_blk;
    }
    while (bits_left > info->pix_width) {
        tmp = blk & ((1 << info->pix_width) - 1);
        ctx->dst_pix_buf[ctx->pix_num_unpacked++] = tmp;
        blk >>= info->pix_width;
        bits_left -= info->pix_width;
    }
    if (bits_left > 0) {
        ctx->left_bits_of_last_blk = bits_left;
        tmp = blk & ((1 << bits_left) - 1);
        ctx->last_pix_val = tmp;
    }
}

int unpack_line(void* src_line_buf, unsigned short* dst_line_buf, raw_info_t* info)
{
    int ret = 0;
#ifndef ISA_64
    unsigned int *p_blk = (unsigned int *)src_line_buf;
#else
    unsigned long long *p_blk = (unsigned long long *)src_line_buf;
#endif
    unpack_ctx_t ctx = {dst_line_buf, 0, 0, 0};
    while (ctx.pix_num_unpacked < info->img_width) {
        unpack_blk(*p_blk, info, &ctx);
        p_blk++;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    FILE* f_in = NULL;
    FILE* f_out = NULL;
    unsigned beat_width = 0;
    unsigned line_stride = 0;
    unsigned line_num = 0;
    void* src_line_buf = NULL;
    unsigned short* dst_line_buf = NULL;
    raw_info_t info = {0};

    if (argc < 7) {
        printf("usage: rawpeck img_width img_height pixel_width bus_width path_of_packed_raw path_of_unpacked_raw");
        return -1;
    }
    info.img_width = atoi(argv[1]);
    info.img_height = atoi(argv[2]);
    info.pix_width = atoi(argv[3]);
    info.bus_width = atoi(argv[4]);
    f_in = fopen(argv[5], "rb");
    f_out = fopen(argv[6], "wb");
    beat_width = info.bus_width / 8;
    line_stride = (info.img_width * info.pix_width + info.bus_width -1) / info.bus_width * beat_width;
    src_line_buf = malloc(line_stride);
    dst_line_buf = (unsigned short* )malloc(info.img_width * 2);
    while (line_num < info.img_height) {
        fread(src_line_buf, beat_width, line_stride/beat_width, f_in);
        unpack_line(src_line_buf, dst_line_buf, &info);
        fwrite(dst_line_buf, 2, info.img_width, f_out);
        line_num++;
    }

    return 0;
}

