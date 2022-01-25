#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#define SIZE 4

// TYPE BUFFER //

typedef struct {
	char *array;
	int size;
	int offset;
} Buffer;

void freeBuf(Buffer *buffer) {
	if (buffer != NULL) {
		if (buffer -> array != NULL) {
			free(buffer -> array);
			buffer -> array = NULL;
		}
		free(buffer);
	}
}

Buffer *initBuf() {
	Buffer *buffer = (Buffer *) malloc(sizeof(Buffer));
	if (buffer == NULL) {
		return NULL;
	}

	buffer -> array = (char *) malloc(SIZE);
	if (buffer -> array == NULL) {
		free(buffer);
		return NULL;
	}

	buffer -> size = SIZE;
	buffer -> offset = 0;
	
	return buffer;
}

int appendBuf(Buffer *buffer, char c) {
	char *realloc_result;
	if (buffer -> offset == buffer -> size) {
		realloc_result = (char *) realloc(buffer -> array, buffer -> size + SIZE);
		if (realloc_result == NULL) {
			freeBuf(buffer);
			buffer = NULL;
			return -1;
		} 
		buffer -> array = realloc_result;
		buffer -> size += SIZE;
	}

	buffer -> array[buffer -> offset] = c;
	buffer -> offset += 1;

	return c;
}

// TYPE PROGRAM //

typedef enum Token Token;
enum Token {
	TOKEN_NULL,
	TOKEN_EXEC,
	TOKEN_RDIR,
	TOKEN_EXIT,
	TOKEN_CD,
	TOKEN_PWD,	
};

typedef struct Instruction Instruction;
struct Instruction {
	Token token;
	Buffer *buffer;
	char **argv;
	char *in_file;
	char *out_file;
	char *err_file;
	int out_append;
	Instruction *next;
};

typedef Instruction* Program;

void freeInst(Instruction *inst) {
	if (inst != NULL) {
		if (inst -> buffer != NULL) {
			freeBuf(inst -> buffer);
			inst -> buffer = NULL;
		}
		if (inst -> argv != NULL) {
			free(inst -> argv);
			inst -> argv = NULL;
		}
		free(inst);
	}
}

void freeProgram(Program program) {
	if (program != NULL) {
		freeProgram(program -> next);
		program -> next = NULL;
		freeInst(program);
	}
}	

Instruction *initInst() {
	Instruction *inst = (Instruction *) malloc(sizeof(Instruction));
	if (inst == NULL) {
		return NULL;
	}

	inst -> token = TOKEN_NULL;
	inst -> buffer = initBuf();
	inst -> argv = NULL;
	inst -> in_file = NULL;
	inst -> out_file = NULL;
	inst -> err_file = NULL;
	inst -> out_append = 0;
	if (inst -> buffer == NULL) {
		freeInst(inst);
		return NULL;
	}
	inst -> next = NULL;
	return inst;
}

Program initProgram() {
	return initInst();
}

