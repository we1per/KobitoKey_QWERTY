# ブランチ運用

- `upstream` は `juichi50iii/KobitoKey_QWERTY`、`origin` は `we1per/KobitoKey_QWERTY`。
- `main` は upstream の `main` を保持するブランチ。独自変更のコミットや作業ブランチのマージ先にしない。upstream と同期する場合は fast-forward のみとし、独自のマージコミットを作らない。
- `we1per_main` は独自変更を育てる統合ブランチであり、origin のデフォルトブランチ。
- 通常の作業ブランチは `we1per_main` を起点に作成する。各ブランチで変更と検証を行い、検証済みの内容を `we1per_main` へ取り込む。PR のベースも `we1per_main` とする。
- 検証状況は静的確認、ファームウェアのビルド、実機確認を区別して記録する。未確認の項目を検証済みとして扱わない。
- 既存の作業途中の変更を破棄せず、ブランチ構成の変更だけを理由に未検証の変更を統合しない。
