# keyball39 配列を基にした小人キーのキーマップ

移植元は `zmk-MoooseFree` の `change-keymap` ブランチ、コミット `12a3050b7dc0d68004ad5edb61e947b777f10d34` の `config/MoooseFree.keymap`。
小人キーの40キーに収まるキーを移植した。レイヤー0はその後 keyball39 の `V5.4_260830.pdf` とユーザー指定に合わせて修正し、左下の Shift+F10 を右下の Dot／FUNCTION の1つ左へ移した。
レイヤー1〜4・6も同PDFを基に修正した。レイヤー5は小人キー用のBluetooth設定を維持する。

## レイヤー

| 小人キー | 内容 | 現在の配列の参照元 |
| --- | --- | --- |
| 0 | QWERTY | keyball39の0 |
| 1 | NUMBER（左側は数字、右側は矢印・ショートカット） | keyball39の1 |
| 2 | SYMBOL（記号） | keyball39の2 |
| 3 | FUNCTION（Fキーなど） | keyball39の3 |
| 4 | SHORTCUT | keyball39の4 |
| 5 | BLUETOOTH | 小人キーのレイヤー2左側 |
| 6 | MOUSE | keyball39の6＋小人キーのマウス1〜3 |

レイヤー番号はNUMBER=1、SYMBOL=2、FUNCTION=3、SHORTCUT=4、BLUETOOTH=5、MOUSE=6。レイヤー番号の `#define` は使わず、バインディングに番号を直接記述する。
レイヤーノード名は元の `default_layer` と `layer1`〜`layer6`。
マウスレイヤーは `layer6`。マウス1〜3は小人キーの位置6（左クリック）・7（中クリック）・8（右クリック）を維持し、その他はkeyball39 PDFのレイヤー6に合わせた。keyball39側のマウス1〜3の元位置には重複配置せず、`&none` とした。`&trans` は使わない。
トラックボールの自動マウス遷移先も6に変更し、滞留時間、除外位置、センサー設定は維持した。
移植した MoooseFree のレイヤー3への遷移キーは 3、元のレイヤー4への遷移キーは 4を参照する。

Bluetoothレイヤーの左側には、小人キーの元のレイヤー2から、プロファイル0〜4の選択、選択中プロファイルの解除、全プロファイルの解除を移した。
Bluetooth操作と右上の復帰キー以外はすべて `&none` とし、下位レイヤーのキーを発火させない。
Space長押しはPDFどおりLeft Shift。Bluetoothレイヤーへの遷移はレイヤー3の右上（位置9）の `&tog 5` とし、FUNCTIONを離してもBluetoothレイヤーを維持する。レイヤー5の同じ位置は `&to 0` とし、レイヤー0へ戻る。

## 配列の対応

上3段は QWERTY のキー位置で対応させた。AはPDFと同じタップでA・長押しでLeft Alt。右手ホーム段の最後は Backspace、右手下段の最後は `/`。
キー位置は `kobitokey.dtsi` の matrix transform に従う0始まりの番号。

下段の10キーは次の配置。長押し判定は hold-preferred、220ms、quick-tap 120ms。

| 小人キー位置 | ZMKバインディング | タップ／通常入力 | 長押し |
| --- | --- | --- | --- |
| 30（左から1番目） | `&lt 4 ESC` | Escape | SHORTCUT（4） |
| 31 | `&kp LWIN` | Left Win | Left Win |
| 32 | `&mt LCTRL GRAVE` | 半角／全角（日本語配列） | Left Ctrl |
| 33 | `&mt LSHIFT SPACE` | Space | Left Shift |
| 34 | `&mt LC(LSHIFT) MINUS` | Minus | Ctrl+Shift |
| 35（右手の1番目） | `&lt 2 TAB` | Tab | SYMBOL（2） |
| 36 | `&lt 1 ENTER` | Enter | NUMBER（1） |
| 37 | `&none` | 無効 | 無効 |
| 38 | `&kp LS(F10)` | Shift+F10 | Shift+F10 |
| 39 | `&lt 3 DOT` | Dot | FUNCTION（3） |

上表はレイヤー0での動作。レイヤー6が有効な場合は既存のマウス用配列が適用される。
日本語配列向けの記号キーコード・マクロは移植元のまま。

