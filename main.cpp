#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "formats/format_twobit.h"

typedef enum {
	FILE_FORMAT_UNKNOWN,
	FILE_FORMAT_2BIT,
} file_format_t;


file_format_t determineFileFormat(FILE *fp) {
	char buf[32];
	memset(buf,0,sizeof(buf));
	fread(buf,1,sizeof(buf),fp);

	if (
		(*(uint32_t*)buf == 0x1a412743) ||
		(*(uint32_t*)buf == 0x4327411a)
	)
		return FILE_FORMAT_2BIT;

	return FILE_FORMAT_UNKNOWN;
}

int usage(void) {
	printf(
		"Usage:\n"
		"	ggt <command> [<args>]\n"
		"Commands:\n"
		"	info   Information about a file (2bit)\n"
		"\n"
		"See 'ggt help <command>' for more information about a specific command.\n"
	);
	//	"Options:\n"
	//	"	-s (sequence)  Sequence number\n"
	return -1;
}

int cmdHelp(int argc, char **argv) {
	return 0;
}

class Input {
private:
	void _init() { filename = NULL; fp = NULL; seq = -1; }
public:
	Input() { _init(); }

	const char *cmd;
	const char *filename;
	FILE *fp;
	int seq;
} input;

int cmdInfo(int argc, char **argv) {
	for (unsigned i = 2; i < argc; i++) {
		if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--seq")) {
			if (i + 1 >= argc) return usage();
			input.seq = atol(argv[++i]);
			continue;
		}
		input.filename = argv[i];
		if (++i != argc)
			return usage();
	}
	input.fp = fopen(input.filename, "r");
	if (!input.fp) {
		printf("Error, unable to open '%s'\n", input.filename);
		return -1;
	}
	FormatTwoBit::info(input.fp);
	fclose(input.fp);
	
	return 0;
}

struct sCommands {
	const char *cmd;
	int (*fn)(int argc, char **argv);
} commands[] = {
	{ "help", cmdHelp },
	{ "info", cmdInfo }
};

int main(int argc, char **argv) {
	if (argc <= 1)
		return usage();
	
	const char *cmd = argv[1];
	// input.cmd = argv[1];

	for (unsigned i = 0; i < sizeof(commands)/sizeof(struct sCommands); i++) {
		if (!strcasecmp(cmd, commands[i].cmd))
			return (*commands[i].fn)(argc, argv);
	}

	return usage();
}
