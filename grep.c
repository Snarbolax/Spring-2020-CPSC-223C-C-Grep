/*  Editor */

#include "grep.h"

int	peekc, lastc, ninbuf, io, oflag, col, nbra;
char linebuf[LBSIZE], expbuf[ESIZE + 4], genbuf[LBSIZE], *nextip, *loc1, *loc2, *braslist[NBRA], *braelist[NBRA], Q[] = "", line[70], *linp = line, current[400];
unsigned int *zero;
long count;
DIR* directory;
struct dirent *dir;

int main(int argc, char *argv[]) {
	argv++;
	if (argc > 2) {
		compile(*(argv++));
		init(argc, *argv); }
	exit(0);
	return 0;
}
void error(char *s) {
	put(s);
	if (io > 0) { close(io); io = -1; }
	exit(1);
}
void cerror() { //goto substitute
	expbuf[0] = 0;
	nbra = 0;
	error(Q);
}
void defchar(char* ep, int c) { //goto substitute
	*ep++ = CCHR;
	*ep++ = c;
}
int begin(char *string1, char *string2) { // 1 = same, 0 = not 
	while ((*string1 == *string2) && (*string2 != '\0')  && (*string1 != '\0')) { ++string1; ++string2; }
	if (*string2 == '\0') { return 1; }
	return 0;
}
int end(char *string1, char *string2) { // 1 = same, 0 = not, but end
	char *i = (char *)string2;
	while ((*string1 != *string2) && (*string1 != '\0')) { ++string1; }
	while ((*string1 == *i) && (*string1 != '\0') && (*i != '\0')) { ++string1; ++i; }
	if ((*string1 == '\0') && (*i == '\0')) { return 1; }
	if (*string1 == '\0') { return 0; }
	return end(string1, string2);
}
void find_lines() {
	char *linepoint = linebuf, *genpoint = genbuf;
	while (*genpoint != '\0') {
		if (*genpoint == '\n') {
			*linepoint++ = '\0';
			if (execute()) {
				if (*current != '\0') {
					putchr('[');
					puts(current);
					putchr(']'); }
				else { puts(current); }
				puts(linebuf);
				putchr('\n');
			}
			linepoint = linebuf;
			++genpoint;
		}
		else { *linepoint++ = *genpoint++;
		}
	}
}
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
			fp = genbuf;
		}
		c = *fp++;
		if (c=='\0') continue;
		if (c&0200 || lp >= &linebuf[LBSIZE]) { lastc = '\n'; error(Q); }
		*lp++ = c;
		count++;
	} while (c != '\n');
	*--lp = 0;
	nextip = fp;
	return(0);
}
void open_f(char* file) {
	if ((io = open(file, 0)) < 0) error(file);
	getfile();
	close(io);
}
int init(char* file) {
	char *fEnd, *fEnd2, folder[30];
	fEnd = file;
	fEnd2 = strchr(end, '*');
	strcpy(folder, file);
	directory = opendir(folder);
	if (directory) {
		while ((dir = readdir(directory)) != NULL) {
			if (begin(dir->d_name, fEnd) && end(dir->d_name, fEnd2)) {
				while (*(dir->d_name) != '\0') { *current++ = *(dir->d_name)++; }
				*current++ = '\0';
			}
				open_f(current);
				find_lines();
		} 
	}
		closedir(directory);
	else {
		close(io);
		open_f(file);
		find_lines();
	}
}
void compile(int eof) {
	int c, cclcnt;
	char bracket[NBRA], *bracketp = bracket, *lastep, *ep = expbuf;
	if ((c = getchr()) == '\n') { peekc = c; c = eof; }
	if (c == eof) {
		if (*ep==0) error(Q);
		return;
	}
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
			return;
		}
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
						if (ep>=&expbuf[ESIZE]) cerror();
					}
				}
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
		return(0);
	}
	do { /* regular algorithm */
		if (advance(p1, p2)) {
			loc1 = p1;
			return(1); }
	} while (*p1++);
	return(0);
}
int star(char* lp, char* ep, char* curlp) { //goto substitute
	do {
		lp--;
		if (advance(lp, ep)) return(1);
	} while (lp > curlp);
	return(0);
}
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
			continue;
		}
		return(0);
	case NCCL:
		if (cclass(ep, *lp++, 0)) {
			ep += *ep;
			continue;
		}
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
			continue;
		}
		return(0);
	case CBACK|STAR:
		if (braelist[i = *ep++] == 0) error(Q);
		curlp = lp;
		while (backref(i, lp))
			lp += braelist[i] - braslist[i];
		while (lp >= curlp) {
			if (advance(lp, ep)) return(1);
			lp -= braelist[i] - braslist[i];
		}
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
	default: error(Q);
	}
}
int backref(int i, char *lp) {
	char *bp = braslist[i];
	while (*bp++ == *lp++)
		if (bp >= braelist[i]) return(1);
	return(0);
}
int cclass(char *set, int c, int af) {
	int n;
	if (c==0) return(0);
	n = *set++;
	while (--n)
		if (*set++ == c)
			return(af);
	return(!af);
}
void puts(char *sp) {
	col = 0;
	while (*sp) putchr(*sp++);
	putchr('\n');
}
void putchr(int ac) {
	char *lp = linp;
	int c = ac;
	if (lp >= &line[64] || c == '\n') {
		linp = line;
		write(oflag ? 2 : 1, line, lp - line);
		return;
	}
	linp = lp;
}