int setArgvAndFiles(Instruction *inst, int word_count) {
	int argv_p = 0, new_argv_p = 0, char_p = 0, first_char_p = 0;
	int waiting_in_file = 0, waiting_out_file = 0, waiting_err_file = 0;
	char *array = inst -> buffer -> array;
	char **argv, **realloc_result;
	
	inst -> argv = (char **) malloc(sizeof(char *) * (word_count + 1));
	if (inst -> argv == NULL) {
		return -1;
	}
	argv = inst -> argv;

	// Set argv only //

	while (word_count > 0) {
		while (array[char_p] != '\0') {
			char_p += 1;
		}

		if (first_char_p != char_p) {
			argv[argv_p] = &array[first_char_p];
			argv_p += 1;
		}

		char_p += 1;
		first_char_p = char_p;
		word_count -= 1;

		if (argv_p > 1) {
			if (memcmp(argv[argv_p - 2], ">\0\0>", 5) == 0) {
				argv[argv_p - 2][2] = '>';
				argv[argv_p - 2] = &argv[argv_p - 2][2];
				argv_p -= 1;
			} else if (memcmp(argv[argv_p - 2], "2\0>", 4) == 0) {
				argv[argv_p - 2][1] = '2';
				argv[argv_p - 2] = &argv[argv_p - 2][1];
				argv_p -= 1;
			}
		}


	}
	
	argv[argv_p] = NULL;

	// Set files //
	
	argv_p = 0;
	while (argv[argv_p] != NULL) {
		if (strcmp(argv[argv_p], "<") == 0) {
			if (waiting_in_file || waiting_out_file || waiting_err_file) {
				return -2;
			}
			waiting_in_file = 1;
		} else if (strcmp(argv[argv_p], ">") == 0) {
			if (waiting_in_file || waiting_out_file || waiting_err_file) {
				return -2;
			}
			waiting_out_file = 1;
			inst -> out_append = 0;
		} else if (strcmp(argv[argv_p], ">>") == 0) {
			if (waiting_in_file || waiting_out_file || waiting_err_file) {
				return -2;
			}
			waiting_out_file = 1;
			inst -> out_append = 1;
		} else if (strcmp(argv[argv_p], "2>") == 0) {
			if (waiting_in_file || waiting_out_file || waiting_err_file) {
				return -2;
			}
			waiting_err_file = 1;
		} else {
			if (waiting_in_file) {
				inst -> in_file = argv[argv_p];
				waiting_in_file = 0;
			} else if (waiting_out_file) {
				inst -> out_file = argv[argv_p];
				waiting_out_file = 0;
			} else if (waiting_err_file) {
				inst -> err_file = argv[argv_p];
				waiting_err_file = 0;
			} else {
				argv[new_argv_p] = argv[argv_p];
				new_argv_p += 1;
			}
		}
		argv_p += 1;
	}

	if (waiting_in_file || waiting_out_file || waiting_err_file) {
		return -2;
	}
	
	argv[new_argv_p] = NULL;

	realloc_result = (char **) realloc(inst -> argv, sizeof(char *) * (new_argv_p + 1));
	if (realloc_result == NULL) {
		return -1;
	} 
	inst -> argv = realloc_result;

	return 0;
}

void printArgv(FILE *stream, Instruction *inst) {
	int i;
	for (i = 0; inst -> argv[i] != 0; i++) {
		fprintf(stream, " %s", inst -> argv[i]);
	}
}

// HISTORY //

typedef struct History History;
struct History {
	Program *array;
	int size;
	int offset;	
};

void freeHistory(History *history) {
	if (history != NULL) {
		if (history -> array != NULL) {
			free(history -> array);
			history -> array = NULL;
		}
		free(history);
	}
}

History *initHistory() {
	History *history = (History *) malloc(sizeof(History));
	if (history == NULL) {
		return NULL;
	}

	history -> array = (Program *) malloc(SIZE * sizeof(Program));
	if (history -> array == NULL) {
		free(history);
		return NULL;
	}

	history -> size = SIZE;
	history -> offset = 0;
	
	return history;
}

int appendHistory(History *history, Program program) {
	if (history -> offset == history -> size) {
		history -> array = (Program *) realloc(history -> array, history -> size + SIZE);
		if (history -> array == NULL) {
			freeHistory(history);
			history = NULL;
			return -1;
		}
		history -> size += SIZE;
	}

	history -> array[history -> offset] = program;
	history -> offset += 1;

	return 0;
}

// PIPES //

int _pipes_[2][2];

void initPipes() {
	int i, j;
	for (i = 0; i < 2; i++) {
		for (j = 0; j < 2; j++) {
			_pipes_[i][j] = j;
		}
	}
}

void closePipes() {
	int i, j;
	for (i = 0; i < 2; i++) {
		for (j = 0; j < 2; j++) {
			if (_pipes_[i][j] != j) {
				close(_pipes_[i][j]);
			}
		}
	}
}

void swapPipes(int **pipe1, int **pipe2) {
	int *temp;
       	temp = *pipe1;
	*pipe1 = *pipe2;
	*pipe2 = temp;
}

// SIGNALS //

pid_t _curr_pid_ = -1;
Instruction **_curr_inst_p_ = NULL;

