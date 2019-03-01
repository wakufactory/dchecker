<?
// http request 

//GETパラメータの組み立て
function setget($u,$p,$enc="UTF-8") {
  $uu = $u['p'] ;
  if($p) {
    foreach($p as $k=>$v ) {
      $uu[$k] = $v ;
    }
  }
  $s = array() ;
  foreach($uu as $k=>$v ) {
    if(is_array($v)) {
      for($i=0;$i<count($v);$i++) $s[] = $k."=".urlencode($v[$i]) ;
    } else  $s[] = $k."=".urlencode($v) ;
  }
  return $u['u']."?".join("&",$s) ;
}

function HTTPhead($host,$path,$opt) {
	$port = $opt['port'] ;
	if($port==0) $port=80 ;
	$fp = @fsockopen($host,$port,$en,$es,30) ;
	if(!$fp) {
		return null ;
	} 
	$o = "HEAD $path HTTP/1.0\n" ;
	$o.= "Host: $host\n\n" ;
	fwrite($fp,$o) ;
	$i = array() ;
	while(!feof($fp)) {
		$l = fgets($fp,128) ;	
		if(preg_match("/^HTTP\/1.1 ([0-9]+)/",$l,$s)) $response = $s[1] ;
		if(preg_match("/^Content-Length: ([0-9]+)/",$l,$s)) $len = $s[1] ;
		if(preg_match("/^Last-Modified: (.+)/",$l,$s)) $mod = $s[1] ;
		if(preg_match("/^Content-Type: (.+)/i",$l,$s)) $type = $s[1] ;
		$i[] = $l ;
	}
	fclose($fp) ;
	return Array('header'=>$i,'resp'=>$response,'size'=>$len,'mod'=>date("Y-m-d H:i:s",strtotime($mod)),'type'=>$type) ;
}

function HTTPget($host,$path,$opt=null) {
	$charset = $opt['charset'] ; 
	if(!$charset) $charset="UTF-8" ;
	$ua= $opt['ua'] ; 
	$mod = $opt['mod'] ;
	$ofs = $opt['ofs'] ;
	$basic = $opt['basic'] ;
	$size = $opt['size'] ;
	$wsse = $opt['wsse'] ;
	$timeout = $opt['timeout'] ;
	$port = $opt['port'] ;
	if($port==0) $port=80 ;
	$cookie = $opt['cookie'] ;
	if($timeout==0) $timeout = 30 ;
	$fp = fsockopen($host,$port,$en,$es,$timeout) ;
	if(!$fp) {
		return null ;
	} 

	$o = "GET $path HTTP/1.1\n" ;
	$o.= "Host: $host\n" ;
//	if($ofs==0) $o.= "Accept-Encoding: gzip\n" ;
	if($mod!="") $o.="If-Modified-Since: $mod\n" ;
	if($ua!="") $o.= "User-Agent: $ua\n" ;
	if($basic!="") $o.="Authorization: Basic ".base64_encode($basic)."\n" ;
	if($ofs>0) {
		$end="" ;
		if($size>0) $end = $ofs + $size ;
		$o.= "Range: bytes=$ofs-$end\n" ;
	}
	if($cookie) {
		$cstr="" ;
		foreach($cookie as $k=>$v) {
			$cstr .= $k."=".$v."; " ;
		}
		$o.= "Cookie: ".$cstr."\n" ;
	}
	if($wsse) {
		$created = date('Y-m-d\TH:i:s\Z');
		$nonce = pack('H*', sha1(md5(time())));
		$pass_digest = base64_encode(pack('H*', sha1($nonce.$created.$wsse[pass])));
		$wsse = 'UsernameToken Username="'.$wsse[user].'", PasswordDigest="'.$pass_digest.'", Created="'.$created.'", Nonce="'.base64_encode($nonce).'"';
		$o.= 'Authorization: WSSE profile="UsernameToken"'."\n";
		$o.= 'X-WSSE: '.$wsse."\n" ;
	}
	$o.= "\n" ;
//echo $o ;
	fwrite($fp,$o) ;
	stream_set_timeout($fp,$timeout) ;
	$h = array() ;
	while(!feof($fp)) {
		$l = fgets($fp,1024) ;	
		if(preg_match("/^HTTP\/1.[0|1] ([0-9]+)/",$l,$s)) $response = $s[1] ;
		if(preg_match("/^Content-Length: ([0-9]+)/",$l,$s)) $len = $s[1] ;
		if(preg_match("/^Last-Modified: (.+)/",$l,$s)) $mod = $s[1] ;
		if(preg_match("/^Content-Range: bytes ([0-9]+)-([0-9]+)/",$l,$s)) {$start = $s[1];$end=$s[2];} ;
		if(preg_match("/^Content-Encoding: gzip/",$l,$s)) {$gzip =1;} ;
		if(preg_match("/^Content-Type: (.+)/i",$l,$s)) $type = $s[1] ;
		if(preg_match("/^Location: (.+)/i",$l,$s)) $loc = $s[1] ;
		if($l=="\r\n") break ;
		$h[] = $l ;
	}
print_r($h) ;
	if($gzip) {
		$b = "" ;
		while(!feof($fp)) {
			$b.= fgets($fp,32768) ;
		}
		$tmpfile = tempnam("/tmp","htgz") ;

		$proc = popen("/usr/bin/gzip -c -d -f >$tmpfile","w") ;
		if(is_resource($proc)) {
				fwrite($proc,$b) ;
				pclose($proc) ;
				$len = filesize($tmpfile) ;
				$f = fopen($tmpfile,"r") ;
				$b = array() ;
				while(!feof($f)) {
					$b[] = @mb_convert_encoding(fgets($f,32768),mb_internal_encoding(),$charset) ;
				}
				fclose($f) ;
				unlink($tmpfile) ;
		}
	} else {
		$b = array() ;
		while(!feof($fp)) {
//			$b[] = fgets($fp,32768);
			$b[] = mb_convert_encoding(fgets($fp),mb_internal_encoding(),$charset) ;
		}
	}
	fclose($fp) ;
	return Array('header'=>$h,'resp'=>$response,'size'=>$len,'type'=>$type,'start'=>$start,'end'=>$end,'mod'=>$mod,'location'=>$loc,'body'=>$b) ;
}

