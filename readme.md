# Documentation

可支持联机对战的象棋软件.

2018年夏季学期程序设计小学期第二周作业.

- [Documentation](#documentation)
  - [User Guide](#user-guide)
    - [Fundamental Functionality](#fundamental-functionality)
    - [Create](#create)
    - [Connect](#connect)
    - [Save](#save)
    - [Surrender](#surrender)
    - [How to Play](#how-to-play)
  - [Feature](#feature)
  - [Development](#development)
    - [Work Process](#work-process)
    - [Transmission Protocol](#transmission-protocol)
    - [Network Framework](#network-framework)
  - [Acknowledgement](#acknowledgement)

## User Guide

### Fundamental Functionality

- Create: 创建游戏并等待连接
- Connect: 连接主机进行游戏.
- Save: 保存当前棋局.
- Surrender: 主动发起投降.

### Create

从棋局文件中加载棋局, 创建新游戏, 等待其他人连接以开始游戏.
- 可通过快捷键`Alt+C`调用
- 在游戏过程中无法使用
- 默认开启一局新游戏
- 可选择端口号
    - 默认端口号: `7777`
- 可选择先/后手
- 等待连接时可随时取消等待
- 棋局文件在`campaign`文件夹中, 以`.txt`为文件扩展名.

### Connect

输入IP地址及端口号, 连接正在等待连接的主机, 开始一局游戏.
- 可通过快捷键`Alt+O`调用
- 在游戏过程中无法使用
- 连接成功后自动开始游戏
- 无法选择先/后手
- 会导入连接主机所选择的棋局及先后手信息

### Save

保存当前棋局, 以当前正在行棋的一方为先手.
- 可通过快捷键`Alt+S`调用
- 仅能在游戏过程中使用
- 保存格式包括两方的棋盘信息, 在文件前部的是先手, 每一方的棋盘信息包括如下内容:
    - 第一行一个字符串`red`/`black`, 表明该方颜色
    - 接下来七行, 每行代表一种棋子, 以将/帅, 士, 象, 马, 车, 炮, 兵/卒的顺序, 每行开始是一个数字`k`, 代表该种棋子的数量, 接下来k个数对`<x,y>`, 表示该种的棋子的坐标.
- 棋盘以`.txt`后缀名在`campaign`文件夹中读取/保存.
- 注意: 因为并未对棋盘的合法性作检验, 请**不要**手动修改棋盘.

### Surrender

- 当满足一下条件时, 会触发投降
    - 主动发起
        - 可通过快捷键`Alt+U`调用
        - 仅能在游戏过程中使用
    - 被将死
    - 行棋时间超过20s时间限制
- 一旦一方投降, 双方会弹出胜利/失败信息, 游戏结束, 连接断开.

### How to Play

- 屏幕下方为己方
- 每步棋有20s时间限制, 会显示于底部状态栏
- 通过点击鼠标下棋
    - 第一次点击选中棋子
    - 第二次点击移动棋子/吃子
    - 若移子/吃子点击不符合象棋规则/导致自己被将军, 会清空选中信息. (可以此来取消选中)
- 未选中棋子时, 会显示敌方上一步行棋
- 吃子/将军会有音效提示
- 不能送将
- 被将死则会自动判负

## Feature

- 功能快捷键均通过下划线提示
- 显示敌方上一步行棋
- 处于选中状态的棋子会被框出

## Development

### Work Process

除了*向socket中写入信息*, 其余工作均为异步实现.

本地工作
- [save](#save)
- game
    - start
        - 连接建立时游戏自动开始
        - 发送/接受初始棋局
        - 在棋盘中绘出初始棋局
        - 更改功能`create`, `connect`的状态为**无法使用**
        - 更改功能`save`, `surrender`的状态为**可以使用**
    - end
        - 任意一方触发投降(主动或被将死)时游戏结束
        - 会弹出消息提示你方胜利/失败
        - 更改功能`create`, `connect`的状态为**可以使用**
        - 更改功能`save`, `surrender`的状态为**无法使用**
- think: 思考阶段, 在一定时间(20s)内等待一步行棋.
    - start
        - 由游戏开始或敌方行棋事件触发
        - 开始计时, 当前剩余时间显示于底部状态栏
        - 若上一步为敌方行棋, 会显示敌方行棋
    - end
        - 任意一方投降时思考阶段结束.
        - 做出行棋则思考阶段结束.
        - 计时停止, 下方状态栏消息被清空.
        - 你需要等待敌方思考并行棋.
        - 己方行棋仍然会被留存显示

网络工作
|           | send                                                 | receive                                          |
| --------- | ---------------------------------------------------- | ------------------------------------------------ |
| game      | 由[create](#create)触发, 发送棋局并开始游戏          | 由[connect](#connect)触发, 接受棋局并开始游戏    |
| campaign  | 当游戏开始时被Server端发送                           | 当游戏开始时被Client端接受                       |
| step      | 通过捕获鼠标点击事件触发                             | 收到行棋后将开启思考阶段, 并在棋盘中显示敌方行棋 |
| surrender | 由[触发条件](#surrender)触发, 弹出失败消息, 游戏结束 | 收到投降信息后, 弹出胜利消息, 游戏结束           |

### Transmission Protocol

信息以`json`格式传输, 每一个信息流必包含一个`key`值`head`, 其`value`是以下三个字符串之一: `campaign`, `step`, `surrender`. 据此对信息流作进一步解析.
- `campaign`: 初始棋局信息, 此时信息流中会有一个`campaign`值.
    - `campaign`(`QString`): 初始棋局, 信息格式与[Save](#save)中所述棋局文件格式相同.
- `step`: 一步行棋信息, 此时信息流中会有`x1`, `y1`, `x2`, `y2`, 表示一个在`(x1,y1)`的棋子移动到了`(x2,y2)`, 如果`(x2,y2)`原本已有棋子, 则会吃掉那里原有的棋子并触发吃子音效.
    - `x1`(`qint32`): 棋子初始横坐标.
    - `y1`(`qint32`): 棋子初始纵坐标.
    - `x2`(`qint32`): 棋子目标横坐标.
    - `y2`(`qint32`): 棋子目标纵坐标.
- `surrender`: 投降信息, 表示信息发出者请求投降.

### Network Framework

基于TCP/IP协议簇和`QTcpServer`, `QTcpSocket`开发.

`QTcpServer`
- 创建游戏之后会在指定端口监听全部IP.
- 监听状态可被随时取消.

`QTcpSocket`
- 创建/连接游戏之后建立.
- 以[Transmission Protocol](#transmission-protocol)中所述的通信协议进行通信.
- 当有一方投降时, 游戏结束, 连接断开.

## Acknowledgement

感谢[象棋巫师](https://www.xqbase.com/)提供的图片及音频素材.

感谢ColinDuquesnoy提供的[QDartStyle](https://github.com/ColinDuquesnoy/QDarkStyleSheet).

感谢助教, 老师和同学的帮助. 