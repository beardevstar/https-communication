#include "app.h"
#include "util.h"
#include "s_socket.h"
#include "cJSON.h"

#pragma warning(disable : 4996)

char* generateUserRequest(C_REQ* _param, int* _buf_len) {
	if (_param->role != USER) return NULL;
	char rolestr[16], tpstr[16];
	sprintf(rolestr, "%d", _param->role);
	sprintf(tpstr, "%d", _param->tp);

	CMD_TP _tp = _param->tp;
	switch (_tp)
	{
	case ASK_NAME: {
		char* keys[] = { ROLE_TITLE, TYPE_TITLE };
		char* vals[] = { rolestr , tpstr};
		return generateFormRequest(_param->server, _param->url, NULL, 0, keys, vals, 2, _buf_len);
		break;
	}
	case POOLING: {
		char* keys[] = { ROLE_TITLE, TYPE_TITLE,UNAME_TITLE };
		char* vals[] = { rolestr , tpstr, _param->uname};
		return generateFormRequest(_param->server, _param->url, NULL, 0, keys, vals, 3, _buf_len);
		break;
	}
	case UPLOAD: {
		char subpos[16];
		sprintf(subpos, "%d", _param->file_index);
		char* keys[] = { ROLE_TITLE, TYPE_TITLE,UNAME_TITLE, FINDEX_TITLE };
		char* vals[] = { rolestr , tpstr, _param->uname, subpos };
		return generateFormRequest(_param->server, _param->url, _param->bin, _param->bin_len, keys, vals, 4, _buf_len);
		break;
	}
	case DOWNLOAD: {
		char subpos[16];
		sprintf(subpos, "%d", _param->file_index);
		char* keys[] = { ROLE_TITLE, TYPE_TITLE,UNAME_TITLE, FINDEX_TITLE};
		char* vals[] = { rolestr , tpstr, _param->uname, subpos};
		return generateFormRequest(_param->server, _param->url, NULL, 0, keys, vals, 4, _buf_len);
		break;
	}
	case RETURNING: {
		char* keys[] = { ROLE_TITLE, TYPE_TITLE,UNAME_TITLE,CONTENT_TITLE};
		char* vals[] = { rolestr , tpstr, _param->uname,_param->content };
		return generateFormRequest(_param->server, _param->url, NULL, 0, keys, vals, 4, _buf_len);
		break;
	}
	default:
		break;
	}
	return 0;
}

char* generateAdminRequest(C_REQ* _param, int *_buf_len) {
	if (_param->role != ADMIN) return NULL;
	char rolestr[16], tpstr[16],prevtpstr[16];
	sprintf(rolestr, "%d", _param->role);
	sprintf(tpstr, "%d", _param->tp);
	sprintf(prevtpstr, "%d", _param->prev_tp);

	CMD_TP _tp = _param->tp;
	switch (_tp)
	{
	case LIST: {
		char* keys[] = { ROLE_TITLE, TYPE_TITLE };
		char* vals[] = { rolestr , tpstr };
		return generateFormRequest(_param->server, _param->url, NULL, 0, keys, vals, 2, _buf_len);
		break;
	}
	case EXECUTE: {
		char* keys[] = { ROLE_TITLE, TYPE_TITLE, UNAME_TITLE ,CONTENT_TITLE};
		char* vals[] = { rolestr , tpstr ,_param->uname, _param->content};
		return generateFormRequest(_param->server, _param->url, NULL, 0, keys, vals, 4, _buf_len);
		break;
	}
	case POOLING: {
		char* keys[] = { ROLE_TITLE, TYPE_TITLE, UNAME_TITLE, PRVTYPE_TITLE };
		char* vals[] = { rolestr , tpstr ,_param->uname, prevtpstr};
		return generateFormRequest(_param->server, _param->url, NULL, 0, keys, vals, 4, _buf_len);
		break;
	}
	case UPLOAD: {
		char subpos[16];
		sprintf(subpos,"%d",_param->file_index);
		char* keys[] = { ROLE_TITLE, TYPE_TITLE, UNAME_TITLE, PRVTYPE_TITLE,FINDEX_TITLE};
		char* vals[] = { rolestr , tpstr ,_param->uname, prevtpstr , subpos};
		return generateFormRequest(_param->server, _param->url, _param->bin, _param->bin_len, keys, vals, 5, _buf_len);
		break;
	}
	case DOWNLOAD: {
		char subpos[16];
		sprintf(subpos, "%d", _param->file_index);
		char* keys[] = { ROLE_TITLE, TYPE_TITLE, UNAME_TITLE, PRVTYPE_TITLE,FINDEX_TITLE };
		char* vals[] = { rolestr , tpstr ,_param->uname, prevtpstr , subpos };
		return generateFormRequest(_param->server, _param->url, _param->bin, _param->bin_len, keys, vals, 5, _buf_len);
		break;
	}
	default:
		break;
	}
	return 0;
}

