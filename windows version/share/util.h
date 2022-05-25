#ifndef UTIL_HEADER_INC
#define UTIL_HEADER_INC

#include <stdlib.h>
#include <stdbool.h>
#include <string>

void	beginProgress();
void	showProgress(int val, int total);
void	endProgress();

int		strToHex(char* _str);
char*	findPos(char* _src, char* _delimiter);
char**	splitline(char* trimmedline, int* len/*param count*/);
bool	dirExists(const char* const path);
bool	fileExists(const char* const path);
int		getFileSize(const char* path);
char*	strtrim(char const* str);


#endif // !UTIL_HEADER_INC
