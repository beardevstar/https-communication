#ifndef S_SOCKET_HEADER_INC
#define S_SOCKET_HEADER_INC

#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <openssl/evp.h>	//OpenSSL_add_all_algorithms()
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#define BUF_LEN (8192)	//max is 8192
#define NAME_LEN 256

#define TEMP_FILE_PATH (char*)"./temp"
#define TEMP_SAVE (char*)"./tempsave"
#define FILE_NAME_SEP '?'

int		connectSocket(char* hostname, int port);
int		httpsGet(SSL* sslsock, char* _host, char* _url);
int		httpsPost(SSL* sslsock, char* _host, char* _url, char* _data);
char*	generateFormRequest(char* _host, char* _url, char* buffer, int buflen, char* keys[], char* vals[], int _len, int* _buf_len);
char*	readHttpsResponse(SSL* _ssl, int* _len);

SSL*	initSSL(char* webserver);
bool	exitSSL(SSL* ssl);

#endif // !S_SOCKET_HEADER_INC
