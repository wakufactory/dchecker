<?php
//The TemplateComponent class
//ver.1.1

require_once("htmltemplate.inc");

class TemplateComponent{
	var $arg;
	var $template;
	
	//Constructor
	function TemplateComponent($template="",$arg=""){
		$this->template=$template;
		if($arg!="") $this->arg=$arg;
	}
	
	
	//get HTML data from the template & data
	function getString(){
		if($this->template){
			$arg=$this->_buildArray($this->arg);
			return Htmltemplate::t_buffer($this->template,$arg);
		}
	}
	
	
	//private method
	function _buildArray($arg){
		if(gettype($arg)=="array"){

			if($this->__isHash($arg)){
			 foreach($arg as $k => $v) 
			 		$ans[$k]=$this->_buildArray($v);
			}else{
				for($i=0;$i<count($arg);$i++) 
					$ans[$i]=$this->_buildArray($arg[$i]);
			}
		}elseif(is_object($arg)&& (get_class($arg)=='templatecomponent' || 
				is_subclass_of($arg,'templatecomponent'))) 
						$ans=$arg->getString();
		else $ans=$arg;
		return $ans;
	}
	
	
	//private method
	function __isHash($hash){
		if(gettype($hash)!="array") return false;
		$k=array_keys($hash);
		$f=false;
		for($i=0;$i<count($k);$i++){if($k[$i]!=(string)$i) $f=true; }
		return $f;
	}
}
?>
