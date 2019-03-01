<?php
$ar_dump_f = false ;
function ar_dump($ar) {
	global $ar_dump_f ;
	if(!$ar_dump_f) {
		echo <<<EOT
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" /> 
<style type="text/css">
div.dump div {
	padding-left:4em ;
	overflow:hidden ;
}
div.dump div.dump_a {
	border-top:1px solid green;
	border-left:1px solid green;
}
div.dump div.dump_h {
	border-top:1px solid blue ;
	border-left:1px solid blue ;
}
div.dump span.dk {
	font-weight: bold ;
}
div.dump span.dk_s {
	font-weight: bold ;
	background-color:#eee ;
	cursor:pointer ;
}
div.dump span.dh {
	background-color:#fee ;
}
</style>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript">
$(function() {
	$('span.dk_s').on("click",function() {
		var o = $(this).next('div') ;
		if(o.length!=1) return ;
		if(o.css('height')=="0px") {
			$(this).removeClass('dh') ;
			o.css('height',"auto") ;
		}
		else {
			$(this).addClass('dh') ;
			o.css('height',0) ;
		}
	});
})
</script>
EOT;
		$ar_dump_f = true ;
	}
	echo '<div class=dump>' ;
	echo _dump($ar) ;
	echo "</div>\n" ;
}

function _dump($ar,$lv=0) {
	if(!is_array($ar)) return ;

	$j = array() ;
	$m = "" ;
	$pa = false ;
	foreach( $ar as $k=>$v ) {
		if($m==="") { if($k=="0") $m = 0 ; else $m = 1 ; }
		if(is_array($v)) { 
			$v = _dump($v,($lv+1)) ;
			if(count($j)!=0 && !$pa) $br = "<br/>" ;
			$pa = true ;
		} else {
			if(!is_int($v)) $v = "\"".str_replace("\r","",str_replace("\n","\\n",str_replace('"','\"',(str_replace("\0","",$v)))))."\"" ;
			$br = "" ;
			$pa = false ;
		}
		$cs = ($pa)?"dk_s":"dk" ;
		if($m==0) $j[] = $br.'<span class="'.$cs.' l'.$lv.'"> '.(count($j))."</span>=>".$v  ;
		else $j[] = $br.'<span class="'.$cs.' l'.$lv.'"> '.$k."</span>=>".$v ;
	}
	if($m==0) return "<div class=dump_a $hid>".join("  ",$j)."</div>" ;
	else return "<div class=dump_h $hid>".join(" ",$j)."</div>" ;
}
