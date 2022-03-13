# windows-rtsp-yinli_plan
功能：
pusher：实现对windows的抓屏，并将画面通过RTSP协议推流到云服务器端；同时接收云上发来的指令(本代码中发来的是客户端发来的按键键值)
client: 实现windows下拉流并显示；同时向云上的指令中转服务器发送键值
云上：自行搭建RTSP服务器
server.cpp:放在云服务器上的指令中转服务器

延时约100ms
![image](https://user-images.githubusercontent.com/83794882/158041498-9c57440d-2ae9-4c46-b85b-552bfb3c142e.png)


指令中转：（实现在拉流端通过键盘可以控制推流端的界面，本代码中实现拉流端通过键盘控制推流端鼠标，从而打开文档，并实现输入）
![image](https://user-images.githubusercontent.com/83794882/158041437-39ce7de1-de30-4705-89ca-4e0e2f6be1a7.png)
