import numpy as np

import unittest
import lsst.utils.tests as utilsTests

import lsst.meas.base.catalogCalculation as catCalc
import lsst.afw.table as afwTable
from lsst.meas.base.pluginRegistry import register
from lsst.meas.base.baseLib import MeasurementError
from lsst.meas.base import FlagHandler

@register("catalogCalculationFail")
class FailCC(catCalc.CatalogCalculationPlugin):
    '''
    catalogCalculation plugin which is guaranteed to fail, testing the failure framework
    '''
    @classmethod
    def getExecutionOrder(cls):
        return cls.DEFAULT_CATALOGCALCULATION

    def __init__(self, config, name, schema, metadata):
        catCalc.CatalogCalculationPlugin.__init__(self, config, name, schema, metadata)
        self.failKey = schema.addField(name + "_fail", type="Flag", doc="Failure test")

    def calculate(self, measRecord):
        raise MeasurementError("Supposed to fail", FlagHandler.FAILURE)

    def fail(self, measRecord, error=None):
        measRecord.set(self.failKey, True)

@register("singleRecordCatalogCalculation")
class SingleRecordCC(catCalc.CatalogCalculationPlugin):
    '''
    CatalogCalculation plugin which works in single mode. It takes a single record, reads a value, squares it,
    and writes out the results to the record
    '''
    @classmethod
    def getExecutionOrder(cls):
        return cls.DEFAULT_CATALOGCALCULATION

    def __init__(self, config, name, schema, metadata):
        catCalc.CatalogCalculationPlugin.__init__(self, config, name, schema, metadata)
        self.failKey = schema.addField(name + "_fail", type="Flag", doc="Failure flag")
        self.squareKey = schema.addField(name + "_square", type="D", doc="Square of input catalog")

    def calculate(self, measRecord):
        value = measRecord.get("start")
        measRecord.set(self.squareKey, value**2)

    def fail(self, measRecord, error=None):
        measRecord.set(self.failKey, True)

@register("multiRecordCatalogCalculation")
class MultiRecordAb(catCalc.CatalogCalculationPlugin):
    '''
    CatalogCalcuation plugin to test the framework in multimode. This plugin takes the whole source catalog at
    once, and loops over the catalog internally. The algorithm simply reads a value, cubes it, and writes the
    results out to the table
    '''
    plugType = 'multi'

    @classmethod
    def getExecutionOrder(cls):
        return cls.DEFAULT_CATALOGCALCULATION

    def __init__(self, config, name, schema, metadata):
        catCalc.CatalogCalculationPlugin.__init__(self, config, name, schema, metadata)
        self.failKey = schema.addField(name + "_fail", type="Flag", doc="Failure flag")
        self.cubeKey = schema.addField(name + "_cube", type="D", doc="Cube of input catalog")

    def calculate(self, catalog):
        for rec in catalog:
            value = rec.get("start")
            rec.set(self.cubeKey, value**3)

    def fail(self, catalog, error=None):
        for rec in catalog:
            value = rec.set(self.failKey, True)

@register("dependentCatalogCalulation")
class DependentAb(catCalc.CatalogCalculationPlugin):
    '''
    CatalogCalculation plugin to test the runlevel resolution. This plugin takes in single records, reads a
    value from a previous plugin computes a square root, and writes the results to the table.
    '''
    @classmethod
    def getExecutionOrder(cls):
        return cls.DEFAULT_CATALOGCALCULATION + 1

    def __init__(self, config, name, schema, metadata):
        catCalc.CatalogCalculationPlugin.__init__(self, config, name, schema, metadata)
        self.failKey = schema.addField(name + "_fail", type="Flag", doc="Failure flag")
        self.sqrtKey = schema.addField(name + "_sqrt", type="D",
                                       doc="Square root of singleRecord catalogCalculation")

    def calculate(self, measRecord):
        value = measRecord.get("singleRecordCatalogCalculation_square")
        measRecord.set(self.sqrtKey, value**0.5)

    def fail(self, measRecord, error=None):
        measRecord.set(self.failKey, True)

class CatalogCalculationTest(unittest.TestCase):
    '''
    Test the catalogCalculation framework, running only the plugins defined above
    '''
    def setUp(self):
        # Create a schema object, and populate it with a field to simulate results from measurements on an
        # image
        schema = afwTable.SourceTable.makeMinimalSchema()
        startKey = schema.addField("start", type="D")
        # Instantiate a config object adding each of the above plugins, and use it to create a task
        abConfig = catCalc.CatalogCalculationConfig()
        abConfig.plugins.names = ["catalogCalculationFail", "singleRecordCatalogCalculation",
                "multiRecordCatalogCalculation", "dependentCatalogCalulation"]
        abTask = catCalc.CatalogCalculationTask(schema=schema, config=abConfig)
        # Create a catalog with five sources as input to the task
        self.catalog = afwTable.SourceCatalog(schema)
        self.numObjects = 5
        for i in range(self.numObjects):
            rec = self.catalog.addNew()
            rec.set("start", float(i + 1))
        # Run the catalogCalculation task, outputs will be checked in test methods
        abTask.run(self.catalog)

    def testCatalogCalculation(self):
        # Verify the failure flag got set for the plugin expected to fail
        self.assertEqual(len(self.catalog), self.numObjects)
        for src in self.catalog:
            self.assertTrue(src.get("catalogCalculationFail_fail"))

        # Verify the single record plugin ran successfully
        for rec in self.catalog:
            self.assertAlmostEqual(rec.get("start")**2, rec.get("singleRecordCatalogCalculation_square"), 4)

        # Verify that the system correctly handled a plugin which expects a full catalog to be passed
        for rec in self.catalog:
            self.assertAlmostEqual(rec.get("start")**3, rec.get("multiRecordCatalogCalculation_cube"), 4)

        # Verify that the system runs plugins in the correct run order
        for rec in self.catalog:
            self.assertAlmostEqual(rec.get("start"), rec.get("dependentCatalogCalulation_sqrt"), 4)

    def tearDown(self):
        del self.catalog, self.numObjects,

###########################################################################################################################


def suite():
    '''
    Returns a suite containing all the test cases in this module.
    '''
    utilsTests.init()

    suites = []
    suites += unittest.makeSuite(CatalogCalculationTest)
    suites += unittest.makeSuite(utilsTests.MemoryTestCase)
    return unittest.TestSuite(suites)


def run(exit=False):
    '''
    Run the utilsTests
    '''
    utilsTests.run(suite(), exit)

if __name__ == "__main__":
    run(True)
