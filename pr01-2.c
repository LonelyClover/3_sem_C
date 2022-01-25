#include <stdio.h>

int main() {
	int symbol, sum = 0;
	while ((symbol = getchar()) != EOF) {
		if (symbol >= '0' && symbol <= '9') {
			sum += symbol - '0';
		} else if (symbol >= 'a' && symbol <= 'f') {
			sum += symbol - 'a' + 10;
		} else if (symbol >= 'A' && symbol <= 'F') {
			sum += symbol - 'A' + 10;
		}
	}
	printf("%d\n", sum);
	return 0;
}
