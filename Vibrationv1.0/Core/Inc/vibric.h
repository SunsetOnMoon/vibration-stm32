/*
 * vibric.h
 *
 *  Created on: Nov 30, 2023
 *      Author: artse
 */

#ifndef INC_VIBRIC_H_
#define INC_VIBRIC_H_

typedef struct VIBRIC_HEADER vibric_header;
typedef enum VIBRIC_STATUS VIBRIC_STATUS;

struct VIBRIC_HEADER {
	int file_sign; // сигнатура файла - "TMB1"
	int channel_cnt; // количество каналов
	int one_sample_size; // размер выборки на один канал
	int spect_lines_cnt; // количество спектральных линий
	int slice_freq; // частота среза
	int freq_resol; // частотное разрешение
	int block_rv_time; // время приёма блока данных
	int total_rv_time; // общее время приёма данных
	int usr_block_rv_cnt; // количество принятых блоков (задано пользователем)
	int data_size; // размер данных
	int sys_block_rv_cnt; // количество принятых блоков (принято системой)
	int rv_max_value; // максимальное значение принятых данных
	int rv_min_value; // минимальное значение принятых данных
};

enum VIBRIC_STATUS {
	VIBRIC_SUCCESS,
	VIBRIC_FAILED
};

VIBRIC_STATUS initialize_vibric_header(vibric_header* header, int data_size);
VIBRIC_STATUS get_vibric_file_header(vibric_header* header, char* str);



#endif /* INC_VIBRIC_H_ */
