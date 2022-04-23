## 基础实现

以gbn协议为基础进行实现。

首先根据当前要发送的文件大小以及包容量确认一共需要发送多少个包，作为最大包数。建立标识self.complFlag作为传输是否完成的标志，如果当前的ack等于最大包数，说明传输完成，将其置为True。

在Sender中将带有`start`标识，即起始包发送至Receiver，直到确认接收为止，进入主循环。

主循环：当self.complFlag不为真时进行该循环，该循环流程如下：

1. 接收Receiver端的消息，如果等待了timeout时长后仍没有接收到消息则确认丢包，触发超时重传，将滑动窗口中的包全部重发
2. 如果接受到了消息，首先检查包内容的校验和，校验和不正确直接跳过对包的处理阶段，后续重发坏包。
3. 如果接收到了消息且校验和正确，根据反馈消息中的ack调整滑动窗口。将窗口中序列号小于新ack的包出队，并将窗口填充到满为止。如果连续三次接收到的ack相同，则触发重复包重传，将滑动窗口最前端的包重发一次。

## 额外实现

### 兼容可变环路时间

初始实现：

在Sender中创建`sendtime`字典用于存放每一个ack包被发送时的时间

```python
self.sentime = {}   # 记录发送的时间
```

在每次发送包的时候更新该包ack对应的发送时间

```python
self.sentime[int(packet.split('|')[1])] = time.time()
```

再用接受Receiver发回的确认消息时ack对应的时间减去记录的发送时间获得当前的环路时间

```python
RTT = time.time() - self.sentime[self.currentAck]
```

通过指数滑动平均实现动态调整可变环路时间

```python
self.timeout = 0.9 * self.timeout + 0.1 * RTT
```

兼容可变环路时间实现在后续处理高延迟问题时有较大的调整，详情请见最后一个问题讨论。

### 可变大小滑动窗口

在网络较为稳定的环境下，一个较大的窗口可以带来更高的效率；而在网络情况较差的情况下，大窗口带来的更大的重发负担会让传输性能大幅下降。因此实现可变大小的滑动窗口时很有必要的。我采用了网络流量控制中的AIMD算法的思想进行实现。首先使用self.currentAck记录当前接收到的包的ack标识。如果下一次接收到的ack与self.currentAck相同，说明位于窗口最前端的包可能已经丢失。出现丢包情况，说明当前网络情况可能并不是很好，将当前的窗口大小除以二。而若下一次接收到的ack与self.currentAck不同，且RTT较小，则将窗口大小加一。

```python
if self.currentAck == newack:
    self.dupTime += 1
    self.windowSize = max(5, self.windowSize / 2)
else:
    if(self.timeout < 0.5):
		self.windowSize += 1
```

通过该实现可以有效提升在网络较好情况（BasicTest）下的传输速率，并且在网络情况糟糕的情况（RandomDropTest）下丢包数量也没有大量增加。

## 遇到的问题

### 接收端被同一个ack阻塞

在实验一开始的时候，我构建整体网络发送的思路是：

1. 发送当前滑动窗口中的所有内容。

2. 接收消息。如果接收到了消息就使用新消息的ack更新滑动窗口，如果没有接收到消息视为丢包，将当前滑动窗口中的所有内容重发

3.  重复以上步骤直到发送完所有的包为止

然而，这样的架构很容易让网络卡在某一个固定的包上很久（接受到的ack一直是一个固定的值），就算跳出了当前卡顿的包也会很快再次被卡死。我调试了很久也没有解决这个问题，于是将代码整体重构，按照一开始的思路重新写了一份代码。这次没有出现卡顿在一个包上的情况，但却出现了固定时间间隔卡顿的情况。这个问题很快就被发现了：我漏写了重发窗口中所有包的实现，实际上所有的包都是通过超时重传发送出去的。

这就很奇怪了，明明是错误的实现却比最初版本有更高的效率，不会出现单个ack卡死的现象，还能正常通过测试。当我又加上发送滑窗中所有包的代码段，果然又出现了卡死在单个ack上的情况。至此我已经有了一个初步的猜测：过于频繁的发包可能是导致卡顿情况的发生的原因。

为了弄清卡顿的原因，我进行了一些实验

当每次更新ack后发送整个窗口包产生的单个ack卡顿现象：

当给发送窗口包加上微小的时延(time.sleep(0.001))后产生的ack结果

加上一定时延后卡顿的现象得到了一定的缓解，但在单个包上仍会出现3~5次卡顿。这时我们去读receiver.py中接收端关键实现部分

```python
def ack(self,seqno, data):
    res_data = []
    self.updated = time.time()
    if seqno > self.current_seqno and seqno <= self.current_seqno + self.max_buf_size:
        self.seqnums[seqno] = data
        for n in sorted(self.seqnums.keys()):
            if n == self.current_seqno + 1:
                self.current_seqno += 1
                res_data.append(self.seqnums[n])
                del self.seqnums[n]
                else:
                    break # when we find out of order seqno, quit and move on
```

首先，接收端的滑动窗口有大小为5的buffer，如果当前处理的包号大于`current_seqno+buffer_size`就会被直接忽略。在取到新包后，为了防止拿到的包是乱序的，会首先将取到的包按照其键值进行排序再进行比对。

问题就出在有限的窗口大小以及包的排序上。

