#include "util.h"
#include <string.h>
#include <math.h>
#include <sys/stat.h>

#pragma warning(disable : 4996)
void beginProgress() {
	//system("color 0A");
	char a = 177;
	printf("\n");
	for (int i = 0; i < 100; i++)
		printf("%c", a);
	printf("\r");
}

void endProgress() {
	system("color 0F");
	printf("\n\n");
}

void showProgress(int val, int total)
{
	int i = 100 * val / total;
	char b = 219;
	printf("%c%c%d%%\r",20,20, i);
	for (int j = 0; j < i + 1; j++)
		printf("%c", b);
}

int strToHex(char* _str) {
	if (_str == NULL) return 0;
	int ret = 0;
	char* temp = _str + strlen(_str) - 1;
	int upper = 0;
	while (temp >= _str) {
		char c = temp[0] - 0x30;
		if (c < 10) ret += pow(16, upper) * c;
		else if (c > 16) { //65-48 //97-65
			c = temp[0] - 0x41 + 10;
			if (c < 16) ret += 16 ^ upper * c;
			else if (c >= 42) {
				c = temp[0] - 0x61 + 10;
				if (c < 16) ret += pow(16, upper) * c;
				else {
					ret = 0; break;
				}
			}
			else {
				ret = 0; break;
			}
		}
		else {
			ret = 0; break;
		}
		upper++;
		temp--;
	}
	return ret;
}

char* findPos(char* _src, char* _delimiter) {
	if (_src == NULL || _delimiter == NULL) return NULL;

	int slen = strlen(_src), dlen = strlen(_delimiter);
	int spos = 0, dpos = 0;
	while (1) {
		int i = 0;
		for (i = 0; i < dlen; i++) {
			if (_delimiter[i] != _src[spos + i]) {
				break;
			}
		}
		if (i == dlen) return _src + spos;
		spos++;
	}
	return NULL;
}

//after trimed
char** /*array of params in line*/
splitline(char* trimmedline, int* len/*param count*/) {
	if (trimmedline == NULL || len == NULL) return NULL;
	int space_cnt = 0;
	//calc space count
	for (size_t i = 0; i < strlen(trimmedline); i++)
		if (trimmedline[i] == ' ') space_cnt++;
	*len = space_cnt + 1;
	char** ret = (char**)calloc(*len, sizeof(char*));
	const char* delimiter = " ";
	int pos = 0;
	char* next_token = NULL, * token = strtok_r(trimmedline, delimiter, &next_token);
	ret[pos++] = token;

	/* walk through other tokens */
	while (token != NULL) {
		token = strtok_r(NULL, delimiter, &next_token);
		ret[pos++] = token;
	}
	return ret;
}


/******************************************************************************
 * Checks to see if a directory exists. Note: This method only checks the
 * existence of the full path AND if path leaf is a dir.
 *
 * @return  >0 if dir exists AND is a dir,
 *           0 if dir does not exist OR exists but not a dir,
 *          <0 if an error occurred (errno is also set)
 *****************************************************************************/
bool dirExists(const char* const path)
{
	struct stat info;

	int statRC = stat(path, &info);
	if (statRC != 0)
	{
		if (errno == ENOENT) { return 0; } // something along the path does not exist
		if (errno == ENOTDIR) { return 0; } // something in path prefix is not a dir
		return false;
	}

	return (info.st_mode & S_IFDIR) ? true : false;
}


bool fileExists(const char* const path) {
	FILE* fp = fopen(path,"r");
	bool flag = fp == NULL ? false : true;
	if(fp) fclose(fp);
	return flag;
}


int	getFileSize(const char* path) {
	if (!fileExists(path)) return -1;

	FILE* fp = fopen(path, "rb");
	fseek(fp, 0, SEEK_END);
	int filesize = ftell(fp);
	fclose(fp);
	return filesize;
}











































/*
** Get the length of a string
*/
int get_str_len(char const* str) {
	int len = 0;
	while (str[len] != '\0') {
		len += 1;
	}
	return (len);
}
/*
** Auxiliary function to skip white spaces
*/
int is_white_space(char c) {
	return (c == ' ' || c == '\t' || c == '\n' || c== '\r');
}
/*
** Iterate through the white spaces at the beginning of the string
*/
int get_first_position(char const* str) {
	int i = 0, len = get_str_len(str);
	while (is_white_space(str[i]) && i < len) {
		i++;
	}
	return (i);
}
/*
** Find the last position in a string that is not a white space
*/
int get_last_position(char const* str) {
	int i = get_str_len(str) - 1;
	while (is_white_space(str[i]) && i >= 0) {
		i--;
	}
	return (i);
}
/*
** Returns the correct length of a trimmed string
*/
int get_trim_len(char const* str) {
	return (get_last_position(str) - get_first_position(str));
}

/*
** Allocates a new string with removed whitespace characters from the beginning of the source string `str`
*/
char* strtrim(char const* str) {
	char* trim = NULL;
	int i = 0, len = 0, start = 0, end = 0;
	if (str != NULL) {
		i = 0;
		len = get_trim_len(str) + 1;
		if (len < 0) return NULL;

		trim = (char*)malloc(len + 1);
		start = get_first_position(str);
		while (i < len) {
			trim[i] = str[start];
			i++;
			start++;
		}
		trim[i] = '\0';
	}
	return (trim);
}
