#!/usr/bin/env python
#
# LSST Data Management System
# Copyright 2008-2013 LSST Corporation.
#
# This product includes software developed by the
# LSST Project (http://www.lsst.org/).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the LSST License Statement and
# the GNU General Public License along with this program.  If not,
# see <http://www.lsstcorp.org/LegalNotices/>.
#

import unittest

import lsst.utils.tests
import lsst.meas.base.tests
from lsst.meas.base.tests import (AlgorithmTestCase, CentroidTransformTestCase,
                                  SingleFramePluginTransformSetupHelper)

class GaussianCentroidTestCase(AlgorithmTestCase):
    def setUp(self):
        self.center = lsst.afw.geom.Point2D(50.1, 49.8)
        self.bbox = lsst.afw.geom.Box2I(lsst.afw.geom.Point2I(-20, -30),
                                        lsst.afw.geom.Extent2I(140, 160))
        self.dataset = lsst.meas.base.tests.TestDataset(self.bbox)
        self.dataset.addSource(100000.0, self.center)

    def tearDown(self):
        del self.center
        del self.bbox
        del self.dataset

    def testSingleFramePlugin(self):
        task = self.makeSingleFrameMeasurementTask("base_GaussianCentroid")
        exposure, catalog = self.dataset.realize(10.0, task.schema)
        task.run(exposure, catalog)
        record = catalog[0]
        x = record.get("base_GaussianCentroid_x")
        y = record.get("base_GaussianCentroid_y")
        self.assertFalse(record.get("base_GaussianCentroid_flag"))
        self.assertFalse(record.get("base_GaussianCentroid_flag_noPeak"))
        self.assertClose(x, self.center.getX(), atol=None, rtol=.01)
        self.assertClose(y, self.center.getY(), atol=None, rtol=.01)


class GaussianCentroidTransformTestCase(CentroidTransformTestCase, SingleFramePluginTransformSetupHelper):
    controlClass = lsst.meas.base.GaussianCentroidControl
    algorithmClass = lsst.meas.base.GaussianCentroidAlgorithm
    transformClass = lsst.meas.base.GaussianCentroidTransform
    flagNames = ('flag', 'flag_noPeak')
    singleFramePlugins = ('base_GaussianCentroid',)
    forcedPlugins = ('base_GaussianCentroid',)


def suite():
    """Returns a suite containing all the test cases in this module."""

    lsst.utils.tests.init()

    suites = []
    suites += unittest.makeSuite(GaussianCentroidTestCase)
    suites += unittest.makeSuite(GaussianCentroidTransformTestCase)
    suites += unittest.makeSuite(lsst.utils.tests.MemoryTestCase)
    return unittest.TestSuite(suites)

def run(shouldExit=False):
    """Run the tests"""
    lsst.utils.tests.run(suite(), shouldExit)

if __name__ == "__main__":
    run(True)
