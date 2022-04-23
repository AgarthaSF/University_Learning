# -*- coding:utf8 -*-
import Tkinter
import threading
from Tkinter import *
from socket import *
import time
import inspect
import ctypes
import tkMessageBox as msg
import sys
reload(sys)
sys.setdefaultencoding('utf-8')


# 使用ctypes，以异常抛出的方式强制结束线程
def _async_raise(tid, exctype):
    """raises the exception, performs cleanup if needed"""
    tid = ctypes.c_long(tid)
    if not inspect.isclass(exctype):
        exctype = type(exctype)
    res = ctypes.pythonapi.PyThreadState_SetAsyncExc(tid, ctypes.py_object(exctype))
    if res == 0:
        raise ValueError("invalid thread id")
    elif res != 1:
        # """if it returns a number greater than one, you're in trouble,
        # and you should call it again with exc=NULL to revert the effect"""
        ctypes.pythonapi.PyThreadState_SetAsyncExc(tid, None)
        raise SystemError("PyThreadState_SetAsyncExc failed")
 
def stop_thread(thread):
    _async_raise(thread.ident, SystemExit)

class mainInterface(Tkinter.Frame):

    def showChatWindow(self, Button):
        id = Button.cget('text').encode('utf-8')
        chatWidown = myChatWindow(id)

    # 开启监听线程获取聊天窗口中发送的信息，并将消息转发至服务器
    def getWindowInfo(self):
        self.localUdpSerSock.bind(self.localADDR)
        while True:
            localData, localaddr = self.localUdpSerSock.recvfrom(1024)

            if(localData[:2]== "04"):
                # 由04协议语句进行哈希知晓端口号，更改端口号
                tempstr = localData[3:-1]
                port = int(tempstr)%65535
                self.sendlocalADDR = ('localhost', port)

            self.outUdpCliSock.sendto(localData, self.outsideADDR)

            # 获取服务器信息，以便调试
            data1, ADDR = self.outUdpCliSock.recvfrom(1024)
            print ("from mainInterface: ",data1)

            # 如果为03消息，确定对方是否在线
            if data1[:2] == "03":
                self.outUdpCliSock.sendto(data1, self.sendlocalADDR)
            
            # 如果消息为04，说明正在查询未读消息
            if data1[:2] == "04":
                # 先将04协议消息传至子窗口使其知晓有多少条消息需要处理
                self.outUdpCliSock.sendto(data1, self.sendlocalADDR)
                number = data1.split(':')[1]
                number = int(number)
                # number为0说明没有未读消息
                if number == 0:
                    pass
                # 不为0则读取所有未读消息
                else:
                    for i in range(number):

                        # 先将消息由子窗口发送至主菜单
                        sendInfo, ADDR = self.localUdpSerSock.recvfrom(1024)
                        # 再由主菜单转发至服务器
                        self.outUdpCliSock.sendto(sendInfo, self.outsideADDR)
                        # info即为从服务器接受所得消息
                        info, ADDR = self.outUdpCliSock.recvfrom(1024)
                        # 再将消息转发至子窗口进行处理
                        self.outUdpCliSock.sendto(info, self.sendlocalADDR)

    def quit_logout(self):
        # 退出登录，结束UDP连接，关闭线程
        logout_data = "06#"
        self.outUdpCliSock.sendto(logout_data, self.outsideADDR)
        self.localUdpSerSock.close()
        self.outUdpCliSock.close()
        # 结束程序
        sys.exit()

    # 退出对话框
    def create_quit_window(self):
        quit_window = Tkinter.Toplevel(self.master)
        #Give our quit window a title and minimum size.
        quit_window.title("Quit?")
        quit_window.minsize(width=150, height=50)
        #Display a message to the user asking if they want to quit.
        quit_label = Tkinter.Label(quit_window, text="Are you sure you want to quit?")
        quit_label.pack()
        #We give our window a yes and no button.  One quits the application and one quits
        #the window.
        yes_button = Tkinter.Button(quit_window, text="Yes", command=self.quit_logout)
        yes_button.pack()
        no_button = Tkinter.Button(quit_window, text="No", command=quit_window.destroy)
        no_button.pack()

    # 构造函数
    def __init__(self, master, id, pw):


        self.localADDR = ('', 22580)
        self.outsideADDR = ('208.167.253.85',21568)
        self.sendlocalADDR = ('localhost', 23000)

        self.localUdpSerSock = socket(AF_INET, SOCK_DGRAM)
        self.outUdpCliSock = socket(AF_INET, SOCK_DGRAM)

        Tkinter.Frame.__init__(self, master, padx=10, pady=10)
        master.title("主界面")
        master.minsize(width=300, height=500)

        # 打开时应用时自动登陆，并检查是否登陆成功
        login_send = "02#" + id + "#" + pw + "#"
        self.outUdpCliSock.sendto(login_send, self.outsideADDR)
        data1, ADDR = self.outUdpCliSock.recvfrom(1024)
        print (data1)

        # 创建按钮，并连接按钮与函数
        self.fristfButton = Button(master, text="20201002303", command=lambda: self.showChatWindow(self.fristfButton), padx=10, pady=10)
        self.fristfButton.pack()  

        self.secondfButton = Button(master, text="20201003102", command=lambda: self.showChatWindow(self.secondfButton), padx=10, pady=10)
        self.secondfButton.pack()

        self.thirdfButton = Button(master, text="20201002690", command=lambda: self.showChatWindow(self.thirdfButton), padx=10, pady=10)
        self.thirdfButton.pack()

        self.forthfButton = Button(master, text="20201003729", command=lambda: self.showChatWindow(self.forthfButton), padx=10, pady=10)
        self.forthfButton.pack()

        self.fifthfButton = Button(master, text="20201002176", command=lambda: self.showChatWindow(self.fifthfButton), padx=10, pady=10)
        self.fifthfButton.pack()

        # 退出与登出按钮
        self.quitButton = Button(self, text="logout", command=self.create_quit_window,padx=10, pady=10)
        self.quitButton.pack()

        self.pack()

        # 开启子窗口消息监听线程，设置其为Daemon线程，使得主线程退出时子线程会与其一同结束
        self.listenThread = threading.Thread(target=self.getWindowInfo)
        self.listenThread.setDaemon(True)
        self.listenThread.start()

        # 重写窗口关闭事件
        master.protocol('WM_DELETE_WINDOW', self.quit_logout)

        master.mainloop()

