#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "formats/format_twobit.h"

class Input {
private:
	void _init() { filename = NULL; fp = NULL; seq = -1; verbose = 1; }
public:
	Input() { _init(); }

	const char *cmd;
	const char *filename;
	FILE *fp;
	int seq;
	int verbose;
} input;

int cmdDump(int argc, char **argv); int helpDump();
int cmdHelp(int argc, char **argv); int helpHelp();
int cmdInfo(int argc, char **argv); int helpInfo();
int cmdShow(int argc, char **argv); int helpShow();

struct sCommands {
	const char *cmd;
	int (*fnCmd)(int argc, char **argv);
	int (*fnHelp)();
	const char *help;
} commands[] = {
	{ "dump", cmdDump, helpDump, "Display a dump of base pair information" },
	{ "help", cmdHelp, helpHelp, "Provides additional help about an option" },
	{ "info", cmdInfo, helpInfo, "Information about a file" },
	{ "show", cmdShow, helpShow, "Dump base pair information with offsets" }
};

int usage() {
	printf(
		"Usage:\n"
		"	ggt <command> [<args>]\n"
		"Commands:\n"
	);
	for (unsigned i = 0; i < sizeof(commands)/sizeof(struct sCommands); i++)
		printf("	%-5s   %s\n", commands[i].cmd, commands[i].help);

	printf(
		"\n"
		"See 'ggt help <command>' for more information about a specific command.\n"
	);
	return -1;
}

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

int cmdDump(int argc, char **argv) {
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
	FormatTwoBit::dump(input.fp);
	fclose(input.fp);
	
	return 0;
}
int helpDump() {
	printf(
		"NAME\n"
		"	ggt dump - Display a dump of base pair information\n"
		"SYNOPSIS\n"
		"	ggt dump <filename>\n"
		"OPTIONS\n"
	);
	//	"Options:\n"
	//	"	-s (sequence)  Sequence number\n"
	return 0;
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

int cmdShow(int argc, char **argv) {
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
	FormatTwoBit::show(input.fp);
	fclose(input.fp);
	
	return 0;
}
int helpShow() {
	printf(
		"NAME\n"
		"	ggt show - Dump base pair information with offsets\n"
		"SYNOPSIS\n"
		"	ggt show <filename>\n"
		"OPTIONS\n"
	);
	//	"Options:\n"
	//	"	-s (sequence)  Sequence number\n"
	return 0;
}


int helpInfo() {
	printf(
		"NAME\n"
		"	ggt info - Information about a file\n"
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
		if (!strcmp(argv[i], "-v"))   { input.verbose = 2; continue; }
		if (!strcmp(argv[i], "-vv"))  { input.verbose = 3; continue; }
		if (!strcmp(argv[i], "-vvv")) { input.verbose = 4; continue; }
		if (!strcmp(argv[i], "-v0"))  { input.verbose = 0; continue; }
		if (!strcmp(argv[i], "-v1"))  { input.verbose = 1; continue; }
		if (!strcmp(argv[i], "-v2"))  { input.verbose = 2; continue; }
		if (!strcmp(argv[i], "-v3"))  { input.verbose = 3; continue; }
		if (!strcmp(argv[i], "-v4"))  { input.verbose = 4; continue; }
		if (!strcmp(argv[i], "-v5"))  { input.verbose = 5; continue; }
		if (!strcmp(argv[i], "-v6"))  { input.verbose = 6; continue; }
		if (!strcmp(argv[i], "-v7"))  { input.verbose = 7; continue; }
		if (!strcmp(argv[i], "-v8"))  { input.verbose = 8; continue; }
		if (!strcmp(argv[i], "-v9"))  { input.verbose = 9; continue; }
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
	FormatTwoBit::info(input.fp, input.verbose);
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
