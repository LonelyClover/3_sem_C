#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
struct Node {
	struct Node * prev, * next;
	char * elem;
};

struct List {
	struct Node * first, * last;
};
*/



void process (struct List * pl, const char * str) {
	struct Node * curr_p = pl -> first, * end_p = pl -> last, * temp_p = NULL;
	int end_flag = 0;

	if (pl -> first == NULL || pl -> last == NULL) {
		return;
	}

	while (1) {
		if (curr_p == end_p) {
			end_flag = 1;
		}

		temp_p = curr_p;
		curr_p = curr_p -> next;

		if (strcmp(temp_p -> elem, str) >= 0) {
			if (temp_p -> prev == NULL) {
				pl -> first = temp_p -> next;
			} else {
				(temp_p -> prev) -> next = temp_p -> next;
			}

			if (temp_p -> next == NULL) {
				pl -> last = temp_p -> prev;
			} else {
				(temp_p -> next) -> prev = temp_p -> prev;
			}
			
			if (strcmp(temp_p -> elem, str) == 0) {
				free(temp_p -> elem);
				free(temp_p);
			} else {
				if ((pl -> last) != NULL) {
					(pl -> last) -> next = temp_p;
				}
				temp_p -> prev = pl -> last;
				temp_p -> next = NULL;
				pl -> last = temp_p;
			}
		} 
		
		if (end_flag) {
			if (pl -> first == NULL && pl -> last != NULL) {
				pl -> first = pl -> last;
			}
			return;
		}	
	}
}

/*
int main() {
	struct List * pl;
	struct Node * curr_p = NULL;

	pl = (struct List *) malloc(sizeof(struct List));

	curr_p = pl -> first = (struct Node *) malloc(sizeof(struct Node));
	curr_p -> prev = NULL;
	curr_p -> elem = (char *) malloc(sizeof(char) * 4);
	strcpy(curr_p -> elem, "ccc");

	curr_p -> next = (struct Node *) malloc(sizeof(struct Node));
	(curr_p -> next) -> prev = curr_p;
	curr_p = curr_p -> next;
	curr_p -> elem = (char *) malloc(sizeof(char) * 4);
	strcpy(curr_p -> elem, "bbb");

	curr_p -> next = (struct Node *) malloc(sizeof(struct Node));
	(curr_p -> next) -> prev = curr_p;
	curr_p = curr_p -> next;
	curr_p -> elem = (char *) malloc(sizeof(char) * 4);
	strcpy(curr_p -> elem, "aaa");
	
	curr_p -> next = NULL;
	pl -> last = curr_p;
	
	process(pl, "aab");
	
	if (pl -> first == NULL && pl -> last == NULL) {
		printf("NULL\n");
	}

	curr_p = pl -> first;
	while ((pl -> first) != NULL) {
		pl -> first = curr_p -> next;
		printf("%s\n", curr_p -> elem);
		free(curr_p -> elem);
		free(curr_p);
		curr_p = pl -> first;
	}
	free(pl);

	return 0;
}
*/
