<?php
require_once "epub.inc" ;
require_once "chc.inc" ;
require_once "lib_gaiji.inc" ;
require_once "egrep.inc" ;
require_once "ndlsearch.inc" ;
require_once "ar_dump.inc" ;
require "ctlist.inc" ;

$version = "1.1.2" ;
function check($src,$tdir,$isbn) {
	global $version ;
	// epub open
	$epub = new epub ;
	if( $epub->getepub($src,$tdir)===null) {
		return -1 ;
	} ;
	
	// epub basic check 
	$e = $epub->checkepub() ;
	if( $e==null ) {
		return -1 ;
	}
	// meta info format
	epubmeta($e) ;
	$meta = $e['meta'] ;
	
	//NDLsearch 
	if($isbn) {
		$ndl = ndlsearch(array('isbn'=>$isbn)) ;
	}

	$r = array() ;
	
	// epub validator
	$r = epubcheck($e) ;
	
	//opf checker ;
	$r = array_merge($r, check_opf($e) );

	// check fix epub
	if($e['fixed']) {
		$r = array_merge($r,check_fix($e)) ;
	}
	
	// check character
	$r = array_merge($r,check_code($e) );
	
	// check tags 
	$r = array_merge($r,ctext($tdir)) ;
	
	$arg = array('version'=>$version,'tdir'=>$tdir,'isbn'=>$isbn,'e'=>$e,'c'=>$r) ;
	if($ndl) $arg['ndl'] = $ndl ;
//ar_dump($arg);

	
	return $arg ;
}


// execute epubcheck 
function epubcheck($epub) {

	global $java,$epubcheck ;
	
	$c = "$java -jar $epubcheck -mode exp '".$epub['basedir']."' 2>&1";
//	echo $c ;
	exec($c,$out) ;
//	print_r($out) ;
	$st = 0 ;
	
	if(!preg_match("/Epubcheck Version 3.0/",$out[0])) $st = -1 ;
	if(preg_match("/No errors or warnings detected./",$out[3])) $st = 1 ;

	$ret = array('epubcheck'=>array(
		'result'=>join("\n",$out)
	));
	if($st==1) $ret['epubcheck']['ok'] = 1 ;
	else $ret['epubcheck']['ng'] = 1 ;
	
	return $ret ;
}

// epub meta formatting 
function epubmeta(&$e) {
	$meta = $e['meta'] ;
	$opf = pathinfo($e['opf']) ;	
	$fs = array($e['opf']) ;
	$types = array("application/xhtml+xml","text/css","text/javascript","application/x-dtbncx+xml");
	$images = array("image/jpeg","image/png","image/gif") ;

	$ref = array() ;
	if($meta['meta']['refines']) {
	foreach($meta['meta']['refines'] as $k=>$v) {
		if($v) {
			foreach($v as $id=>$vv) {
				$ref[substr($id,1)][$k] = $vv ;
			}
		}
	}
	foreach($meta as $id=>$v) {
		if(!$v[0]) {
			if($ref[$v['id']]) $e['meta'][$id]['refines'] = $ref[$v['id']] ;	
		
		} else {
			for($i=0;$i<count($v);$i++) { 
				if($ref[$v[$i]['id']]) $e['meta'][$id][$i]['refines'] = $ref[$v[$i]['id']] ;
			}
		}
	}
	}

	foreach($e['item'] as $id=>$v) {
		if( array_search($v['media-type'],$types)!==false) {
			$fs[] = $opf['dirname']."/".$v['href'];
		}
		if(array_search($v['media-type'],$images)!==false) {
			$img[] = $opf['dirname']."/".$v['href'];
		}
	}
	foreach($e['spine'] as $id=>$v) {

			$sp[] = $opf['dirname']."/".$v['idref']['href'];
	}
	$e['files'] = array('sp'=>$sp,'fs'=>$fs,'img'=>$img) ;

}

