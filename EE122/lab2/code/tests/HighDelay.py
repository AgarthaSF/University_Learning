# -*- coding:utf8 -*-
import sys
reload(sys)
import random
import time
from BasicTest import *


# 制造1000ms的延迟
class HighDelay(BasicTest):
    def handle_tick(self, tick_interval):
        time.sleep(0.1)