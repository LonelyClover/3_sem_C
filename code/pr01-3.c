#include <stdio.h>

int main() {
	int a, b, n, i, j;
	scanf("%d%d%d", &a, &b, &n);

	printf("%*c", n + 1, ' ');
	for (j = a; j < b; j++) {
		printf("%*d", n, j);
		if (j == b - 1) {
			printf("%c", '\n');
		} else {
			printf("%c", ' ');
		}
	}

	for (i = a; i < b; i++) {
		printf("%*d%c", n, i, ' ');
		for (j = a; j < b; j++) {
			printf("%*lld", n, (long long int) i * j);
			if (j == b - 1) {
				printf("%c", '\n');
			} else {
				printf("%c", ' ');
			}
		}
	}
	return 0;
}