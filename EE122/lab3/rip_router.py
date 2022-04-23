# coding=utf-8
from sim.api import *
from sim.basics import *

class RIPRouter(Entity):
    def __init__(self):
        self.sendPort = {}         # 存储目标发送端口
        self.distanceTable = {}    # 二维路由表，第一维代表dest，第二位代表neighbor
        self.currentShortest = {}  # 当前路由表中dest的最短路径

    def minimumCost(self, dest):  # 在路由表中查询当前dest的最短路径及其对应邻居
        minimum = float('inf')
        neighborName = None
        if len(self.distanceTable[dest]) == 0:
            return
        for neighbor in self.distanceTable[dest]:
            if self.distanceTable[dest][neighbor] < minimum:
                minimum = self.distanceTable[dest][neighbor]
                neighborName = neighbor
        return minimum, neighborName

    def handle_rx(self, packet, port):

        if isinstance(packet, DiscoveryPacket):
            updatePacket = RoutingUpdate()

            if packet.is_link_up == True:
                if packet.src not in self.distanceTable:
                    self.distanceTable[packet.src] = {}
                self.distanceTable[packet.src][packet.src] = 1
                self.sendPort[packet.src] = port
                self.currentShortest[packet.src] = 1
                updatePacket.add_destination(packet.src, 1)
                self.send(updatePacket, port, flood=True)
                
            else:       # 如果出现断路则需更新表中信息
                if self.sendPort[packet.src] != None:
                    for dest in self.sendPort:
                        # 如果走的端口相同，说明前往这一目的地的路径中也要经过packet.src
                        # 而此时这条路已经不能走了，将距离更新为无穷大
                        if self.sendPort[dest] == port:
                            updatePacket.add_destination(dest, float('inf'))
                            self.distanceTable[dest][packet.src] = float('inf')
                    
                    for dest in self.distanceTable:  # 因为将不能走的路删除，需要再寻找一条新的路径将数据发出
                        mini, nbname = self.minimumCost(dest)
                        if nbname != None:           # 找不到就说明这条路已经彻底断了
                            self.currentShortest[dest] = mini
                            self.sendPort[dest] = self.sendPort[nbname]
                            updatePacket.add_destination(dest, self.currentShortest[dest])
                self.send(updatePacket, port, flood=True)

        elif isinstance(packet, RoutingUpdate):
            updated = False
            updatePacket = RoutingUpdate()

            for dest in packet.paths:
                if dest == self:
                    continue
                if dest not in self.distanceTable:  # 如果是未接收过的终点，将其加入终点列表中
                    updated = True
                    self.distanceTable[dest] = {}
                    self.distanceTable[dest][packet.src] = packet.paths[dest] + 1
                    self.currentShortest[dest] = self.distanceTable[dest][packet.src]
                    self.sendPort[dest] = port
                    updatePacket.add_destination(dest, self.currentShortest[dest])
                else:  
                    self.distanceTable[dest][packet.src] = packet.paths[dest] + 1
                    newflag = False

                    if packet.paths[dest] + 1 < self.currentShortest[dest]: # 如果是已经出现过的，则比较大小决定是否更新最短路径
                        updated = True
                        self.currentShortest[dest] = packet.paths[dest] + 1
                        self.sendPort[dest] = self.sendPort[packet.src]
                        updatePacket.add_destination(dest, self.currentShortest[dest])
                        newflag = True

                    if not newflag:  # 如果没有最短路径没有更新，那么查看最短路径是否变长了，变长则说明有断路，需进行更新
                        newminimum, neighborName = self.minimumCost(dest)
                        if newminimum != self.currentShortest[dest]:
                            updated = True
                            self.currentShortest[dest] = newminimum
                            self.sendPort[dest] = neighborName
                            updatePacket.add_destination(dest, self.currentShortest[dest])
            if updated:
                self.send(updatePacket, port, flood=True)

        else: # 都不是则为ping发送的包，向当前最优路径的端口发送数据
            minimum, neighbor = self.minimumCost(packet.dst)
            self.send(packet, self.sendPort[neighbor], flood=False)