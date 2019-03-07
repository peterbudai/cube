#!/usr/bin/env python3

from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *

from connection import CubeConnection


class CubeSystemWidget(QDockWidget):
    def __init__(self, parent):
        super().__init__("System", parent)

        self.connection = parent.connection
        self.show()
