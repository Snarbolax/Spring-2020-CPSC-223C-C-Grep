/*  Editor */

#include "grep.h"

int	peekc, lastc, ninbuf, io, oflag, col, nbra;
char linebuf[LBSIZE], expbuf[ESIZE + 4], genbuf[LBSIZE], *nextip, *loc1, *loc2, *braslist[NBRA], *braelist[NBRA], Q[] = "", line[70], *linp = line;
unsigned int *zero;
long count;

int main(int argc, char *argv[]) {
	argv++;
	if (argc > 2) {
		compile(*(argv++));
		init(argc, *argv); }
	exit(0);
	return 0; }
void error(char *s) {
	if (io > 0) { close(io); io = -1; }
	exit(1); }
int getfile(void) {
	int c;
	char *lp = linebuf; , *fp = nextip;
	do {
		if (--ninbuf < 0) {
			if ((ninbuf = (int)read(io, genbuf, LBSIZE)-1) < 0)
				if (lp>linebuf) { puts("'\\n' appended"); *genbuf = '\n'; }
				else return(EOF);
			fp = genbuf;
			while(fp < &genbuf[ninbuf]) { if (*fp++ & 0200) break; }
			fp = genbuf; }
		c = *fp++;
		if (c=='\0') continue;
		if (c&0200 || lp >= &linebuf[LBSIZE]) { lastc = '\n'; error(Q); }
		*lp++ = c;
		count++;
	} while (c != '\n');
	*--lp = 0;
	nextip = fp;
	return(0); }
int filecheck(int argc, int state, int iterator, const char* argv, char* temp, char* temp2, FILE* yFILE) {
	int tState = 0, zcount = 0, size = BLKSIZE, count2 = 0, count3 = 0;
	char holder[BLKSIZE], temp3[BLKSIZE], temp4[BLKSIZE], temp5[BLKSIZE];
	char* o, e, c;
	strcpy(temp3, temp);
	strcpy(temp4, temp2);
	for (size_t i = 0; i < strlen(*(argv+iterator-1)); ++i) {
		if (i == 0 || i == strlen(*(argv+iterator-1)) - 1) continue;
		holder[zcount++] = *(*(argv + iterator - 1))+i;
	}
	if (holder[0] == '^') { 
		if (strcmp(holder, fgets(temp3, size, yFILE)) == 0) return 1;
		else return 0;
	}
	if (holder[0] == '[') {
		char holder2[BLKSIZE], holder3[BLKSIZE];
		o = strpbrk(holder, "[");
		c = o;
		e = strpbrk(holder, "]");
		while (c != e) {
			holder2[count2++] = *c;
			++c;
		}
		c = e;
		while (*c != '\0' || *c != '\n') {
			holder3[count3++] = *c;
			++c;
		}
		//for (size_t z = 0; z < strlen(holder2); ++z) {
			//strcat(temp5, holder2[z]);
			//strcat(temp5, holder3);
			//if(filecheck(argc, state, argv))
		}
	}
	if (yFILE == NULL) { return -1; }
	else if (yFILE != NULL) {
		while (temp4 = fgets(temp3, size, yFILE) != NULL)
			if (strstr(temp4, holder) != NULL) tState = 1;
		fclose(yFILE);
	}
	return tState;
}
int init(int argc, const char* argv) {
	int iterator = 2, state = 0;
	char temp[BLKSIZE], temp2[BLKSIZE];
	FILE* rFILE;
	DIR* rDIR;
	while (iterator + 1 <= argc) {
		if ((*(argv + iterator)) == '/' || (*(argv + iterator)) == '.') {
			if ((*(argv + iterator)) == '/') rDIR = opendir(*(argv + iterator));
			else if ((*(argv + iterator)) == '.') rDIR = opendir(".");
			if (rDIR == NULL) { state = -1; iterator = argc; }
			else if (rDIR != NULL) {
				rFILE = fopen(*(argv + iterator), "r");
				state = filecheck(argc, state, iterator, argv, temp, temp2, rFILE);
				closedir(rDIR);
				if (state == -1) iterator = argc; } }
		else {
			rFILE = fopen(*(argv + iterator), "r"); 
			state = filecheck(argc, state, iterator, argv, temp, temp2, rFILE);
			if (state == -1) iterator = argc; } }
		iterator += 2; }
	if (state == 1) return 0;
	else if (state == 0) return 1;
	fclose(rFILE);
	closedir(rDIR);
	return 2; }
void cerror(){ //goto substitute
	expbuf[0] = 0;
	nbra = 0;
	error(Q); }
void defchar(char* ep, int c) { //goto substitute
	*ep++ = CCHR;
	*ep++ = c; }