char* generateRequest(C_REQ* _req, int *_buf_len) {
	if (_req->role == ADMIN) 
		return generateAdminRequest(_req, _buf_len);
	if (_req->role == USER) 
		return generateUserRequest(_req, _buf_len);
	*_buf_len = 0;
	return 0;
}

bool sendFormData(C_REQ* _req) {
	//m_mutex.lock();
	bool flag = false;
	SSL* ssl = initSSL(_req->server);
	if (ssl == NULL) {
		_req->code = SSL_INIT_FAIL;
		return false;
	}

	int buf_len = 0;
	char* reqbuf = generateRequest(_req, &buf_len);
	
	if (reqbuf == NULL || buf_len < 1) {
		_req->code = GENERATE_FAIL;
	}
	else {
		int sended = SSL_write(ssl, reqbuf, buf_len);
		char *_buf = readHttpsResponse(ssl, NULL);
		if (_buf != NULL) free(_buf);

		if (sended < 1)
			_req->code = SSL_SEND_FAIL;
		else {
			_req->code = SUCCESS;
		}
	}

	if(reqbuf) free(reqbuf);
	exitSSL(ssl);
	//m_mutex.unlock();
	return true;
}

bool sendFormDataRead(C_REQ* _req, char** _buf, int* _len) {
	//m_mutex.lock();
	SSL* ssl = initSSL(_req->server);
	if (ssl == NULL) {
		_req->code = SSL_INIT_FAIL;
		return false;
	}

	int buf_len = 0;
	char* reqbuf = generateRequest(_req, &buf_len);
	if (reqbuf == NULL || buf_len < 1) _req->code = GENERATE_FAIL;
	else {
		int sended = SSL_write(ssl, reqbuf, buf_len);
		if (sended < 1) _req->code = SSL_SEND_FAIL;
		else {
			*_buf = readHttpsResponse(ssl, _len);
			if (_buf == NULL) _req->code = SSL_READ_FAIL;
			else _req->code = SUCCESS;
		}
	}
	
	if(reqbuf) free(reqbuf);
	exitSSL(ssl);
	//m_mutex.unlock();
	return true;
}

char* getMessage(char* _buf) {
	cJSON* json = cJSON_Parse(_buf);
	if (json == NULL) return NULL;
	return cJSON_GetObjectItem(json, "msg")->valuestring;
}

int getStatusCode(char* _buf) {
	cJSON* json = cJSON_Parse(_buf);
	if (json == NULL) {
		//printf("parse error!\n");
		//if (_buf != NULL) printf("%s\n",_buf);
		return -1;
	}
	int status = cJSON_GetObjectItem(json, "status")->valueint;
	return status;
}

int getTotalCnt(char* _buf) {
	cJSON* json = cJSON_Parse(_buf);
	if (json == NULL) {
		//printf("parse error!\n");
		//if (_buf != NULL) printf("%s\n", _buf);
		return -1;
	}
	int status = cJSON_GetObjectItem(json, "total")->valueint;
	return status;
}

bool getFileName(char* path, char* buf) {
	if (path == NULL) return false;

	int len = strlen(path);
	for (int i = len - 1; i >= 0; i--) {
		if (path[i] == '\\' || path[i] == '/') {
			memcpy(buf, path + i + 1, len - i - 1);
			return true;
		}
	}
	memcpy(buf,path,len);
	return true;
}