<?php

$mnt = "/mnt/nas01/kindigi-files" ;
$mnt = "/mnt/nas06" ;


$jp_e_reg = "[0-9a-z]{20}" ;

function fullpath($jpe) {
	$c = substr($jpe,0,1) ;
	if($c=="9"||$c=="8") $mnt = "nas03" ;
	else $mnt = "nas06" ;
	return array("/mnt/$mnt",$c."/".$jpe) ;
}

function joinpath($a) {
	if(count($a)==0) return null ;
	$r = $a[0] ;
	for($i=1;$i<count($a);$i++) {
		$c = (substr($r,-1)=="/")+(substr($a[$i],0,1)=="/") ;
		if($c==0) $r = $r."/".$a[$i] ;
		else if($c==1) $r = $r.$a[$i] ;
		else $r = $r . substr($a[$i],1) ; 
	}
	return $r ;
}

function pubcode($jpe) {
	$s = intval(substr($jpe,0,2)) ;
	if($s<20) $p =substr($jpe,0,2) ;
	else if($s<70) $p = substr($jpe,0,3) ;
	else if($s<85) $p = substr($jpe,0,4) ;
	else if($s<90) $p = substr($jpe,0,5) ;
	else if($s<99) $p = substr($jpe,0,6) ;
	else $p = substr($jpe,0,7) ;
	return $p ;
}


function scheck() {
	$c = "ssh densho@192.168.1.221 'echo aaa'" ;

	exec($c,$out) ;
	return (trim($out[0])=="aaa")?0:-1 ;
}
function titledir($c) {
	$r = array() ;
	for($i=0;$i<count($c);$i++) {
		$f = $c[$i] ;
		$n = mb_convert_kana($f['n'],"a") ;
		if($f['t']!="d" || mb_substr($n,0,1)=="★") continue ;

		if(preg_match("/^h1$/i",$n)) {
			$r['h1'] = $f ;
		} else 
		if(preg_match("/^cover$/i",$n)) {
			$r['cover'] = $f ;
		} else 
		if(preg_match("/^tiff?$/i",$n)) {
			$r['tiff'] = $f ;
		} else 
		if(preg_match("/^xmdf$/i",$n)) {
			$r['xmdf'] = $f ;
		} else 
		if(preg_match("/^dotbook$/i",$n)) {
			$r['dotbook'] = $f ;
		} else 
		if(preg_match("/^epub3?$/i",$n)) {
			$r['epub3'] = $f ;
		} else 
		if(preg_match("/^workfiles?$/i",$n)) {
			$r['workfiles'] = $f ;
		} else 
		if(preg_match("/^samples?$/i",$n)) {
			$r['sample'] = $f ;
		} else if(!preg_match("/^[a-z0-9_]+$/i",$n)){
			$r['title'] = $f ;
		} 
	}
	return $r ;
}
function titledir2($path) {
	global $mnt ;
	$d = new dirpath($mnt) ;
	$o = $d->getdir($path,3) ;
	$r = array() ;
	for($i=0;$i<count($o);$i++) {
		$f = $o[$i] ;
		$n = mb_convert_kana($f['n'],"a") ;
		if($f['t']!="d" || mb_substr($n,0,1)=="★") continue ;
		$s = array('n'=>$f['n']) ;
		if(preg_match("/^h1$/i",$n)) {
			for($c=0;$c<count($f['c']);$c++) {
				if(preg_match("/jpe?g$/i",$f['c'][$c]['n'])) $s['c'][] = $f['n']."/".$f['c'][$c]['n'] ;
			}
			$r['h1'][] = $s ;
		} else 
		if(preg_match("/^cover$/i",$n)) {
			for($c=0;$c<count($f['c']);$c++) {
				if(preg_match("/tiff?$/i",$f['c'][$c]['n'])) $s['c'][]  = $f['n']."/".$f['c'][$c]['n'] ;
			}
			$r['cover'][] = $s ;
		} else 
		if(preg_match("/^tiff?$/i",$n)) {
			$s['c'] = count($f['c']) ;
			$r['tiff'][] = $s ;
		} else 
		if(preg_match("/^xmdf$/i",$n)) {
			for($c=0;$c<count($f['c']);$c++) {
				if(preg_match("/zbf$/i",$f['c'][$c]['n'])) $s['c'][] = $f['n']."/".$f['c'][$c]['n'] ;
			}
			$r['xmdf'][] = $s ;
		} else 
		if(preg_match("/^dotbook$/i",$n)) {
			for($c=0;$c<count($f['c']);$c++) {
				if(preg_match("/book$/i",$f['c'][$c]['n'])) $s['c'][] = $f['n']."/".$f['c'][$c]['n'] ;
			}
			$r['dotbook'][] = $s ;
		} else 
		if(preg_match("/^epub3?$/i",$n)) {
			for($c=0;$c<count($f['c']);$c++) {
				if(preg_match("/epub$/i",$f['c'][$c]['n'])) $s['c'][] = $f['n']."/".$f['c'][$c]['n'] ;
			}
			$r['epub3'][] = $s ;
		} else 
		if(preg_match("/^workfiles?$/i",$n)) {
			$s['c'] = count($f['c']) ; 
			$r['workfiles'][] = $s ;
		} else 
		if(preg_match("/^sample/i",$n)) {
			// sampleの下のboox,zbf,epubをさがす
			for($c=0;$c<count($f['c']);$c++) {
				if($f['c'][$c]['t']=="d") {
					for($cc=0;$cc<count($f['c'][$c]['c']);$cc++) {
						if(preg_match("/(book|zbf|epub)$/i",$f['c'][$c]['c'][$cc]['n'])) $s['c'][] = $f['n']."/".$f['c'][$c]['n']."/".$f['c'][$c]['c'][$cc]['n'] ;
					}
				} else if(preg_match("/(book|zbf|epub)$/i",$f['c'][$c]['n'])) $s['c'][] = $f['n']."/".$f['c'][$c]['n'] ;
			}
			$r['sample'][] = $s ;
		} else if(!preg_match("/^[a-z0-9_]+$/i",$n)){
			$r['title'][] = $s ;
		} else {
			$r['other'][] = $s ;
		}
	}
	return $r ;

}


