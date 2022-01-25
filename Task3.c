#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define MEMORY 10

#define MEM_ERR "Memory error: the program can't allocate memory properly\n"
#define EOF_ERR "\nThe program has reached the end of file\n"

#define MENU "\nEnter '1' to sort substrings in ascending order;\nEnter '2' to sort substrings in descending order;\nEnter '3' and then number N to delete all substrings longer then N symbols;\nEnter '4' to show initial string;\nEnter '5' to show menu.\n"

#define BIGGER 1
#define EQUAL 0
#define SMALLER -1

// Получение строки

char * getString(int * size, int * eof_flag)  {
	char * string;
	int temp_size = MEMORY, c;
	*size = 0;
	*eof_flag = 1;

	if ((string = (char *) malloc(temp_size * sizeof(char))) == NULL) {
		printf(MEM_ERR);
		return NULL;
	}

	while ((c = getchar()) != EOF) {
		if (c == '\n') {
			*eof_flag = 0;
			break;		
		} 
		if (*size == temp_size - 1) {
			temp_size += MEMORY;
			if ((string = (char *) realloc(string, temp_size)) == NULL) {
				printf(MEM_ERR);
				free(string);
				return NULL;
			}
		}
		string[(*size)++] = c;		
	}
	string[(*size)++] = 0;
	
	if ((string = (char *) realloc(string, *size)) == NULL) {
		printf(MEM_ERR);
		free(string);
		return NULL;
	}

	return string;
}

// Получение массива подстрок

void freeArray(int size, char ** array) {
	int i;
	for (i = 0; i < size; i++) {
		free(array[i]);
	}
	free(array);
}

char ** separate(int * string_size, char * string, char separator) {
	int i, array_size = 0, substring_size = 0;
	char ** array;	

	// Инициализация массива строк
	if ((array = (char **) malloc(*string_size * sizeof(char *))) == NULL) {
		printf(MEM_ERR);
		return NULL;
	}

	// Инициализация первой строки
	if ((array[0] = (char *) malloc(*string_size * sizeof(char))) == NULL) {
		printf(MEM_ERR);
		return NULL;
	}


	for (i = 0; string[i] != 0; ++i) {
		if (string[i] != separator) {
			array[array_size][substring_size++] = string[i];
		} else {
			// Завершение текущей строки
			if (substring_size == 0) {
				continue;
			}
			array[array_size][substring_size++] = 0;
			if ((array[array_size] = realloc(array[array_size], substring_size * sizeof(char))) == NULL) {
				printf(MEM_ERR);
				freeArray(*string_size, array);
				return NULL;
			}
			array_size++;

			// Инициализация следующей строки
			if ((array[array_size] = malloc((*string_size - i) * sizeof(char))) == NULL) {
				printf(MEM_ERR);
				return NULL;
			}
			substring_size = 0;
		}
	}
	if (substring_size == 0) {
		free(array[array_size]);
	} else {
		// Завершение текущей строки
		array[array_size][substring_size++] = 0;
		if ((array[array_size] = realloc(array[array_size], substring_size * sizeof(char))) == NULL) {
			printf(MEM_ERR);
			freeArray(*string_size, array);
			return NULL;
		}
		array_size++;
	}
	// Завершение массива
	array[array_size++] = NULL;
	if ((array = realloc(array, array_size * sizeof(char *))) == NULL) {
		printf(MEM_ERR);
		freeArray(*string_size, array);
		return NULL;
	}
	
	*string_size = array_size;
	return array;
}

// Сортировка подстрок

int compare(const char * string1, const char * string2) {
	int i;
	for (i = 0; string1[i] != 0 && string2[i] != 0; i++) {
		if (string1[i] > string2[i]) {
			return BIGGER;
		}
		if (string1[i] < string2[i]) {
			return SMALLER;
		}
	}
	if (string1[i] == 0 && string2[i] == 0) {
		return EQUAL;
	}
	if (string1[i] == 0) {
		return SMALLER;
	} else {
		return BIGGER;
	}
}

void swap(char ** string1, char ** string2) {
	char * temp = *string1;
	*string1 = *string2;
	*string2 = temp;
	return;
}

void sort(char ** array, int direction) {
	int i, j;
	for (i = 0; array[i] != NULL; i++) {
		for (j = i; j > 0; j--) {
			if (compare(array[j-1], array[j]) == direction) {
				swap(&array[j-1], &array[j]);
			} else {
				break;
			}
		}
	}
}

// Удаление подстрок

int getLength(const char * string) {
	int i = 0;
	while (string[i] != 0) {
		i++;
	}
	return i;
}

