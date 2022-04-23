# coding=utf-8
from sim.api import *
from sim.basics import *

class LearningSwitch (Entity):
    def __init__(self):
        # Add your code here!
        self.transport = {}

    def handle_rx(self, packet, port):
        if packet.dst in self.transport:  # 如果表中有直接进行发送
            if self.transport[packet.dst] != port:
                self.send(packet, self.transport[packet.dst], flood=False)
        else:  # 表中没有就进行泛洪
            if packet.src not in self.transport:
                if(type(packet.src) == BasicHost):
                    self.transport[packet.src] = port
                    self.send(packet, port, flood=True)

