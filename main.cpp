#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef enum {
	FILE_FORMAT_UNKNOWN,
	FILE_FORMAT_2BIT,
} file_format_t;


file_format_t determineFileFormat(FILE *fp) {
	char buf[256];
	int n = fread(buf,1,sizeof(buf),fp);

	if (
		(*(uint32_t*)buf == 0x1a412743) ||
		(*(uint32_t*)buf == 0x4327411a)
	)
		return FILE_FORMAT_2BIT;

	return FILE_FORMAT_UNKNOWN;
}

int usage(void) {
	printf(
		"ggt (options) <filename>\n"
		"Options:\n"
		"	-s (sequence)  Sequence number\n"
	);
	return -1;
}

class Input {
private:
	void _init() { filename = NULL; fd = NULL; seq = -1; }
public:
	Input() { _init(); }

	const char *filename;
	FILE *fd;
	int seq;
} input;

int main(int argc, char **argv) {
	if (argc <= 1)
		return usage();
	
	for (unsigned i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--seq")) {
			if (i + 1 >= argc) return usage();
			input.seq = atol(argv[++i]);
			continue;
		}
		input.filename = argv[i];
		if (++i != argc)
			return usage();
	}

	input.fd = fopen(input.filename, "r");
	if (!input.fd) {
		printf("Error, unable to open '%s'\n", input.filename);
		return -1;
	}
	// file_2bit_info(input.fd);
	fclose(input.fd);
	return 0;
}
