/*
 * vibric.c
 *
 *  Created on: Nov 30, 2023
 *      Author: artse
 */

#include "vibric.h"

VIBRIC_STATUS initialize_vibric_header(vibric_header* header, int data_size)
{
	/*
	 * Инициализирует шапку файла для Vibric
	 * Параметры: data_size - размер данных (в байтах), который будет занесён в один файл
	 * Возвращаемое значение: Инициализированая шапка файла для Vibric
	*/
	VIBRIC_STATUS status;

	header->file_sign = 826428756;
	header->channel_cnt = 1;
	header->one_sample_size = 2048;
	header->spect_lines_cnt = 1024;
	header->slice_freq = 1024;
	header->freq_resol = 1065353216;
	header->block_rv_time = 1065353216;
	header->total_rv_time = 4;
	header->usr_block_rv_cnt = 4;
	header->data_size = data_size;
	header->sys_block_rv_cnt = 4;
	header->rv_max_value = 4096;
	header->rv_min_value = 0;

	status = VIBRIC_SUCCESS;
	return status;

}

VIBRIC_STATUS get_vibric_file_header(vibric_header* header, char* str)
{
//	memcpy(str, header, sizeof(*header));
	memcpy(str, &header->file_sign, 4);
	memcpy(str + 4, &header->channel_cnt, 4);
	memcpy(str + 8, &header->one_sample_size, 4);
	memcpy(str + 12, &header->spect_lines_cnt, 4);
	memcpy(str + 16, &header->slice_freq, 4);
	memcpy(str + 20, &header->freq_resol, 4);
	memcpy(str + 24, &header->block_rv_time, 4);
	memcpy(str + 28, &header->total_rv_time, 4);
	memcpy(str + 32, &header->usr_block_rv_cnt, 4);
	memcpy(str + 36, &header->data_size, 4);
	memcpy(str + 40, &header->sys_block_rv_cnt, 4);
	memcpy(str + 44, &header->rv_max_value, 4);
	memcpy(str + 48, &header->rv_min_value, 4);

	return VIBRIC_SUCCESS;
}