function check_opf($epub) {

	$c = array() ;
//echo "<pre>";print_r($epub) ;
	$meta = $epub['meta'] ;
	$status = 1 ;
	
//print_r($meta) ;
	if(!$meta['meta']['ebpaj:guide-version']) {
		$status = -1;
		$c['dver'] = array("stat"=>"wn","msg"=>"電書協バージョンタグ無し" ) ;
	} else $c['dver'] = array("stat"=>"ok","msg"=>$meta['meta']['ebpaj:guide-version']['_content'] );
	
	if(!$meta['dc-identifier']) {
		$status = -2 ;
		$c['id'] = array("stat"=>"ng","msg"=>"IDタグがありません" );
	} else {
	
		if($meta['dc-identifier'][0]) $meta['dc-identifier']= $meta['dc-identifier'][0] ;
		
		
		$c['id'] = array('stat'=>"ok",'msg' => "ID種別: ".$meta['dc-identifier']['id']."<br/> ID:" .$meta['dc-identifier']['_content'])  ;
		
		if($meta['dc-identifier']['id']=="jpecode") {
			if(!preg_match("/^[0-9a-z]{20}$/i",$meta['dc-identifier']['_content'])) {
				$c['id'] = array('stat'=>"ng","msg"=>"JP-Eコードのフォーマットが違います( ".$meta['dc-identifier']['_content'].")") ;
			}
		}
		
		if(preg_match("/sample/",$file) && $meta['dc-identifier']['_content']=="sample_".$id) {
			$status = -1 ;
			$c['id'] = array('stat'=>"wn","msg"=>"OPF Warning (2) jpecodeがsample付き '".$meta['dc-identifier']['_content']."'" );	
		} 		
	}  
	
	if(!$meta['dc-creator']) {
		$status = -2 ;
		$c['creator'] = array('stat'=>"ng",'msg'=>"著者タグがありません") ;
	} else if(!$meta['dc-creator'][0] && $meta['dc-creator']['_content']=="") {
		$status = -2 ;	
		$c['creator'] = array('stat'=>"ng",'msg'=>"著者タグの内容が空白です") ;
	} else {
		$c['creator'] = array('stat'=>"ok",'msg'=>$meta['dc-creator']['_content']) ;
		if($meta['dc-creator'][0]) {
			$c['creator']['msg'] = "" ;
			for($i=0;$i<count($meta['dc-creator']);$i++) $c['creator']['msg'] .= $meta['dc-creator'][$i]['_content'].", " ;
		}
	}
	
	if(!$meta['dc-publisher']) {
		$status = -2 ;
		$c['publisher'] = array('stat'=>"ng",'msg'=>"出版社タグがありません") ;

	} else if(!$meta['dc-publisher'][0] && $meta['dc-publisher']['_content']=="") {
		$status = -2 ;
		$c['publisher'] = array('stat'=>"ng",'msg'=>"出版社タグが空白です") ;	
	} else {
		$c['publisher'] = array('stat'=>"ok",'msg'=>$meta['dc-publisher']['_content']) ;
		if($meta['dc-publisher'][0]) {
			$c['publisher']['msg'] = "" ;
			for($i=0;$i<count($meta['dc-publisher']);$i++) $c['publisher']['msg'] .= $meta['dc-publisher'][$i]['_content'].", " ;
		}		
	}
	
	if(!$meta['dc-title']) {
		$status = -2 ;
		$c['title'] = array('stat'=>"ng",'msg'=>"タイトルタグがありません") ;
	} else if(!$meta['dc-title'][0] && $meta['dc-title']['_content']=="") {
		$status = -2 ;
		$c['title'] = array('stat'=>"ng",'msg'=>"タイトルタグが空白です") ;	
	} else if(!preg_match("/sample/",$file) && preg_match("/試/",$meta['dc-title']['_content'])) {
		$status = -2 ;
		$c['title'] = array('stat'=>"wn",'msg'=>"タイトルに'試'の文字が含まれています。") ;			
	} else {
		$c['title'] = array('stat'=>"ok",'msg'=>$meta['dc-title']['_content'] ) ;
		if($meta['dc-title'][0]) {
			$c['title']['msg'] = "" ;
			for($i=0;$i<count($meta['dc-title']);$i++) $c['title']['msg'] .= $meta['dc-title'][$i]['_content'].", " ;
		}
	}
	
	
	$item = $epub['item'] ;
	foreach($item as $k=>$v) {
		if(preg_match("/cover-image/i",$v['properties'])) {
			$cov = $v ;
		}
	}
	if($meta['cover']) {
		$c['cover'] = array('stat'=>"ok",'msg'=>"カバー画像あり","img"=>$meta['cover']['href']) ;
	} else if($cov) {
		$c['cover'] = array('stat'=>"ok",'msg'=>"カバー画像あり","img"=>$cov['href']) ;

	} else {
		$status = -1 ;
		$c['cover'] = array('stat'=>"wn",'msg'=>"カバー画像なし" );

	}
	if($c['cover']['img']) {
		$cv = pathinfo($c['cover']['img']) ;
		$c['cover']['imgname'] = $cv['basename'] ;
	}
	
	$ret = array( 'opfcheck'=>array(
		'st'=>$status,
		"c"=>$c
	));
	
	if($status == -2 ) $ret['opfcheck']['ng'] = 1 ;
	else if($status == -1) $ret['opfcheck']['wn'] = 1 ;
	else $ret['opfcheck']['ok'] = 1 ;
	
	return $ret ;
}




