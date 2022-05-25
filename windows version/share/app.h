#ifndef APP_HEADER_INC
#define APP_HEADER_INC

#define SLEEP_TIME	1000
#define MAX_LEN		1024

#define ADMIN_ROLE	(char*)"admin"
#define USER_ROLE	(char*)"user"

#define UPLOADCMD	(char*)"upload"
#define DOWNLOADCMD	(char*)"download"

#define WEB_URL		(char*)"/"
#define SERVER		(char*)"192.168.114.97"

enum ROLE {
	USER, ADMIN, TEST
};
#define ROLE_TITLE (char*)"role"

enum CMD_TP{
	NONE = 0,
	LIST /*admin*/,
	EXECUTE /*admin*/,

	POOLING /*user_admin*/, 
	UPLOAD /*user_admin*/,
	DOWNLOAD /*user_admin*/,
	
	ASK_NAME,
	RETURNING /*user*/
};
#define TYPE_TITLE (char*)"type"
#define PRVTYPE_TITLE (char*)"prvtype"
#define UNAME_TITLE (char*)"uname"
#define CONTENT_TITLE (char*)"content"
#define FINDEX_TITLE (char*)"subpos"

enum RUN_CODE {
	EMPTY,
	SUCCESS,
	SSL_INIT_FAIL,
	GENERATE_FAIL,
	SSL_SEND_FAIL,
	SSL_READ_FAIL
};

enum CLIENT_STATUS {
	CONNECTED = 1, DISCONNECTED = 2, CMD_RETURN = 3
};

typedef struct _REQUEST
{
	ROLE	role = ADMIN;
	//upload, download, excute, list clients, pooling - admin || result, pooling, download - user
	CMD_TP	tp = POOLING, prev_tp = NONE;
	RUN_CODE code = EMPTY;	//out param
	char* server = SERVER, * url = WEB_URL;
	//admin - command, user- result...
	char* content = 0;
	//only for admin
	char* uname = 0;
	
	//for fileupload
	char* bin = 0;
	int	bin_len = 0;
	int file_index = -1;

	struct _REQUEST(ROLE _role, CMD_TP _tp, CMD_TP _prev, char *_content = 0, char* _bin = 0,int _bin_len = 0, char* _uname = 0) {
		role = _role; tp = _tp; prev_tp = _prev;
		content = _content; bin = _bin;
		uname = _uname; 
		bin_len = _bin_len;
	}

	struct _REQUEST(ROLE _role, char* _bin, int _bin_len , char* _uname , int _file_pos) {
		role = _role; tp = UPLOAD; prev_tp = NONE;
		bin = _bin;bin_len = _bin_len;
		uname = _uname;file_index = _file_pos;
	}

	struct _REQUEST(ROLE _role, char* _uname, int _file_pos) {
		role = _role; tp = DOWNLOAD; prev_tp = NONE;
		uname = _uname; file_index = _file_pos;
	}
} C_REQ;

bool	sendFormData(C_REQ* _req);
bool	sendFormDataRead(C_REQ* _req, char** _buf, int* _len);

int		getStatusCode(char* _buf);
char*	getMessage(char* _buf);
int		getTotalCnt(char* _buf);

bool	getFileName(char* path, char* buf);

#endif // !APP_HEADER_INC