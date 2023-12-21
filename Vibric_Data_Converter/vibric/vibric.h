//
// Created by artse on 13.12.2023.
//

#ifndef VIBRIC_DATA_CONVERTER_VIBRIC_H
#define VIBRIC_DATA_CONVERTER_VIBRIC_H

typedef struct VIBRIC_TITLE vibric_title;

struct VIBRIC_TITLE{
    char file_sign[4];
    int channels_count;
    int one_sample_size;
    int spect_lines_count;
    int freq_slice;
    float freq_resolution;
    float rx_block_time;
    int rx_total_time;
    int usr_block_count;
    int data_size;
    int sys_block_count;
    float max_value;
    float min_value
};

vibric_title initialize_vibric_title(int channels_count, int one_sample_size, int freq_slice,
                                     int data_size, float max_value, float min_value);

void get_vibric_file_header(vibric_title title, char* header);

#endif //VIBRIC_DATA_CONVERTER_VIBRIC_H
