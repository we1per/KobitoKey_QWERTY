# D+FでAlt+Tabを使う

keyballの `qmk_firmware/keyboards/keyball/keyball39/keymaps/via/keymap.c` の
`CMB_ALTTAB` と `process_combo_key_repress` を基にした、小人キー用の専用ビヘイビア。

## 操作

- レイヤー0でD+Fを80ms以内に押すと、Altを保持してTabを1回送る。
- Dを保持し、Fを離して押し直すと次のウィンドウへ進む。
- コンボ保持中にSを押すとShift+Tabで前へ戻る。
- D/Fを両方離すとAltを解放して確定する。
- 通常時のD/F/Sは従来どおり入力する。動作中のF/Sは文字として送信しない。

F/Sは押すたびに1回切り替える。F/Sの押下中にコンボを終了した場合、そのF/Sの解放も消費する。
Altは通常のZMKキーイベントで登録・解除するため、別のAltキーの保持とは別に扱う。

## DYA Studioでの設定

専用ビヘイビア `Alt+Tab switcher`（`&alttab`）を追加した。
Runtime Comboの初期値は以下。

| 項目 | 値 |
| --- | --- |
| Slot | 3（表示順では4番目） |
| Name | CMB_ALTTAB |
| Positions | 12、13（D、F。0始まり） |
| Binding | Alt+Tab switcher |
| Layers | 0 |
| Timeout | 80ms |
| Slow Release | ON（コンボの全キーを離すまで保持） |

既存のコンボ0〜2、マクロ、各レイヤーのキー配置は変更しない。
保存済みのslot 3がある場合、その設定を優先するため上書きしない。
その場合は空いているコンボ枠へ上記内容を手動で作成する。
`settings_reset` やコンボ全体のリセットは不要。

コンボ位置・有効レイヤー・判定時間はDYA Studioで変更できる。
**Slow ReleaseはONのまま使う。** OFFでは最初のキー解放でAltも解放される。
操作中のF/SはHIDキーコードで判定する固定仕様。コンボのキー位置を変えてもF/Sの役割は変わらない。
FはD+Fコンボの判定対象でもあるため、押し直し時にコンボ判定待ち（初期値で最大80ms）が入る場合がある。

更新前にStudio設定をエクスポートし、左FWを更新してUSB接続する。
更新後、Macros & Combos画面でCMB_ALTTABとSlow Releaseを確認して保存する。
キー操作中にコンボ内容を変更する使い方は対象外。

## 検証

```sh
cc -std=c11 -Wall -Wextra -Werror -pedantic pointing/tests/alt_tab_policy_test.c -o /tmp/alt_tab_policy_test
/tmp/alt_tab_policy_test
python pointing/tests/check_firmware.py build/zephyr/zmk.elf
```

Cテストでは通常入力への非干渉、前後移動、複数の呼び出し元でのAlt保持、終了後のF/S解放処理、再発動を確認する。
リンク後チェックでは、F/SをHIDへ送る前に専用リスナーが消費できる順序を確認する。

2026-11-03のローカル検証結果:

- Alt-TabのCテストと既存自動マウスのCテストが成功（Zig 0.13.0、C11）。
- Zephyr SDK 0.17.0で左右FWのビルド・UF2生成が成功。UF2のブロック構造・family IDを確認。
- リンク後のリスナー順序チェックが成功。
- 既存7レイヤーのbindingsとコンボ0〜2の定義が変更前と同一であることを確認。
- 左FLASH 351124 B／RAM 127736 B。右FWは前回の生成物とSHA-256も同一のため、今回は左のみ更新可能。
- 依存コードの既存警告は残る。専用ビヘイビアの追加分にはコンパイル警告なし。

実機ではD+Fの発動・Fで連続移動・Sで逆順・両方解放で確定・通常のD/F/S入力を確認する。
先にDを離す順序、S/Fを押したままDを離す順序、通常のAltキーとの併用も確認対象。
実機でのAlt+TabとStudio保存の確認は未実施。
