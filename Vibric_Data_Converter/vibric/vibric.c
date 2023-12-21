//
// Created by artse on 13.12.2023.
//

#include "vibric.h"

#include <string.h>
#include <math.h>

vibric_title initialize_vibric_title(int channels_count, int one_sample_size, int freq_slice,
                                     int data_size, float max_value, float min_value)
{
    vibric_title title;

    strcpy(title.file_sign, "TMB1");
    title.channels_count = channels_count;
    title.one_sample_size = one_sample_size;
    title.spect_lines_count = one_sample_size / 2;
    title.freq_slice = freq_slice;
    title.freq_resolution = (float)title.freq_slice / (float)title.spect_lines_count;
//    title.freq_resolution = 1 / (float)(title.rx_total_time);
    title.rx_block_time = 1 / title.freq_resolution;
    title.data_size = data_size;
    title.usr_block_count = (int)ceil((float)title.data_size / (float)title.one_sample_size);
    title.rx_total_time = (int)(title.usr_block_count * title.rx_block_time);
    title.sys_block_count = title.usr_block_count;
    title.max_value = max_value;
    title.min_value = min_value;

    return title;
}

void get_vibric_file_header(vibric_title title, char* header)
{
    memcpy(header, &title, 52);
}
