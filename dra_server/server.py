
import json

from PyQt5.QtCore import pyqtSignal
from PyQt5.QtCore import QObject

from .chromium import Chromium
from . import constants
from .wssd import WSSDController

'''
Controller for server side.

Start:
    * start websocket
    * start chromium

Stop:
    * stop chromium
    * stop websocket
'''


class Server(QObject):

    peerIdUpdated = pyqtSignal(str)

    def __init__(self, parent=None):
        super().__init__(parent=parent)
        self.wssd = None
        self.chromium = None

    def start(self):
        '''Start desktop sharing service'''
        self.start_wssd()
        self.start_chromium()

    def stop(self):
        '''Stop desktop sharing service'''
        self.stop_chromium()
        self.stop_wssd()

    def start_wssd(self):
        '''Start websocket service'''
        self.stop_wssd()
        self.wssd = WSSDController(self)
        self.wssd.start()
        self.wssd.worker.browserCmd.connect(self.handleBrowserCmd)

    def stop_wssd(self):
        '''Stop websocket service'''
        if self.wssd:
            self.wssd.stop()

    def start_chromium(self):
        '''Launch chromium in background'''
        self.stop_chromium()
        self.chromium = Chromium(self)
        self.chromium.start()

    def stop_chromium(self):
        '''Kill chromium'''
        if self.chromium:
            self.chromium.stop()

    def handleBrowserCmd(self, msg):
        '''Handle command message sent from browser side.

        Some of these messages will be converted to Qt mssage'''
        event = json.loads(msg)
        if event['Type'] == constants.SERVER_MSG_ECHO:
            self.peerIdUpdated.emit(event['Payload'])