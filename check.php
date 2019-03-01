<?php
require_once "env.inc" ;
require_once "OhbaTemplate.inc" ;
require_once "dcheck.inc" ;


putenv("LANG=ja_JP.UTF-8") ;
setlocale(LC_CTYPE, "ja_JP.UTF-8");

session_start() ;

$ssid = ($_SERVER['QUERY_STRING'])?($_SERVER['QUERY_STRING']):session_id() ;

$tdir = $tmpdir.$ssid;


if($_POST && $_POST['upload']) {

	if($_FILES['f1'] ) {
		$f1 = $_FILES['f1'] ;
		if( $f1['error']==0 ) {
			$src = $f1['tmp_name'] ;
			$_SESSION['file_name'] = $_FILES['f1']['name'] ;
			$_SESSION['isbn'] = $_REQUEST['isbn'] ;
			docheck($src,$tdir) ;
		 
		} else {
				
			$t = new TemplateComponent("complete_t.html",array("error"=>"アップロードエラー")) ;
			echo $t->getString() ;
		}
	}
} else {
	docheck("",$tdir) ;
}

function docheck($src,$tdir) {

	$isbn = $_SESSION['isbn'] ;

	$r = check($src,$tdir,$isbn) ;
	if($r<0) {
		$t = new TemplateComponent("complete_t.html",array("error"=>"正しいEPUBファイルではないようです ")) ;
		echo $t->getString() ;		
	}
	$arg = $r ;
	$arg['fname'] =$_SESSION['file_name'] ;
	$arg['datetime'] = date('Y-m-d H:i:s') ;

	savelog($arg) ;
	
	// html output
	
	$t = new TemplateComponent("lib/check_t.html",$arg) ;
	$html =  $t->getString() ;
	$f = fopen($tdir."/index.html","w") ;
	fputs($f,$html) ;
	fclose($f) ;
	//copy cover 
	@mkdir($tdir."/img");
	if($r['opfcheck']['c']['cover']['img']) {
		copy($e['basedir'].$e['opfpath'].$r['opfcheck']['c']['cover']['img'],$tdir."/img/".$r['opfcheck']['c']['cover']['imgname']);

	}
	
	$zname = str_replace(".epub","_check.zip",$_SESSION['file_name']) ;
	exec("cd $tdir ;zip -r $zname index.html img");
	
//	echo $html ;
	//delete file
	exec("rm -rf '".$tdir."/package"."'") ;
	
	$arg = array('tmp'=>$tdir,'fname'=>$_SESSION['file_name'],'zname'=>$zname) ;
	$t = new TemplateComponent("complete_t.html",$arg) ;
	echo $t->getString() ;	
}

function savelog($log) {
	global $logpath ;
	$dn = date('Ymd');
	$fn = date('His') ;
	@mkdir("$logpath/$dn");
	$fp = fopen("$logpath/$dn/$dn$fn","w" ) ;
	fputs($fp,serialize($log)) ;
	fclose($fp) ;
}