function HTTPpost($host,$path,$postdata,$opt) {
	$charset = $opt[charset] ; 
	$ua= $opt[ua] ; 
	$mod = $opt[mod] ;
	$ofs = $opt[ofs] ;
	$type = $opt[type] ;
	if($type=="") $type = "application/x-www-form-urlencoded" ;
	$basic = $opt[basic] ;
	$size = $opt[size] ;
	$fp = @fsockopen($host,80,$en,$es,30) ;
	if(!$fp) {
		return null ;
	} 
	$o = "POST $path HTTP/1.0\n" ;
	$o.= "Host: $host\n" ;
//	if($ofs==0) $o.= "Accept-Encoding: gzip\n" ;
	if($mod!="") $o.="If-Modified-Since: $mod\n" ;
	if($ua!="") $o.= "User-Agent: $ua\n" ;
	if($type!="") $o.= "Content-type: $type\n" ;
	if($basic!="") $o.="Authorization: Basic ".base64_encode($basic)."\n" ;
	if($ofs>0) {
		$end="" ;
		if($size>0) $end = $ofs + $size ;
		$o.= "Range: bytes=$ofs-$end\n" ;
	}
	$o.="Content-length: ".strlen($postdata)."\n" ;
	$o.= "\n" ;
//echo "<pre>".$o.$postdata ;

	fwrite($fp,$o) ;
	fwrite($fp,$postdata) ;

	$h = array() ;
	while(!feof($fp)) {
		$l = fgets($fp,1024) ;	
		if(preg_match("/^HTTP\/1.1 ([0-9]+)/",$l,$s)) $response = $s[1] ;
		if(preg_match("/^Content-Length: ([0-9]+)/",$l,$s)) $len = $s[1] ;
		if(preg_match("/^Last-Modified: (.+)/",$l,$s)) $mod = $s[1] ;
		if(preg_match("/^Content-Range: bytes ([0-9]+)-([0-9]+)/",$l,$s)) {$start = $s[1];$end=$s[2];} ;
		if(preg_match("/^Content-Encoding: gzip/",$l,$s)) {$gzip =1;} ;
		if($l=="\r\n") break ;
		$h[] = $l ;
	}
		$b = array() ;
		while(!feof($fp)) {
			$b[] = mb_convert_encoding(fgets($fp),mb_internal_encoding(),$charset) ;
		}

	fclose($fp) ;
	return Array('header'=>$h,'resp'=>$response,'size'=>$len,'start'=>$start,'end'=>$end,'mod'=>$mod,'body'=>$b) ;
}