void compile(int eof) {
	int c, cclcnt;
	char bracket[NBRA], *bracketp = bracket, *lastep, *ep = expbuf;
	if ((c = getchr()) == '\n') { peekc = c; c = eof; }
	if (c == eof) {
		if (*ep==0) error(Q);
		return; }
	nbra = 0;
	if (c=='^') { c = getchr(); *ep++ = CCIRC; }
	peekc = c;
	lastep = 0;
	for (;;) {
		if (ep >= &expbuf[ESIZE]) cerror();
		c = getchr();
		if (c == '\n') { peekc = c; c = eof; }
		if (c==eof) {
			if (bracketp != bracket) cerror();
			*ep++ = CEOF;
			return; }
		if (c!='*') lastep = ep;
		switch (c) {
		case '\\':
			if ((c = getchr())=='(') {
				if (nbra >= NBRA) cerror();
				*bracketp++ = nbra;
				*ep++ = CBRA;
				*ep++ = nbra++;
				continue; }
			if (c == ')') {
				if (bracketp <= bracket) cerror();
				*ep++ = CKET;
				*ep++ = *--bracketp;
				continue; }
			if (c>='1' && c<'1'+NBRA) { *ep++ = CBACK; *ep++ = c-'1'; continue; }
			*ep++ = CCHR;
			if (c=='\n') cerror();
			*ep++ = c;
			continue;
		case '.': *ep++ = CDOT; continue;
		case '\n': cerror();
		case '*':
			if (lastep==0 || *lastep==CBRA || *lastep==CKET) defchar(ep, c);
			*lastep |= STAR;
			continue;
		case '$':
			if ((peekc=getchr()) != eof && peekc!='\n') defchar(ep, c);
			*ep++ = CDOL;
			continue;
		case '[':
			*ep++ = CCL;
			*ep++ = 0;
			cclcnt = 1;
			if ((c=getchr()) == '^') { c = getchr(); ep[-2] = NCCL; }
			do {
				if (c=='\n') cerror();
				if (c=='-' && ep[-1]!=0) {
					if ((c=getchr())==']') { *ep++ = '-'; cclcnt++; break; }
					while (ep[-1]<c) {
						*ep = ep[-1]+1;
						ep++;
						cclcnt++;
						if (ep>=&expbuf[ESIZE])
							cerror(); } }
				*ep++ = c;
				cclcnt++;
				if (ep >= &expbuf[ESIZE])
					cerror();
			} while ((c = getchr()) != ']');
			lastep[1] = cclcnt;
			continue;
		default:
			*ep++ = CCHR;
			*ep++ = c; } } }
int execute(unsigned int *addr) {
	char *p1, *p2 = expbuf;
	int c;
	for (c=0; c<NBRA; c++) {
		braslist[c] = 0;
		braelist[c] = 0; }
	if (addr == (unsigned *)0) {
		if (*p2==CCIRC) return(0); p1 = loc2;
	} else if (addr==zero) return(0);
	else p1 = getline(*addr);
	if (*p2==CCIRC) { loc1 = p1; return(advance(p1, p2+1)); }
	if (*p2==CCHR) { /* fast check for first character */
		c = p2[1];
		do {
			if (*p1!=c) continue;
			if (advance(p1, p2)) { loc1 = p1; return(1); }
		} while (*p1++);
		return(0); }
	do { /* regular algorithm */
		if (advance(p1, p2)) {
			loc1 = p1;
			return(1); }
	} while (*p1++);
	return(0); }
int star(char* lp, char* ep, char* curlp) { //goto substitute
	do {
		lp--;
		if (advance(lp, ep)) return(1);
	} while (lp > curlp);
	return(0); }
int advance(char *lp, char *ep) {
	char *curlp;
	int i;
	for (;;) switch (*ep++) {
	case CCHR: if (*ep++ == *lp++) continue; return(0);
	case CDOT:
		if (*lp++) continue; return(0);
	case CDOL: if (*lp==0) continue; return(0);
	case CEOF:
		loc2 = lp;
		return(1);
	case CCL:
		if (cclass(ep, *lp++, 1)) {
			ep += *ep;
			continue; }
		return(0);
	case NCCL:
		if (cclass(ep, *lp++, 0)) {
			ep += *ep;
			continue; }
		return(0);
	case CBRA:
		braslist[*ep++] = lp;
		continue;
	case CKET:
		braelist[*ep++] = lp;
		continue;
	case CBACK:
		if (braelist[i = *ep++]==0) error(Q);
		if (backref(i, lp)) {
			lp += braelist[i] - braslist[i];
			continue; }
		return(0);
	case CBACK|STAR:
		if (braelist[i = *ep++] == 0) error(Q);
		curlp = lp;
		while (backref(i, lp))
			lp += braelist[i] - braslist[i];
		while (lp >= curlp) {
			if (advance(lp, ep)) return(1);
			lp -= braelist[i] - braslist[i]; }
		continue;
	case CDOT|STAR:
		curlp = lp;
		while (*lp++);
		return star(lp, ep, curlp);
	case CCHR|STAR:
		curlp = lp;
		while (*lp++ == *ep);
		ep++;
		return star(lp, ep, curlp);
	case CCL|STAR:
	case NCCL|STAR:
		curlp = lp;
		while (cclass(ep, *lp++, ep[-1]==(CCL|STAR)));
		ep += *ep;
		return star(lp, ep, curlp);
	default: error(Q); } }
int backref(int i, char *lp) {
	char *bp = braslist[i];
	while (*bp++ == *lp++)
		if (bp >= braelist[i])
			return(1);
	return(0); }
int cclass(char *set, int c, int af) {
	int n;
	if (c==0) return(0);
	n = *set++;
	while (--n)
		if (*set++ == c)
			return(af);
	return(!af); }
void puts(char *sp) {
	col = 0;
	while (*sp) putchr(*sp++);
	putchr('\n'); }
void putchr(int ac) {
	char *lp = linp;
	int c = ac;
	if (lp >= &line[64] || c == '\n') {
		linp = line;
		write(oflag ? 2 : 1, line, lp - line);
		return; }
	linp = lp; }