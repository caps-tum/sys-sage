import py_sys_sage as pysage
import unittest
import sys
import os

#class stream_suppressor:
#    def __init__(self, stream):
#        self.stream = stream
#
#    def __enter__(self):
#        self.original_stream = getattr(sys, self.stream)
#        self.devnull = open(os.devnull, "w")
#        setattr(sys, self.stream, self.devnull)
#
#    def __exit__(self, *_):
#        setattr(sys, self.stream, self.original_stream)
#        self.devnull.close()

class TestRelations(unittest.TestCase):
    def test_interconnection(self):
        foo = pysage.Component()
        bar = pysage.Component()
        v = [foo, bar]
        r = pysage.Relation(v)
        self.assertEqual(r.GetComponent(0), foo)
        self.assertEqual(r.GetComponent(1), bar)
        #with stream_suppressor("stderr"):
        #    self.assertEqual(r.GetComponent(2), None)
        self.assertEqual(r.GetComponent(2), None)
        self.assertEqual(r.components, v)
        self.assertEqual(foo.GetRelations(pysage.RELATION_TYPE_RELATION), [r])
        self.assertEqual(bar.GetRelations(pysage.RELATION_TYPE_RELATION), [r])

    def test_deletion(self):
        foo = pysage.Component()
        bar = pysage.Component()
        v = [foo, bar]
        r = pysage.Relation(v)
        r.Delete()
        
        self.assertEqual(len(foo.GetRelations(pysage.RELATION_TYPE_RELATION)), 0)
        self.assertEqual(len(bar.GetRelations(pysage.RELATION_TYPE_RELATION)), 0)

    def test_getters_and_setters(self):
        r = pysage.Relation([])

        self.assertEqual(r.type, pysage.RELATION_TYPE_RELATION)

        r.id = 2
        self.assertEqual(r.id, 2)
        self.assertTrue(r.ordered)

    def test_adding_and_updating_components(self):
        r = pysage.Relation([])
        self.assertEqual(len(r.components), 0)

        foo = pysage.Component()
        r.AddComponent(foo)
        self.assertTrue(r.ContainsComponent(foo))

        bar = pysage.Component()
        #with stream_suppressor("stderr"):
        #    self.assertEqual(r.UpdateComponent(bar, bar), 1)
        self.assertEqual(r.UpdateComponent(bar, bar), 1)

        r.UpdateComponent(foo, bar)
        self.assertEqual(r.components, [bar])

        foobar = pysage.Component()
        #with stream_suppressor("stderr"):
        #    self.assertEqual(r.UpdateComponent(1, foobar), 1)
        self.assertEqual(r.UpdateComponent(1, foobar), 1)

        r.UpdateComponent(0, foobar)
        self.assertEqual(r.components, [foobar])

    def test_attributes(self):
        r = pysage.Relation([])

        r["foo"] = 1
        r["bar"] = 2.0
        r["foobar"] = "test"
        self.assertEqual(r["foo"], 1)
        self.assertEqual(r["bar"], 2.0)
        self.assertEqual(r["foobar"], "test")
        with self.assertRaises(AttributeError):
            r["fail"]
        # indexing follows lexicographical order of keys
        self.assertEqual(r[0], 2.0)
        self.assertEqual(r[1], 1)
        self.assertEqual(r[2], "test")
        with self.assertRaises(TypeError):
            r[1] = 2

    def test_inheritance_data_path(self):
        foo = pysage.Component()
        bar = pysage.Component()

        r = pysage.DataPath(foo, bar, pysage.DATAPATH_ORIENTATION_ORIENTED, pysage.DATAPATH_TYPE_ANY)
        # test if inherited class can access members of the base class
        self.assertEqual(r.type, pysage.RELATION_TYPE_DATAPATH)

    def test_inheritance_quantum_gate(self):
        foo = pysage.Qubit()
        bar = pysage.Qubit()
        v = [foo, bar]
        r = pysage.QuantumGate(len(v), v)

        self.assertEqual(r.type, pysage.RELATION_TYPE_QUANTUMGATE)

        r.SetGateProperties("cx", 1.0, "[1 0 0 0; 0 1 0 0; 0 0 0 1; 0 0 1 0]")
        self.assertEqual(r.gate_type, pysage.QUANTUMGATE_TYPE_CNOT)
        self.assertEqual(r.fidelity, 1.0)
        self.assertEqual(r.unitary, "[1 0 0 0; 0 1 0 0; 0 0 0 1; 0 0 1 0]")

    def test_inheritance_coupling_map(self):
        foo = pysage.Qubit()
        bar = pysage.Qubit()
        r = pysage.CouplingMap(foo, bar)

        self.assertEqual(r.type, pysage.RELATION_TYPE_COUPLINGMAP)

        r.fidelity = 1.0
        self.assertEqual(r.fidelity, 1.0)

if __name__ == "__main__":
    unittest.main()
