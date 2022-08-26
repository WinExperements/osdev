#include<mstring.h>
#include<mm/pmm.h>
void memcpy(void *vd, const void *vs, unsigned length) {
	char *d = vd;
	const char *s = vs;
	while(length) {
		*d = *s;
		d++;
		s++;
		length--;
	}
}
bool strcmp(char *s1,char *s2) {
	int len1 = strlen(s1);
	int len2 = strlen(s2);
	int checkedLen = 0;
	if (len1 > len2 || len1 < len2) {
		return false;
	}
	/* If length are equal then check the symbols */
	for (int i = 0; i < len1; i++) {
		if (s1[i] == s2[i]) {
			checkedLen++;
		}
	}
	if (checkedLen == len1) {
		return true;
	} else {
		return false;
	}
}
bool strcpy(char *to,char *from) {
	int i = strlen(from);
	for (int u = 0; u < i; u++) {
		to[u] = from[u];
	}
	return true;
}
const char *strchr(const char *s, char ch)
{
	while(*s) {
		if(*s == ch)
			return s;
		s++;
	}
	return 0;
}

char *strtok(char *s, const char *delim)
{
	static char *oldword = 0;
	char *word;

	if(!s)
		s = oldword;

	while(*s && strchr(delim, *s))
		s++;

	if(!*s) {
		oldword = s;
		return 0;
	}

	word = s;
	while(*s && !strchr(delim, *s))
		s++;

	if(*s) {
		*s = 0;
		oldword = s + 1;
	} else {
		oldword = s;
	}

	return word;
}
char *strdup(char *src) {
	int len = strlen(src);
	char *p = pmml_allocPages((len/4096)+1,true);
	if (!p) return NULL;
	memcpy(p,src,len);
	return p;
}