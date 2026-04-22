import py_sys_sage as pysage
import unittest

class TestDatapaths(unittest.TestCase):
    def test_constructor_1(self):
        a = pysage.Component()
        b = pysage.Component()
        dp = pysage.DataPath(a,b, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_TYPE_PHYSICAL)
        self.assertEqual(a, dp.source)
        self.assertEqual(b, dp.target)
        self.assertEqual(pysage.DATAPATH_ORIENTATION_ORIENTED, dp.orientation)
        self.assertEqual(pysage.DATAPATH_TYPE_PHYSICAL, dp.dp_type)
    def test_constructor_2(self):
        a = pysage.Component()
        b = pysage.Component()
        dp = pysage.DataPath(a,b, pysage.DATAPATH_ORIENTATION_ORIENTED, 5.0, 42.0)
        self.assertEqual(a, dp.source)
        self.assertEqual(b, dp.target)
        self.assertEqual(pysage.DATAPATH_ORIENTATION_ORIENTED, dp.orientation)
        self.assertEqual(pysage.DATAPATH_TYPE_NONE, dp.dp_type)
        self.assertEqual(5.0, dp.bandwidth)
        self.assertEqual(42.0, dp.latency)
    def test_constructor_3(self):
        a = pysage.Component()
        b = pysage.Component()
        dp = pysage.DataPath(a,b, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_TYPE_PHYSICAL, 5.0, 42.0)
        self.assertEqual(a, dp.source)
        self.assertEqual(b, dp.target)
        self.assertEqual(pysage.DATAPATH_ORIENTATION_ORIENTED, dp.orientation)
        self.assertEqual(pysage.DATAPATH_TYPE_PHYSICAL, dp.dp_type)
        self.assertEqual(5.0, dp.bandwidth)
        self.assertEqual(42.0, dp.latency)
    def test_unidirectional_data_path(self):
        a = pysage.Component()
        b = pysage.Component()
        dp = pysage.DataPath(a,b, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_TYPE_PHYSICAL)
        self.assertEqual([], a.GetAllDataPaths(pysage.DATAPATH_TYPE_ANY, pysage.DATAPATH_DIRECTION_INCOMING))
        self.assertEqual([dp], a.GetAllDataPaths(pysage.DATAPATH_TYPE_ANY, pysage.DATAPATH_DIRECTION_OUTGOING))
        
        self.assertEqual([dp], b.GetAllDataPaths(pysage.DATAPATH_TYPE_ANY, pysage.DATAPATH_DIRECTION_INCOMING))
        self.assertEqual([], b.GetAllDataPaths(pysage.DATAPATH_TYPE_ANY, pysage.DATAPATH_DIRECTION_OUTGOING))
    def test_bidirectional_data_path(self):
        a = pysage.Component()
        b = pysage.Component()
        dp = pysage.DataPath(a,b, pysage.DATAPATH_ORIENTATION_BIDIRECTIONAL, pysage.DATAPATH_TYPE_PHYSICAL)
        self.assertEqual([dp], a.GetAllDataPaths(pysage.DATAPATH_TYPE_ANY, pysage.DATAPATH_DIRECTION_INCOMING))
        self.assertEqual([dp], a.GetAllDataPaths(pysage.DATAPATH_TYPE_ANY, pysage.DATAPATH_DIRECTION_OUTGOING))
        self.assertEqual([dp], b.GetAllDataPaths(pysage.DATAPATH_TYPE_ANY, pysage.DATAPATH_DIRECTION_INCOMING))
        self.assertEqual([dp], b.GetAllDataPaths(pysage.DATAPATH_TYPE_ANY, pysage.DATAPATH_DIRECTION_OUTGOING))
        
    def test_get_data_path_by_type(self):
        a = pysage.Component()
        b = pysage.Component()
        dp1 = pysage.DataPath(a, b, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_TYPE_LOGICAL)
        dp2 = pysage.DataPath(a, b, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_TYPE_PHYSICAL)
        dp3 = pysage.DataPath(a, b, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_TYPE_PHYSICAL)
        dp4 = pysage.DataPath(b, a, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_TYPE_PHYSICAL)

        self.assertEqual(dp1, a.GetDataPathByType(pysage.DATAPATH_TYPE_LOGICAL, pysage.DATAPATH_DIRECTION_OUTGOING))
        self.assertEqual(dp2, a.GetDataPathByType(pysage.DATAPATH_TYPE_PHYSICAL, pysage.DATAPATH_DIRECTION_OUTGOING))
        self.assertEqual(None, a.GetDataPathByType(pysage.DATAPATH_TYPE_L3CAT, pysage.DATAPATH_DIRECTION_OUTGOING))
        self.assertEqual(dp4, a.GetDataPathByType(pysage.DATAPATH_TYPE_PHYSICAL, pysage.DATAPATH_DIRECTION_INCOMING))
        self.assertEqual(dp4, b.GetDataPathByType(pysage.DATAPATH_TYPE_PHYSICAL, pysage.DATAPATH_DIRECTION_OUTGOING))
        self.assertEqual(dp2, b.GetDataPathByType(pysage.DATAPATH_TYPE_PHYSICAL, pysage.DATAPATH_DIRECTION_ANY))

    def test_get_all_data_paths_by_type(self):
        a = pysage.Component()
        b = pysage.Component()
        dp1 = pysage.DataPath(a, b, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_TYPE_LOGICAL)
        dp2 = pysage.DataPath(a, b, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_TYPE_PHYSICAL)
        dp3 = pysage.DataPath(a, b, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_TYPE_PHYSICAL)
        dp4 = pysage.DataPath(b, a, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_TYPE_PHYSICAL)

        v = a.GetAllDataPaths(pysage.DATAPATH_TYPE_LOGICAL, pysage.DATAPATH_DIRECTION_INCOMING)
        self.assertEqual([], v)

        v = a.GetAllDataPaths(pysage.DATAPATH_TYPE_PHYSICAL, pysage.DATAPATH_DIRECTION_INCOMING)
        self.assertEqual([dp4], v)

        v = a.GetAllDataPaths(pysage.DATAPATH_TYPE_LOGICAL, pysage.DATAPATH_DIRECTION_OUTGOING)
        self.assertEqual([dp1], v)

        v = a.GetAllDataPaths(pysage.DATAPATH_TYPE_PHYSICAL, pysage.DATAPATH_DIRECTION_OUTGOING)
        self.assertEqual([dp2, dp3], v)

        v = a.GetAllDataPaths(pysage.DATAPATH_TYPE_PHYSICAL, pysage.DATAPATH_DIRECTION_ANY)
        self.assertEqual([dp2, dp3, dp4], v)

if __name__ == "__main__":
    unittest.main()