## 他レイヤーのPDF対応

全レイヤーで、左下6キーのうち元のShift+F10位置を小人キーの位置38へ対応させる。右側で余る位置37は `&none`。
PDFの `▽` は `&trans`、空欄は `&none`。ただし、レイヤー5は既存のBluetooth配列を維持する。

- レイヤー1：A位置はテンキー乗算／長押しLeft Alt。N位置の `Alt*` はRight Alt。Macro 0・1は移植せず `&none`。下段はPDFの透明キーに対応させ、Space位置のみF10／長押しLeft Shift。
- レイヤー2：Ctrl／半角全角の位置32は `&trans`、位置33はShift+Space。位置34の `Kb 27` は `&none`。
- レイヤー3：位置30はCtrl+Alt+Delete、位置34はShift+英数によるCaps Lock。右上の `&tog 5` はBluetoothレイヤーへの遷移として維持。
- レイヤー4：日本語配列の括弧・ウィンドウ操作・タブ切替など、既存のバインディングがPDFと一致しているため変更なし。
- レイヤー6：PDFのT・Y・A／長押しAlt・D・F・G・H・C・V、およびCtrl／半角全角・Shift／Space・Ctrl+Shift／Minusを移植。現在のB・N位置、および `Kb 21`〜`Kb 24`・`Kb 31` は `&none`。独自のレイヤー0復帰キーは置かず、既存の自動マウスのタイムアウトで復帰する。

PDFのMouse Btn8は、現在のZMK v0.3がマウス5ボタンまでの対応であるため `&none` とした。
確認元：<https://github.com/zmkfirmware/zmk/blob/v0.3/app/include/zmk/hid.h> および <https://github.com/zmkfirmware/zmk/blob/v0.3/app/include/dt-bindings/zmk/pointing.h>。
Right Altの表示確認元：<https://github.com/remap-keys/remap/blob/main/src/services/hid/KeycodeInfoList.ts>。

## コンボ

レイヤー0のみ、判定時間80ms。元の位置番号を小人キーの対応位置へ変換した。

| 同時押し | 小人キー位置 | 出力 |
| --- | --- | --- |
| C + V | 22 + 23 | Ctrl + Shift + V |
| G + H | 14 + 15 | `()` |
| T + Y | 4 + 5 | `「」`（日本語配列） |

既存の小人キーの英数・かな・Esc・Tabコンボは、この移植元のコンボ構成へ置き換えた。

## 未移植

- 外周の専用キー、補助キー、bootloaderキー。Bluetooth操作は小人キーの元の定義をレイヤー5に復元した。
- 下段で選択しなかったキー（左右・中クリック専用キー、Print Screenなど）。左右・中クリックは自動マウスレイヤーの小人キーと同じ位置で使用できる。
- ロータリーエンコーダーの挙動と `sensor-bindings`。小人キーは既存のトラックボール入力処理を使う。
- 移植後のキーから参照されないマクロと、移植元の透明なレイヤー5。
- keyball39 PDFのMacro 0・1、`Kb xx` のカスタムキーコード、Mouse Btn8。対象位置は `&none`。MoooseFree由来のコンボと、コンボから使用する `my_kakko`・`my_kagi` は維持。未使用の `to_layer_0`・`shift_space`・`lt_to_layer_0` は削除した。

未移植の機能には、新しい代替キーを割り当てていない。
ZMKの依存バージョン、ビルド設定は変更していない。
LED設定はレイヤー番号の変更に合わせ、FUNCTION（3）を青、SHORTCUT（4）を青、BLUETOOTH（5）をマゼンタ、MOUSE（6）をシアンに設定した。

## 検証

全7レイヤーが40バインディングであること、PDF対応、Bluetoothの配列と出入口、レイヤー遷移先、自動マウスの遷移先、マウス1〜3の位置・重複防止、およびレイヤー5・6の未割り当て位置が `&none` であることを静的に確認。今回のレイヤー1〜4・6の移植では、レイヤー0と5を変更していない。
ローカル環境には ZMK/Zephyr のビルド環境がないため、ファームウェアのコンパイルと実機動作は未確認。
既存の `.github/workflows/build.yml` の ZMK ビルドで、左右両方のファームウェアを確認する必要がある。
