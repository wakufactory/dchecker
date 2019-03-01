<?php
require_once "utf2jis.inc" ;
class chcheck {

	public $sum ;
	public $ref ;
	public $wch ;
	public $img ;
	public $spch = array(
		"0000"=>array('e'=>"NULL"),
		"000A"=>array('e'=>"改行(LF)"),
		"000D"=>array('e'=>"改行(CR)"),
		"001B"=>array('e'=>"ESC"),
		"0009"=>array('e'=>"TAB"),
		"00A0"=>array('e'=>"nbsp"),
		"FEFF"=>array('e'=>"BOM"),
	);
	public $striptags ;
	private $fn ;
	function chcheck() {
		$this->img = array() ;
	}
	function putstr($s,$fn=null) {
		$this->fn = $fn ;
		if(!is_array($s)) $s = array($s) ;
		foreach($s as $l) {
			if(preg_match_all("|<img([^>]+)>|i",$l,$m)) {
				$this->img = array_merge($this->img,$m[1]) ;
			}
			if($this->striptags) $l = strip_tags($l) ;
			for($i=0;$i<mb_strlen($l);$i++) {
				$ch = mb_substr($l,$i,1) ;
				if(!$this->sum[$ch]) $this->sum[$ch] = array('n'=>1,'u8'=>utox($ch),'u'=>utox(mb_convert_encoding($ch,"UCS-4","UTF-8"))) ;
				else $this->sum[$ch]['n']++ ;
				
				if(!$this->sum[$ch]['u']) {
					$this->sum[$ch]['u'] = "000000".$this->sum[$ch]['u8'] ;
				}
			}
			if(preg_match_all("|(&[^;]+;)|",$l,$m)) {
				for($i=0;$i<count($m[0]);$i++) {
					$this->ref[$m[0][$i]]++ ;
				}
			}

		}
	}

	function cat(&$v,$ref="") {
		$xu = "x".$v['u'] ;
		$uv = $v['uv'] ;
//		if($uv=="") print_r($v);
		$cat = 'k' ;
		//スペース
		if($this->spch[$uv]||$xu<"x00000020"||($xu>="x00000080"&&$xu<="x0000009F")) {
			$cat ="c" ;
			$v['j'] = $this->spch[$uv]['e'] ;
			
			if($uv=="FEFF") $this->wch['bom'][] = $this->fn ;
			if($uv!="000A"&&$uv!="000D"&&$uv!="0009"&&$uv!="00A0"&&$uv!="FEFF") $this->wch['cc'][] = $uv ;
			
		} 
		// ascii
		else if($xu<"x00000080") $cat="h" ;
		//全角
		else if($xu>"x0000FF00" && $xu<"x0000FF5F" || $xu=="x0000FFE5") $cat="z" ;
		// 半角
		else if($xu>"x0000FF60" && $xu<"x0000FFA0") $cat="a" ;
		// 記号
		else if($xu<"x00003400"||($xu>="x0000FB00"&&$xu<="x0000FFFF"))  {
			$cat="n" ;
			if($xu>="x00002000"&&$xu<="x0000200B") $this->wch[$ref.'usp'][] = $uv ;
			else if(!$v['j'] && $uv!="2015") $this->wch[$ref.'kigo'][] = $uv ;
		}

		// ivs 
		else if($xu>="x000E0100" && $xu<="x000E01FF") {
			$cat="v" ;
			$this->wch[$ref.'ivs'][] = $uv ;
		}
		// sgp
		else if($xu>"x0000FFFF") {
			$cat="s" ;
			$this->wch[$ref.'sgp'][] = $uv ;
		} else {
			if(!$v['j']) $this->wch[$ref.'kanji'][] = $uv ;
		}	
		return $cat ;		
	}
	function getsum() {

		global $utoj ;
		$r = array() ;
		foreach($this->sum as $k=>$v) {
			$v['c'] = $k ;
			if(substr($v['u'],0,4)=="0000") $v['uv']  = substr($v['u'],4) ;
			else if(substr($v['u'],0,3)=="000") $v['uv'] = substr($v['u'],3) ;
			$uv = $v['uv'] ;
			if($utoj[$uv]) {
				$j = $utoj[$uv] ;
				$v['class'] = "m".$j[5] ;
				$v['j'] = $j[0]."-".$j[1]."-".$j[2] ;
			}
			$cat = $this->cat($v) ;
			$r[$cat][] = $v; 
		}
		

		if($this->ref) {
			foreach($this->ref as $k=>$v) {
				$ch = array(
					'n'=>$v, 'c'=>$k
				) ;
				$u = null ;
				if(preg_match("/&#([0-9]+);/",$k,$a)) {
					$u = strtoupper(dechex($a[1])) ;
				}
				if(preg_match("/&#x([0-9a-f]+);/i",$k,$a)) {
					$u = strtoupper($a[1]) ;
				}
				if($u) {
					if(strlen($u)<4) $u = substr("000".$u,-4) ;
					$ch['uv'] = $u ;
					if($utoj[$u]) {
						$j = $utoj[$u] ;
						$ch['class'] = "m".$j[5] ;
						$ch['j'] = $j[0]."-".$j[1]."-".$j[2] ;
					}
					$ch['u'] = substr("00000".$u,-8) ;
					$r['m'][] = $ch ;	
					$this->cat($ch,"ref_");			
				} else {
					$ch['u'] = str_replace("&","&amp;",$k) ;
					$r['r'][] = $ch ;
				}


			}
		}
	
		if($r['c']) usort($r['c'],ch_sfunc) ;
		if($r['h']) usort($r['h'],ch_sfunc) ;
		if($r['n']) usort($r['n'],ch_sfunc) ;
		if($r['k']) usort($r['k'],ch_sfunc) ;
		if($r['z']) usort($r['z'],ch_sfunc) ;
		if($r['a']) usort($r['a'],ch_sfunc) ;
		if($r['v']) usort($r['v'],ch_sfunc) ;	
		if($r['m']) usort($r['m'],ch_sfunc) ;
		if($r['r']) usort($r['r'],ch_sfunc) ;				
		if($r['s']) usort($r['s'],ch_sfunc) ;	
		
//		print_r($this->wch);	
		return $r ;
	}
	function getimg() {
		return $this->img ;
	}
	function merge($a,$b) {
		
	}
	function clear() {
		$this->sum = null ;
		$this->ref = null ;
		$this->wch = null ;
		$this->img = array() ;
	}
	
}
function ch_sfunc($a,$b) {
	if($a['u']==$b['u']) return 0 ;
	return ("x".$a['u']>"x".$b['u'])?1:-1;
}

function utox($ch) {
	$c = "" ;
	for($i=0;$i<strlen($ch);$i++) {
		$h = strtoupper(dechex(ord(substr($ch,$i,1)))) ;
		if(strlen($h)==1) $h = "0".$h ;
		$c .= $h;
	}

	return $c ;
}

