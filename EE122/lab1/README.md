## 实现功能

### 登陆界面

#### 基本界面

在`Id`与`Password`中分别输入用户名与密码后按`Login`按钮确认登陆。在函数中获取两个输入栏中的输入，将其封装为`02`协议的对应形式，按下`Login`按钮后将封装的数据发送至服务器，根据服务器的返回值判断登陆是否成功。

#### 登陆成功

输入正确的用户名与密码后就会弹出登陆成功弹窗，并打开主界面。

#### 用户已登陆

在编写代码与理解协议时我发现了这样一个情况：如果不由用户主动发送`06#`协议离线，服务器将认为用户仍在线上。但实际情况是，如果一段时间不进行操作，服务器就会自动与用户断开连接。这就导致了很多时候我们是在未发送`06#`协议情况下被强制下线的。这个时候再次连接服务器就会返回`02:04`，即用户已在线，但实际进行的操作却是登陆。考虑到这个情况，此处将`02:04`与`02:01`一同视为登陆成功。

### 离线提示

当与不在线的好友进行聊天时，会在发送的消息后提示对方已经离线。每次发送信息时，消息由聊天窗口发送至主菜单，再由主菜单转发至服务器。紧接着，主菜单再将从服务器接受到的`03`协议返回值发送至聊天窗口。如果接受到的返回值为`03:02`，则说明发送消息的对方已离线，在文字后给出提示。

### 双向聊天

每30秒使用一次`04`协议查询当前聊天对象的未接受消息是否为0。如果未接受消息不为0，则处理收到的协议返回值确定有多少条未读消息，并向主菜单发送相应次数的`05`协议获取消息，主菜单再将这些消息转发至服务器。在接受消息时由服务器接收到好友发送的消息，再转发至聊天窗口。在聊天窗口中对内容进行处理后将其展示在聊天框中。此

### 同时与多人双向聊天

为了确保在多个聊天窗口中接收的消息不混乱，我们需要在每一个聊天窗口中单独开启一个服务端进程，并为他们指定不同的端口。为保证端口不同并在端口范围内，我将端口号取为学号模65535。在主菜单的监听函数中，每当收到的协议为`04`类型，处理数据获得与聊者的学号，将学号模65535作为向聊天窗口发送的UDP服务端的新端口。通过这样的设计，在主菜单中只需使用一个监听端口，通过在不同发送端口之间的来回切换就可将消息转发至各个聊天窗口中。

## 遇到的问题

### Tk框架的使用

由于对Tk框架并不熟悉且其在网络上的参考资料较少，在了解代码含义与实现自己所需功能，如重写窗口退出函数进行合理析构，开启新窗口等功能的实现中花费了较多时间。在开启新窗口，由于不知道要将容器的`master`属性设为对应窗口的`Tk()`类实例，导致新窗口的部件显示在了原窗口上，经过数小时的资料查询后找到了该问题的解决方法。

### 子线程的销毁

如果在关闭聊天窗口时不销毁监听线程，就会导致窗口无法彻底关闭，使程序卡死。同理，如果在关闭主菜单时候不销毁监听线程，也同样会出现这种问题。对于聊天窗口的子线程，还要注意在销毁它的时候需要同时释放在聊天窗口中单独打开的UDP端口，否则下次再次点开该聊天窗口时就会因为端口已被占用而报错。将聊天窗口中的监听线程设为守护线程，这样在聊天窗口实例销毁时线程也会一同退出。但经过测试发现这样处理有时会出现`error: [Errno 9] Bad file descriptor`的错误，因此重写窗口关闭函数，在其中使用抛出异常的方法强行终止聊天窗口监听子线程。对于主界面，将监听线程设为守护线程，这样在主线程退出时监听线程就会一同结束。

### 与多人聊天的实现

设计在主菜单中只使用一个监听线程的实现是较为耗时的一个部分。一开始我在构思出了大致实现的方法后就开始编写代码，但并没有成功。后整体考虑了各个部件之间的消息收发情况，确定了要编写的函数内容与聊天窗口与主菜单间的通信，主菜单与服务器间的通信情况后成功实现了功能。

### UDP端口的释放

一个比较自然的想法是，我们只需要在聊天窗口的析构函数中释放对应的UDP端口，就可以完成各个端口的释放，避免后续UDP端口的使用发生冲突。然而，如果直接这样去写，端口并没有如我们所想一样得到释放。关于为何析构函数不能成功释放端口目前我也没有得到比较明确的答案，我猜想关闭聊天窗口是一种强行终止的操作，当进行这种操作时聊天窗口类中的对象被直接终止，没有执行析构函数就被销毁了，因此导致端口没有正常释放。

通过查询资料我发现可以重写窗口的关闭操作，所以我重写了关闭函数，在窗口关闭之前就先将占用的端口释放，这样果然成功的释放了相应的UDP端口。

### Python2中的中文编码问题

之前在使用Python3时，遇到中文编码无法解析的情况在程序开头加上编码标识即可解决问题。

```python
# -*- coding:utf8 -*-
```

然而在Python2中加上该编码表示并不起作用，在经过资料查询后发现额外加上以下消息才可以正常解析中文编码

```python
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
```

## 外部帮助

本次实习全程由自己完成，在实现功能时查询了许多网上资料去了解相关知识。大多查询的内容过于碎片化，在此处给出几个我在查询过程中觉得帮助很大的资料。

### 子线程销毁

查询`StackOverflow`获取了解决方法

https://stackoverflow.com/questions/323972/is-there-any-way-to-kill-a-thread

https://zhuanlan.zhihu.com/p/142781154

## 程序问题

由于设置了每30秒向查询一次未读消息，在查询期间主菜单会持续与服务器进行通信。如果这个时候正好自己也发送了消息，就会导致一些异常情况的发生，如处理`05`协议的函数接收到的是`03`协议的返回值，使程序出现错误。这种错误可能是本应接受到的消息并没有收到，而`recvfrom`一直等待接受，导致程序卡死。也可能是接收的协议消息并不是函数本应处理的消息，导致对数据的处理上出现了错误，导致程序的某个部分卡死，如在重新启动之前不能再收到外部信息（接收轮询卡死）等。

这个问题在同时打开多个窗口后会进一步凸显。因为每一个窗口每隔一段时间都会进行一次查询，如果有未读消息还会继续进行消息的查询转发。当打开多个窗口后因接受消息错乱发生错误的可能性也进一步增加了。

一个比较幼稚的想法是为使用较频繁的几种协议单独开启线程，如为`03`与`05`协议单独开启一条收发线程，但是这并没有在本质上解决问题，如果消息量进一步增大，仍然有可能出现因消息混乱导致的错误。这个问题还需进一步思考。

## 测试相关

在测试时如需指定额外的好友进行聊天，请在约126行的位置加上如下语句，并在text属性中输入测试账号名以创建新的按钮。

```python
        self.testButton = Button(master, text="测试账号Id", command=lambda: self.showChatWindow(self.testButton), padx=10, pady=10)
        self.testButton.pack()
```
