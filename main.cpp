#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "formats/format_twobit.h"

int cmdHelp(int argc, char **argv);
int cmdInfo(int argc, char **argv);
int helpHelp();
int helpInfo();

struct sCommands {
	const char *cmd;
	int (*fnCmd)(int argc, char **argv);
	int (*fnHelp)();
} commands[] = {
	{ "help", cmdHelp, helpHelp },
	{ "info", cmdInfo, helpInfo }
};


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

int usage() {
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

int helpHelp() {
	return usage();
}
int cmdHelp(int argc, char **argv) {
	if (argc <= 2)
		return usage();

	const char *cmd = argv[2];

	for (unsigned i = 0; i < sizeof(commands)/sizeof(struct sCommands); i++) {
		if (!strcasecmp(cmd, commands[i].cmd))
			return (*commands[i].fnHelp)();
	}
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

int helpInfo() {
	printf(
		"NAME\n"
		"	git info - Information about a file\n"
		"SYNOPSIS\n"
		"	ggt info [-v | -vv | -v(n) ] <filename>\n"
		"OPTIONS\n"
		"	-v, -v1, -v2, -v(n), -vv, -vvv\n"
		"		Increase the level of information returned about\n"
		"		a file. -v9 is maximum verbosity, although most formats\n"
		"		will have no further details around -v3.\n"
		"		-v is the same as -v2, the default is -v1\n"
	);
	return 0;
}

int cmdInfo(int argc, char **argv) {
	for (int i = 2; i < argc; i++) {
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
	FormatTwoBit::info(input.fp, 1);
	fclose(input.fp);
	
	return 0;
}

int main(int argc, char **argv) {
	if (argc <= 1)
		return usage();
	
	const char *cmd = argv[1];
	// input.cmd = argv[1];

	for (unsigned i = 0; i < sizeof(commands)/sizeof(struct sCommands); i++) {
		if (!strcasecmp(cmd, commands[i].cmd))
			return (*commands[i].fnCmd)(argc, argv);
	}

	return usage();
}
