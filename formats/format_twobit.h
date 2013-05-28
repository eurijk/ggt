#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "format.h"

class FormatTwoBit {
private:
	typedef struct __attribute__((packed)) {
		uint32_t signature;
		uint32_t version;
		uint32_t sequenceCount;
		uint32_t reserved;
	} _2bit_header_t;

	typedef struct {
		uint8_t nameSize;
		char name[256];
		uint32_t offset;
	} _2bit_seq_hdr_t;

	typedef struct __attribute__((packed)) {
		uint32_t dnaSize;          // number of bases of DNA in the sequence
		uint32_t nBlockCount;      // the number of blocks of Ns in the file (representing unknown sequence)
		uint32_t *nBlockStarts;    // an array of length nBlockCount of 32 bit integers indicating the starting position of a block of Ns
		uint32_t *nBlockSizes;     // an array of length nBlockCount of 32 bit integers indicating the length of a block of Ns
		uint32_t maskBlockCount;   // the number of masked (lower-case) blocks
		uint32_t *maskBlockStarts; // an array of length maskBlockCount of 32 bit integers indicating the starting position of a masked block
		uint32_t *maskBlockSizes;  // an array of length maskBlockCount of 32 bit integers indicating the length of a masked block
		uint32_t reserved;         // always zero for now
		long     packedDna_offset; // the DNA packed to two bits per base,
			// represented as so: T - 00, C - 01, A - 10, G - 11.  The first base is
			// in the most significant 2-bit byte; the last base is in the least
			// significant 2 bits.  For example, the sequence TCAG is represented as
			// 00011011.
		_2bit_seq_hdr_t seq_hdr;
	} _2bit_index_t;

public:
	int load();

private:
	static int helper(FILE *fp, int action, int verbose);

public:
	static int dump(FILE *fp);
	static int info(FILE *fp, int verbose);
	static int show(FILE *fp);
};
