#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define MESSAGE_SIZE 1024
#define FATHER -1

volatile sig_atomic_t err_flag = 0;
volatile sig_atomic_t ok_flag = 0;

typedef struct Letter {
	int snd_p_number;
	int rcv_p_number;
	char message[MESSAGE_SIZE + 1];
} Letter;


int digits(int n) {
	int _digits = 0;

	do {
		_digits += 1;
		n /= 10;
	} while (n > 0);

	return _digits;
}

Letter *getLetter(int p_number, int p_amount) {
	FILE *config_file;
	char *config_file_name;
	Letter *letter;
	int scanf_result, c, i;

	p_number += 1;

	config_file_name = (char *) malloc(sizeof(char) * (8 + digits(p_number)));
	if (config_file_name == NULL) {
		fprintf(stderr, "Can't allocate memory to get name of config file %d\n", p_number);
		return NULL;
	}
	sprintf(config_file_name, "config_%d", p_number);

	config_file = fopen(config_file_name, "r");
	free(config_file_name);
	if (config_file == NULL) {
		fprintf(stderr, "Can't open config file %d\n", p_number);
		return NULL;
	}
	
	letter = (Letter *) malloc(sizeof(Letter));
	if (letter == NULL) {
		fprintf(stderr, "Can't allocate memory to get letter from config file %d\n", p_number);
		fclose(config_file);
		return NULL;
	}
	
	letter->snd_p_number = p_number - 1;

	scanf_result = fscanf(config_file, "%d", &letter->rcv_p_number);
	if (scanf_result != 1) {
		fprintf(stderr, "Config file %d must contain the number of process-reciever\n", p_number);
		fclose(config_file);
		free(letter);
		return NULL;
	}
	letter->rcv_p_number -= 1;
	if (letter->rcv_p_number < 0) {
		fprintf(stderr, "The number of process-reciever in config file %d must be positive\n", p_number);
		fclose(config_file);
		free(letter);
		return NULL;
	}
	if (letter->rcv_p_number >= p_amount) {
		fprintf(stderr, "The number of process-reciever in config file %d must be less then amount of processes (%d)\n", p_number, p_amount);
		fclose(config_file);
		free(letter);
		return NULL;
	}
	while (1) {
		c = fgetc(config_file);
		if (c == '\n') {
			break;
		} else if (c != ' ' && c != '\t') {
			fprintf(stderr, "First line of config file %d must contain olny the number of process-reciever\n", p_number);
			fclose(config_file);
			free(letter);
			return NULL;
		}
	}
	
	for (i = 0; i < MESSAGE_SIZE + 1; i++) {
		letter->message[i] = '\0';
	}
	fread(letter->message, sizeof(char), MESSAGE_SIZE, config_file);
	if (ferror(config_file)) {
		fprintf(stderr, "Can't read message from config file %d\n", p_number);
		fclose(config_file);
		free(letter);
		return NULL;
	}

	c = fgetc(config_file);
	if (c != EOF) {
		fprintf(stderr, "Message's length in config file %d exceeds the limit of %d characters\n", p_number, MESSAGE_SIZE);
		fclose(config_file);
		free(letter);
		return NULL;
	}

	fclose(config_file);

	return letter;
}



void raiseOkFlag(int s) {
	signal(SIGUSR1, raiseOkFlag);
	ok_flag = 1;
	return;
}

void raiseErrFlag(int s) {
	signal(SIGUSR2, raiseErrFlag);
	err_flag = 1;
	return;
}