function merge_cp($s,$id,$fpm,$flag=true) {

	$src = "/mnt/nas04/".$s ;
	list($mnt,$path)= fullpath($id) ;
	$dest = $mnt."/".$path ;
	$kind = array() ;

	if(file_exists($dest)) {
		fputs($fpm, "merge\t$src\t$dest ") ;
//		$r = titledir2($dest) ;
		echo "merge\n" ;
//		print_r($r) ;
		
	} else {
		fputs($fpm, "copy \t$src\t$dest ") ;
		echo "mkdir $dest\n" ;
		if($flag) system("mkdir $dest" ) ;		
	}

	if(preg_match("/修正納品_ワイズネット/",$src)) {
		$sd = new dirpath($src,0) ;
		$sdir = $sd->getdir(".") ;
//		print_r($sdir);
		for($i=0;$i<count($sdir);$i++) {
			$td  = null ;
			if($sdir[$i]['t']=="jpg") {
				$td = "h1" ;
			} else if(preg_match("/^sample_/",$sdir[$i]['n'])) {
				$td = "sample" ;
			} else if($sdir[$i]['t']=="epub" ) {
				$td = "epub3" ;
				$kind['epub3'] = 1 ;
			}else if($sdir[$i]['t']=="book" ) {
				$td = "dotbook" ;
				$kind['dotbook'] = 1 ;
			}else if($sdir[$i]['t']=="zbf" ) {
				$td = "xmdf" ;
				$kind['xmdf'] = 1 ;
			}else if($sdir[$i]['t']=="xls" ) {
				$td = "" ;
				$kind['xls'] = 1 ;
			}
			if($td==null) continue ;
			echo "cp $src/".$sdir[$i]['n']." -> $dest/".$td."\n" ;
			if($flag) copy($src."/".$sdir[$i]['n'], $dest."/".$td."/".$sdir[$i]['n']) ;
		}
		
	} else {
	
		$sdir = titledir2($src) ;
//		echo "src $src" ; print_r($sdir);
	
		$c =  "'".$src."/' '".$dest."/'" ;
		$dir = new dirpath("/mnt/nas04/") ;
	
		foreach($sdir as $k=>$v) {
			for($i=0;$i<count($v);$i++) {
				$d = $k ;
				if($k=="other"||$k=="title") $d = $v[$i]['n'] ;
				if($k=="other") {
					echo "illegal folder " ;
					fputs($fpm,"illegal folder $d ") ;
				}
				if($k=="h1") {
					$r = $dir->getdir($s."/".$v[$i]['n'],1) ;
					for($j=0;$j<count($r);$j++) {
						if(preg_match("/jpe?g$/i",$r[$j]['n'])) {
							$sp = $src."/".$v[$i]['n']."/".$r[$j]['n'] ;
							$dp = $dest."/".$d."/".$id.".jpg" ;

							if($flag)  {
								@mkdir($dest."/".$d) ;
								$ret =  exec("/usr/local/bin/rdjpgcom -verbose '".$sp."'") ;
								if($ret !="JPEG process: Baseline" ) {
									exec("/usr/local/bin/jpegtran -outfile '".$dp."' '".$sp."'") ;
									echo "convert progressive $sp to $dp \n" ;
								} else {
							echo "copy $sp to $dp\n" ;
									copy($sp,$dp) ;
								}
							}		
							break ;
						}
					}
				} else {
					echo "copy '".$src."/".$v[$i]['n']."/' to '".$dest."/".$d."'\n" ;
					if($flag)  system( "cp -r '".$src."/".$v[$i]['n']."/' '".$dest."/".$d."'") ;
				}
	
				if($k=="dotbook") $kind['dotbook'] = 1 ;
				if($k=="xmdf") $kind['xmdf'] = 1 ;
				if($k=="epub3") $kind['epub3'] = 1 ;
				if($k=="sample") {
					$r = searchdir($src."/".$v[$i]['n'],"/(epub|book|zbf)$/i",2) ;
					for($s=0;$s<count($r);$s++) {
						if(preg_match("/epub/i",$r[$s])) $kind['epub3'] = 1 ;
						if(preg_match("/book/i",$r[$s])) $kind['dotbook'] = 1 ;
						if(preg_match("/zbf/i",$r[$s])) $kind['xmdf'] = 1 ;
					}
				}
			}
		}
	
	}
	fputs($fpm,join(",",array_keys($kind))."\n") ;

	return $kind ;
}



