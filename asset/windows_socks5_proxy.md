打开注册表编辑器，路径输入：计算机\HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Internet Settings里面有一项 ProxyServer，修改值为 socks://HOST:PORT 即可。注意这里不是 socks5，是 socks，改完之后类似于浏览器之类的软件就会走 Socks5 代理了。

ProxyEnable为1，即为开启状态才生效