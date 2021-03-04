typedef enum{
    BRUTE,
    DICT3,
    DICT4,
	AFL
} STAGE;


u8 DICT3B[256][256][256];

u8 DICT4B[256][256][256][256];

typedef struct _line{
	u8 *p;
	u_int size;
} line;

#define LINES_SIZE 1024*1024

line lines[LINES_SIZE];