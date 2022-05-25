#include "s_socket.h"
#include "util.h"

#include <unistd.h>	//close()
#include <string.h>
#include <sys/socket.h>	//connect, socket
#include <netinet/in.h>	//inet_addr
#include <netdb.h>

#pragma warning(disable : 4996)

//mem_complete
int httpsGet(SSL* sslsock,char* _host, char* _url) {
	char* sndbuf = (char*)malloc(strlen(_url) + 64 + strlen(_host));
	sprintf(sndbuf, \
		"GET %s HTTP/1.1\r\n"
		"Host: %s\r\n\r\n"
		, _url, _host);
	int sended = SSL_write(sslsock, sndbuf, strlen(sndbuf));
	free(sndbuf);
	return sended;
}

int httpsPost(SSL* sslsock, char* _host, char* _url, char* _data) {
	int datalen = strlen(_data);
	char* header = (char*)malloc(128 + strlen(_url) + strlen(_host));
	sprintf(header, "POST %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"Content-Length: %d\r\n\r\n", _url, _host, datalen);

	int totallen = strlen(header) + datalen;
	char* sndbuf = (char*)malloc(totallen + 1);
	sprintf(sndbuf, "%s%s", header, _data);
	int sended = SSL_write(sslsock, sndbuf, totallen);
	free(sndbuf);
	free(header);
	return sended;
}

char* generateFormRequest(char* _host, char* _url, char* buffer, int buflen, char* keys[], char* vals[], int _len, int *_buf_len) {
	char* header1 = NULL, * parambuf = NULL, * sendbuf = NULL;
	char boundary[64], lenheader[64], fileheader[256], tail[128];

	sprintf(boundary, "----------%d", (int)time(NULL));
	sprintf(tail, "--%s--\r\n", boundary);

	header1 = (char*)malloc(256 + strlen(_url) + strlen(_host));
	sprintf(header1, "POST %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Content-Type: multipart/form-data; boundary=%s\r\n", _url, _host, boundary);
	
	parambuf = (char*)calloc(1,1);

	for (int i = 0; i < _len; i++) {
		char* key = keys[i];
		char* val = vals[i];
		int templen = strlen(parambuf);
		parambuf = (char*)realloc(parambuf, templen + strlen(key) + strlen(val) + 256);
		sprintf(parambuf + templen, 
			"--%s\r\n"
			"Content-Disposition: form-data; name=\"%s\"\r\n\r\n"
			"%s\r\n"
			,boundary, key, val);		
	}

	sprintf(fileheader, \
			"--%s\r\n"
			"Content-Disposition: form-data; name=\"file\"; filename=\"default\"\r\n"
			"\r\n", boundary);
	
	int contentlen = strlen(tail) + strlen(parambuf);
	if (buffer != NULL) contentlen += strlen(fileheader)+buflen;
	sprintf(lenheader, "Content-Length: %d\r\n\r\n", contentlen - 1);

	int total_len = strlen(header1) + strlen(lenheader) + contentlen;
	sendbuf = (char*)malloc(total_len + 3);	//\r\n
	
	if (buffer == NULL) 
		sprintf(sendbuf,"%s%s%s%s", header1, lenheader, parambuf, tail);	//first copy request header
	else {
		sprintf(sendbuf, "%s%s%s%s", header1, lenheader, parambuf,fileheader);
		
		int pos = strlen(sendbuf), len = buflen;
		memcpy(sendbuf + pos, buffer, len);
		
		pos += len;
		sendbuf[pos] = '\r'; sendbuf[pos + 1] = '\n';
		pos += 2;
		
		len = strlen(tail);
		memcpy(sendbuf + pos, tail, len);
		pos += len;
	}
	free(header1);
	free(parambuf);
	*_buf_len = total_len;
	return sendbuf;
}

char* readChunkResponse(SSL* _ssl, char* _first, int _hlen, int* _len) {
	int readed = _hlen, readlen = 0;

	char* buf = (char*)malloc(BUF_LEN);
	char* _header = (char*)malloc(_hlen+1);
	memcpy(_header, _first,_hlen);

	while ((readlen = SSL_read(_ssl, buf, BUF_LEN)) > 0) {
		_header = (char*)realloc(_header, readed + readlen); //realloc additional space for later readed data
		memcpy(_header + readed, buf, readlen);//copy data to end.
		readed += readlen;
	}
	//saved all chunk list to retbuf.
	buf = (char*)realloc(buf, readed);
	memset(buf, 0, readed);

	char *rpos = findPos(_header, (char*)"\r\n");
	if (rpos == NULL) {
		free(_header);
		return NULL;
	}

	char lenbuf[NAME_LEN]; memset(lenbuf, 0, NAME_LEN);
	memcpy(lenbuf, _header, rpos - _header);

	int totallen = strToHex(strtrim(lenbuf));
	if (totallen <= 0) {
		free(_header);
		return NULL;
	}
	memcpy(buf, rpos + 2, totallen);
	free(_header);

	if (_len) *_len = totallen;
	return buf;
}

char* readHttpsResponse(SSL* _ssl, int *_len) {
	char* buf1 = (char*)calloc(1, BUF_LEN);	//later realloc
	char* buf2 = (char*)calloc(1, BUF_LEN);	//later realloc

	int readlen = 0, readed = 0;
	const char* c_delimiter = "\r\n\r\n", * delimiter = "\r\n";
	const char* len_sign = "Content-Length:", * len_chunk = "Transfer-Encoding: chunked";
	int content_len = -1;
	bool chunked = false;

	if ((readlen = SSL_read(_ssl, buf1, BUF_LEN)) > 0)
	{
		char* pos = findPos(buf1, (char*)c_delimiter);//find data pos
		if (pos == NULL) return NULL;

		int body_len = readlen - (pos - buf1) - 4;	//\r\n
		memcpy(buf2, pos + 4, body_len);

		//header parse
		char* token = 0, * next_token = 0;
		token = strtok_r(buf1, (char*)delimiter, &next_token);
		while (token != NULL) {
			if (memcmp(token, len_sign, strlen(len_sign)) == 0) {//if not chunked
				content_len = atoi(token + strlen(len_sign));
				break;
			}
			if (memcmp(token, len_chunk, strlen(len_chunk)) == 0) {//if chunked
				chunked = true;
				break;
			}
			token = strtok_r(NULL, "\r\n", &next_token);
		}
		
		if (content_len >= 0) {
			free(buf1);
			if(_len) *_len = body_len;
			return buf2;
		}

		if (chunked == true) {
			char* ret = readChunkResponse(_ssl, buf2, body_len, _len);
			free(buf1); 
			free(buf2);
			return ret;
		}
	}
	free(buf1); free(buf2);
	return NULL;
}






X509* cert = NULL;
X509_NAME* certname = NULL;
SSL_CTX* ctx;
int   ssl_sock = 0;


int connectSocket(char* hostname, int port) {
	int sockfd;
	char      proto[6] = "";
	char* tmp_ptr = NULL;
	struct hostent* host;
	struct sockaddr_in dest_addr;

	if ((host = gethostbyname(hostname)) == NULL) {
		//printf("\nError: Cannot resolve hostname %s.\n", hostname);
		return 0;
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	dest_addr.sin_addr.s_addr = *(long*)(host->h_addr);

	memset(&(dest_addr.sin_zero), '\0', 8);

	//tmp_ptr = inet_ntoa(dest_addr.sin_addr);

	if (connect(sockfd, (struct sockaddr*)&dest_addr,
		sizeof(struct sockaddr)) == -1) {
		//printf("\nError: Server May be turned off.\n#Cannot connect to host %s [%s] on port %d.\n", hostname, tmp_ptr, port);
		return 0;
	}
	return sockfd;
}

SSL* initSSL(char* webserver) {
	int err = 0;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	OpenSSL_add_all_algorithms();
	ERR_load_BIO_strings();
	ERR_load_crypto_strings();//ERR_load_crypto_strings
	SSL_load_error_strings();
	err = SSL_library_init();
#else
	err = OPENSSL_init_ssl(0, NULL);
#endif

	if (err < 0) {
		//printf("Could not initialize the OpenSSL library !\n");
		return 0;
	}
	const SSL_METHOD* method = SSLv23_client_method();
	if ((ctx = SSL_CTX_new(method)) == NULL) {
		//printf("Unable to create a new SSL context structure.\n");
		return 0;
	}
	SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
	SSL* ssl = SSL_new(ctx);
	ssl_sock = connectSocket(webserver, 443);
	SSL_set_fd(ssl, ssl_sock);
	if (SSL_connect(ssl) != 1) {
		//printf("\nError: Could not build a SSL session to: %s.\n", webserver);
		return 0;
	}
	cert = SSL_get_peer_certificate(ssl);
	if (cert == NULL) {
		//printf("\nError: Could not get a certificate from: %s.\n", webserver);
		return 0;
	}
	certname = X509_NAME_new();
	certname = X509_get_subject_name(cert);
	return ssl;
}

bool exitSSL(SSL* ssl) {
	SSL_free(ssl);
	
	X509_free(cert);
	SSL_CTX_free(ctx);

	close(ssl_sock);
	return true;
}