class myChatWindow():
    # 初始化界面布局，ip，端口号
    def __init__(self, id):
        self.window = Tk()
        # 初始化ip，端口号，建立udp连接
        self.ADDR = ('localhost', 22580)
        self.serADDR = ('', int(id)%65535)
        self.udpCliSock = socket(AF_INET, SOCK_DGRAM)
        self.windowSerSock = socket(AF_INET, SOCK_DGRAM)

        # 初始化图形化界面
        title = "与" + id + "聊天中"
        self.window.title(title)

        self.id = id

        # 定义frame容器，将其root设置为聊天窗口
        frame_left_top = Frame(self.window, width=380, height=270, bg='white')
        frame_left_center = Frame(self.window, width=380, height=100, bg='white')
        frame_left_bottom = Frame(self.window, width=380, height=30)
        frame_right = Frame(self.window, width=170, height=410, bg='white')

        # 连接按钮与函数
        button_sendmsg = Button(frame_left_bottom, text="发送", command=lambda: self.sendmessage(id))

        # 获取容器中的内容
        self.text_msglist = Text(frame_left_top)
        self.text_msg = Text(frame_left_center);
        # 设置tag green和blue的具体效果
        self.text_msglist.tag_config('green', foreground='#008B00')
        self.text_msglist.tag_config('blue', foreground='blue')
        self.text_msglist.tag_config('red', foreground='#FF0000')

        # 设置容器位置
        frame_left_top.grid(row=0, column=0, padx=2, pady=5)
        frame_left_center.grid(row=1, column=0, padx=2, pady=5)
        frame_left_bottom.grid(row=2, column=0, pady=3)
        frame_right.grid(row=0, column=1, rowspan=3, padx=4, pady=5)
        frame_left_top.grid_propagate(0)
        frame_left_center.grid_propagate(0)
        frame_left_bottom.grid_propagate(0)

        # 将元素填充进frame
        self.text_msglist.grid()
        self.text_msg.grid()
        button_sendmsg.grid(sticky=E)

        # 开启新线程用于接受聊天对象聊天内容

        self.getInfoThread = threading.Thread(target=self.getfriendInfo)
        self.getInfoThread.setDaemon(True)
        self.getInfoThread.start()

         # 重写窗口关闭事件
        self.window.protocol('WM_DELETE_WINDOW', self.quit_close)

        self.window.mainloop()

    def quit_close(self):
        # 在关闭窗口时将端口释放，防止下次打开时会占用端口
        self.udpCliSock.close()
        self.windowSerSock.close()
        stop_thread(self.getInfoThread)
        self.window.destroy()

    def sendmessage(self, id):
        # 消息发送者以及发送时的本地时间
        msgcontent = "我" + " " + time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()) + '\n '
        # 创建text_msglist的链表，将msgcontent内容以绿色的形式加入该链表
        self.text_msglist.insert(END, msgcontent, 'green')
        # 将输入框中的内容追加至链表中，使其显示在文本框中
        self.text_msglist.insert(END, self.text_msg.get('0.0', END))

        # 将输入框中的消息按协议进行处理后发送至服务器
        send_data = "03#" + id + "#" + self.text_msg.get('0.0', END)
        send_data = send_data[:-1] + "#"
        self.udpCliSock.sendto(send_data, self.ADDR)

        info, ADDR = self.windowSerSock.recvfrom(1024)
        if info == "03:02":
            self.text_msglist.insert(END, "(对方已离线)", 'red')

        # 清空文本框中的内容
        self.text_msg.delete('0.0', END)

    def getfriendInfo(self):
        # 获取来自主窗口的消息
        self.windowSerSock.bind(self.serADDR)

        while True:
            # 获取未读信息条数
            acquireInfo = "04#" + self.id + "#"
            self.udpCliSock.sendto(acquireInfo, self.ADDR)

            number, ADDR = self.windowSerSock.recvfrom(1024)
            print("from window: ", number)
            number = number.split(':')[1]
            number = int(number)

            # 如果未读信息条数为0则不作处理
            if number == 0:
                pass
            # 不为0则使用协议05获取所有消息
            else:
                for i in range(number):
                    # 先将消息转发至主菜单
                    gainInfo = "05#" + self.id + "#"
                    self.udpCliSock.sendto(gainInfo, self.ADDR)
                    
                    # 接受主菜单获得的由服务器发送的消息，进行处理
                    info, ADDR = self.windowSerSock.recvfrom(1024)
                    print("from window: ", info)
                    strlist = info.split(':')

                    strtime = strlist[2] + ":" + strlist[3] + ":" + strlist[4]
                    # 查找第五个出现的：的位置，获取好友发送的信息
                    n = 5
                    start = info.find(":")
                    while start >= 0 and n > 1:
                        start = info.find(":", start+len(":"))
                        n -= 1
                    strChat = info[start+1 :] + '\n'

                    # 消息发送者以及发送时的时间
                    msgcontent = self.id + " " + strtime + '\n '
                    # 创建text_msglist的链表，将msgcontent内容以蓝色的形式加入该链表
                    self.text_msglist.insert(END, msgcontent, 'blue')
                    # 将输入框中的内容追加至链表中，使其显示在文本框中
                    self.text_msglist.insert(END, strChat)

            # 每10秒查询一次未读消息
            time.sleep(10)
        

