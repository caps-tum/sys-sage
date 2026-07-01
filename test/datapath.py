import unittest
import py_sys_sage as pysage 

class TestDatapaths(unittest.TestCase):
    def test_constructor_1(self):
        a = pysage.Component()
        b = pysage.Component()
        dp = pysage.DataPath(a,b, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_CATEGORY_PHYSICAL)
        self.assertEqual(a, dp.source)
        self.assertEqual(b, dp.target)
        self.assertEqual(pysage.DATAPATH_ORIENTATION_ORIENTED, dp.orientation)
        self.assertEqual(pysage.DATAPATH_CATEGORY_PHYSICAL, dp.dp_category)
    def test_constructor_2(self):
        a = pysage.Component()
        b = pysage.Component()
        dp = pysage.DataPath(a,b, pysage.DATAPATH_ORIENTATION_ORIENTED, 5.0, 42.0)
        self.assertEqual(a, dp.source)
        self.assertEqual(b, dp.target)
        self.assertEqual(pysage.DATAPATH_ORIENTATION_ORIENTED, dp.orientation)
        self.assertEqual(pysage.DATAPATH_CATEGORY_NONE, dp.dp_category)
        self.assertEqual(5.0, dp.bandwidth)
        self.assertEqual(42.0, dp.latency)
    def test_constructor_3(self):
        a = pysage.Component()
        b = pysage.Component()
        dp = pysage.DataPath(a,b, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_CATEGORY_PHYSICAL, 5.0, 42.0)
        self.assertEqual(a, dp.source)
        self.assertEqual(b, dp.target)
        self.assertEqual(pysage.DATAPATH_ORIENTATION_ORIENTED, dp.orientation)
        self.assertEqual(pysage.DATAPATH_CATEGORY_PHYSICAL, dp.dp_category)
        self.assertEqual(5.0, dp.bandwidth)
        self.assertEqual(42.0, dp.latency)
    def test_unidirectional_data_path(self):
        a = pysage.Component()
        b = pysage.Component()
        dp = pysage.DataPath(a,b, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_CATEGORY_PHYSICAL)
        self.assertEqual([], a.FindDataPaths(pysage.DATAPATH_CATEGORY_ANY, pysage.DATAPATH_DIRECTION_INCOMING))
        self.assertEqual([dp], a.FindDataPaths(pysage.DATAPATH_CATEGORY_ANY, pysage.DATAPATH_DIRECTION_OUTGOING))
        
        self.assertEqual([dp], b.FindDataPaths(pysage.DATAPATH_CATEGORY_ANY, pysage.DATAPATH_DIRECTION_INCOMING))
        self.assertEqual([], b.FindDataPaths(pysage.DATAPATH_CATEGORY_ANY, pysage.DATAPATH_DIRECTION_OUTGOING))
    def test_bidirectional_data_path(self):
        a = pysage.Component()
        b = pysage.Component()
        dp = pysage.DataPath(a,b, pysage.DATAPATH_ORIENTATION_BIDIRECTIONAL, pysage.DATAPATH_CATEGORY_PHYSICAL)
        self.assertEqual([dp], a.FindDataPaths(pysage.DATAPATH_CATEGORY_ANY, pysage.DATAPATH_DIRECTION_INCOMING))
        self.assertEqual([dp], a.FindDataPaths(pysage.DATAPATH_CATEGORY_ANY, pysage.DATAPATH_DIRECTION_OUTGOING))
        self.assertEqual([dp], b.FindDataPaths(pysage.DATAPATH_CATEGORY_ANY, pysage.DATAPATH_DIRECTION_INCOMING))
        self.assertEqual([dp], b.FindDataPaths(pysage.DATAPATH_CATEGORY_ANY, pysage.DATAPATH_DIRECTION_OUTGOING))
        
    def test_get_data_path_by_type(self):
        a = pysage.Component()
        b = pysage.Component()
        dp1 = pysage.DataPath(a, b, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_CATEGORY_LOGICAL)
        dp2 = pysage.DataPath(a, b, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_CATEGORY_PHYSICAL)
        dp3 = pysage.DataPath(a, b, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_CATEGORY_PHYSICAL)
        dp4 = pysage.DataPath(b, a, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_CATEGORY_PHYSICAL)

        self.assertEqual(dp1, a.GetDataPathByCategory(pysage.DATAPATH_CATEGORY_LOGICAL, pysage.DATAPATH_DIRECTION_OUTGOING))
        self.assertEqual(dp2, a.GetDataPathByCategory(pysage.DATAPATH_CATEGORY_PHYSICAL, pysage.DATAPATH_DIRECTION_OUTGOING))
        self.assertEqual(None, a.GetDataPathByCategory(pysage.DATAPATH_CATEGORY_L3CAT, pysage.DATAPATH_DIRECTION_OUTGOING))
        self.assertEqual(dp4, a.GetDataPathByCategory(pysage.DATAPATH_CATEGORY_PHYSICAL, pysage.DATAPATH_DIRECTION_INCOMING))
        self.assertEqual(dp4, b.GetDataPathByCategory(pysage.DATAPATH_CATEGORY_PHYSICAL, pysage.DATAPATH_DIRECTION_OUTGOING))
        self.assertEqual(dp2, b.GetDataPathByCategory(pysage.DATAPATH_CATEGORY_PHYSICAL, pysage.DATAPATH_DIRECTION_ANY))

    def test_get_all_data_paths_by_type(self):
        a = pysage.Component()
        b = pysage.Component()
        dp1 = pysage.DataPath(a, b, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_CATEGORY_LOGICAL)
        dp2 = pysage.DataPath(a, b, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_CATEGORY_PHYSICAL)
        dp3 = pysage.DataPath(a, b, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_CATEGORY_PHYSICAL)
        dp4 = pysage.DataPath(b, a, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_CATEGORY_PHYSICAL)

        v = a.FindDataPaths(pysage.DATAPATH_CATEGORY_LOGICAL, pysage.DATAPATH_DIRECTION_INCOMING)
        self.assertEqual([], v)

        v = a.FindDataPaths(pysage.DATAPATH_CATEGORY_PHYSICAL, pysage.DATAPATH_DIRECTION_INCOMING)
        self.assertEqual([dp4], v)

        v = a.FindDataPaths(pysage.DATAPATH_CATEGORY_LOGICAL, pysage.DATAPATH_DIRECTION_OUTGOING)
        self.assertEqual([dp1], v)

        v = a.FindDataPaths(pysage.DATAPATH_CATEGORY_PHYSICAL, pysage.DATAPATH_DIRECTION_OUTGOING)
        self.assertEqual([dp2, dp3], v)

        v = a.FindDataPaths(pysage.DATAPATH_CATEGORY_PHYSICAL, pysage.DATAPATH_DIRECTION_ANY)
        self.assertEqual([dp2, dp3, dp4], v)

if __name__ == "__main__":
    unittest.main()