# windows-rtsp-yinli_plan
功能：
pusher：实现对windows的抓屏，并将画面通过RTSP协议推流到云服务器端；同时接收云上发来的指令(本代码中发来的是客户端发来的按键键值)
client: 实现windows下拉流并显示；同时向云上的指令中转服务器发送键值
云上：自行搭建RTSP服务器
server.cpp:放在云服务器上的指令中转服务器
