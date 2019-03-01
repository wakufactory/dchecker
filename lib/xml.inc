<?php

//xml2array
function parsexml($f,$encode="",$flag="") {
	$str = "" ;
	if($encode=="") $encode="UTF-8" ;
	for($i=0;$i<count($f);$i++) {
		if(chop($f[$i])!="") $str .= mb_convert_encoding($f[$i],"UTF-8",$encode) ;
	}
  $x = xml_parser_create();
  if(xml_parse_into_struct($x,$str,$v,$i)!=1) {
		return xml_error_string(xml_get_error_code($x)) ;
	} else {

		return xml2array($v,$flag) ;
	}
}
function xml2array(&$v,$flag) {
	$r = array() ;
	while($n = array_shift($v)) {
	
		$tag = str_replace(":","-",strtolower($n[tag])) ;
		$type = $n[type] ;
		if($type=="cdata") continue;
		if( $type=="open" ) {
			$rr = xml2array($v,$flag) ;
			if(array_key_exists('attributes',$n)) foreach($n[attributes] as $k=>$g ) $rr[strtolower($k)] = $g ;
			if(!$r[$tag] )  $r[$tag] = $rr ;
			else if($r[$tag][0]) $r[$tag][] = $rr ;
			else { $t = $r[$tag] ;unset($r[$tag]);$r[$tag][0] = $t; $r[$tag][1] = $rr ; }
		}
		if( $type=="close" ) {
			return $r ;
		}
		if($type=="complete") {
			if($n[value]!="") {
				if($flag) $n[attributes] = $n[value] ;
			 	else $n[attributes][_content] = $n[value] ;
			}
			if(array_key_exists('attributes',$n)) {
				$a = array() ;
				if(is_array($n[attributes])) foreach($n[attributes] as $k=>$g) $a[strtolower($k)] = $g ;
				else $a = $n[attributes] ;
				
				if($r[$tag]=="") $r[$tag] = $a ;
				else if($r[$tag][0]) $r[$tag][] = $a ;
				else { 
					$t = $r[$tag] ; unset($r[$tag]); $r[$tag][0] = $t ; $r[$tag][1] = $a ; 
				}
			}
		}
	}
	return $r ;
}


function getxml($url,$encode="") {
    $v = array() ;
    $f = file($url) ;
		if($encode=="") {
		if(preg_match("/encoding=\"(.+)\"/",$f[0],$a)) {
			$encode = ($a[1]);
		} else 
			$encode = ("UTF-8");
		mb_internal_encoding($encode);
		}
		return parsexml($f,$encode) ;
}
