#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"
#define THREADS 4

unsigned char *img, *gray_img;
size_t img_size, gray_img_size;
int channels, gray_channels;
int width, height;

void *apply_filter(void *arg){

    int threadId = *(int *)arg;


    for (unsigned char *p = (img + ( (img_size / THREADS) * threadId )) ,
        *pg = (gray_img + ( (gray_img_size / THREADS) * threadId ));

        p != (img + ( (img_size / THREADS) * ( threadId + 1 ) )); 
        
        p += channels, pg += gray_channels)

    {
        *p = (uint8_t)((*p + *(p + 1) + *(p + 2)) / 3.0);
        if (channels == 4)
        {
            *(pg + 1) = *(p + 3);
        }
    }
}

int main(void)
{
    int threadId[THREADS], i, *retval;
    pthread_t thread[THREADS];
    struct timeval tval_before, tval_after, tval_result;
    gettimeofday(&tval_before, NULL);
    img = stbi_load("1.png", &width, &height, &channels, 0);
    if (img == NULL)
    {
        printf("Error in loading the image\n");
        exit(1);
    }
    
    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);
    img_size = width * height * channels;
    int gray_channels = channels == 4 ? 2 : 1;
    gray_img_size = width * height * gray_channels;
    gray_img = malloc(gray_img_size);
    if (gray_img == NULL)
    {
        printf("Unable to allocate memory for the gray image.\n");
        exit(1);
    }

    
    for(i = 0; i < THREADS; i++){
        threadId[i] = i;
        pthread_create(&thread[i], NULL, apply_filter, &threadId[i]);
    }

    for(i = 0; i < THREADS; i++){
        pthread_join(thread[i], (void **)&retval);
    }

    //for(i = 1; i < THREADS; i++){
    //    piTotal[0] = piTotal[0] + piTotal[i];
    //}
    /*
    for (unsigned char *p = img, *pg = gray_img; p != img + img_size; p += channels, pg += gray_channels)
    {
        *pg = (uint8_t)((*p + *(p + 1) + *(p + 2)) / 3.0);
        if (channels == 4)
        {
            *(pg + 1) = *(p + 3);
        }
    }
*/



    stbi_write_png("1_gray.png", width, height, channels, img, width * channels);
    stbi_image_free(img);
    free(gray_img);
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);
    printf("Time elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
}

/*
int loadAndConvertImg(int *width, int *height, int channels){

}*/