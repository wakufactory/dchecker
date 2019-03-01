<?php
require_once "xml.inc" ;
$unzip_path = "/usr/bin/unzip" ;

class epub {
	public $tdir ;
	public $basedir ;
	public $errmsg ;

	function getepub($src,$dest) {
		global $unzip_path ;
		$tdir = $dest ;
		$this->tdir = $tdir ;
		$this->basedir = $tdir."/package" ;
				
		if($src!="") {
			if(file_exists($tdir)) {
				exec("rm -r $tdir") ;
				if(file_exists($tdir)) {
					$this->errmsg = "Server Error" ;
					return null  ;
				}
			}
	
			mkdir($tdir) ;
			mkdir($tdir."/package") ;
			exec("$unzip_path '$src' -d $tdir/package ",$out,$st) ;
			if($st!=0) {
				$this->errmsg = "unzip error" ;
				return null ;
			}
			for($i=0;$i<count($out);$i++) {
				if(preg_match("|".$tdir."/(.+)|",trim($out[$i]),$a)) {
					$fl[] = $a[1] ;
				}
			}
		} else $fl = array() ;
		return $fl ;
	}
	function clean() {
		exec("rm -r ".$this->tdir) ;
	}

	function checkepub($dir="") {
		if($dir=="") $dir = $this->basedir ;
		$m = @file($dir."/mimetype") ;
		if(!preg_match("|application/epub\+zip|i",trim($m[0]))) {
			$this->errmsg = "mimetype " ;
			return null ;
		}
		$m = @file($dir."/META-INF/container.xml") ;
		if($m===FALSE) {
			$this->errmsg = "no meta-inf" ;
			return null ;
		}
		$mm = parsexml($m) ;
		$fopf = $mm['container']['rootfiles']['rootfile']['full-path'] ;
		if($fopf=="") {
			$this->errmsg = "no opf" ;
			return null ;
		}
		
		$opf = @file($dir."/".$fopf) ;
		if($opf===FALSE) {
			$this->errmsg = "no opf file" ;
			return null ;
		}
		$o = parsexml($opf) ;
		$o = $o['package'] ;	
		$item = array() ;
		foreach($o['manifest']['item'] as $v) {
			$item[$v['id']] = $v ;
			if($v['properties']=="nav") $nav = $v ;
		}
		$meta = array() ;
//print_r($o['metadata']);
		foreach($o['metadata'] as $k=>$v) {
			if(preg_match("|^dc\-|",$k)) $meta[$k] = $v ;
			else if($k=="meta") {
				if(!$v[0]) $v = array($v) ;
				for($i=0;$i<count($v);$i++) {
					if($v[$i]['name']=="cover") $meta['cover'] = $item[$v[$i]['content']] ;
					if($v[$i]['refines']) {
							$meta['meta']['refines'][$v[$i]['property']][$v[$i]['refines']] = $v[$i] ;
					}else {
						if(!$v[$i]['property']) $v[$i]['property'] = "_noprop" ;
						if(!$meta['meta'][$v[$i]['property']]) $meta['meta'][$v[$i]['property']] = $v[$i] ;
						else {
							if($meta['meta'][$v[$i]['property']][0]) $meta['meta'][$v[$i]['property']][] = $v[$i] ;
							else $meta['meta'][$v[$i]['property']] = array($meta['meta'][$v[$i]['property']],$v[$i]) ;
						}
					}
				}
			}
		}
		$spine = array() ;
		$idx = 0 ;
		foreach($o['spine']['itemref'] as $v) {
			$v['idref'] = $item[$v['idref']] ;
			$v['idx'] = $idx ;
			$spine[] = $v ;
			$idx++ ;
		}
		$css = array() ;
		foreach($item as $k=>$v) {
			if($v['media-type']=="text/css") {
				$css[] = $v ;
			}
		}
		$js = array() ;
		foreach($item as $k=>$v) {
			if($v['media-type']=="text/javascript") {
				$js[] = $v ;
			}
		}
	
		$r = array('meta'=>$meta,'item'=>$item,'spine'=>$spine,'opf'=>$fopf) ;
		if(count($css)>0) $r['css'] = $css ;
		if(count($js)>0) $r['js'] = $js ;
		$r['version'] = $o['version'] ;
		if($nav) $r['nav'] = $nav ;
		if($o['spine']['toc']) $r['toc'] = $item[$o['spine']['toc']];
		if($o['spine']['page-progression-direction']) $r['page-dir'] = $o['spine']['page-progression-direction'] ;
		if($o['guide']) {
			$r['guide'] = $o['guide']['reference'] ;
			if(!$r['guide'][0]) $r['guide'] = array($r['guide']) ;
		}
		$opf = pathinfo($fopf) ;
		$r['opfpath'] = $opf['dirname']."/" ;
		$r['basedir'] = $this->basedir."/" ;
		if($meta['meta']['rendition:layout']) {
			if($meta['meta']['rendition:layout']['_content']=="pre-paginated") $r['fixed'] = true ;
		}
//print_r($r);
		return $r ;
	}

}