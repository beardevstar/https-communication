<?php
  //for admin respect
	$tempcnt = 0;
	define("USER_ROLE", $tempcnt++);
	define("ADMIN_ROLE", $tempcnt++);
	define("TEST_ROLE", $tempcnt++);

	$tempcnt = 0;
	define("NONE", $tempcnt++);
	define("CLIENTS", $tempcnt++);
	define("EXECUTE", $tempcnt++);
	define("POOLING", $tempcnt++);
	define("UPLOAD", $tempcnt++);
	define("DOWNLOAD", $tempcnt++);
	define("ASK_NAME", $tempcnt++);
	define("RESULT", $tempcnt++);

	$tempcnt = 1;
	define("CONNECTED",$tempcnt++);
	define("DISCONNECTED",$tempcnt++);
	define("CMD_RETURN",$tempcnt++);


	define("SLASH",DIRECTORY_SEPARATOR);
	
  define("INTERVAL", 2);	//2s

	define("COMMADN_DIR","./commands/send/");
	define("RESULT_DIR", "./commands/receive/");
	
  define("RESIDENCE_DIR", "./living/");
  
  define("FILE_SEND_DIR", "./uploads/send/");
  define("FILE_RECEIVE_DIR", "./uploads/receive/");
	
  define("UPLOAD_CMD","upload ");
  define("DOWNLOAD_CMD","download ");
  define("CLIENTS_CMD","clients");

	if(!file_exists(COMMADN_DIR)) mkdir(COMMADN_DIR,0777,true);
	if(!file_exists(RESULT_DIR)) mkdir(RESULT_DIR,0777,true);

  if(!file_exists(FILE_SEND_DIR)) mkdir(FILE_SEND_DIR,0777,true);
  if(!file_exists(FILE_RECEIVE_DIR)) mkdir(FILE_RECEIVE_DIR,0777,true);

	if(!file_exists(RESIDENCE_DIR)) mkdir(RESIDENCE_DIR,0777,true);	//only for users

	define("ROLE_TITLE","role");
	define("TYPE_TITLE","type");

?>