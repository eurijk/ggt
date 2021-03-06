#include "format_twobit.h"

// Fixme: Here for debugging
#define MAX_ALLOC 524288

enum {
	ACTION_DUMP,
	ACTION_INFO,
	ACTION_SHOW
};

int FormatTwoBit::dump(FILE *fp) {
	return helper(fp, ACTION_DUMP, 0);
}

int FormatTwoBit::info(FILE *fp, int verbose) {
	return helper(fp, ACTION_INFO, verbose);
}

int FormatTwoBit::show(FILE *fp) {
	return helper(fp, ACTION_SHOW, 0);
}

int FormatTwoBit::helper(FILE *fp, int action, int verbose) {
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

		// Todo: Verify the blocks are in order
		// Related, how are these used?

		n = fread(&index[i].maskBlockCount,sizeof(uint32_t),1,fp);
		if (verbose >= 2) {
			printf(
				"Seq #%3d: %6d kbase, blocks %3d, masks %6d, \"%s\" \n",
				i,
				index[i].dnaSize / 1000,
				index[i].nBlockCount, index[i].maskBlockCount,
				index[i].seq_hdr.name
			);
		}

		if (verbose >= 3) {
			for (unsigned o = 0; o < index[i].nBlockCount; o++) {
				printf("   Block #%d: start: %d, size: %d\n",
					o, index[i].nBlockStarts[o], index[i].nBlockSizes[o]
				);
			}
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

			// Todo: Walk the list of masks to ensure they are saved in alphabetical order
		}

		// Reserved
		n = fread(&index[i].reserved,sizeof(uint32_t),1,fp);
		
		/** Fixme: In some cases, we probably actually want to load this data. */
		index[i].packedDna_offset = ftell(fp);
		if (action == ACTION_INFO)
			fseek(fp,(index[i].dnaSize+3)/4,SEEK_CUR);
		else {
			unsigned cur_block_i = 0;      // Note: Requires the block list to be in order
			unsigned cur_mask_i = 0;       // Note: Requires the mask list to be in order
			unsigned cur_bp_pos = 0;       // Current base pair position.
			unsigned cur_print_width = 50; // Number of base pairs to print per line (for future show command)
			char buf[8192];
			int bpc = index[i].dnaSize;
			int len = (index[i].dnaSize+3)/4;
			while (len > 0) {
				int toread = len > sizeof(buf) ? sizeof(buf) : len;
				int available = index[i].dnaSize;
				// printf("ToRead :%d\n", toread);
				n = fread(&buf[0], 1, toread, fp);
				if (toread != n) {
					printf("Error: Unable to read DNA data\n");
					return -1;
				}
				for (unsigned print_pos = 0; print_pos < toread; print_pos++) {
					const char c[4] = { 'T', 'C', 'A', 'G' };
					char ch = buf[print_pos];
					for (unsigned o = 0; o < 4; o++) {
						if (available) {
							if (action == ACTION_SHOW)
								if (cur_bp_pos % cur_print_width == 0)
									printf("\n%9d: ", cur_bp_pos);
							// Fixme: Check blocks and masks as appropriate. And don't go over the end
						test_again:
							if (cur_block_i < index[i].nBlockCount) {
								if (cur_bp_pos >= index[i].nBlockStarts[cur_block_i]) {
									if (cur_bp_pos > index[i].nBlockStarts[cur_block_i] + index[i].nBlockSizes[cur_block_i]) {
										cur_block_i++;
										goto test_again;
									}
									printf("N");
								} else {
									printf("%c", c[(ch >> 6) & 3]);
								}
							} else {
								printf("%c", c[(ch >> 6) & 3]);
							}
							ch <<= 2;
							available--;
							cur_bp_pos++;
						}
					}
				}
				len -= n;
			}
		}
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
