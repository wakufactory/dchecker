<?php
require_once 'Common.inc';
require_once 'TemplateComponent.inc';
require_once 'postget.inc' ;

/**
 * ���å����������饹
 * session_start()�ؿ����ڡ�����ǣ��󤷤��¹Ԥ���ʤ����Ȥ��ݾڤ���
 *
 * @author ������ҥ���ǥ���������������<ohba@kardia.co.jp>
 */
class SessionManager {
	/**
	 * ���å�����ѿ��򥹡��ѡ��������Х��ѿ�$_SESSION�˥����ɤ���
	 */
	function start() {
		static $session_already_started;

		if (! $session_already_started) {
			session_start();
			$session_already_started = TRUE;
		}
	}
}

/**
 * ���٥�ȥǥ����ѥå����饹
 * �ƥڡ����Ϥ��Υ��饹��Ѿ����ƺ�������
 */
class Dispatcher {
	/**
	 * ���٥��ư����ڤ��ؤ���GET���뤤��POST�ѿ�
	 */
	var $_event_variable = 'event';

	/**
	 * event������ư�Default��ȤäƤ��ɤ���
	 */
	var $_nullevent_method = 'main';

	var $_compat = 0; ///���ߴ����ե饰

	/**
	 * ���٥�Ȥ�ǥ����ѥå�����
	 */
	function dispatch(){
		// GET�ѿ��򥹥����
		// �㡧<a href="***.php?event=���٥��̾">
		if ($_GET[$this->_event_variable]) {
			$event_name = $_GET[$this->_event_variable];
		}

		// POST�ѿ��򥹥����
		if ($_POST[$this->_event_variable]) {
			// ���٥���ѿ����ͤ���Ф�
			// �㡧<form method="POST" action="?event=���٥��̾">
			$event_name = $_POST[$this->_event_variable];
//print_r( $event_name );
			if (is_array($event_name)) {
				// ���٥���ѿ����ͤ�����ξ��
				// �㡧<input type="submit" name="event[���٥��̾]" value="����">

				// ���󤫤饤�٥��̾������κǽ�Υ����ˤ���Ф�
				$event_name = array_shift(array_keys($event_name));
			}
		}
//print_r( $event_name );
		if($event_name=='') $event_name = $this->_nullevent_method;

		// ���٥��̾���б�����ؿ���¸�ߤ��뤫������å�
		if ($event_name){
			if (method_exists($this, 'On_'.$event_name)) {
				$event = 'On_'.$event_name;
			}
			elseif ($this->_compat && method_exists($this, $event_name)) {
				$event = $event_name;
			}
		}

		if ($event ==''){
			if ($this->_compat && method_exists($this, 'main')) {
				$event = 'main';
			}
			else {
				$event = 'On_Default';
			}
		}
//		$get = arrayDecode($_GET) ;
		$get = $_GET ;
//		$post =arrayDecode($_POST) ;
		$post =$_POST ;

		// �ǥ����ѥå����줿�ؿ���ƤӽФ�
		//		������get��post���Ϥ�
		return $this->$event( array('get'=>$get,'post'=>$post ));
	}
}

/**
 * �ڡ����ѤΥ١������饹
 * �ƥڡ����Ϥ��Υ��饹��Ѿ����ƺ�������
 */
class Page extends Dispatcher {
	/**
	 * �ڡ�����ɽ������
	 *
	 * @param $template �ǥ�����ƥ�ץ졼�ȤΥե�����̾
	 * @param $args �ƥ�ץ졼���Ѱ���
	 */
	function _show_page($template, $args = NULL) {
		$args['self'] = $_SERVER['PHP_SELF'];

		$object = new TemplateComponent($template, $args);
		print $object->getString();
	}

	/**
	 *�֤��ͤ�_show_page()���Ϥ�
	 */
	function dispatch(){
		list($t,$a)=parent::dispatch();
		if($t==null) echo array2json($a) ;
		else { $this->_show_page($t,$a); }
	}

	/**
	 *�ǥե���ȥϥ�ɥ顣���顼�ۼ���
	 */
	function OnDefault(){
		return null;
	}
}

/**
 * ���å������Ѥ���ڡ����ѥ١������饹
 * �ƥڡ����Ϥ��Υ��饹��Ѿ����ƺ�������
 */
class SessionPage extends Page {
	/**
	 * ���å�����ѤΥڡ������å��ѿ�
	 */
	var $_session_page_set;

	/**
	 * ���󥹥ȥ饯��
	 */
	function SessionPage() {
		// ���å�����ѿ�������ɤ���
		SessionManager::start();
	}

	/**
	 *
	 */
	function _session_set($key, $value) {
		Common::set_value_to_hash(
			$_SESSION,
			"$this->_session_page_set/$key",
			$value
		);
	}

	/**
	 *
	 */
	function _session_merge($key, $value) {
		Common::merge_value_to_hash(
			$_SESSION,
			"$this->_session_page_set/$key",
			$value
		);
	}

	/**
	 *
	 */
	function _session_get($key) {
		return Common::get_value_from_hash(
			$_SESSION,
			"$this->_session_page_set/$key"
		);
	}

	/**
	 *
	 */
	function _session_destroy() {
		Common::set_value_to_hash(
			$_SESSION, $this->_session_page_set, NULL);
	}
}

/**
 * ��������ɬ�פʥڡ����ѥ١������饹
 * �ƥڡ����Ϥ��Υ��饹��Ѿ����ƺ�������
 */
class LoginPage extends SessionPage {
	/**
	 * ����������󤬥��åȤ��줿���å�����ѿ�<br>
	 * �㡧$_SESSION['download']['user']�ξ��<br>
	 * ��var $_login_session_variable = 'download/user';
	 */
	var $_login_session_variable;

	/**
	 * �������󤷤Ƥ��뤫�ɤ���
	 *
	 * @return �������󤷤Ƥ�����:TRUE,���Ƥ��ʤ����:FALSE
	 */
	function _is_logged_in() {
		if(Common::get_value_from_hash(
			$_SESSION, $this->_login_session_variable))
		{
			return TRUE;
		}
		else {
			return FALSE;
		};
	}
}

/**
 * ��������ɬ�פʥڡ����ѥ١������饹
 * �ƥڡ����Ϥ��Υ��饹��Ѿ����ƺ�������
 */
class LoginRedirectPage extends LoginPage {
	/**
	 * �������󤷤Ƥ��ʤ����Υ�����쥯����URL
	 */
	var $_not_login_redirect_url;

	/**
	 * ���󥹥ȥ饯��
	 */
	function LoginRedirectPage() {
		$this->SessionPage();

		if (! $this->_is_logged_in()) {
			header("Location: $this->_not_login_redirect_url");
			exit;
		}
	}
}
?>