function septag($str,$tags) {
	$r = preg_match_all("|<([^>]+)>|",$str,$m) ;
//	print_r($m[1]) ;
	$t = array() ;
	for($i=0;$i<count($m[1]);$i++) {
		if(preg_match("|^(/?)([a-z0-9\-]+)(.*)$|i",$m[1][$i],$a)) {
			$tt = array('tag'=>strtolower($a[2]),'se'=>(($a[1]=="/")?"E":"S")) ;
			if($tags && array_search($tt['tag'],$tags)===FALSE) continue ;
			if($a[3]) {
				if( preg_match_all("|([a-z0-9]+)=([^ $]+)|i",$a[3],$at)) {
					$tt['attr'] = array() ;
					for($j=0;$j<count($at[0]);$j++) {
						$tt['attr'][$at[1][$j]] = str_replace("'","",str_replace('"',"",trim($at[2][$j]))) ;
					}
				}
			}
			$t[] = $tt ;	
		}

	}
	return $t ;
}


function mega($s ) {
	$m = $s/1000/1000 ;
	return floor($m*100)/100.0 ;
}
function chksize($s,$reflow) {
	if($reflow) {
		if($s<0.02) return -1 ;
		if($s>10) return 1 ;
	} else {
		if($s<1) return -1 ;
		if($s>200) return -1 ;
		if($s>100) return 1 ;
	}
	return 0 ;
}