按照我之前设计的做法，每当取到新包时都会将窗口中的所有包发送一遍。假设当前的窗口buffer大小为10，第一次发送的包为1,2,3,4,5。由于消息从Sender发出到Receiver接收有着短暂的延迟，当Sender开始监听接收端发回的消息时，Receiver可能刚刚处理到第三个包。这时返回的ack为4，Sender重新调整窗口，向Receiver发送4,5,6,7,8。这时问题就出现了。

Receiver可能并不是没有收到4,5，只是需要再等待一些时间才能读取到这个包，但这时Sender发送的新包已经来了，通过排序，现在Receiver的窗口中的包序列为 4,4,5,5,6,7,8。这时4号包将会被处理两次，更糟糕的是，由于我们有处理收到重复包的机制`handle_dup`，还可能会出现更严重的问题。

```python
def handle_dup(self):
    self.send(self.slidingWindow[0])
    self.sentime[int(self.slidingWindow[0].split('|')[1])] = time.time()
```

我对于重复包的处理为：当收到的ack连续三次都是同一个值时，我们可以认为这个包在传输的过程中已经丢失了，所以将滑动窗口最前端的包重发一次。

如果发送速度较快，以上面的处理方式完全有可能在并没有丢包的情况向Receiver发送三次同样的包导致`handle_dup`函数的调用。这个时候最坏的情况就出现了，我们在不断的接受同一个已经收到的包，又因为接受到的ack一直为同一个值，我们认为这个包在传输过程中丢失了，于是再次向发送端发送它。这就使得ack出现了阻塞，一直卡死在了同一个包上。

弄清楚了问题是如何产生的，解决起来就比较简单了。关键就在于调整“每当拿到新包时，就将滑动窗口中的包全部重新发送一次”这个策略。既然过快的发包有可能导致ack阻塞，那么我们就应该只发送新拿到的包，对于已经发出过的包，只在超时重传的情况下重新发送。在`handle_new_ack`函数中我实现了这个过程，首先记录下窗口中有哪些包是已经确认接收到的，将新包入队，只向Receiver发送新获取的包。

通过这种调整，丢包率和阻塞问题都得到了有效的解决。

### 高延迟网络下的问题

继承BasicTest类，重写其`handle_tick`函数模拟延时，在这里`time.sleep(0.1)`将会造成约1s的高延迟。

```
class HighDelay(BasicTest):
    def handle_tick(self, tick_interval):
        time.sleep(0.1)
```

在高延迟网络下，如果没有合理的timeout时间，那么就会出现类似于之前遇到的ack阻塞问题。由于延迟很高，甚至高到了比timeout还要大，在Receiver端收到消息之前Sender就会因为触发了超时重传而再次向Receiver发包。这样一来Receiver又会因为收到大量重复的包进入堵塞状态。所以，要解决高延迟情况下发送卡死的问题，根本上是解决timeout的问题。

经过大量测试，我发现仅仅使用简单的可变环路时间去动态的调整timeout时间是不够的。如果一开始设定的高延迟为1s，初始timeout时间为500ms，那么在Sender将timeout时间动态的调整为1s之前发送端已经重发了很多次相同的包了，已经进入了堵塞状态。所以我们需要对一开始的可变环路时间实现作出一些调整。

我们将一个包从Sender中发送出去再到接受到反馈消息的时间作为RTT。

经过很多次尝试，我得出了以下结论：

- 如果第一次收发的RTT较小，那么直接使用较小的重发时间，否则一个较大的重发时间是比较安全的。
- 动态调整延时的公式中，给予最新的环路时间RTT有较大的权重能使传输更快地适应当前网络环境，防止出现卡死情况。

基于这些结论，我将兼容可变环路时间的实现进行了调整：

首先，将timeout的初始值设为1.5s

1. ```python
   RTT = 2 * (time.time() - self.sentime[self.currentAck])
   if self.currentAck == 0: 
       if(RTT < 0.8 ):
       	self.timeout = RTT
   ```

   将RTT设定为当前收发时间的两倍，如果网络略微出现波动，延时只比平均延时稍大一些，两倍的RTT将使代码不触发超时重传。如果当前的ack为0，即为第一次收发，且RTT<0.8，说明当前网络延迟并不是很大，直接使用当前的RTT作为timeout时间，否则使用1.5s作为初始重发时间，防止在将timeout调整好之前窗口已经卡死。

2. ```python
   self.timeout = 0.8 * self.timeout + 0.2 * RTT
   ```

   给予当前RTT 0.2的权重，使得timeout能够更快地适应新的网络环境。



## 额外测试样例

为了更好的模拟网络的真实情况，我额外实现了两种测试样例。

### 乱序发包

当包发出时，其到达顺序可能是混乱的。

```python
# 乱序发包测试
class RandomPacket(BasicTest):
    def handle_packet(self):
        random.shuffle(self.forwarder.in_queue)
        for p in self.forwarder.in_queue:
            self.forwarder.out_queue.append(p)
        # empty out the in_queue
        self.forwarder.in_queue = []
```

### 高延迟发包

从发送到接收，这个过程可能有着较大的延迟。

```python
# 制造1000ms的延迟
class HighDelay(BasicTest):
    def handle_tick(self, tick_interval):
        time.sleep(0.1)
```

个人认为高延迟发包应该是程序需要特别注意的一点，如果不处理好timeout时间，在高延迟的情况下会出现许多问题，如丢包过多，传输阻塞等。

