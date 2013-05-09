#pragma once

class  Format {
public:
	typedef enum {
		LOAD_ALL,    // Load entire contents into memory, closing reference file.
		LOAD_SPARSE, // Load references, and keep on-disk file open.
	} load_flags_t;

	Format();
	virtual ~Format();

	virtual int load(const char *filename, load_flags_t flags);
	virtual int save(const char *filename);
};