function check_code($epub) {
	
	$basedir = $epub['basedir'] ;
	$fs = $epub['files']['fs'] ;
	$msg = array() ;
	
	$ch = new chcheck ;
//	$ch->striptags = true ;
	$fc = 0 ;	
	$bom = array() ;
	for($i=0;$i<count($fs);$i++) {
		$fp = fopen($basedir.$fs[$i],"r") ;
		if(!$fp) continue;
		$f = true  ;
		while($l = fgets($fp)) {
			if($f) {
				if( substr($l,0,3) == "\xef\xbb\xbf" ) {
					$bom[] = $fs[$i] ;
				}
				
				$f = false ;
			}
			$ch->putstr($l,$fs[$i]) ;
		}
		$fc++ ;
	}
	
	$sum = $ch->getsum() ;

	$wch = $ch->wch ;
	$wch['bom'] = $bom ;
//ar_dump($wch) ;
	$sum['wch'] =$wch ;
	if($ch->img) {
		$g = array() ;
		for($i=0;$i<count($ch->img);$i++) {
			$im = $ch->img[$i] ;
			if(preg_match("|class ?= ?\"([^\"]+)\"|i",$im,$m)) {
				if(preg_match("|gaiji|i",$m[1])) {
					if(preg_match("|src ?= ?\"([^\"]+)|i",$im,$mm)) {
						$path = $mm[1] ;
						$g[$path]++ ; ;
					}
				}
			}
		}
		$sum['gaiji'] = $g ;
	}
//print_r($sum['h']) ;
	
	$ch_warn = array("bom","cc","sgp","ref_sgp","kigo","ref_kigo","kanji","ref_kanji") ;
	$ch_err = array("usp","ref_usp","ivs","ref_ivs") ;
//print_r($wch);
	if($wch) {
		$wch['ok'] = 1 ;
		foreach($ch_warn as $c) {
			if($wch[$c]) {
				unset($wch['ok']) ;
				$wch['wn'] =1 ;
			}
		}
		foreach($ch_err as $c) {
			if($wch[$c]) {
				unset($wch['wn']) ;
				$wch['ng'] =1 ;
			}
		}
		if($wch['ref_sgp']) $wch['sgp'] = array_merge( (array)$wch['ref_sgp'], (array)$wch['sgp']) ;
		if($wch['ref_kigo']) $wch['kigo'] = array_merge( (array)$wch['kigo'], (array)$wch['ref_kigo']) ;
		if($wch['ref_kanji']) $wch['kanji'] = array_merge( (array)$wch['kanji'], (array)$wch['ref_kanji']) ;
		if($wch['ref_usp']) $wch['usp'] = array_merge( (array)$wch['usp'], (array)$wch['ref_usp']) ;
		if($wch['ref_ivs']) $wch['ivs'] = array_merge( (array)$wch['ivs'], (array)$wch['ref_ivs']) ;
	}

	
//	print_r($sum['gaiji']);
//	print_r($msg) ;
	$u2c = file("lib/cdb.dat") ;
	$u = unserialize(join("",$u2c)) ;
	$r = $sum ;
//ar_dump($u) ;
	if($wch) {
	foreach($wch as $k=>$l) {
		switch($k) {
			case 'sgp':
			case 'kigo':
			case 'kanji':
				for($i=0;$i<count($l);$i++) {
					$db = $u[substr("0".$l[$i],-5)] ;
					$c = "j" ;
					if($db['j']) $c = "cc_j" ;
					else if($db['a']) $c = "cc_a" ;
					$wch[$k][$i] = array('u'=>$l[$i],'c'=>$c) ;
					
				}
		}
	}
	}
/*
	foreach($r as $k=>$l) {
		if($k=="gaiji") {
			$g = $l  ;
			continue ;
		}
		for($i=0;$i<count($l);$i++) {
			$c = $l[$i] ;
			if($c['class']=="") {
				if($u[strtolower($c['u'])]['cid'] ) {
					$cc = $u[strtolower($c['u'])]['cid'][0] ;
					$r[$k][$i]['class'] = "aj".$cc[1] ;

					$r[$k][$i]['j'] = $cc[0]." (".$cc[1].")" ;
				}
				if($k=="k") $kw[] = $r[$k][$i] ;
			}
				
			$ch = $c['c'] ;
			if($k=='n'&&!(($ch>='ぁ'&&$ch<='ん')||($ch>='ァ'&&$ch<='ヶ'))) $kk[] = $r[$k][$i] ; 
		}
	}
	$r['kw'] = $kw ;
	$r['kk'] = $kk ;
*/
	$g = $r['gaiji'] ;

	if($g) {
		$tdir = $basedir."/../img/" ;
		@mkdir($tdir ) ;
		$gl = array() ;
		$gp = $epub['files']['img'] ;
		foreach($g as $d=>$n) {
			$dp = pathinfo($d) ;
			$d = "" ;
			for($i=0;$i<count($gp);$i++) {
				if(strpos($gp[$i],$dp['filename'])!==false) {
					$d = $gp[$i] ;
					break ;
				}
			}
			if($d=="") continue ;
			$f = $epub['basedir'].$d ;
			$png = png_info($f) ;
			if($png<=0) {
				$png = array('height'=>" not png",'bit'=>$png);
			}
			$pi = pathinfo($d) ;
			if($png['chunk']['tRNS']) $png['color'].="T";
			$png['name'] = $pi['basename'] ;
			$png['path'] = $d ;
			unset($png['chunk']);
			$gl[] = $png ;
			copy($f, $tdir.$pi['basename']) ;
		}
	}
//	print_r($gl);
//print_r($gl);

	$ret = array() ;
	if($wch) $ret['chc']=$wch ;
	if(count($gl)>0) $ret['gaiji'] = $gl ;
//ar_dump($ret);
	return $ret ;
}




