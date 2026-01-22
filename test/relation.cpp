#include "sys-sage.hpp"
#include <boost/ut.hpp>
#include <iostream>
#include <vector>

namespace ut = boost::ut;
using namespace sys_sage;

// Disable std::cerr to prevent logging in `GetComponent()` when causing an
// exception. We need RAII for that to make sure that std::cerr gets enabled again.
//
// This wrapper assumes that the stream is valid during the lifetime of the wrapper
class stream_suppressor {
public:
  stream_suppressor(std::ostream &_stream) : stream(_stream), buf (_stream.rdbuf(nullptr)) {}
  ~stream_suppressor() { stream.rdbuf(buf); }

private:
  std::ostream &stream;
  std::streambuf *buf;
};

ut::suite<"relation"> _ = []
{
  ut::test("interconnection") = []
  {
    Component foo, bar;
    std::vector<Component *> v {&foo, &bar};
    Relation r (v);

    ut::expect(ut::that % r.GetComponent(0) == &foo);
    ut::expect(ut::that % r.GetComponent(1) == &bar);
    {
      stream_suppressor suppressor (std::cerr);
      ut::expect(ut::that % r.GetComponent(2) == nullptr);
    }
    ut::expect(ut::that % r.GetComponents() == v);

    ut::expect(ut::that % foo.GetRelationsByType(RelationType::Relation) == std::vector<Relation *> {&r});
    ut::expect(ut::that % bar.GetRelationsByType(RelationType::Relation) == std::vector<Relation *> {&r});
  };

  ut::test("deletion") = []
  {
    Component foo, bar;
    std::vector<Component *> v {&foo, &bar};
    Relation *r = new Relation (v);
    r->Delete();

    ut::expect(ut::that % foo.GetRelationsByType(RelationType::Relation).size() == 0U);
    ut::expect(ut::that % bar.GetRelationsByType(RelationType::Relation).size() == 0U);
  };

  ut::test("getters & setters") = []
  {
    std::vector<Component *> v;
    Relation r (v);

    ut::expect(ut::that % r.GetType() == RelationType::Relation);

    r.SetId(2);
    ut::expect(ut::that % r.GetId() == 2);
    ut::expect(ut::that % r.IsOrdered());
  };

  ut::test("adding & updating components") = []
  {
    std::vector<Component *> v;
    Relation r (v);
    ut::expect(ut::that % r.GetComponents().size() == 0U);

    Component foo;
    r.AddComponent(&foo);
    ut::expect(ut::that % r.ContainsComponent(&foo));

    Component bar;
    {
      stream_suppressor suppressor (std::cerr);
      ut::expect(ut::that % r.UpdateComponent(&bar, &bar) == 1);
    }

    r.UpdateComponent(&foo, &bar);
    ut::expect(ut::that % r.GetComponents() == std::vector<Component *> {&bar});

    Component foobar;
    {
      stream_suppressor suppressor (std::cerr);
      ut::expect(ut::that % r.UpdateComponent(1, &foobar) == 1);
    }

    r.UpdateComponent(0, &foobar);
    ut::expect(ut::that % r.GetComponents() == std::vector<Component *> {&foobar});
  };

  ut::test("attributes") = []
  {
    std::vector<Component *> v (0);
    Relation r (v);

    int *i = new int (3);
    r.attrib["foo"] = reinterpret_cast<void *>(i);
    auto it = r.attrib.find("foo");
    ut::expect(ut::that % (it != r.attrib.end()));
    ut::expect(ut::that % *reinterpret_cast<int *>(it->second) == 3);
    delete i;
  };

  ut::test("inheritance") = []
  {
    ut::test("DataPath") = []
    {
      Component foo, bar;
      Relation *r = new DataPath(&foo, &bar, DataPathOrientation::Oriented, DataPathType::Any);
      ut::expect(ut::that % r->GetType() == RelationType::DataPath);
      dynamic_cast<DataPath *>(r)->SetBandwidth(1.0);
      dynamic_cast<DataPath *>(r)->SetLatency(2.0);
      r->Delete();
    };

    ut::test("QuantumGate") = []
    {
      Qubit foo, bar;
      std::vector<Qubit *> v {&foo, &bar};
      Relation *r = new QuantumGate(v.size(), v);

      ut::expect(ut::that % r->GetType() == RelationType::QuantumGate);
      dynamic_cast<QuantumGate *>(r)->SetGateProperties("cx", 1.0, "[1 0 0 0; 0 1 0 0; 0 0 0 1; 0 0 1 0]");
      ut::expect(ut::that % dynamic_cast<QuantumGate *>(r)->GetQuantumGateType() == QuantumGateType::Cnot);
      ut::expect(ut::that % dynamic_cast<QuantumGate *>(r)->GetFidelity() == 1.0);
      ut::expect(ut::that % (dynamic_cast<QuantumGate *>(r)->GetUnitary() == "[1 0 0 0; 0 1 0 0; 0 0 0 1; 0 0 1 0]"));
      r->Delete();
    };

    ut::test("CouplingMap") = []
    {
      Qubit foo, bar;
      Relation *r = new CouplingMap(&foo, &bar);
      ut::expect(ut::that % r->GetType() == RelationType::CouplingMap);
      dynamic_cast<CouplingMap *>(r)->SetFidelity(1.0);
      ut::expect(ut::that % dynamic_cast<CouplingMap *>(r)->GetFidelity() == 1.0);
      r->Delete();
    };
  };
};
