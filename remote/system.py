#!/usr/bin/env python3

from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtChart import *

from connection import CubeConnection


class CubeDynamicChart(QChartView):
    def __init__(self, title, hrange, parent=None):
        super().__init__(parent)

        self.tick = 0
        self.hAxis = QValueAxis()
        self.hAxis.setRange(0, hrange)
        self.hAxis.setLabelFormat('%d')

        self.series = []
        self.chart = QChart()
        self.chart.setTitle(title)
        self.chart.addAxis(self.hAxis, Qt.AlignBottom)
        self.setChart(self.chart)

    def addSeries(self, fmt, vrange, color, align):
        vAxis = QValueAxis()
        vAxis.setRange(0, vrange)
        vAxis.setLabelFormat('%d')
        self.chart.addAxis(vAxis, align)

        series = QLineSeries()
        series.setName(fmt.format(0))
        self.chart.addSeries(series)
        series.attachAxis(self.hAxis)
        series.attachAxis(vAxis)

        self.series.append((fmt, series, vAxis))

    def appendSeries(self, values):
        for i, (f, s, a) in enumerate(self.series):
            s.append(self.tick, min(max(0, values[i]), a.max()))
            s.setName(f.format(values[i]))
        if self.tick > self.hAxis.max():
            self.hAxis.setRange(self.tick - self.hAxis.max(), self.tick)
            for f, s, a in self.series:
                s.remove(0)
        self.tick += 1


class CubeSystemWidget(QDockWidget):
    def __init__(self, parent):
        super().__init__("System", parent)

        self.connection = parent.connection

        self.setWidget(QPushButton('Hello'))

        self.setFeatures(QDockWidget.NoDockWidgetFeatures)
        self.setVisible(False)
