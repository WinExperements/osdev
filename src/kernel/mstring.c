#include<mstring.h>
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