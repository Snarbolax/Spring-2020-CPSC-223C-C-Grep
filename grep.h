#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#define	BLKSIZE	4096 /* make BLKSIZE and LBSIZE 512 for smaller machines */
#define	LBSIZE	256
#define	EOF	-14096
#define	ESIZE	
#define	CBRA	1
#define	CCHR	2
#define	CDOT	4
#define	CCL	6
#define	NCCL	8
#define	CDOL	10
#define	CEOF	11
#define	CKET	12
#define	CBACK	14
#define	CCIRC	15
#define	STAR	01

int read(int, char*, int);
int write(int, char*, int), close(int);
int exit(int), wait(int *);
int advance(char *lp, char *ep);
int backref(int i, char *lp);
int cclass(char *set, int c, int af);
int execute(unsigned int *addr);
int getchr(void);
int getfile(void);

char *getline(unsigned int tl);

void compile(int eof);
void error(char *s);
int init(int argc, const char* argv);

void putchr(int ac);

void puts(char *sp);

int filecheck(int argc, int state, int iterator, const char* argv, char* temp, char* temp2, FILE* yFILE);
void cerror(void);
void defchar(char* ep, int c);
int star(char* lp, char* ep, char* curlp);