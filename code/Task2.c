#include <stdio.h>

#define mask_d 0xf800
#define shift_d 11

#define mask_m 0x0780
#define shift_m 7

#define mask_y 0x007f
#define shift_y 0


int clearInput() {
	char temp, scanf_result;
	while ((scanf_result = scanf("%c", &temp)) != 1 || temp != '\n') {
		if (scanf_result == EOF) {
			return 1;
		}
	}
	return 0;
}

int check(unsigned short day, unsigned short month, unsigned short year) {
	enum months {jan, feb, mar, apr, may, jun, jul, aug, sep, oct, nov, dec};

	if (day < 1 || day > 31 || month < jan || month > dec || year < 0 || year > 99) {
		return 0;
	}
	switch (month) {
		case feb:
			if (day > 29 || (day == 29 && year % 4 != 0)) {
				return 0;
			}
		case apr:
		case jun:
		case sep:
		case nov:
			if (day > 30) {
				return 0;
			}
		default:
			return 1;
	}
}

unsigned short getDate() {
	unsigned short day, month, year;
	if (scanf("%hu %hu %hu", &day, &month, &year) == 3 && check(day, month, year)) {
		return (day << shift_d) | (month << shift_m) | (year << shift_y);
	}
	return 0;
}

unsigned short getDay(unsigned short date) {
	return (date & mask_d) >> shift_d;
}

unsigned short getMonth(unsigned short date) {
	return (date & mask_m) >> shift_m;
}

unsigned short getYear(unsigned short date) {
	return (date & mask_y) >> shift_y;
}

unsigned short setDay(unsigned short date, unsigned short day) {
	return (date & ~mask_d) + (day << shift_d);
}

unsigned short setMonth(unsigned short date, unsigned short month) {
	return (date & ~mask_m) + (month << shift_m);
}

unsigned short setYear(unsigned short date, unsigned short year) {
	return (date & ~mask_y) + (year << shift_y);
}

void printDay(unsigned short date) {
	unsigned short day = getDay(date);
	if (day <= 9) {
		printf("0");
	}
	printf("%hd\n", day);
	return;
}

void printMonth(unsigned short date) {
	unsigned short month = getMonth(date);
	if (month <= 9) {
		printf("0");
	}
	printf("%hd\n", month);
	return;
}

void printYear(unsigned short date) {
	unsigned short year = getYear(date);
	if (year <= 49) {
		printf("20");
		if (year <= 9) {
			printf("0");
		}
	}
	else {
		printf("19");
	}
	printf("%hd\n", year);
	return;
}

void printDate(unsigned short date) {
	unsigned short day = getDay(date), month = getMonth(date), year = getYear(date);	
	if (day <= 9) {
		printf("0");
	}
	printf("%hd ", day);
	if (month <= 9) {
		printf("0");
	}
	printf("%hd ", month);
	if (year <= 9) {
		printf("0");
	}
	printf("%hd\n", year);
	return;
}

void printBinary(unsigned short n) {
	unsigned short mask;
	for (mask = 1 << (sizeof(short)*8 - 1); mask > 0; mask >>= 1) {
		if (n & mask) {
			printf("1");
		}
		else {
			printf("0");
		}
	}
	printf("\n");
	return;
}

void printMenu() {
	printf("Enter '0' to print current date;\nEnter '1' to print current day;\nEnter '2' to print current month;\nEnter '3' to print current year;\nEnter '4' to change current day;\nEnter '5' to change current month;\nEnter '6' to change current year;\nEnter '7' to print curret date in binary form;\nEnter any other symbol to exit.\n");
	return;
}

int main() {
	unsigned short date, input;
	char action = '0', next = 'a';
	int scanf_result, check_result;

	printf("Enter current date:\n");
	while (!(date = getDate()) || scanf("%c", &next) != 1 || next != '\n') {
		while (next == ' ' || next == '\t') {
			if (scanf("%c", &next) == EOF) {
				printf("Program reached end of file\n");
				return 0;
			}
		}
		if (next == '\n' && date) {
			break;
		}
		if (next != '\n') {
			if (clearInput()) {
				printf("Program reached end of file\n");
				return 0;
			}
		}
		printf("Enter correct date:\n");
	}

	printMenu();

	while (action  >= '0' && action <= '8') {
		printf("\nChoose action (or enter '8' to show menu):\n");
		if (scanf("%c%c", &action, &next) != 2) {
			return 1;
		}
		if (next != '\n') {
			return 0;
		}
		switch (action) {
			case '0':
				printf("Current date: ");
				printDate(date);
				break;	
			case '1':
				printf("Current day: ");
				printDay(date);
				break;
			case '2':
				printf("Current month: ");
				printMonth(date);
				break;
			case '3':
				printf("Current year: ");
				printYear(date);
				break;
			case '4':
				printf("Enter new day: \n");
				next = 'a';
				while (scanf("%hu%c", &input, &next) != 2 || !(check_result = check(input, getMonth(date), getYear(date))) || next != '\n') {
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
					if (next == '\n' && check_result) {
						break;
					}
					if (next != '\n') {
						if (clearInput()) {
							printf("Program reached end of file\n");
							return 0;
						}
					}
					next = 'a';
					printf("Enter correct day: \n");
				}
				date = setDay(date, input);
				break;
			case '5':
				printf("Enter new month: \n");
				next = 'a';
				while (scanf("%hu%c", &input, &next) != 2 || !(check_result = check(getDay(date), input, getYear(date))) || next != '\n') {
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
					if (next == '\n' && check_result) {
						break;
					}
					if (next != '\n') {
						if (clearInput()) {
							printf("Program reached end of file\n");
							return 0;
						}
					}
					next = 'a';
					printf("Enter correct month: \n");
				}
				date = setMonth(date, input);
				break;
			case '6':
				printf("Enter new year: \n");
				next = 'a';	
				while ((scanf_result = scanf("%hu%c", &input, &next)) != 2 || !(check_result = check(getDay(date), getMonth(date), input)) || next != '\n') {
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
					if (next == '\n' && check_result) {
						break;
					}
					if (next != '\n') {
						if (clearInput()) {
							printf("Program reached end of file\n");
							return 0;
						}
					}
					next = 'a';
					printf("Enter correct year: \n");
				}
				date = setYear(date, input);
				break;
			case '7':
				printf("Current date in binary: ");
				printBinary(date);
				break;
			case '8':
				printMenu();
				break;
		}
	}

	return 0;
}
