#include <boost/ut.hpp>

#include "sys-sage.hpp"

using namespace boost::ut;

static suite<"data-path"> _ = []
{
    "Constructors"_test = []
    {
        "Constructor #1"_test = []
        {
            Component a, b;
            DataPath dp{&a, &b, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_PHYSICAL};
            expect(that % &a == dp.GetSource());
            expect(that % &b == dp.GetTarget());
            expect(that % SYS_SAGE_DATAPATH_ORIENTED == dp.GetOrientation());
            expect(that % SYS_SAGE_DATAPATH_TYPE_PHYSICAL == dp.GetDataPathType());
        };
        "Constructor #2"_test = []
        {
            Component a, b;
            DataPath dp{&a, &b, SYS_SAGE_DATAPATH_ORIENTED, 5.0, 42.0};
            expect(that % &a == dp.GetSource());
            expect(that % &b == dp.GetTarget());
            expect(that % SYS_SAGE_DATAPATH_ORIENTED == dp.GetOrientation());
            expect(that % SYS_SAGE_DATAPATH_TYPE_NONE == dp.GetDataPathType());
            expect(that % 5.0 == dp.GetBandwidth());
            expect(that % 42.0 == dp.GetLatency());
        };
        "Constructor #3"_test = []
        {
            Component a, b;
            DataPath dp{&a, &b, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_PHYSICAL, 5.0, 42.0};
            expect(that % &a == dp.GetSource());
            expect(that % &b == dp.GetTarget());
            expect(that % SYS_SAGE_DATAPATH_ORIENTED == dp.GetOrientation());
            expect(that % SYS_SAGE_DATAPATH_TYPE_PHYSICAL == dp.GetDataPathType());
            expect(that % 5.0 == dp.GetBandwidth());
            expect(that % 42.0 == dp.GetLatency());
        };
    };

    "Unidirectional data path"_test = []
    {
        Component a, b;
        DataPath dp{&a, &b, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_PHYSICAL};

        expect(that % (nullptr != a.GetDataPaths(SYS_SAGE_DATAPATH_INCOMING)) >> fatal);
        expect(that % (nullptr != a.GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING)) >> fatal);
        expect(that % a.GetDataPaths(SYS_SAGE_DATAPATH_INCOMING)->empty());
        expect(that % (std::vector{&dp}) == *a.GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING));

        expect(that % (nullptr != b.GetDataPaths(SYS_SAGE_DATAPATH_INCOMING)) >> fatal);
        expect(that % (nullptr != b.GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING)) >> fatal);
        expect(that % (std::vector{&dp}) == *b.GetDataPaths(SYS_SAGE_DATAPATH_INCOMING));
        expect(that % b.GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING)->empty());
    };

    "Bidirectional data path"_test = []
    {
        Component a, b;
        DataPath dp{&a, &b, SYS_SAGE_DATAPATH_BIDIRECTIONAL, SYS_SAGE_DATAPATH_TYPE_PHYSICAL};

        expect(that % (nullptr != a.GetDataPaths(SYS_SAGE_DATAPATH_INCOMING)) >> fatal);
        expect(that % (nullptr != a.GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING)) >> fatal);
        expect(that % (std::vector{&dp}) == *a.GetDataPaths(SYS_SAGE_DATAPATH_INCOMING));
        expect(that % (std::vector{&dp}) == *a.GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING));

        expect(that % (nullptr != b.GetDataPaths(SYS_SAGE_DATAPATH_INCOMING)) >> fatal);
        expect(that % (nullptr != b.GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING)) >> fatal);
        expect(that % (std::vector{&dp}) == *b.GetDataPaths(SYS_SAGE_DATAPATH_INCOMING));
        expect(that % (std::vector{&dp}) == *b.GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING));
    };

    "Get data path by type"_test = []
    {
        Component a, b;
        DataPath dp1{&a, &b, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_LOGICAL};
        DataPath dp2{&a, &b, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_PHYSICAL};
        DataPath dp3{&a, &b, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_PHYSICAL};
        DataPath dp4{&b, &a, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_PHYSICAL};

        expect(that % &dp1 == a.GetDataPathByType(SYS_SAGE_DATAPATH_TYPE_LOGICAL, SYS_SAGE_DATAPATH_OUTGOING));
        expect(that % &dp2 == a.GetDataPathByType(SYS_SAGE_DATAPATH_TYPE_PHYSICAL, SYS_SAGE_DATAPATH_OUTGOING));
        expect(that % nullptr == a.GetDataPathByType(SYS_SAGE_DATAPATH_TYPE_L3CAT, SYS_SAGE_DATAPATH_OUTGOING));
        expect(that % &dp4 == a.GetDataPathByType(SYS_SAGE_DATAPATH_TYPE_PHYSICAL, SYS_SAGE_DATAPATH_INCOMING));
        expect(that % &dp4 == b.GetDataPathByType(SYS_SAGE_DATAPATH_TYPE_PHYSICAL, SYS_SAGE_DATAPATH_OUTGOING));
        expect(that % &dp4 == b.GetDataPathByType(SYS_SAGE_DATAPATH_TYPE_PHYSICAL, SYS_SAGE_DATAPATH_INCOMING | SYS_SAGE_DATAPATH_OUTGOING));
    };

    "Get all data paths by type"_test = []
    {
        Component a, b;
        DataPath dp1{&a, &b, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_LOGICAL};
        DataPath dp2{&a, &b, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_PHYSICAL};
        DataPath dp3{&a, &b, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_PHYSICAL};
        DataPath dp4{&b, &a, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_PHYSICAL};

        std::vector<DataPath *> v;

        "Get all incoming logical data paths"_test = [&]()
        {
            a.GetAllDataPathsByType(&v, SYS_SAGE_DATAPATH_TYPE_LOGICAL, SYS_SAGE_DATAPATH_INCOMING);
            expect(that % v.empty());
        };

        "Get all incoming physical data paths"_test = [&]()
        {
            std::vector<DataPath *> v;
            a.GetAllDataPathsByType(&v, SYS_SAGE_DATAPATH_TYPE_PHYSICAL, SYS_SAGE_DATAPATH_INCOMING);
            expect(std::vector{&dp4} == v);
        };

        "Get all outgoing logical data paths"_test = [&]()
        {
            std::vector<DataPath *> v;
            a.GetAllDataPathsByType(&v, SYS_SAGE_DATAPATH_TYPE_LOGICAL, SYS_SAGE_DATAPATH_OUTGOING);
            expect(that % std::vector{&dp1} == v);
        };

        "Get all outgoing physical data paths"_test = [&]()
        {
            std::vector<DataPath *> v;
            a.GetAllDataPathsByType(&v, SYS_SAGE_DATAPATH_TYPE_PHYSICAL, SYS_SAGE_DATAPATH_OUTGOING);
            expect(that % std::vector{&dp2, &dp3} == v);
        };

        "Get all physical data paths"_test = [&]()
        {
            std::vector<DataPath *> v;
            a.GetAllDataPathsByType(&v, SYS_SAGE_DATAPATH_TYPE_PHYSICAL, SYS_SAGE_DATAPATH_INCOMING | SYS_SAGE_DATAPATH_OUTGOING);
            expect(that % std::vector{&dp2, &dp3, &dp4} == v);
        };
    };
};
