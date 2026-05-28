#include <sys-sage.hpp>
#include <boost/ut.hpp>
#include <vector>

using namespace boost::ut;
using namespace sys_sage;

static suite<"data-path"> _ = []
{
    "Constructors"_test = []
    {
        "Constructor #1"_test = []
        {
            Component a, b;
            DataPath dp{&a, &b, DataPathOrientation::Oriented, DataPathCategory::Physical};
            expect(that % &a == dp.GetSource());
            expect(that % &b == dp.GetTarget());
            expect(that % DataPathOrientation::Oriented == dp.GetOrientation());
            expect(that % DataPathCategory::Physical == dp.GetDataPathCategory());
        };
        "Constructor #2"_test = []
        {
            Component a, b;
            DataPath dp{&a, &b, DataPathOrientation::Oriented, 5.0, 42.0};
            expect(that % &a == dp.GetSource());
            expect(that % &b == dp.GetTarget());
            expect(that % DataPathOrientation::Oriented == dp.GetOrientation());
            expect(that % DataPathCategory::None == dp.GetDataPathCategory());
            expect(that % 5.0 == dp.GetBandwidth());
            expect(that % 42.0 == dp.GetLatency());
        };
        "Constructor #3"_test = []
        {
            Component a, b;
            DataPath dp{&a, &b, DataPathOrientation::Oriented, DataPathCategory::Physical, 5.0, 42.0};
            expect(that % &a == dp.GetSource());
            expect(that % &b == dp.GetTarget());
            expect(that % DataPathOrientation::Oriented == dp.GetOrientation());
            expect(that % DataPathCategory::Physical == dp.GetDataPathCategory());
            expect(that % 5.0 == dp.GetBandwidth());
            expect(that % 42.0 == dp.GetLatency());
        };
    };

    "Unidirectional data path"_test = []
    {
        Component a, b;
        DataPath dp{&a, &b, DataPathOrientation::Oriented, DataPathCategory::Physical};

        expect(that % a.FindDataPaths(DataPathCategory::Any, DataPathDirection::Incoming).empty());
        expect(that % std::vector{&dp} == a.FindDataPaths(DataPathCategory::Any, DataPathDirection::Outgoing));

        expect(that % b.FindDataPaths(DataPathCategory::Any, DataPathDirection::Outgoing).empty());
        expect(that % std::vector{&dp} == b.FindDataPaths(DataPathCategory::Any, DataPathDirection::Incoming));
    };

    "Bidirectional data path"_test = []
    {
        Component a, b;
        DataPath dp{&a, &b, DataPathOrientation::Bidirectional, DataPathCategory::Physical};

        expect(that % std::vector{&dp} == a.FindDataPaths(DataPathCategory::Any, DataPathDirection::Incoming));
        expect(that % std::vector{&dp} == a.FindDataPaths(DataPathCategory::Any, DataPathDirection::Outgoing));

        expect(that % std::vector{&dp} == b.FindDataPaths(DataPathCategory::Any, DataPathDirection::Incoming));
        expect(that % std::vector{&dp} == b.FindDataPaths(DataPathCategory::Any, DataPathDirection::Outgoing));
    };

    "Get data path by type"_test = []
    {
        Component a, b;
        DataPath dp1{&a, &b, DataPathOrientation::Oriented, DataPathCategory::Logical};
        DataPath dp2{&a, &b, DataPathOrientation::Oriented, DataPathCategory::Physical};
        DataPath dp3{&a, &b, DataPathOrientation::Oriented, DataPathCategory::Physical};
        DataPath dp4{&b, &a, DataPathOrientation::Oriented, DataPathCategory::Physical};

        expect(that % &dp1 == a.GetDataPathByCategory(DataPathCategory::Logical, DataPathDirection::Outgoing));
        expect(that % &dp2 == a.GetDataPathByCategory(DataPathCategory::Physical, DataPathDirection::Outgoing));
        expect(that % nullptr == a.GetDataPathByCategory(DataPathCategory::L3CAT, DataPathDirection::Outgoing));
        expect(that % &dp4 == a.GetDataPathByCategory(DataPathCategory::Physical, DataPathDirection::Incoming));
        expect(that % &dp4 == b.GetDataPathByCategory(DataPathCategory::Physical, DataPathDirection::Outgoing));
        expect(that % &dp2 == b.GetDataPathByCategory(DataPathCategory::Physical, DataPathDirection::Any));
    };

    "Get all data paths by type"_test = []
    {
        Component a, b;
        DataPath dp1{&a, &b, DataPathOrientation::Oriented, DataPathCategory::Logical};
        DataPath dp2{&a, &b, DataPathOrientation::Oriented, DataPathCategory::Physical};
        DataPath dp3{&a, &b, DataPathOrientation::Oriented, DataPathCategory::Physical};
        DataPath dp4{&b, &a, DataPathOrientation::Oriented, DataPathCategory::Physical};

        "Get all incoming logical data paths"_test = [&]()
        {
            std::vector<DataPath *> v;
            a.FindDataPaths(v, DataPathCategory::Logical, DataPathDirection::Incoming);
            expect(that % v.empty());
        };

        "Get all incoming physical data paths"_test = [&]()
        {
            std::vector<DataPath *> v;
            a.FindDataPaths(v, DataPathCategory::Physical, DataPathDirection::Incoming);
            expect(std::vector{&dp4} == v);
        };

        "Get all outgoing logical data paths"_test = [&]()
        {
            std::vector<DataPath *> v;
            a.FindDataPaths(v, DataPathCategory::Logical, DataPathDirection::Outgoing);
            expect(that % std::vector{&dp1} == v);
        };

        "Get all outgoing physical data paths"_test = [&]()
        {
            std::vector<DataPath *> v;
            a.FindDataPaths(v, DataPathCategory::Physical, DataPathDirection::Outgoing);
            expect(that % std::vector{&dp2, &dp3} == v);
        };

        "Get all physical data paths"_test = [&]()
        {
            std::vector<DataPath *> v;

            a.FindDataPaths(v, DataPathCategory::Physical, DataPathDirection::Any);
            expect(that % std::vector{&dp2, &dp3, &dp4} == v);
        };
    };
};
