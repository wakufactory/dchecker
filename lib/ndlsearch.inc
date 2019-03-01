<?php
require_once "xml.inc" ;
require_once "ar_dump.inc" ;
require_once "wget.inc" ;

function ndlsearch($p) {
	$host = "iss.ndl.go.jp" ;
	$path = "/api/sru" ;
	
	if(!$p['isbn']) return null ;
	$q = "isbn=".preg_replace("/isbn/i","",$p['isbn']) ;
	
	$param = array( 
		"operation"=>"searchRetrieve",
		"recordSchema"=>"dcndl",
		"onlyBib"=>"true",
		"query" => $q
	);
	
	$u = "http://".$host.setget(array('u'=>$path,'p'=>$param),null) ;
	$c = "curl '".$u."'" ;
//	exec($c,$out) ;
	$out = file_get_contents($u) ;
	$r = parsexml(array($out)) ;
	
	if($r['searchretrieveresponse']['numberofrecords']['_content']==0) return null  ;
	else {
		$rr = htmlspecialchars_decode($r['searchretrieveresponse']['records']['record']['recorddata']['_content']) ;
		$meta = parsexml(array($rr)) ;
//ar_dump($meta) ;
		return $meta['rdf-rdf']['dcndl-bibresource'] ;
	}

}
