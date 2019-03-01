<?php
if (realpath($_SERVER["SCRIPT_FILENAME"]) == realpath(__FILE__)){
	require_once "lib.inc" ;
	if($id = $argv[1]) {
		getmeta($id,true) ;
	}
}
///meta
function getmeta($id,$save=true) {
	global $mnt ;
	list($mnt,$path) = fullpath($id) ;	
	$r = titledir2($path) ;
//print_r($r) ;
	$e = $r['epub3'] ;
	$w = $r['workfiles'] ;
	$x = $r['xmdf'] ;

	$a = array('id'=>$id) ;
	if($e) {
		$pb = $path."/".$e[0]['c'][0] ;
		$b = epubmeta($pb) ;
	
		$e = $r['sample'] ;
		if($e) {
			for($i=0;$i<count($e[0]['c']);$i++) {
				if(preg_match("/epub$/i",$e[0]['c'][$i])) {
						$ps = $path."/".$e[0]['c'][$i] ;
						$s = epubmeta($ps) ;
						break ;
				}
			}
		}
		$a['epub'] = array('bp'=>$pb,'bm'=>$b,'sp'=>$ps,'sm'=>$s) ;
	}
	if($w) {
		$ttx = searchdir($mnt."/".$path."/".$w[0]['n'],"/ttx$/",3);
//		print_r($ttx) ;
		if(count($ttx)>0) {
			for($i=0;$i<count($ttx);$i++) {
				$n = $w[0]['n']."/".$ttx[$i] ;
				$m = ttxmeta($path."/".$n) ;
//				print_r($m); 
				
				if(preg_match("/sample|s_/i",$n)) {
					$a['ttx']['sp'] = $path."/".$n ;
					$a['ttx']['sm'] = $m ;
				} else {
					$a['ttx']['bp'] = $path."/".$n ;
					$a['ttx']['bm'] = $m ;					
				}
			}
		}
	}
	if($x) {
		$pb = $path."/".$x[0]['c'][0] ;
		$b = xmdfmeta($pb) ;

		$x = $r['sample'] ;
		if($x) {
			for($i=0;$i<count($x[0]['c']);$i++) {
				if(preg_match("/zbf$/i",$x[0]['c'][$i])) {
						$ps = $path."/".$x[0]['c'][$i] ;
						$s = xmdfmeta($ps) ;
						break ;
				}
			}
		}
		$a['xmdf'] = array('bp'=>$pb,'bm'=>$b,'sp'=>$ps,'sm'=>$s) ;	
//		print_r($a['xmdf']);
	}
//print_r($a) ;
	if($save) {
		umask(0) ;
		$fp = fopen("meta/metalog/".$id,"w") ;
		fputs($fp,serialize($a)) ;
		fclose($fp) ;
	}
	return $a ;
}

function ttxmeta($p) {
	global $mnt ;
	if(!file_exists($mnt."/".$p)) return null ;	
	$f = file($mnt."/".$p) ;
	$m = "" ;
	for($i=0;$i<count($f);$i++) {
		$l = mb_convert_encoding(trim($f[$i]),"UTF-8","sjis-win") ;
		$m .= $l ;
		if(preg_match("|</head>|i",$l)) break ;
	}
//	echo $m ;
	if(preg_match("|<title>(.+)</title>|i",$m,$a)) {
		$r['title'] = $a[1] ;
	}
	if(preg_match("|<T-TIME(.+)>|i",$m,$a)) {
		if(preg_match("|author=\"([^\"]+)\"|i",$a[1],$aa)) $r['author'] = $aa[1] ;
		if(preg_match("|publisher=\"([^\"]+)\"|i",$a[1],$aa)) $r['publisher'] = $aa[1] ;
	}
	$f = file($mnt."/".str_replace(".ttx",".prop",$p)) ;
	for($i=0;$i<count($f);$i++) {
		$l = mb_convert_encoding($f[$i],"UTF-8","sjis-win") ;
		if(preg_match("/title *\: *\"([^\"]+)\"/",$l,$a)) $r['p_title'] = $a[1] ;
		if(preg_match("/author *\: *\"([^\"]+)\"/",$l,$a)) $r['p_author'] = $a[1] ;
		if(preg_match("/publishcompany *\: *\"([^\"]+)\"/",$l,$a)) $r['p_publisher'] = $a[1] ;
		if(preg_match("/comment *\: *\"([^\"]+)\"/",$l,$a)) $r['p_comment'] = $a[1] ;
	}
	
	return $r ;
}
function epubmeta($p) {
	global $mnt ;
	if(!file_exists($mnt."/".$p)) return null ;
	$r = epub($mnt."/".$p,session_id()) ;
	$r = $r['epub'] ;
	$sp = $r['spine'] ;
	
	$spi = array() ;
	$c = 0 ;
	for($i=0;$i<count($sp);$i++) {
		$p = "-" ;
		if($sp[$i]['properties']) {
			if(preg_match("|page-spread-([a-z]+)|",$sp[$i]['properties'],$a)) {
				$p = strtoupper(substr($a[1],0,1)) ;
				$c++ ;
			} 
		} 
		$spi[] = $p ;
	}
	$r['spine'] = ($c==0)?null:$spi ;
	
	unset($r['item']) ;
	unset($r['css']) ;
	return $r ;
}
function xmdfmeta($p) {
	global $mnt ;
	$fn = $mnt."/".$p.".check.csv" ;
	if(!file_exists($fn)) return null ;
	$f = file($fn) ;
	$l = str_getcsv(mb_convert_encoding($f[1],"UTF-8","sjis-win")) ;
	$r['title'] = $l[32] ;
	$r['author'] = $l[33] ;
	$r['publisher'] = $l[34] ;	
	
	return $r ;
}

function png_info($f)
{
	$c = array("G","","F","I","GA","","FA") ;
	$fp = @fopen($f,"r");
	if(!$fp) return 0 ;
	$bin = fread($fp,33);

    $r = unpack('N2h/Nlength/A4ihdr/Nwidth/Nheight/Cbit/Ccolor/Ccompress/Cfilter/Cinterlace/Ncrc', $bin);
    if($r['h1']!=2303741511 || $r['h2']!=218765834) return -1 ;
    if($r['ihdr']!="IHDR") return -2 ;
    $r['color'] = $c[$r['color']] ;
    $chunk = array() ;
	while($h = fread($fp,8)) {
		$ch = unpack("Nlen/A4chunk",$h) ;
		$dat = fread($fp,$ch['len']+4) ;
		if($ch['chunk']=="tRNS"||$ch['chunk']=="PLTE") {
			$ch['dat'] = unpack("C*",substr($dat,0,-4)) ;
		}
		if($ch['chunk']=="tEXt") {
			$ch['text'] = unpack("A*",substr($dat,0,-4)) ;
			
		}
		$chunk[$ch['chunk']] = $ch ;
	}
	$r['chunk'] = $chunk ;
	fclose($fp) ;    
	if($r['chunk']['tRNS']) $r['trans'] = 1 ;
    return $r ;
}