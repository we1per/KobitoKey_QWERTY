# DYA Studioでコンボ・マクロを編集する

## このブランチの変更

- ZMKをcormoranのcustom Studio RPC対応forkへ更新（Zephyr 4.1）。
- Runtime Macro・Runtime Combo・Custom Settingsを追加。依存モジュールは `config/west.yml` のコミットに固定。
- XIAO BLEのボード指定を `xiao_ble//zmk` へ更新。PMW3610はZephyr 4対応のALTドライバへ更新。
- 既存の7レイヤー・40キー、左右の配線、CPI（左200／右600）、回転角、スクロール倍率、自動マウスの遷移先・待ち時間・除外キー、LED色は維持。
- 旧コンボ定義をRuntime Comboの初期値に置き換え、同じコンボを二重登録しない。
- 編集機能はPCにつながる左側（central）で有効化。接続は既存のUSB Studio RPCを使う。

更新前の動作確認済みFWの基準コミットは `b906b632f1d05c1532673ede4bbd387d524eff03`。

## 初期値と編集枠

| 種類 | 初期値 | 編集枠 |
| --- | --- | --- |
| コンボ0 | C+V → Ctrl+Shift+V | 合計8個 |
| コンボ1 | G+H → Runtime Macro 0 | 同上 |
| コンボ2 | T+Y → Runtime Macro 1 | 同上 |
| マクロ `my_kakko` | 日本語配列の `()` | 合計8個、共有プール1024バイト、1個256バイトまで |
| マクロ `my_kagi` | 日本語配列の `「」` | 同上 |

全コンボはレイヤー0のみ、判定80ms。従来どおりShift付きHIDキーコードを使い、ホスト側は日本語配列を想定する。
マクロのタップ時間は30ms、2キー間の待ち時間は15ms。

マクロの番号は名前と別に割り当てられる。保存済みRuntime Macroがない初回起動では、`my_kakko` が0、`my_kagi` が1に入る想定。
既存の保存済みマクロや作成・削除の順序によって割当番号が変わることがあるため、DYA Studioで実際の番号を確認し、コンボ1・2の呼び出し先も合わせる。

## 導入・確認

1. 既存のStudio設定を変更している場合は、更新前にエクスポートして控える。
2. 左右両方のFWをビルドし、両方のマイコンに更新版を書き込む。片側だけ旧ZMKのままにしない。
3. 左側をUSB接続して [DYA Studio](https://studio.dya.cormoran.works/) を開き、USB接続先を選択する。
4. キーマップが7レイヤー・各40キーで表示されることを確認する。保存済みStudio設定がFWの配列を上書きしている場合は、バックアップ後にStudioの初期値への復帰操作で確認する。
5. Macros & Combos画面で、初期コンボ3個・マクロ2個とマクロ番号を確認する。
6. 入力位置・判定時間・実行内容を編集して保存し、USBを抜き差し／再起動しても編集内容が維持されることを確認する。
7. 左スクロール・右ポインター、回転方向、クリック1〜3、自動マウス、Bluetooth切替、LEDも確認する。

`settings_reset` は設定やペアリングを消すため、通常の更新手順では使わない。
既存の `CONFIG_ZMK_STUDIO_LOCKING=n` を維持しているため、別のStudioアンロックキーは追加していない。

## 保存と初期値の注意

- Studioの変更はキーボード内の設定に保存され、Gitの `.keymap` へは自動反映されない。編集後はStudioからもバックアップする。
- Runtime Comboの保存値はFW内の初期値より優先される。初期値へ戻す場合は対応するリセット操作を使う。
- Runtime Macroの初期値は未登録時に起動時生成される。編集内容はSaveで保存する。初期マクロをDeleteしただけでは次回起動時に再生成されるため、恒久的に除去する場合はFWの初期値定義も変更する。
- 空きのマクロ枠へ追加したものは、Studioのキー割り当てやコンボからRuntime Macroの番号を指定して使う。
- 接続確認の対象はUSB。Bluetooth経由のStudio接続は未検証。入力ストリーム表示・トラックボールのGUI調整用の追加モジュールは導入していない。

## 検証状態

Windows上の隔離環境（Zephyr SDK 0.17.0、CMake 3.31.10、Python 3.12）で、左・右・settings_resetの3ターゲットをビルドし、UF2生成に成功。生成UF2のブロック構造・nRF52840のfamily IDも確認した。

| ターゲット | FLASH使用量（全788 KiB） | RAM使用量（全256 KiB） |
| --- | --- | --- |
| 左・Studio/Runtime機能あり | 325,752 B | 113,664 B |
| 右 | 207,768 B | 46,976 B |
| settings_reset | 58,916 B | 17,456 B |

7レイヤーのバインディングが動作確認済みの基準コミットと完全一致すること、Runtimeコンボ3個のキー位置・判定時間・対象レイヤー、初期マクロの登録順、生成Kconfig/DTSの左右設定も確認済み。
実機でのDYA Studio接続・編集・再起動後の永続保存・左右トラックボールの確認は未実施。

ローカルではSDK 0.17.4のpicolibcとZephyr 4.1の型定義が不一致となったため、Zephyr 4.1対応SDK 0.17.0で検証した。再現時も0.17.0を使う。


## 参照

- [DYA Studioの導入ガイド](https://studio.dya.cormoran.works/developer-guide/modules/keymap)
- [Runtime Macro](https://github.com/cormoran/zmk-feature-runtime-macro)
- [Runtime Combo](https://github.com/cormoran/zmk-feature-runtime-combo)
- [Zephyr 4対応PMW3610ドライバ](https://github.com/badjeff/zmk-pmw3610-driver/tree/zmk-0.4)
