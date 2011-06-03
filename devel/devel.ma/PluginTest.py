#!/usr/bin/env python
#
# ZYpp plugin
#
import os
import sys
import traceback
import time

from zypp_plugin import Plugin

class MyPlugin(Plugin):

  def PLUGINBEGIN(self, headers, body):
    # commit is going to start.
    #self.error( {}, 'oops' )
    self.ack()

  def PLUGINEND(self, headers, body):
    # commit ended
    self.ack()

plugin = MyPlugin()
plugin.main()