# 构建登录对话框
window = Tk()
window.title("Login")
window.geometry("350x150")

Label(window, text="Id: ").place(x=50, y=20)
Label(window, text="Password: ").place(x=50, y=60)

var_id = StringVar()
var_pw = StringVar()

entry_id = Entry(window, textvariable=var_id).place(x=160, y=20)
entry_password = Entry(window, textvariable=var_pw, show='*').place(x=160,y=60)

def login():
    id = var_id.get()
    pw = var_pw.get()

    if id == "" or pw == "":
        msg.showwarning(title='Login', message='Please fill the blank')
    else:
        data = "02#" + id + "#" + pw + "#"
        ADDR = ('208.167.253.85',21568)
        udpCliSock = socket(AF_INET, SOCK_DGRAM)
        udpCliSock.sendto(data,ADDR)

        data1,ADDR=udpCliSock.recvfrom(1024)
        print(data1)
        if data1 == "02:01" or data1 == "02:04":
            msg.showinfo(title='Login', message='Login successful')
            window.destroy()
            mainroot = Tk()
            app = mainInterface(mainroot, id, pw)
            app.mainloop()
        elif data1 == "02:02":
            msg.showwarning(title='Login', message='Password error')
        elif data1 == "02:03":
            msg.showwarning(title='Login', message='User does not exist')

button_login = Button(window, text='Login', command=login).place(x=150, y=100)
window.mainloop()