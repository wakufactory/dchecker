<?php
/**
 * 共通関数クラス
 */
class Common {
	/**
	 * 再起的に配列あるいはスカラーの前後の空白を除去する
	 * （全角空白も除去）
	 *
	 * @param &$variable 配列あるいはスカラー
	 * @return 空白を除去した変数
	 */
	function trim(&$variable) {
		if (is_array($variable)) {
			foreach ($variable as $key => $value) {
				$variable[$key] = Common::trim($value);
			}
		}
		else {
			$variable = mb_ereg_replace('^　+', ' ', $variable);
			$variable = mb_ereg_replace('　+$', ' ', $variable);
			$variable = trim($variable);
			$variable = str_replace("\\\"", "\"", $variable);
		}

		return $variable;
	}

	/**
	 * 空?
	 */
	function is_empty(&$v){
		if (is_null($v)) return 1;
		if ($v === '') return 1;
		return 0;
	}

	/**
	 * 非空エントリがある?
	 */
	function has_filled(&$v){
		if (is_array($v)) {
			foreach ($v as $key => $value) {
				if (Common::has_filled($value)) return 1;
			}
		}
		elseif ($v != '') return 1;

		return 0;
	}

	/**
	 * 日付・時刻文字列をUNIX秒に変換する
	 *
	 * @param $time_string '2004-01-01 12:00:00'形式の日付・時刻文字列
	 * @return UNIX秒
	 */
	function parse_time($time_string) {
		$date_time = preg_split('/\s+/', $time_string);
		$date = explode('-', $date_time[0]);
		$time = explode(':', $date_time[1]);

		return mktime($time[0], $time[1], $time[2], $date[1], $date[2], $date[0]);
	}

		function set_value_to_hash(&$hash, $key, $value) {
		if (is_array($key)) {
			$first_key = array_shift($key);

			if (count($key)) {
				Common::set_value_to_hash($hash[$first_key], $key, $value);
			}
			else {
				$hash[$first_key] = $value;
			}
		}
		else {
			Common::set_value_to_hash($hash, explode('/', $key), $value);
		}
	}

	function &get_value_from_hash(&$hash, $key) {
		if (is_array($key)) {
			$first_key = array_shift($key);

			if (!is_array($hash) || !array_key_exists($first_key,$hash)) return NULL;

			if (count($key)) {
				return Common::get_value_from_hash($hash[$first_key], $key);
			}

			else {
				return $hash[$first_key];
			}
		}
		else {
			return Common::get_value_from_hash($hash, explode('/', $key));
		}
	}

	function merge_value_to_hash(&$hash, $key, $value) {
		Common::set_value_to_hash($hash, $key,
			array_merge(Common::get_value_from_hash($hash, $key), $value));
	}

	function unset_key_from_hash(&$hash, $key) {
		if (is_array($key)) {
			$first_key = array_shift($key);

			if (count($key)) {
				Common::unset_key_from_hash($hash[$first_key], $key);
			}

			else {
				unset($hash[$first_key]);
			}
		}
		else {
			Common::unset_key_from_hash($hash, explode('/', $key));
		}
	}

	function set_selected(&$args, $fields) {
		if (is_array($fields)) {
			foreach ($fields as $field => $value) {
				Common::set_select_object($args, $field, $value);
			}
		}
	}

	function set_select_object(&$args, $field, $value) {
		Common::set_value_to_hash(
			$args,
			$field,
			array(
				Common::get_value_from_hash($args, $field) => $value
			)
		);
	}

	/**
	 * ファイルの拡張子を返す
	 *
	 * @param $file ファイル名
	 * @return 拡張子
	 */
	function file_extension($file) {
		if (strpos($file, '.')) {
			return array_pop(preg_split("/\./", $file));
		}
	}

	function array_and(&$array, $keys) {
		$result = TRUE;

		if (is_array($array)) {
			foreach ($keys as $key) {
				if (! $array[$key]) {
					$ressult = FALSE;
				}
			}
		}

		return $result;
	}

	function array_or(&$array, $keys) {
		$result = FALSE;

		if (is_array($array)) {
			foreach ($keys as $key) {
				if ($array[$key]) {
					$ressult = TRUE;
				}
			}
		}

		return $result;
	}

	function save_array_to_csv_file($file_name, &$array, $order) {
		$fp = fopen($file_name, 'a');

		foreach ($order as $key) {
			$record[$key] = $array[$key];
		}

		fputs($fp, join(',', $record) . "\n");

		fclose($fp);
	}

	function unhtmlentities($string) {
		$trans_tbl = get_html_translation_table (HTML_ENTITIES);
		$trans_tbl = array_flip ($trans_tbl);
		return strtr ($string, $trans_tbl);
	}

	/**
	 * シリアライズされた文字列からPHPのデータを取得する
	 *
	 * @param $serialized_string シリアライズされた文字列
	 * @return PHPのデータ
	 */
	function unserialize($serialized_string) {
		return unserialize(
			str_replace(
				"\\\"",
				"\"",
				$serialized_string
			)
		);
	}
}
?>
