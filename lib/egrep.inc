<?php
function egrep($path,$pat,$opt=null) {
	$epat = escapeshellarg ($pat);
	try {
		$ret = exec("/bin/grep -rEn $epat $path; echo 'end'",$out) ;
	} catch (Exception $err) {
		echo  $e->getMessage() ;
	}
	if(count($out)==0) return null  ;

	$d = array() ;
	$epat = "/".str_replace("/","\/",$pat)."/u" ;
	for($i=0;$i<count($out)-1;$i++) {
		$l = rtrim($out[$i]) ;
		
		if(preg_match("/^([^:]+):([0-9]+):(.+)/",$l,$a)) {
			$p = substr($a[1],strlen($path)) ;
			$t = $a[3];	
				
			if($opt && $opt['skip'] && preg_match("/".$opt['skip']."/u",$t)) continue ;
			

			$n = preg_match_all($epat,$t,$m,PREG_OFFSET_CAPTURE) ;
//			print_r($m);
			if($n>0) {
				$s = "" ;
				$o =0 ;
				foreach($m[0] as $k) {
					$s = $s.htmlspecialchars(substr($t,$o,$k[1]-$o))."<span class=s>".htmlspecialchars(substr($t,$k[1],strlen($k[0])))."</span>" ;
					$o = $k[1]+strlen($k[0]) ;
				}
				$s = $s.htmlspecialchars(substr($t,$o)) ;
			} else $s = htmlspecialchars($t) ;
			$d[$p][$a[2]] = $s ;
		} 
	}
	ksort($d);
//			echo "<pre>";print_r($d) ;
	$r = array() ;
	foreach($d as $p=>$v) {
		if(!$c_opf && preg_match("/\.opf$/i",$p)) continue ;
		if(!$c_css && preg_match("/\.css$/i",$p)) continue ;
		$f = array() ;
		foreach($v as $pp=>$l) {
			$f[] = array('p'=>$pp,'l'=>$l) ;
		}
		$r[] = array('f'=>$p,'m'=>$f) ;
	}
	return $r ;
}
