/*	Removes the carriage return. */


#include <stdio.h>
#define TEMPORARY_FILENAME	"__nothing.txt"


int main(int argc, char ** argsv) {
	if (argc != 2) {
		printf("-1\n");
		return -1;
	}

	FILE * original_stream = fopen(argsv[1], "rb");
	if (original_stream == NULL) {
		printf("-2\n");
		return -2;
	}
	
	FILE * temporary_stream = fopen(TEMPORARY_FILENAME, "w+b");
	if (temporary_stream == NULL) {
		fclose(original_stream);
		printf("-3\n");
		return -3;
	}

	char buffer;
	while ((buffer = fgetc(original_stream)) != EOF) {
		if (buffer != 13) {
			if (fputc(buffer, temporary_stream) == EOF)
			{
				fclose(original_stream);
				fclose(temporary_stream);
				printf("-4\n");
				return -4;
			}
			putchar(buffer);
		}
		else {
			printf("<bosta>");
		}
	}
	printf("***\n");

	fclose(original_stream);
	original_stream = fopen(argsv[1], "wb");
	if (original_stream == NULL) {
		fclose(temporary_stream);
		printf("-5\n");
		return -5;
	}


	fseek(temporary_stream, 0, SEEK_SET);
	fseek(original_stream, 0, SEEK_SET);
	while ((buffer = fgetc(temporary_stream)) != EOF) {
		putchar(buffer);
		if (fputc(buffer, original_stream) == EOF) {
			fclose(original_stream);
			fclose(temporary_stream);
			printf("-5\n");
			return -6;
		}
	}
	printf("***\n");

	fclose(temporary_stream);
	fclose(original_stream);
	
	remove(TEMPORARY_FILENAME);

	return 0;
}