void SIGINThandler(int sig) {
	signal(SIGINT, SIGINThandler);
	if (_curr_pid_ == -1) {
		return;
	} else if (_curr_pid_ == 0) {
		signal(SIGINT, SIG_DFL);
		kill(getpid(), SIGINT);
	} else if (_curr_inst_p_ != NULL) {
		fprintf(stderr, "\n[MyShell]: Process %d ($", _curr_pid_);
		printArgv(stderr, *_curr_inst_p_);
		fprintf(stderr, ") killed\n");
		kill(_curr_pid_, SIGINT);
	}
}

void SIGTSTPhandler(int sig) {
	signal(SIGTSTP, SIGTSTPhandler);
	if (_curr_pid_ == -1) {
		return;
	} else if (_curr_pid_ == 0) {
		signal(SIGTSTP, SIG_DFL);
		kill(getpid(), SIGTSTP);
	} else if (_curr_inst_p_ != NULL) {
		fprintf(stderr, "\n[MyShell]: Process %d ($", _curr_pid_);
		printArgv(stderr, *_curr_inst_p_);
		fprintf(stderr, ") stopped\n");
		kill(_curr_pid_, SIGTSTP);
	}
}
	
// GENERAL FUNCTIONS //

void exitShell(int code, const char *message, Program program) {
	_curr_inst_p_ = NULL;
	if (program != NULL) {
		freeProgram(program);
	}
	closePipes();
	fprintf(stderr, message);
	exit(code);
}

Program getProgram() {
	char curr_c, prev_c = ' ';
	int word_count = 0, set_result;
	Program program;
	Instruction *curr_inst;

	if ((program = initInst()) == NULL) {
		exitShell(1, "\n[MyShell]: Memory allocation error\n", program);
	}

	curr_inst = program;

	while (1) {
		if (read(0, &curr_c, 1) != 1) {
			exitShell(0, "\n[MyShell]: Input stream terminated\n[MyShell]: Exiting MyShell...\n", program);
		}

		switch (curr_c) {
		case '\n':
			if (prev_c != ' ') {
				if (appendBuf(curr_inst -> buffer, '\0') == -1) {
					exitShell(1, "[MyShell]: Memory allocation error\n", program);
				}
				word_count += 1;
			}
			
			set_result = setArgvAndFiles(curr_inst, word_count);
			if (set_result == -1) {
				exitShell(1, "[MyShell]: Memory allocation error\n", program);
			} else if (set_result == -2) {
				fprintf(stderr, "[MyShell]: Stream syntax error\n");
				freeProgram(program);
				return NULL;
			}
			
			if (word_count > 0) {
				if (strcmp(curr_inst -> argv[0], "exit") == 0) {
					curr_inst -> token = TOKEN_EXIT;
				}
				if (strcmp(curr_inst -> argv[0], "cd") == 0) {
					curr_inst -> token = TOKEN_CD;
				}
				if (strcmp(curr_inst -> argv[0], "pwd") == 0) {
					curr_inst -> token = TOKEN_PWD;
				}
			}

			if (curr_inst -> out_file == NULL) {
				curr_inst -> out_append = -1;
			}

			if (curr_inst -> token != TOKEN_NULL) {
				if ((curr_inst -> next = initInst()) == NULL) {
					exitShell(1, "\n[MyShell]: Memory allocation error\n", program);
				}
				curr_inst = curr_inst -> next;
			}

			return program;

		case '|':
			if (prev_c != ' ') {
				if (appendBuf(curr_inst -> buffer, '\0') == -1) {
					exitShell(1, "\n[MyShell]: Memory allocation error\n", program);
				}
				word_count += 1;
			}

			set_result = setArgvAndFiles(curr_inst, word_count);
			if (set_result == -1) {
				exitShell(1, "[MyShell]: Memory allocation error\n", program);
			} else if (set_result == -2) {
				fprintf(stderr, "[MyShell]: Stream syntax error\n");
				while (fgetc(stdin) != '\n');
				freeProgram(program);
				return NULL;
			}
			
			if (word_count > 0) {
				if (strcmp(curr_inst -> argv[0], "exit") == 0) {
					curr_inst -> token = TOKEN_EXIT;
				}
				if (strcmp(curr_inst -> argv[0], "cd") == 0) {
					curr_inst -> token = TOKEN_CD;
				}
				if (strcmp(curr_inst -> argv[0], "pwd") == 0) {
					curr_inst -> token = TOKEN_PWD;
				}
			}

			if (curr_inst -> token != TOKEN_NULL) {
				if ((curr_inst -> next = initInst()) == NULL) {
					exitShell(1, "\n[MyShell]: Memory allocation error\n", program);
				}
				curr_inst = curr_inst -> next;
			}

			curr_inst -> token = TOKEN_RDIR;
			if ((curr_inst -> next = initInst()) == NULL) {
				exitShell(1, "\n[MyShell]: Memory allocation error\n", program);
			}
			curr_inst = curr_inst -> next;
				
			word_count = 0;
			prev_c = ' ';
			break;

		case ' ':
		case '\t':
			if (appendBuf(curr_inst -> buffer, '\0') == -1) {
				exitShell(1, "\n[MyShell]: Memory allocation error\n", program);
			}
			prev_c = ' ';
			word_count += 1;
			break;

		case '>':
		case '<':
			if (appendBuf(curr_inst -> buffer, '\0') == -1) {
				exitShell(1, "\n[MyShell]: Memory allocation error\n", program);
			}
			if (appendBuf(curr_inst -> buffer, curr_c) == -1) {
				exitShell(1, "\n[MyShell]: Memory allocation error\n", program);
			}
			if (appendBuf(curr_inst -> buffer, '\0') == -1) {
				exitShell(1, "\n[MyShell]: Memory allocation error\n", program);
			}
			word_count += 2;
			prev_c = ' ';
			break;

		default:
			if (curr_inst -> token == TOKEN_NULL) {
				curr_inst -> token = TOKEN_EXEC;
			}
			if (appendBuf(curr_inst -> buffer, curr_c) == -1) {
				exitShell(1, "\n[MyShell]: Memory allocation error\n", program);
			}
			prev_c = curr_c;
		}
	}
}

