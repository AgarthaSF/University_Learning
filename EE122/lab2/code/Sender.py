# -*- coding:utf8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
import getopt
import Checksum
import BasicSender
import os
import time

'''
This is a skeleton sender class. Create a fantastic transport protocol here.
'''

class Sender(BasicSender.BasicSender):
    def __init__(self, dest, port, filename, debug=False):
        super(Sender, self).__init__(dest, port, filename, debug)
        self.filesize = 4000                             # 指定读取文件大小
        size = os.path.getsize(filename)                 # 读取文件中的总字节数，首先确定包的总量
        self.maxPacketNum = (int(size) / self.filesize)  # 最大包数
        self.windowSize  = 5                             # 初始滑动窗口大小为5
        self.windowFront = 0                             # 滑动窗口起始位置
        self.windowEnd = self.windowSize                 # 滑动窗口终止位置
        self.slidingWindow = []                          # 滑动窗口
        self.seqno = 0                                   # 当前发送包号
        self.complFlag = False                           # 标识文件是否传输完成
        self.currentAck = 0                              # 记录当前ack
        self.dupTime = 0                                 # 标识重复次数
        self.timeout = 1.1                               # 丢包的重发时间
        self.packetNum = 0                               # 发包数量，用于检查废包率
        self.sentime = {}                                # 记录发送的时间

    def start(self):
        self.msg_type = None 
        while self.seqno < self.windowEnd and self.msg_type != 'end':
            msg = self.infile.read(self.filesize)  # 如果序列号为0说明为起始数据，数据长度不足filesize说明为结束数据，剩余情况都为中间的数据段
            if self.seqno == 0:
                self.msg_type = 'start'
            elif len(msg) < self.filesize:
                self.msg_type = 'end'
            else:
                self.msg_type = 'data'

            # 打包数据进入队列
            packet = self.make_packet(self.msg_type, self.seqno, msg)
            self.slidingWindow.append(packet)
            self.seqno += 1

        for packet in self.slidingWindow:  # 发送滑窗数据
            self.send(packet)
            self.sentime[int(packet.split('|')[1])] = time.time()
            self.packetNum += 1

        while self.complFlag != True:
            
            while True:                        # 接受消息直到得到新的ack为止
                response = self.receive(self.timeout + 0.01) 
                if response == None:           # 未接受有效信息，超时将窗口中的包重发
                    self.handle_timeout()      # 超时重发窗口中的所有包
                    break
                else:
                    # 如果校验和不正确直接跳过
                    if Checksum.validate_checksum(response) == True:
                        newack = int(response.split('|')[1])
                        
                        if self.currentAck == newack:
                            self.dupTime += 1
                            self.windowSize = max(5, self.windowSize / 2)
                        else:
                            RTT = 2 * (time.time() - self.sentime[self.currentAck])  # 调整重发时间
                            if self.currentAck == 0:  # 如果第一次时延较小，那么直接使用较小的重发时间，否则一个较大的重发时间是比较安全的
                                if(RTT < 0.8 ):
                                    self.timeout = RTT
                            self.timeout = 0.8 * self.timeout + 0.2 * RTT
                            self.dupTime = 0
                            self.currentAck = newack
                            if(self.timeout < 0.5):
                                self.windowSize = min(self.windowSize + 1, 20)
                                
                        if self.dupTime >= 3:        # 如果重复次数大于3认为最窗口最前方的包已丢失，将其重发
                            self.handle_dup()
                        self.handle_new_ack(newack)  # 处理新的ack

        self.infile.close()

    def handle_timeout(self):   # 处理超时，将窗口中的包重发
        for packet in self.slidingWindow:
            self.send(packet)
            self.sentime[int(packet.split('|')[1])] = time.time()
            self.packetNum += 1

    def handle_new_ack(self, ack):            # 处理新ack
        if ack == self.maxPacketNum + 1:      # 如果ack等于最大包容量+1则说明传输完成
            self.complFlag = True
            return 
        while ack > self.windowFront:         # 将已发送的包出队
            self.slidingWindow.pop(0)
            self.windowFront += 1
        
        currentsize = len(self.slidingWindow) # 标识当前已发送的包的位置

        self.windowEnd = min(self.maxPacketNum + 1, self.windowFront + self.windowSize)  # 将新包入队
        while self.seqno < self.windowEnd and self.msg_type != 'end':
            msg = self.infile.read(self.filesize)
            packet = self.make_packet(self.msg_type, self.seqno, msg)
            self.slidingWindow.append(packet)
            self.seqno += 1

        # 将新获取的包发送出去
        for i in range(currentsize, self.windowEnd - self.windowFront):
            self.send(self.slidingWindow[i])
            self.sentime[int(self.slidingWindow[i].split('|')[1])] = time.time()
            self.packetNum += 1
        
    def handle_dup(self):
        self.send(self.slidingWindow[0])
        self.sentime[int(self.slidingWindow[0].split('|')[1])] = time.time()
        self.packetNum += 1
        

    def log(self, msg):
        if self.debug:
            print (msg)

'''
This will be run if you run this script from the command line. You should not
change any of this; the grader may rely on the behavior here to test your
submission.
'''
if __name__ == "__main__":
    def usage():
        print "BEARS-TP Sender"
        print "-f FILE | --file=FILE The file to transfer; if empty reads from STDIN"
        print "-p PORT | --port=PORT The destination port, defaults to 33122"
        print "-a ADDRESS | --address=ADDRESS The receiver address or hostname, defaults to localhost"
        print "-d | --debug Print debug messages"
        print "-h | --help Print this usage message"

    try:
        opts, args = getopt.getopt(sys.argv[1:],
                               "f:p:a:d", ["file=", "port=", "address=", "debug="])
    except:
        usage()
        exit()

    port = 33122
    dest = "localhost"
    filename = "README"
    debug = False

    for o,a in opts:
        if o in ("-f", "--file="):
            filename = a
        elif o in ("-p", "--port="):
            port = int(a)
        elif o in ("-a", "--address="):
            dest = a
        elif o in ("-d", "--debug="):
            debug = True

    s = Sender(dest,port,filename,debug)
    try:
        s.start()
    except (KeyboardInterrupt, SystemExit):
        exit()
