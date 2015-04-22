
'''
Capture global keyboard event when needed.

Note:
* CaptureController controlls CaptureWorker
* CaptureWorker is running in a QThread
* CaptureWorker contains a Capture object
* The Capture object is used to capture global keyboard event
'''
import json

from PyQt5 import QtCore
from PyQt5 import QtWidgets

from pykeyboard import PyKeyboardEvent

from . import keyboard


class Capture(PyKeyboardEvent):

    def __init__(self, worker):
        super().__init__()
        self.worker = worker

        self.capture = True
        print('Capture created')

    def __del__(self):
        print('-------->delete capture object')

    def escape(self, event):
        '''Override escape() method in PyKeyboardEvent.

        When True is returned, keyboard listener will be stopped.
        '''
        return False

    def tap(self, keycode, character, press):
        '''Handle keyboard event here
        
        @keycode, keyboard code
        @character, keyboard char, if available
        @press, True if event is KeyPressEvent, False if is KeyReleaseEvent
        '''
        msg = {
            'keycode': keycode,
            'character': character,
            'press': press,
        }
        #self.worker.tapped.emit(json.dumps(msg))
        print(msg)

    def stop(self):
        print('Capture.stop()')
        PyKeyboardEvent.stop(self)


class CaptureWorker(QtCore.QObject):

    tapped = QtCore.pyqtSignal(str)

    def __init__(self, parent=None):
        super().__init__(parent)
        self._capture = None

    def capture(self):
        print('CaptureWorker.capture()')
        if not self._capture:
            self._capture = Capture(self)
            self._capture.run()

    def uncapture(self):
        print('CaptureWorker.uncapture()')
        if self._capture:
            self._capture.stop()
            del self._capture
            self._capture = None


class KeyboardCaptureController(QtCore.QObject):

    captured = QtCore.pyqtSignal()
    uncaptured = QtCore.pyqtSignal()

    def __init__(self, parent=None):
        super().__init__(parent)

        self.captureThread = QtCore.QThread()
        self.worker = CaptureWorker()
        self.worker.moveToThread(self.captureThread)
        #self.worker.tapped.connect(keyboard.send_message)
        QtWidgets.qApp.aboutToQuit.connect(self.stop)
        self.captured.connect(self.worker.capture)

        self.captureThread.start()

    def capture(self):
        print('Controller.capture()')
        self.captured.emit()

    def uncapture(self):
        print('Controller.uncapture()')
        self.worker.uncapture()

    def stop(self):
        self.uncapture()
        if self.captureThread.isRunning():
            self.captureThread.quit()
            self.captureThread.wait(1)
