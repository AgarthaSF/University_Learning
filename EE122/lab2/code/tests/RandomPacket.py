# -*- coding:utf8 -*-
import sys
reload(sys)
import random
import time
from BasicTest import *


# 乱序发包测试
class RandomPacket(BasicTest):
    def handle_packet(self):
        random.shuffle(self.forwarder.in_queue)
        for p in self.forwarder.in_queue:
            self.forwarder.out_queue.append(p)
        # empty out the in_queue
        self.forwarder.in_queue = []