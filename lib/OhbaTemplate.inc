<?php
require_once 'Common.inc';
require_once 'TemplateComponent.inc';

/**
 * セッション管理クラス
 * session_start()関数がページ内で１回しか実行されないことを保証する
 *
 * @author 株式会社カルディア　大庭誉士和<ohba@kardia.co.jp>
 */
class SessionManager {
	/**
	 * セッション変数をスーパーグローバル変数$_SESSIONにロードする
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
 * イベントディスパッチクラス
 * 各ページはこのクラスを継承して作成する
 */
class Dispatcher {
	/**
	 * イベント動作を切り替えるGETあるいはPOST変数
	 */
	var $_event_variable = 'event';

	/**
	 * event空時の動作。Defaultを使っても良い。
	 */
	var $_nullevent_method = 'main';

	var $_compat = 0; ///過去互換性フラグ

	/**
	 * イベントをディスパッチする
	 */
	function dispatch(){
		// POST変数をスキャン
		if ($_POST[$this->_event_variable]) {
			// イベント変数の値を取り出す
			// 例：<form method="POST" action="?event=イベント名">
			$event_name = $_POST[$this->_event_variable];
		}
		// GET変数をスキャン
		// 例：<a href="***.php?event=イベント名">
		elseif ($_GET[$this->_event_variable]) {
			$event_name = $_GET[$this->_event_variable];
		}

		if (is_array($event_name)) {
			// イベント変数の値が配列の場合
			// 例：<input type="submit" name="event[イベント名]" value="送信">

			// 配列からイベント名（配列の最初のキー）を取り出す
			$event_name = array_shift(array_keys($event_name));
		}

		if($event_name=='') $event_name = $this->_nullevent_method;

		// イベント名に対応する関数が存在するかをチェック
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

		// ディスパッチされた関数を呼び出す
		return $this->$event( array('get'=>$_GET,'post'=>$_POST,'request'=>$_REQUEST ) );
	}
}

/**
 * ページ用のベースクラス
 * 各ページは、これかPageクラスを継承して作成する
 * TemplateComponentを使わない単純ページ用
 */
class SimplePage extends Dispatcher {
	/**
	 * ページを表示する
	 *
	 * @param $template デザインテンプレートのファイル名
	 * @param $args テンプレート用引数
	 */
	function _show_page($template, $args = NULL) {
		$args['self'] = $_SERVER['PHP_SELF'];

		Htmltemplate::t_include($template, $args);
	}

	/**
	 *返り値を_show_page()に渡す
	 */
	function dispatch(){
		list($t,$a)=parent::dispatch();
		if ($t){ $this->_show_page($t,$a); }
	}

	/**
	 *デフォルトハンドラ。エラー吸収用
	 */
	function On_Default(){
		return null;
	}
}

/**
 * TemplateComponentを使う。
 * 大量にデータを抱えたページでは、若干遅くなる。
 */
class Page extends SimplePage {
	/**
	 * ページを表示する
	 *
	 * @param $template デザインテンプレートのファイル名
	 * @param $args テンプレート用引数
	 */
	function _show_page($template, $args = NULL) {
		$args['self'] = $_SERVER['PHP_SELF'];

		$object = new TemplateComponent($template, $args);
		print $object->getString();
	}
}

/**
 * セッションを用いるページ用ベースクラス
 * 各ページはこのクラスを継承して作成する
 */
class SessionPage extends Page {
	/**
	 * セッション用のページセット変数
	 */
	var $_session_page_set;

	/**
	 * コンストラクタ
	 */
	function SessionPage() {
		// セッション変数をロードする
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
 * ログインが必要なページ用ベースクラス
 * 各ページはこのクラスを継承して作成する
 */
class LoginPage extends SessionPage {
	/**
	 * ログイン情報がセットされたセッション変数<br>
	 * 例：$_SESSION['download']['user']の場合<br>
	 * 　var $_login_session_variable = 'download/user';
	 */
	var $_login_session_variable;

	/**
	 * ログインしているかどうか
	 *
	 * @return ログインしている場合:TRUE,していない場合:FALSE
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
 * ログインが必要なページ用ベースクラス
 * 各ページはこのクラスを継承して作成する
 */
class LoginRedirectPage extends LoginPage {
	/**
	 * ログインしていない場合のリダイレクト先URL
	 */
	var $_not_login_redirect_url;

	/**
	 * コンストラクタ
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
