<?php
function getgaiji($id,$ssid) {
	list($mnt,$path) = fullpath($id) ;
	$file = "epub3/$id.epub" ;
	if($ssid=="") $ssid = getmypid() ;
	@system("rm -r tmp/".$ssid);
	@mkdir("tmp/".$ssid) ;
	@mkdir("tmp/".$ssid."/package") ;
	$basedir = "tmp/".$ssid."/package/" ;
	$r = epub($mnt."/".$path."/".$file,$ssid) ;
	if($r==null) {
//		echo "epub open error" ;
		reutnr -1 ;
	}
	$epub = $r['epub'] ;
//echo "<pre>";
//print_r($r['fs']) ;
	$fc = 0 ;
	$g = array() ;	
	for($j=0;$j<count($r['fs']);$j++) {
		@$fp = fopen($basedir.$r['fs'][$j],"r") ;
		if(!$fp) continue;
		$ch = new chcheck ;
		while($l = fgets($fp)) {
			$ch->putstr($l) ;
		}
		fclose($fp);
		if(!$ch->img) continue ;
		
		$fpp = pathinfo($r['fs'][$j]) ;

		for($i=0;$i<count($ch->img);$i++) {
			$im = $ch->img[$i] ;
			
			if(preg_match("|class=\"([^\"]+)\"|i",$im,$m)) {
				if(preg_match("|gaiji|i",$m[1])) {
					if(preg_match("|src=\"([^\"]+)|i",$im,$mm)) {
						$path = $mm[1] ;
						$pi = pathinfo($path) ;
						$g[$fpp['dirname']."/".$path] = array('path'=>$path,'name'=>$pi['basename']) ;
					}
				}
			}
		}
	}
	return $g ;
}

function gaijilog($id,$tdir) {
	$g = getgaiji($id,"") ;
	if($g<0) return false ;
	if(count($g)==0) return false ;
	$ssid = getmypid() ;
	if(!file_exists($tdir)) mkdir($tdir) ;
	$fp = fopen("$tdir/index.html","w") ;
	
$head = <<<'EOD'
<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>meta{rval id}</title>
<style type="text/css">
table {
	border-collapse: collapse ;	
}
div.base {

}
div.g {
	float:left ;
	border:1px dotted black ;
	margin:10px ;
}
div.g img {
	background-color:#fffb80 ;
}
</style>
</head>
<body>
<div class=base>
EOD;
	$head .="<h1>$id</h1>" ;
	fputs($fp,$head) ;
	
	ksort($g) ;
	foreach($g as $p=>$v) {
		$f = "tmp/$ssid/package/$p" ;
		$r = png_info($f) ;
//	print_r($r);
		if($r<=0) {
			$r = array('height'=>" not png",'bit'=>$r);
		} 
		if($r['chunk']['tRNS']) $r['color'].="T";
		
		fputs($fp, '<div class=g><img src="'.$v['name'].'"/><br/>'.$v['name'].'<br/>'.$r['width']."x".$r['height']." ".$r['bit']."-".$r['color'].'</div>') ;
		copy($f,$tdir."/".$v['name']);

	}
	
	fclose($fp) ;

}
function png_info($f)
{
	$c = array("G","","F","I","GA","","FA") ;
	$fp = fopen($f,"r");
	if(!$fp) return 0 ;
	$bin = fread($fp,33);

    $r = unpack('N2h/Nlength/A4ihdr/Nwidth/Nheight/Cbit/Ccolor/Ccompress/Cfilter/Cinterlace/Ncrc', $bin);

 //   if($r['h1']!=2303741511 || $r['h2']!=218765834) return -1 ;
    if($r['h2']!=218765834) return -1 ;

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