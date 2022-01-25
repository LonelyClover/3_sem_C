#include <stdio.h>
#include <stdlib.h>
#include <limits.h>


int main(int argc, char ** argv) {
	FILE ** files;
	int ** elements;
	int i, fscanf_result, min_element, min_index, eof_flag;
	
	// Создание массива файлов
	files = (FILE **) malloc((argc - 1) * sizeof(FILE *));
	if (files == NULL) {
		return 1;
	}
	
	// Создание массива элементов - указателей на текущую цифру в каждом файле
	elements = (int **) malloc((argc - 1) * sizeof(int*));
	if (elements == NULL) {
		return 1;
	}
	
	// Инициализация массивов
	for (i = 0; i < argc - 1; i++) {
		files[i] = fopen(argv[i + 1], "r");
		if (files[i] == NULL) {
			return 1;
		}

		elements[i] = (int *) malloc(sizeof(int));
		if (elements[i] == NULL) {
			return 1;
		}

		fscanf_result = fscanf(files[i], "%d", elements[i]);
		if (fscanf_result != 1) {
			free(elements[i]);
			elements[i] = NULL;
			fclose(files[i]);
		}
	}

	while (1) {
		// Поиск минимального элементы
		eof_flag = 1;
		min_element = INT_MAX;
		min_index = 0;
		for (i = 0; i < argc - 1; i++) {
			if (elements[i] == NULL) {
				continue;
			}
			
			eof_flag = 0;
			if (*elements[i] < min_element) {
				min_element = *elements[i];
				min_index = i;
			}
		}
		// Выход, если достигнут конец всех файлов
		if (eof_flag) {
			break;
		}
		// Вывод минимального элемента и его замена на следующий
		printf("%d\n", min_element);
		fscanf_result = fscanf(files[min_index], "%d", elements[min_index]);
		if (fscanf_result != 1) {
			free(elements[min_index]);
			elements[min_index] = NULL;
			fclose(files[min_index]);
		}
	} 
}