function check_fix($epub) {
		//check fixformat 
	$sp = $epub['files']['sp'] ;
	$basedir = $epub['basedir'] ;
	$msg = array() ;
	$ec = array() ;

	$imx = $imy = null ;
	$ve = array() ;
	$ie = array() ;
	$noimg = array() ;
	$c = 0 ;
	$page =array() ;
	
	for($i=0;$i<count($sp);$i++) {
		$p = $sp[$i]; 
//			echo "\n".$basedir.$p ;
		$f = file($basedir.$p) ;
		$pi = pathinfo($p) ;		
		$w = $h = null;

		for($l=0;$l<count($f);$l++) {
			if(preg_match("/viewport/i",$f[$l])) {
				if(preg_match("/width *= *['|\"]?([0-9]+)/i",$f[$l],$a)) $w = $a[1] ;
				if(preg_match("/height *= *['|\"]?([0-9]+)/i",$f[$l],$a)) $h = $a[1] ;
			}
			if(preg_match("/<image/i",$f[$l])||preg_match("/<img/i",$f[$l])) {

				if(preg_match("/href *= *['|\"]?([^'\"]+)/i",$f[$l],$a)) {
					$im = getimagesize($basedir.$pi['dirname']."/".$a[1]) ;
//		print_r($im);
					if($imx!=null) {
						if($imx!=$im[0] || $imy!=$im[1]) {
							$ie[] = $a[1] ;
						}
					}
					$page[] = array('n'=>$a[1],'w'=>$im[0],'h'=>$im[1],'vw'=>$w,'vh'=>$h) ;
					$imx = $im[0] ;
					$imy = $im[1] ;
					$c++ ;
				} else {
					$noimg[] = $p ;
				}
			}

		}
		if($w==null && $h==null) {
			$ve[] = $p ;			
		}
		
	}
	$ret = array('pages'=>$page ) ;

	if(count($ve)>0) {
		$ret['novp'] = array('stat'=>"ng",'msg'=>"XHTML Error viewport無し <br/> ".join(", ",$ve)) ;
		$st = -2 ;
	} else $ret['novp'] = array('stat'=>"ok",'msg'=>"Viweportは問題ありません" ) ;
	if(count($noimg)>0) {
		$ret['noimg'] = array('stat'=>"ng",'msg'=>"imageタグのないページ <br/>".join(", ",$noimg) );
		$st = -2 ;
	} else $ret['noimg'] = array('stat'=>"ok",'msg'=>"imageタグは問題ありません") ;
	if(count($ie)>0) {
		$ret['imgsize'] = array('stat'=>"wn",'msg'=> "画像サイズ不一致 <br/>".join(", ",$ie)) ;
		$st = -1 ;
	} else $ret['imgsize'] = array('stat'=>"ok",'msg'=> "画像サイズは全て一致しています") ;
	
	if($st==0) $ret['ok'] = 1 ;
	else if($st==-1) $ret['wn'] = 1 ;
	else if($st==-2) $ret['ng'] = 1 ;
	return array('fixcheck'=>$ret) ;

}



function ctext($path) {
	global $ctlist;

	$arg = array() ;

	foreach(range(0,count($ctlist)-1) as $i) {
		$l = $ctlist[$i]['l'] ;
		$k = $ctlist[$i]['cat'] ;
		foreach(range(0,count($l)-1) as $j) {
			$v = $l[$j] ;
			
			$a= array('title'=>$v['title'],'stext'=>$v['stext'],'etext'=>$v['etext'],'count'=>0);
			if(($e = egrep($path,$v['ereg'],$v['opt'])) !=null ){
				$tc = 0 ;
				$a['e'] = $e ;
				for($i=0;$i<count($e);$i++) $tc+= count($e[$i]['m']) ;
				$a['count'] = $tc ;
			}
			
			$arg[$k][] = $a ; 
		}
	}

	return $arg ;

}

