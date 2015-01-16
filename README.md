# lua_auth_plugin

Mosquittoの認証処理をLuaで書くためのプラグインです。

基本的な構成、アイデアは[mosquitto_pyauth](https://github.com/mbachry/mosquitto_pyauth)を参考にしています。

# Install
事前にMosquitto、Luaのインストールが必要です。

なお、Makefileのリンクするライブラリパス、インストール先は各々の環境に応じて適宜修正してください。
```
cd lua_auth_plugin
make
make install
```

# Configuration
`mosquitto.conf`に以下の設定を追加してください。

また、`auth_opt_*`を独自に追加することで、自作プラグイン中で参照可能な設定を追加することができます。

```
auth_plugin /path/to/lua_auth_plugin.so
auth_opt_auth_file /path/to/your_auth_file.lua
```

# Lua function
作成するLuaファイルには以下の関数を用意してください。
- `plugin_init(opts)`：プラグインの初期化時に呼び出されます。`opts`は`mosquitto.conf`の`auth_opts_*`の設定名をキーとしたテーブルです。
- `plugin_cleanup(opts)`：プラグインの終了時に呼び出されます。
- `security_init(opts, reload)`：プラグインの初期化時、設定のリロード時に呼ばれます。リロード時は、`reload`が`true`で呼ばれます。
- `security_cleanup(opts, reload)`：プラグインの終了時、設定のリロード時に呼ばれます。
- `acl_check(clientid, username, topic, access)`：ブローカーがトピックへのアクセスをチェックする際に呼ばれます。`access`はSubscribeの場合、`MOSQ_ACL_READ`が、Publishの場合、`MOSQ_ACL_WRITE`が設定されます。
- `unpwd_check(username, password)`：ブローカーがusername/passwordをチェックする際に呼ばれます。

# C Function
以下の関数は、Luaから使用できるCの関数です。
- `topic_matches_sub(sub, topic)`：`topic`が`sub`に一致するかどうかを判定する関数です。