void delete(char ** array, int N) {
	int i = 0, j;
	while (array[i] != NULL) {
		if (getLength(array[i]) > N) {
			free(array[i]);
			for (j = i; array[j] != NULL; j++) {
				array[j] = array[j+1];
			}
		} else {
			i++;
		}
	}
}

// Умный scanf

int smart_scanf(char * format, ...) {
	va_list args;
	char next;
	int result = 0;
	va_start(args, format);
	if (vscanf(format, args) == EOF) {
		printf(EOF_ERR);
		return EOF;
	}
	while ((next = getchar()) != '\n') {
		if (next == EOF) {
			printf(EOF_ERR);
			return EOF;
		}
		if (next != ' ' && next != '\t') {
			result = 1;
		}
	}
	va_end(args);
	return result;
}

// Программа

int main(int argc, char ** argv) {
	char * string;
	char ** array;
	char separator = 'a', next_symbol = 'a';
	int action, string_size, array_size, max_length, i, scanf_result, eof_flag;
	
	if (argc == 1) {
	// Интерактивный режим
		// Ввод строки
		printf("Enter the string: \n");
		if ((string = getString(&string_size, &eof_flag)) == NULL) {
			return 1;
		}
		if (eof_flag) {
			printf(EOF_ERR);
			free(string);
			return 1;
		}
		while (1) {
			// Получение разделителя
			printf("\nEnter 'q' to exit;\nEnter any other symbol to use as separator:\n");
			while ((scanf_result = smart_scanf("%c", &separator)) != 0) {
				if (scanf_result == EOF) {
					free(string);
					return 1;
				}
				printf("Enter correct separator:\n");
			}
			if (separator == 'q') {
				free(string);
				return 0;
			}
			// Разбиение массива
			array_size = string_size;
			if ((array = separate(&array_size, string, separator)) == NULL) {
				free(string);
				return 1;
			}
			// Выбор режима работы
			printf(MENU);
			printf("\nChoose action:\n");
			while ((scanf_result = smart_scanf("%d", &action)) != 0 || action < 1 || action > 5) {
				if (scanf_result == EOF) {
					freeArray(array_size, array);
					free(string);
					return 1;
				}
				printf("Enter correct action:\n");
			}
			// Обработка массива
			switch (action) {
				case 1:
					sort(array, BIGGER);
					break;
				case 2:
					sort(array, SMALLER);
					break;
				case 3:
					printf("Enter maximal length:\n");
					while ((scanf_result = smart_scanf("%d", &max_length)) != 0) {
						if (scanf_result == EOF) {
							freeArray(array_size, array);
							free(string);
							return 1;
						}
						printf("Enter correct maximal length:\n");
					}
					delete(array, max_length);
					break;
				case 4:
					printf("%s\n", string);
					break;
				case 5:
					printf(MENU);
					break;
			}
			// Печать результата и очистка массива
			if (action != 4 && action != 5) {
				printf("\nResult:\n");
				for (i = 0; array[i] != NULL; i++) {
					printf("%s\n", array[i]);
				}
				if (i == 0) {
					printf("Empty array\n");
				}
			}
			freeArray(array_size, array);			
		}
	} else {
	// Режим командной строки
		// Получение разделителя
		if (sscanf(argv[1], "%c%c", &separator, &next_symbol) != 1) {
			printf("Argument error: incorrect separator\n");
			return 1;
		}
		// Выбор действия
		if (argc < 3 || sscanf(argv[2], "%d%c", &action, &next_symbol) != 1 || action < 1 || action > 3) {
			printf("Argument error: incorrect action\n");
			return 1;
		}
		if (action == 3 && (argc < 4 || sscanf(argv[3], "%d%c", &max_length, &next_symbol) != 1)) {
			printf("Argument error: incorrect maximal length\n");
			return 1;
		}
		// Ввод строки
		printf("Enter the string: \n");
		if ((string = getString(&string_size, &eof_flag)) == NULL) {
			return 1;
		}
		// Разбиение массива
		array_size = string_size;
		if ((array = separate(&array_size, string, separator)) == NULL) {
			free(string);
			return 1;
		}
		// Обработка массива
		switch (action) {
			case 1:
				sort(array, BIGGER);
				break;
			case 2:
				sort(array, SMALLER);
				break;
			case 3:
				delete(array, max_length);
				break;
		}
		// Печать результата 
		printf("\nResult:\n");
		for (i = 0; array[i] != NULL; i++) {
			printf("%s\n", array[i]);
		}
		if (i == 0) {
			printf("Empty array\n");
		}
		freeArray(array_size, array);
		free(string); 
		return 0;
	}
}

