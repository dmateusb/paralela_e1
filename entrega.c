#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

unsigned char *img, *gray_img;
size_t img_size, gray_img_size;
int channels, gray_channels;
int width, height, threads;
char *input_img_name , *output_img_name;
FILE *outputFile;

void *apply_filter(int idThread){

    int threadId = idThread;
    printf("init p: %p \n", (img + ( (img_size / threads) * threadId )) );

    for (unsigned char *p = (img + ( (img_size / threads) * threadId )) ,
        *pg = (gray_img + ( (gray_img_size / threads) * threadId ));

        p != (img + ( (img_size / threads) * ( threadId + 1 )  )); 
        
        p += channels, pg += gray_channels)
    {
        *pg = (uint8_t)((*p + *(p + 1) + *(p + 2)) / 3.0);
        if (channels == 4)
        {
            *(pg + 1) = *(p + 3);
        }
    }
    return 0;
}

unsigned char *load_image(){
    unsigned char *img = stbi_load(input_img_name, &width, &height, &channels, 0);
    if (img == NULL)
    {
        printf("Error in loading the image\n");
        exit(1);
    }
    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);
    return img;
}

void write_image(){
    stbi_write_png(output_img_name, width, height, gray_channels, gray_img, width * gray_channels);
    stbi_image_free(img);
    free(gray_img);
}

void set_global_variables(){
    img_size = width * height * channels;
    gray_channels = channels == 4 ? 2 : 1;
    gray_img_size = width * height * gray_channels;
    gray_img = malloc(gray_img_size);
    if (gray_img == NULL){
        printf("Unable to allocate memory for the gray image.\n");
        exit(1);
    }
}

/*
void create_threads(){
    int i, *retval, threadId[threads];
    pthread_t thread[threads];

    for(i = 0; i < threads; i++){
        threadId[i] = i;
        pthread_create(&thread[i], NULL, apply_filter, &threadId[i]);
    }

    for(i = 0; i < threads; i++){
        pthread_join(thread[i], (void **)&retval);
    }
}*/

int main(int argc, char **argv)
{ 
    input_img_name = argv[1];
    output_img_name =argv[2];
    char *end;
    threads = (int)strtol(argv[3], &end, 10);
    omp_set_num_threads(threads);
    struct timeval tval_before, tval_after, tval_result;
    gettimeofday(&tval_before, NULL);
    img = load_image();
    set_global_variables();
   // create_threads();
      #pragma omp parallel
      {
        int id =  omp_get_thread_num();
        printf("%d",id);
        apply_filter(id);
      }
    write_image();
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);
    printf("Time elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
}
//gcc -std=c17 -pedantic entrega.c -o entrega -fopenmp -lm -D_DEFAULT_SOURCE