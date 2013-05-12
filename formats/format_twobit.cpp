#include "format_twobit.h"

// Fixme: Here for debugging
#define MAX_ALLOC 524288

int FormatTwoBit::info(FILE *fp, int verbose) {
	char buf[256];

	// Fixme: Read header size only
	int n = fread(buf,1,sizeof(_2bit_header_t),fp);
	
	_2bit_header_t *header = (_2bit_header_t*)buf;
	
	if (
		(header->signature != 0x1a412743) &&
		(header->signature != 0x4327411a)
	) {
		printf("2bit: Error, not a 2bit file\n");
		return -1;
	}


	// Determine endian format.
	int endian_is_native = (*(uint32_t*)buf == 0x1a412743) ? 1 : 0;
	// Fixme, we care about big/little not native or swapped
	// Use htonl

	if (verbose >= 1) {
		printf("File format:    2bit\n");
		printf("Endian:         %s\n", endian_is_native ? "native" : "swapped");
		printf("Version:        %d\n", header->version); // Fixme, endians
		printf("Sequence Count: %d\n", header->sequenceCount); // Fixme, endians
		printf("Reserved:       %d\n", header->reserved); // Fixme, endians
		if (verbose == 1)
			return 0;
	}

	_2bit_index_t *index = (_2bit_index_t*)malloc(sizeof(_2bit_index_t)*header->sequenceCount);
	if (!index) {
		fprintf(stderr, "Error: Not enough memory to allocate %d index tables\n", header->sequenceCount);
		return -1;
	}

	for (unsigned i = 0; i < header->sequenceCount; i++) {
		_2bit_seq_hdr_t *seq_hdr = &index[i].seq_hdr;
		// Fixme: Excessive read calls
		n = fread(&seq_hdr->nameSize,1,1,fp);
		n = fread(&seq_hdr->name,1,seq_hdr->nameSize,fp);
		n = fread(&seq_hdr->offset,1,sizeof(uint32_t),fp);
		seq_hdr->name[seq_hdr->nameSize] = 0;
		// printf("Sequence #%d, \"%s\"\n", i, seq_hdr.name);
	}

	for (unsigned i = 0; i < header->sequenceCount; i++) {
		// Fixme: Excessive read calls
		n = fread(&index[i].dnaSize,    sizeof(uint32_t),1,fp);
		n = fread(&index[i].nBlockCount,sizeof(uint32_t),1,fp);
		if (index[i].nBlockCount) {
			if (index[i].nBlockCount > MAX_ALLOC)
				index[i].nBlockCount = MAX_ALLOC;

			index[i].nBlockStarts = (uint32_t*)malloc(sizeof(uint32_t)*index[i].nBlockCount);
			if (!index[i].nBlockStarts) {
				// Fixme: Convert to class to we can auto free.
				printf("Error, unable to allocate %d bytes.\n", sizeof(uint32_t)*index[i].nBlockCount);
				return -1;
			}

			index[i].nBlockSizes  = (uint32_t*)malloc(sizeof(uint32_t)*index[i].nBlockCount);
			if (!index[i].nBlockSizes) {
				// Fixme: Convert to class to we can auto free.
				printf("Error, unable to allocate %d bytes.\n", sizeof(uint32_t)*index[i].nBlockCount);
				return -1;
			}

			fread(index[i].nBlockStarts, sizeof(uint32_t), index[i].nBlockCount, fp);
			fread(index[i].nBlockSizes,  sizeof(uint32_t), index[i].nBlockCount, fp);
		} else {
			index[i].nBlockStarts =
			index[i].nBlockSizes = 0;
		}
		n = fread(&index[i].maskBlockCount,sizeof(uint32_t),1,fp);
		if (verbose >= 2) {
			printf(
				"Seq #%d:\"%s\", DNA Size %d (%dkb), block count %d, mask %d\n",
				i, index[i].seq_hdr.name,
				index[i].dnaSize, index[i].dnaSize/4096,
				index[i].nBlockCount, index[i].maskBlockCount
			);
		}
		if (!index[i].maskBlockCount) {
			index[i].maskBlockStarts = NULL;
			index[i].maskBlockSizes = NULL;
		} else {
			if (index[i].maskBlockCount > MAX_ALLOC)
				index[i].maskBlockCount = MAX_ALLOC;

			index[i].maskBlockStarts = (uint32_t*)malloc(sizeof(uint32_t)*index[i].maskBlockCount);
			index[i].maskBlockSizes  = (uint32_t*)malloc(sizeof(uint32_t)*index[i].maskBlockCount);

			n = fread(index[i].maskBlockStarts, sizeof(uint32_t), index[i].maskBlockCount, fp);
			if ((unsigned)n != index[i].maskBlockCount) {
				printf("Error: Unable to read mask block starts at index %d, short file?\n", i);
				return -1;
			}

			n = fread(index[i].maskBlockSizes,  sizeof(uint32_t), index[i].maskBlockCount, fp);
			if ((unsigned)n != index[i].maskBlockCount) {
				printf("Error: Unable to read mask block count at index %d, short file?\n", i);
				return -1;
			}

			if (verbose >= 3) {
				for (unsigned o = 0; o < index[i].maskBlockCount; o++) {
					printf("   Mask #%d: start: %d, size: %d\n",
						o, index[i].maskBlockStarts[o], index[i].maskBlockSizes[o]
					);
				}
			}
		}

		// Reserved
		n = fread(&index[i].reserved,sizeof(uint32_t),1,fp);
		
		/** Fixme: In some cases, we probably actually want to load this data. */
		index[i].packedDna_offset = ftell(fp);
		fseek(fp,(index[i].dnaSize+3)/4,SEEK_CUR);
	}

	// EOF Sanity Check
	if (1) {
		char buf[8192];
		n = fread(buf,1,sizeof(buf),fp);
		if (n != 0)
			printf("Warning: Additional data past where the end of file should be.\n");
	}

	return 0;
}
