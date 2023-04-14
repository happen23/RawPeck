#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int ret = 0;
    int img_width, img_height, pix_value, pix_idx = 0;
    short pix_high, pix_low;
    FILE *pfile_in = NULL;
    FILE *pfile_out_h = NULL;
    FILE *pfile_out_l = NULL;
    char *path_in = NULL;
    char *path_out = NULL;
    char *token = NULL;
    char path_out_h[128] = {0};
    char path_out_l[128] = {0};

    if (argc < 5) {
        printf("usage:\n\tsplit24 img_width img_height in_path out_path\n");
        return -1;
    }
    img_width = atoi(argv[1]);
    img_height = atoi(argv[2]);
    path_in = argv[3];
    path_out = argv[4];
    token = strtok(path_out, ".");
    if (!token) {
        printf("invalid out_path %s\n", path_out);
        return -2;
    }
    sprintf(path_out_h, "%s_high.raw", token);
    sprintf(path_out_l, "%s_low.raw", token);
    printf("path_out_h: %s\n", path_out_h);
    printf("path_out_l: %s\n", path_out_l);
    //open input raw file
    pfile_in = fopen(path_in, "rb");
    if (pfile_in == NULL) {
        printf("%s is not exist\n", path_in);
        goto cleanup;
    }
    //open 2 output raw file
    pfile_out_h = fopen(path_out_h, "wb");
    if (pfile_out_h == NULL) {
        printf("failed to open %s\n", path_out_h);
        goto cleanup;
    }
    pfile_out_l = fopen(path_out_l, "wb");
    if (pfile_out_l == NULL) {
        printf("failed to open %s\n", path_out_l);
        goto cleanup;
    }
    while (pix_idx < img_width * img_height) {
        fread(&pix_value, 3, 1, pfile_in);
        pix_low = pix_value & 0xfff;
        pix_value >>= 12;
        pix_high = pix_value & 0xfff;
        fwrite(&pix_high, 2, 1, pfile_out_h);
        fwrite(&pix_low, 2, 1, pfile_out_l);
        pix_idx++;
    }
cleanup:
    if (pfile_out_h) {
        fclose(pfile_out_h);
    }
    if (pfile_out_l) {
        fclose(pfile_out_l);
    }
    if (pfile_in) {
        fclose(pfile_in);
    }
    return 0;
}