int main(int argc, char *argv[]) {
	int p_amount, i;
	
	pid_t pid;
	int p_number = FATHER;
	Letter *letter = NULL;
	Letter letter_buf;
	
	pid_t *pids;
	Letter *letters;
	int *correct_letter_flags;

	int pipe_d[2];
	
	if (argc < 2) {
		fprintf(stderr, "No command line argument. Expected the amount of processes\n");
		return 1;
	}

	p_amount = atoi(argv[1]) - 1;
	if (p_amount <= 0) {
		fprintf(stderr, "Incorrect command line argument. Expected integer, bigger then 1\n");
		return 1;
	}

	pids = (pid_t *) malloc(sizeof(pid_t) * p_amount);
	if (pids == NULL) {
		fprintf(stderr, "Can't allocate memory to store process ids\n");
		return 1;
	}
	for (i = 0; i < p_amount; i++) {
		pids[i] = 0;
	}

	letters = (Letter *) malloc(sizeof(Letter) * p_amount);
	if (letters == NULL) {
		fprintf(stderr, "Can't allocate memory to store letters\n");
		free(pids);
		return 1;
	}
	
	correct_letter_flags = (int *) malloc(sizeof(int) * p_amount);
	if (correct_letter_flags == NULL) {
		fprintf(stderr, "Can't allocate memory to store correct letter flags\n");
		free(pids);
		free(letters);
		return 1;
	}
	for (i = 0; i < p_amount; i++) {
		correct_letter_flags[i] = 0;
	}

	if (pipe(pipe_d) == -1) {
		fprintf(stderr, "Can't create pipe\n");
		free(pids);
		free(letters);
		free(correct_letter_flags);
		return 1;
	}
	
	signal(SIGUSR1, raiseOkFlag);
	signal(SIGUSR2, raiseErrFlag);

	for (i = 0; i < p_amount; i++) {
		pid = fork();
		if (pid == -1) {
			fprintf(stderr, "Can't execute process %d\n", i+1);
		} else if (pid == 0) {
			p_number = i;
			break;
		} else {
			pids[i] = pid;
		}
	}
	
	// ОТЕЦ
	if (p_number == FATHER) {	
		for (i = 0; i < p_amount; i++) {
			if (pids[i] == 0) {
				continue;
			}
			
			// Отец говорит сыну сгенерировать письмо
			kill(pids[i], SIGUSR1);
			//pause();
			while(!ok_flag && !err_flag);
			if (err_flag) {
				err_flag = 0;
			} else {
				ok_flag = 0;
				correct_letter_flags[i] = 1;
			}
		}
		
		for (i = 0; i < p_amount; i++) {
			if (!correct_letter_flags[i]) {
				continue;
			} 

			// Отец говорит сыну отправить письмо
			kill(pids[i], SIGUSR1);
			//pause();
			while(!ok_flag && !err_flag);

			// Отец читает письмо сына
			if (err_flag) {
				err_flag = 0;
			} else {
				ok_flag = 0;
				if (read(pipe_d[0], &letters[i], sizeof(Letter)) != sizeof(Letter)) {
					fprintf(stderr, "Father process can't recieve message from process %d\n", i+1);
				}
			}

		}
		close(pipe_d[0]);
		
		for (i = 0; i < p_amount; i++) {
			// Отец говорит сыну принять письмо
			if (!correct_letter_flags[i]) {
				continue;
			}
			if (pids[letters[i].rcv_p_number] == 0) {
				fprintf(stderr, "Father process can't send message to non-existing process %d\n", letters[i].rcv_p_number+1);
				continue;
			}
			if (write(pipe_d[1], &letters[i], sizeof(Letter)) == -1) {
				fprintf(stderr, "Father process can't send message to process %d\n", letters[i].rcv_p_number+1);
			} else {	
				kill(pids[letters[i].rcv_p_number], SIGUSR1);
				
				// Отец ждет, пока сын прочитает письмо
				//pause();
				while(!ok_flag && !err_flag);
				ok_flag = 0;
			}
		}
		close(pipe_d[1]);

		for (i = 0; i < p_amount; i++) {
			if (pids[i] == 0) {
				continue;
			}
			// Отец говорит сыну завершиться
			kill(pids[i], SIGUSR2);
			// Отец ждет, пока сын завершится
			wait(NULL);
		}

		free(pids);
		free(letters);
		free(correct_letter_flags);

		return 0;
	} 

	// СЫН
	else {
		// Сын генерирует письмо
		//pause();
		while(!ok_flag && !err_flag);
		ok_flag = 0;
		free(pids);
		free(letters);
		free(correct_letter_flags);
		
		letter = getLetter(p_number, p_amount);
		if (letter == NULL) {
			letter = &letter_buf;
			kill(getppid(), SIGUSR2);
		} else {
			kill(getppid(), SIGUSR1);

			//Сын отправлят письмо отцу
			//pause();
			while(!ok_flag && !err_flag);
			ok_flag = 0;
			if (write(pipe_d[1], letter, sizeof(Letter)) == -1) {
				fprintf(stderr, "Process %d can't send message to father process\n", p_number+1);
				close(pipe_d[1]);
				kill(getppid(), SIGUSR2);
			} else {
				close(pipe_d[1]);
				kill(getppid(), SIGUSR1);
			}
		}

		//pause();
		while(!ok_flag && !err_flag);
		while (1) {
			if (err_flag) {
				// Сын завершается
				err_flag = 0;
				close(pipe_d[0]);
				
				if (letter != &letter_buf) {
					free(letter);
				}	

				return 0;
			} else {
				// Сын читает письмо отца
				ok_flag = 0;
		
				if (read(pipe_d[0], letter, sizeof(Letter)) != sizeof(Letter)) {
					fprintf(stderr, "Process %d can't recieve message from father process\n", p_number+1);
				} else {
					printf("Process %d recieved message from process %d:\n%s\n", p_number+1, letter->snd_p_number+1, letter->message);
				}
				kill(getppid(), SIGUSR1);
				//pause();
				while(!ok_flag && !err_flag);
			}
		}
	}
}