int isValidProgram(Program program) {
	Instruction *curr_inst = program;
	Token prev_token = TOKEN_NULL;

	if (program == NULL) {
		return 0;
	}

	while (1) {
		if ((prev_token == TOKEN_RDIR && curr_inst -> token == TOKEN_RDIR) ||
		    (prev_token == TOKEN_NULL && curr_inst -> token == TOKEN_RDIR) ||
		    (prev_token == TOKEN_RDIR && curr_inst -> token == TOKEN_NULL) ||
		    (prev_token != TOKEN_NULL && curr_inst -> token == TOKEN_EXIT)) {
			fprintf(stderr, "[MyShell]: Conveyor syntax error\n");	
			return 0;
		}
		if (curr_inst -> token == TOKEN_NULL || curr_inst -> token == TOKEN_EXIT) {
			return 1;
		}
		prev_token = curr_inst -> token;
		curr_inst = curr_inst -> next;
	}
}

void executeProgram(Program program) {
	Instruction *curr_inst = program;
	int exit_code, fd;
	int *pipe_in = _pipes_[0], *pipe_out = _pipes_[1];
	char *temp;

	_curr_inst_p_ = &curr_inst;

	if (pipe(pipe_out) == -1) {
		exitShell(1, "[MyShell]: Stream handlig error\n", program);
	}

	while (1) {
		switch (curr_inst -> token) {
		case TOKEN_NULL:
			closePipes();
			return;
		case TOKEN_EXEC:
			_curr_pid_ = fork();
			if (_curr_pid_ == -1) {
				fprintf(stderr, "[MyShell]: System call fork() error\n");
				return;
			} else if (_curr_pid_ == 0) {

				if (curr_inst -> in_file == NULL) {
					if (dup2(pipe_in[0], 0) == -1) {
						exitShell(1, "[MyShell]: Stream handling error\n", program);
					}
				} else {
					fd = open(curr_inst -> in_file, O_RDONLY);
					if (fd == -1) {
						fprintf(stderr, "[MyShell]: File %s not found\n", curr_inst -> in_file);
						exitShell(1, NULL, program);
					} else if (dup2(fd, 0) == -1) {
						close(fd);
						exitShell(1, "[MyShell]: Stream handling error\n", program);
					} else {
						close(fd);
					}
				}

				if (curr_inst -> out_file == NULL && curr_inst -> out_append != -1) {
					if (dup2(pipe_out[1], 1) == -1) {
						exitShell(1, "[MyShell]: Stream handling error\n", program);
					}
				} else if (curr_inst -> out_file != NULL) {
					if (curr_inst -> out_append == 0) {
						fd = open(curr_inst -> out_file, O_WRONLY | O_TRUNC | O_CREAT, 0644);
					} else {
						fd = open(curr_inst -> out_file, O_WRONLY | O_APPEND | O_CREAT, 0644);
					}
					if (fd == -1) {
						exitShell(1, "[MyShell]: File creation error\n", program);
					} else if (dup2(fd, 1) == -1) {
						close(fd);
						exitShell(1, "[MyShell]: Stream handling error\n", program);
					} else {
						close(fd);
					}
				}

				if (curr_inst -> err_file != NULL) {
					fd = open(curr_inst -> err_file, O_WRONLY | O_TRUNC | O_CREAT, 0644);
					if (fd == -1) {
						exitShell(1, "[MyShell]: File creation error\n", program);
					} else if (dup2(fd, 2) == -1) {
						close(fd);
						exitShell(1, "[MyShell]: Stream handling error\n", program);
					} else {
						close(fd);
					}
				}
				

				closePipes();

				execvp(curr_inst -> argv[0], curr_inst -> argv);
				fprintf(stderr, "[MyShell]: Command %s not found\n", curr_inst -> argv[0]);
				exitShell(1, NULL, program);
			} else {
				if (pipe_in[0] != 0) {
					close(pipe_in[0]);
				} 
				if (pipe_in[1] != 1) {
					close(pipe_in[1]);
				}
				waitpid(_curr_pid_, NULL, WUNTRACED);
				_curr_pid_ = -1;
			}
			break;
		case TOKEN_RDIR:
			swapPipes(&pipe_in, &pipe_out);
			if (pipe(pipe_out) == -1) {
				fprintf(stderr, "[MyShell]: Stream handling error\n");
				return;
			}
			break;
		case TOKEN_EXIT:
			if (curr_inst -> argv[1] == NULL) {
				exit_code = 0;
			} else {
				exit_code = atoi(curr_inst -> argv[1]);
				if (exit_code == 0) {
					fprintf(stderr, "[MyShell]: Numeric argument for $ exit required\n");
				}
			}
			exitShell(exit_code, "[MyShell]: Exiting MyShell...\n", program);
		case TOKEN_CD:
			if (curr_inst -> argv[1] == NULL) {
				fprintf(stderr, "[MyShell]: At least one argument for $ cd required\n");
			} else if (chdir(curr_inst -> argv[1]) == -1) {
				fprintf(stderr, "[MyShell]: Directory %s not found\n", curr_inst -> argv[1]);
			}
			closePipes();
			return;
		case TOKEN_PWD:
			temp = getcwd(NULL, 0);
			if (temp == NULL) {
				fprintf(stderr, "[MyShell]: Pwd executing error\n");
			} else {
				fprintf(stdout, "[MyShell]: %s\n", temp);
			}
			free(temp);
			closePipes();
			return;
		}
		curr_inst = curr_inst -> next;
	}
}


// MAIN //

int main() {
	Program program;
	signal(SIGINT, SIGINThandler);
	signal(SIGTSTP, SIGTSTPhandler);
	while(1) {
		initPipes();
		fprintf(stdout, "[MyShell]$ ");
		fflush(stdout);
		program = getProgram();
		if (isValidProgram(program)) {
			executeProgram(program);
		}
		freeProgram(program);
		program = NULL;
	}

}
