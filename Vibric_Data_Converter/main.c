#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "vibric/vibric.h"

#define VIBRIC_TITLE_SIZE 52

int64_t get_file_size(const char*);

int main()
{
    FILE* source_file, *dest_file;
    uint16_t* source_data;
    float* dest_data;
    char source_filename[100], dest_filename[100];

    printf("Enter path to ADC data file:\n");
    gets(source_filename);

    printf("Enter path to destination file:\n");
    gets(dest_filename);

    int64_t file_size = get_file_size(source_filename);
    if (file_size == -1) {
        printf("Something went wrong in getting size of file.\n");
        return 1;
    }
    else
        printf("File size: %" PRId64 " bytes.\n", file_size);

    if ((source_file = fopen(source_filename, "rb")) == NULL) {
        printf("Can't open ADC data file.\n");
        getchar();
        return 1;
    }

    if ((dest_file = fopen(dest_filename, "wb")) == NULL) {
        printf("Can't create vibric file.\n");
        getchar();
        return 1;
    }

    int64_t buffer_size = file_size / 2;
    source_data = (uint16_t*) malloc(buffer_size * sizeof(uint16_t));
    fread(source_data, sizeof(uint16_t ), buffer_size, source_file);
    fclose(source_file);

    int progress_counter = 0, progress = (int)buffer_size / 100;
    // Transforming data from uint16_t to float
    dest_data = (float*) malloc(buffer_size * sizeof(float));
    printf("_________________________________________________\n");
    printf("Converting to float...\n");
    for (int i = 0; i < buffer_size; i++) {
        dest_data[i] = (float) source_data[i];
        // Add progress bar
        if (i % progress == 0)
            printf("Progress: %d%%\n", progress_counter++);
    }


    char file_header[VIBRIC_TITLE_SIZE];
    vibric_title title;
    int channels_count, one_sample_size, freq_slice;
    float max_value, min_value;

    printf("_________________________________________________\n");
    printf("Enter channels count: \n");
    scanf("%d", &channels_count);
    printf("Enter size of one sample:\n");
    scanf("%d", &one_sample_size);
    printf("Enter frequency slice:\n");
    scanf("%d", &freq_slice);
    printf("Enter max value of data:\n");
    scanf("%f", &max_value);
    printf("Enter min value of data:\n");
    scanf("%f", &min_value);

    printf("_________________________________________________\n");
    printf("Initializing vibric header...\n");
    title = initialize_vibric_title(channels_count, one_sample_size, freq_slice, (int)buffer_size, max_value, min_value);
    get_vibric_file_header(title, file_header);
    printf("Vibric header:\nFile signature:%s\nChannels count: %d\nOne sample size: %d\n"
           "Spectrum lines count: %d\nFrequence slice: %d\nFrequence resolution: %5.3f\n"
           "One block receive time: %5.3f\nTotal receive time: %d\nBlock count (by user): %d\n"
           "Data size: %d\nBlock count (by system): %d\nMax value: %5.3f\nMin value: %5.3f\n",
           title.file_sign, title.channels_count, title.one_sample_size, title.spect_lines_count,
           title.freq_slice, title.freq_resolution, title.rx_block_time, title.rx_total_time,
           title.usr_block_count, title.data_size, title.sys_block_count, title.max_value, title.min_value);
    fwrite(file_header, 1, VIBRIC_TITLE_SIZE, dest_file);
    fwrite(dest_data, sizeof(float), buffer_size, dest_file);

    printf("_________________________________________________\n");
    printf("Converting ended.\nCheck %s file in your directory.\n", dest_filename);
    fclose(dest_file);
    getchar();
    getchar();
    return 0;
}

int64_t get_file_size(const char* filename)
{
    int64_t _file_size = 0;
    FILE *fd = fopen(filename, "rb");
    if (fd == NULL)
        _file_size = -1;
    else {
        fseek(fd, 0, SEEK_END);
        _file_size = ftell(fd);
        fclose(fd);
    }
    return _file_size;

}
