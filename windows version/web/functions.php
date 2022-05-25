<?php
	function getClientList(){
		$files = scandir(RESIDENCE_DIR);
    chdir(RESIDENCE_DIR);
    $ret = "";
    foreach ($files as $key => $filepath) {
      // code...
      if(!is_dir($filepath) && time() - fileatime($filepath) < 2){
          $ret = $ret.$filepath.", ";
      }
    }
    if($ret == "") $ret = "No users connected.";
    return $ret;
	}

	function checkUserLive($username){
		return true;
		$filepath = RESIDENCE_DIR.$username;
		if(file_exists($filepath) && (time() - fileatime($filepath)) < (INTERVAL * 3))
			return true;
		return false;
	}

	function getnerateValidUserId(){
		$cnt = 1;
		while($cnt < 100){
			$filepath = RESIDENCE_DIR."user".$cnt;
			if(file_exists($filepath) && (time() - fileatime($filepath)) < INTERVAL ){
				$cnt++;
				continue;
			}else{
				return "user".$cnt;
			}
		}
		return "user";
	}

	function saveData($path, $content){
		$handle = fopen($path,"wb");
		fwrite($handle, $content);
		fclose($handle);
	}

	function loadData($path){
		
		if(!file_exists($path)) return "";
		
		$handle = fopen($path,"rb");
		$data = fread($handle, filesize($path));
		fclose($handle);
		return $data;
	}

	function delDirTree($dir) {
   $files = array_diff(scandir($dir), array('.','..'));
    foreach ($files as $file) {
      (is_dir("$dir/$file")) ? delTree("$dir/$file") : unlink("$dir/$file");
    }
    return rmdir($dir);
  }

  function fileCntInDir($dir) {
  	if(!file_exists($dir)) return 0;
  	$cnt = 0;
		$files = array_diff(scandir($dir), array('.','..'));
		foreach ($files as $file) 
		  if (!is_dir("$dir/$file")) $cnt++;
    return $cnt;
  }
?>