function getbase2() {
	global $mnt ;
	$o = new dirpath ;
	$o->sjis = false;
	$r = $o->getdir($mnt,1) ;

//echo "<pre>";
	$rr = array() ;
	$tc = 0 ;
	for($s=0;$s<count($r);$s++) {
		$ss = $r[$s]['c'] ;
		if($r[$s]['t']!="d") continue ;

		$ddd = array() ;
		for($d=0;$d<count($ss);$d++) {
			if($ss[$d]['t']!="d") continue ;
//			if(!preg_match("/^20([0-9]{2})([0-9]{2})([0-9]{2})/",$ss[$d]['n'])) continue ;
			$ss[$d]['mt'] = date('m/d H:i:s',$ss[$d]['s']);
			$ss[$d]['path'] = urlencode($r[$s]['n']."/".$ss[$d]['n']) ;
			$ddd[] = $ss[$d] ;
		}
		hsort($ddd,"n");
		$r[$s]['c'] = $ddd ;
		$rr[] = $r[$s] ;
	}
	hsort($rr,"n");
//print_r($rr);
	return $rr ;
}

function searchdir($path,$exp,$level) {
	$o = new dirpath ;
	$d = $o->getdir($path,$level) ;
	$l = array() ;
	for($i=0;$i<count($d);$i++) {
		if($d[$i]['t']=="d") {
			if($level==0) continue ;
			$d1 = $d[$i]['c'] ;
			for($ii=0;$ii<count($d1);$ii++) {
				if($d1[$ii]['t']=="d") {
					if($level==1) continue ;
					$d2 = $d1[$ii]['c'] ;
					for($iii=0;$iii<count($d2);$iii++) {
						if($d2[$iii]['t']!="d") {
							if(preg_match($exp,$d2[$iii]['n'])){
								$l[] = $d[$i]['n']."/".$d1[$ii]['n']."/".$d2[$iii]['n'] ;
							}							
						}
					}
					
				} else {
					if(preg_match($exp,$d1[$ii]['n'])){
						$l[] = $d[$i]['n']."/".$d1[$ii]['n'] ;
					}
				}
			}
			
		} else {
			if(preg_match($exp,$d[$i]['n'])) {
				$l[] = $d[$i]['n'] ;
			}
		}
	}
	return $l ;
}



class dirpath {
	var $sjis ;
	var $mnt ;
	
	function dirpath($m="") {
		global $mnt ;
		if($m!="") $this->mnt = $m ;
		else $this->mnt = $mnt ;
	}

	function getdir($base,$level=0) {
		if(substr($base,0,1)!="/") $base = $this->mnt."/".$base ;
		$h = @opendir($base) ;
		$d = array() ;
		if(!$h) {
			echo "cannot open $base";
			return null ;
		}
		while(($f= readdir($h)) !==false ) {
			$ff = $base."/".$f ;
			if(substr($f,0,1)=="." || substr($f,0,1)=="_" || substr($f,0,1)==":") continue;
			$type = "f" ;
			if(preg_match("/\.([a-z0-9]+)$/i",$f,$a)) $type = strtolower($a[1]) ;
			$c = null ;
			if(is_dir($ff)) {
				$type = "d" ;
				if($level>0) {
					$c = $this->getdir($ff,$level-1) ;
				}
			} else {
				$size = filesize($ff);
			}
			$dd = array("n"=>$this->cconv($f),"t"=>$type,'s'=>filectime($ff)) ; 
			if($type!="d") $dd['v'] = $size ;
			else $dd['c'] = $c ;
			$d[] = $dd ;
		}
		closedir($h) ;
		
		return $d ;
	}
	
	function cconv($a) {
		if($this->sjis) return mb_convert_encoding($a,"UTF-8","SJIS-win") ;
		else return $a ;
	}
}



function hsort(&$d,$key) {
	usort($d,function($a,$b) use($key){
		if($a[$key]==$b[$key]) return 0 ;
		return ($a[$key]>$b[$key])?1:-1 ;
	}) ;
}

class slist {
	var $path = "/mnt/nas06/list/" ;
	var $l ;
	
	function slist($f) {
			$this->l = array() ;
		$fp = @fopen($this->path.$f,"r") ;
		if($fp==null) return ;

		while($l = fgets($fp)) {
			$l = explode(",",trim($l)) ;
			$this->l[] = strtoupper($l[0]) ;	
		}
		sort($this->l) ;
	}


	function is_list($c) {
		$c = strtoupper($c) ;
		for($i=0;$i<count($this->l);$i++) {
			if($this->l[$i]==$c) return true ;
		}
		return false ;
	}


}