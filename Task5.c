#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

enum {OLD = 0, NEW = 1};

char (*setconfig(FILE *config_file, int *process_amount))[2] {
	char (*config)[2];
	int scanf_result, curr_char, curr_process;
	char used_chars[256] = {0};
	enum {
		WAITING_LETTER_OLD,
		WAITING_LETTER_NEW,
		WAITING_SPACE_BETWEEN_LETTERS,
		WAITING_END_OF_LINE,
		WAITING_END_OF_FILE
	} state;


	scanf_result = fscanf(config_file, "%d", process_amount);
	if (scanf_result != 1) {
		fprintf(stderr, "Config file must contain the amount of processes\n");
		return NULL;	
	}
	if (*process_amount <= 0) {
		fprintf(stderr, "The amount of processes must be a positive number\n");
		return NULL;	
	}
	if (*process_amount > 256) {
		fprintf(stderr, "The amount of processes can't exceed 256\n");
		return NULL;	
	}
	do {
		curr_char = fgetc(config_file);
		if (curr_char == EOF) {
			fprintf(stderr, "Config file must contain %d config lines\n", *process_amount);
			return NULL;
		} else if (curr_char != ' ' && curr_char != '\n' && curr_char != '\t') {
			fprintf(stderr, "First line of the config file must contain only the amount of processes\n");
			return NULL;	
		}
	} while (curr_char != '\n');

	config = (char (*)[2]) malloc(*process_amount * 2 * sizeof(char));
	if (config == NULL)
	{
		fprintf(stderr, "Can't allocate memory for config array\n");
		return NULL;	
	}
	
	state = WAITING_LETTER_OLD;
	curr_process = 0;
	while (1) {
		curr_char = fgetc(config_file);
		switch (state) {
		case WAITING_LETTER_OLD:
			switch (curr_char) {
			case ' ':
			case '\n':
			case '\t':
				break;
			case EOF:
				fprintf(stderr, "Not enough config lines. Expected %d, got only %d\n", *process_amount, curr_process);
				free(config);
				return NULL;
			case '.':
			case ',':
			case '*':
				fprintf(stderr, "Letter_old can't be a separator or *\n");
				free(config);
				return NULL;
			default:
				if (used_chars[curr_char]) {
					fprintf(stderr, "Each letter_old must be unique\n");
					free(config);
					return NULL;
				} else {
					config[curr_process][OLD] = curr_char;
					used_chars[curr_char] = 1;
					state = WAITING_SPACE_BETWEEN_LETTERS;
				}
			}
			break;
		case WAITING_SPACE_BETWEEN_LETTERS:
			switch (curr_char) {
			case ' ':
			case '\t':
				state = WAITING_LETTER_NEW;
				break;
			default:
				fprintf(stderr, "Expected at least one space between letter_old and letter_new\n");
				free(config);
				return NULL;
			}
			break;
		case WAITING_LETTER_NEW:
			switch (curr_char) {
			case ' ':
			case '\t':
				break;
			case '\n':
			case EOF:
				fprintf(stderr, "Config lines must contain letter_new\n");
				free(config);
				return NULL;
			case '.':
			case ',':
				fprintf(stderr, "Letter_new can't be a separator\n");
				free(config);
				return NULL;
			default:
				config[curr_process][NEW] = curr_char;
				curr_process += 1;
				state = curr_process < *process_amount ? WAITING_END_OF_LINE : WAITING_END_OF_FILE;
			}
			break;
		case WAITING_END_OF_LINE:
			switch (curr_char) {
			case ' ':
			case '\t':
				break;
			case '\n':
				state = WAITING_LETTER_OLD;
				break;
			case EOF:
				fprintf(stderr, "Not enough config lines. Expected %d, got only %d\n", *process_amount, curr_process);
				free(config);
				return NULL;
			default:
				fprintf(stderr, "Config lines must contain only letter_old and letter_new\n");
				free(config);
				return NULL;
			}
			break;
		case WAITING_END_OF_FILE:
			switch (curr_char) {
			case ' ':
			case '\t':
			case '\n':
				break;
			case EOF:
				return config;
			default:
				fprintf(stderr, "Config file must contain only %d config lines\n", *process_amount);
				free(config);
				return NULL;
			}
			break;
		}
	}
	
}


