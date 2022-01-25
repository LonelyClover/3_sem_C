#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
	int i;
	if (argc >= 2) {
		for (i = 1; i < argc; i++) {
			int status;
			char *program = (char *) malloc((strlen(argv[i]) + 3) * sizeof(char));
			strcpy(program, argv[i]);
			strcat(program, ".c");
			
			if (i != 1) {
				printf("\n-----\n\n");
			}
			printf("Compiling %s ...\n\n", program);
			if (fork() == 0) {
				execl("/usr/bin/gcc", "gcc", program, "-Wall", "-g", "-o", argv[1], NULL);
				free(program);
				fprintf(stderr, "\nGCC isn't executed.\n");
				continue;
			}
			free(program);
			wait(&status);
			if (status == 0) {
				fprintf(stdout, "\nCompilation finished succesfully.\n");
			} else {
				fprintf(stderr, "\nCompilation aborted with status 0x%x.\n", status);
			}
		}
	} else {
		fprintf(stderr, "No cmd arguments\n");
	}
	return 0;	
}
