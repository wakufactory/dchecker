<?php
$ctlist = array(

array('cat'=>"grep",'l'=>array(
	array(
	"ereg"=>"[^ -=\?-}][0-9a-zA-Z]{1,3}[^ -}]",
	"title"=>"縦中横候補(1-3文字の半角英数字)",
	"stext"=>"本文中の、１～３文字の半角英数字を、縦中横候補として表示します。",
	"etext"=>"EPUB内の各XHTMLファイルで、本文縦書き時に縦中横で表示させなければならない可能性があるにもかかわらず、縦中横のタグが付加されていない１～３文字の半角英数字をチェックし、一覧で表示します。必要に応じてタグの追記を行ってください。\nなお現在、全てのビューアで問題なく縦中横表示が可能なのは、数字は3桁、英文字は2文字までです。これを超えた場合は、たとえ縦中横のタグが付加されていたとしても、指定が反映されず横転表示されてしまう可能性があります。表記方法を変更する、外字画像化するなどの対策を検討してください。"
	),
	array(
	"ereg"=>"[0-9０-９一二三四五六七八九〇前次]+(<[^>]+>)?(ページ|頁)",
	"opt"=>array("skip"=>"^<\!\-\-.+\-\->$"),
	"title"=>"数字+「ページ/頁」",
	"stext"=>"「○○ページ参照」などの記述を避けるため、「数字+ページ(頁)」の記述がされている部分を表示します。",
	"etext"=>"コンテンツ内で「数字+ページ(頁)」の表記のある箇所を抽出し、一覧表示する。固定ページの概念がない EPUB では「○ ○ページ参照」といった表記は意味をなさなくなるので、これらの表記を修正する必要がある。一覧抽出することで修正 忘れを抑止することができる。\n参考: 検索正規表現 = [0-9０-９一二三四五六七八九〇前次]+(<[^>]+>)?(ページ|頁)"
	),

	array(
	"ereg"=>"<p[^>]*>(.)",
	"title"=>"パラグラフ開始直後",
	"stext"=>"pタグの直後のパラグラフ開始文字を抽出",
	"etext"=>"コンテンツ内のpタグの直後の文字を一覧表示する。段落頭字下げのチェックのための項目。"
	),
	
	array(
	"ereg"=>"tcy.+?</span>",
	"title"=>"縦中横指定文字",
	"stext"=>"class=tcyで指定されたテキスト部分を表示します。",
	"etext"=>"コンテンツ内の「<span class=\"tcy\"> 〜 </span>」でマークアップされた縦書き時正立指定の文字を抽出し、一覧表示する。"
	),

	array(
	"ereg"=>"upright.+?</span>",
	"title"=>"正立指定文字",
	"stext"=>"class=uprightで指定されたテキスト部分を表示します。",
	"etext"=>"コンテンツ内の「<span class=\"upright\"> 〜 </span>」でマークアップされた縦書き時正立指定の文字を抽出し、一覧表 示する。文字によっては手元の環境で正立表示されていても、一部の RS で横転表示されてしまうものが存在するため、明示的に正立指定のタグを挿入しなければならないものが存在する。該当する文字に関しては、KADOKAWA-EPUB PORTAL
(http://kadokawa-epub.bookwalker.co.jp/)で資料が公開されている。\n※ただし、現状タグのあるもののみを抽出しているので、本来タグ付加が必要でも付加されていない文字を抽出できない。"
	),
	array(
	"ereg"=>"sideways.+?</span>",
	"title"=>"横倒し指定文字",
	"stext"=>"class=sidewaysで指定されたテキスト部分を表示します。",
	"etext"=>"コンテンツ内の「<span class=\"sideways\"> 〜 </span>」でマークアップされた縦書き時横倒し指定の文字を抽出し、一 覧表示する。文字によっては手元の環境で横転表示されていても、一部の RS で正立表示されてしまうものが存在するため、明示的に横転指定のタグを挿入しなければならないものが存在する。該当する文字に関しては、KADOKAWA-EPUB PORTAL(http://kadokawa-epub.bookwalker.co.jp/)で資料が公開されている。\n※ただし、現状タグのあるもののみを抽出しているので、本来タグ付加が必要でも付加されていない文字を抽出できない。"
	),
	array(
	"ereg"=>"<img.+gaiji.+?>",
	"title"=>"画像外字使用",
	"stext"=>"class=gaiji指定の画像外字部分を表示します。",
	"etext"=>"コンテンツ内で画像外字のタグが挿入されている箇所を抽出し、一覧表示する。個々の外字画像の字形に関しては「画像 外字一覧」を参照のこと。
XMDF や .book などから変換した場合などでは、もともと文字コードが Shift_JIS であったために、EPUB で使用される Unicode では画像外字を使用しなくても表現できる文字を外字画像化しているケースが存在する。\nまた、「正字」の表現を目的として画像外字を使用していたケースでは、JISX0213:2000 から JISX0213:2004 への変更に伴い、 多くの字形がデフォルトで表現できるようになっているので、本当に外字表現が必要なのかどうかを確認し、必要なけれ ば通常のテキストに置換すること。
画像外字はアクセシビリティを阻害する側面があり、また、背景色を黒に設定した際に外字が背景に溶け込んで視認できなくなる RS も存在するため、人名漢字などで本当に字形の保持が必要なケース以外では使用を控えた方が良い。"
	),
	array(
	"ereg"=>"\\\\|&#x0+5C|&#92",
	"title"=>"バックスラッシュ",
	"stext"=>"バックスラッシュが円記号として使われていないか確認",
	"etext"=>"Shift-JISでは、0x5Cのコードが¥マークとして使われていたが、Unicodeではバックスラッシュになる。半角の¥マークを使う場合は、¥(U+A5)を使う必要がある。"
	)
)),
array('cat'=>"class",'l'=>array(
	array(
	"ereg"=>"italic.+?</span>",
	"title"=>"イタリック指定文字",
	"stext"=>"class=italicで指定されたテキスト部分を表示します。"
	),
	array(
	"ereg"=>"(super|sub|valign).+?</span>",
	"title"=>"文字位置指定クラス",
	"stext"=>"class=super/sub/valine-で指定されたテキスト部分を表示します。"
	),
	array(
	"ereg"=>"lspacing.+?</span>",
	"title"=>"文字アキ指定クラス",
	"stext"=>"class=lspacing-で指定されたテキスト部分を表示します。"
	),
	array(
	"ereg"=>"class=\"line\-height",
	"title"=>"行間指定クラス",
	"stext"=>"class=line-height-で指定されたテキスト部分を表示します。"
	),
	array(
	"ereg"=>"class=\"m\-left",
	"title"=>"行間アキ指定クラス",
	"stext"=>"class=m-left-で指定されたテキスト部分を表示します。"
	)
))
);