int main(int argc, char **argv) {
	FILE *config_file;
	char (*config)[2];
	int fd, process_amount, i, process_number, label_size, word_size, read_result;
	char label[7];
	char buf[2] = {'*'};
	char first_char, curr_char;
	pid_t pid;

	if (argc < 3) {
		fprintf(stderr, "Not enough comannd line arguments. Expected text file name and config file name\n");
		return 1;
	}

	config_file = fopen(argv[2], "r");
	if (config_file == NULL) {
		fprintf(stderr, "Can't open config file %s\n", argv[2]);
		return 1;
	}

	config = setconfig(config_file, &process_amount);
	fclose(config_file);
	if (config == NULL) {
		return 1;
	}
	
	process_number = 0;
	for (i = 1; i < process_amount; i++) {
		pid = fork();
		if (pid == -1) {
			fprintf(stderr, "Can't execute son process %d\n", i);
		} else if (pid == 0) {
			process_number = i;
			sprintf(label, "SON%d", process_number);
			if (i > 99) {
				label_size = 6;
			} else if (i < 10) {
				label_size = 4;
			} else {
				label_size = 5;
			}
			break;
		}
	}
	if (process_number == 0) {
		sprintf(label, "FATHER");
		label_size = 6;
	}

	fd = open(argv[1], O_RDWR);
	if (fd == -1) {
		fprintf(stderr, "Process %d can't open text file %s\n", process_number, argv[1]);
		free(config);
		if (process_number == 0) {
			for (i = 1; i < process_amount; i++) {
				wait(NULL);
			}
		}
		return 1;
	}
	
	word_size = 0;
	while (1) {
		read_result = read(fd, &curr_char, sizeof(char));
		if (read_result != sizeof(char) || curr_char == ' ' || curr_char == '.' || curr_char == ',' || curr_char == '\n' || curr_char == '\t') {
			if (word_size >= 10 && first_char == config[process_number][OLD]) {
				if (read_result != sizeof(char)) {
					word_size -= 1;
				}
				if (lseek(fd, - (long) word_size - 1, SEEK_CUR) == -1) {
					fprintf(stderr, "Error in lseek() in process %d\n", process_number);
					free(config);
					return 1;
				}
				buf[1] = config[process_number][NEW];
				if (write(fd, buf, sizeof(char) * 2) != sizeof(char) * 2) {
					fprintf(stderr, "Error in write() in process %d\n", process_number);
					free(config);
					return 1;
				}
				if (lseek(fd, (long) word_size - 2 - label_size, SEEK_CUR) == -1) {
					fprintf(stderr, "Error in lseek() in process %d\n", process_number);
					free(config);
					return 1;
				}
				if (write(fd, label, sizeof(char) * label_size) != sizeof(char) * label_size) {
					fprintf(stderr, "Error in write() in process %d\n", process_number);
					free(config);
					return 1;
				}
			}
			word_size = 0;
			if (read_result != sizeof(char)) {
				break;
			}
		} else {
			if (word_size == 0) {
				first_char = curr_char;
			}
			word_size += 1;
		}
	}
	free(config);

	if (read_result == -1) {
		fprintf(stderr, "Process %d doesn't have access to file %s\n", process_number, argv[1]);
		if (process_number == 0) {
			for (i = 1; i < process_amount; i++) {
				wait(NULL);
			}
		}
		return 1;
	}

	if (process_number == 0) {
		for (i = 1; i < process_amount; i++) {
			wait(NULL);
		}
	}

	return 0;
}
