#include <stdio.h>
#include <math.h>

int clearInput() {
	char temp, scanf_result;
	while ((scanf_result = scanf("%c", &temp)) != 1 && temp != '\n') {
		if (scanf_result == EOF) {
			return 1;
		}
	}
	return 0;
}	

int main() {
	double a, b, c, d;
	char action = 'y', next = 'a', scanf_result;
	
	do {
		printf("Enter coefficients of quadratic equation:\n");
		next = 'a';
		while ((scanf_result = scanf("%lf %lf %lf%c", &a, &b, &c, &next)) != 4 || next != '\n') {
			if (scanf_result == EOF) {
				printf("Program reached end of file\n");
				return 0;
			}
			while (next == ' ' || next == '\t') {
				if (scanf("%c", &next) == EOF) {
					printf("Program reached end of file\n");
					return 0;
				}
			}
			if (next == '\n') {
				break;
			}
			if (clearInput()) {
				printf("Program reached end of file\n");
				return 0;
			}	
			printf("Enter correct coefficients:\n");
		}

		if (a != 0) {
			d = b*b - 4*a*c;
			if (d > 0) {
				printf("First root: %f\nSecond root: %f\n", (-b - sqrt(d)) / (2*a), (-b + sqrt(d)) / (2*a));
			}
			else if (d == 0) {
				printf("Root: %f\n", -b / (2*a));
			}
			else {
				printf("There are no roots\n");
			}
		} else if (b != 0) {
			printf("Root: %f\n", -c / b);
		} else if (c != 0) {
			printf("There are no roots\n");
		} else {
			printf("There are infinitely many roots\n");
		}

		printf("If you want to solve another equation, enter 'y'\nIf you want to exit, enter 'n'\n");
		next = 'a';
		while (((scanf_result = scanf("%c%c", &action, &next)) != 2) || (action != 'y' && action != 'n') || next != '\n') {
			if (scanf_result == EOF) {
				printf("Program reached end of file\n");
				return 0;
			}
			while (next == ' ' || next == '\t') {
				next = getchar();
			}
			if (next == '\n' && (action == 'y' || action == 'n')) {
				break;
			}
		        if (next != '\n') {
				if (clearInput()) {
					printf("Program reached end of file\n");
					return 0;
				}
			}
			printf("Enter 'y' or 'n'\n");
		}
	} while (action == 'y');

	return 0;
}
