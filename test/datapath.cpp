#include <boost/ut.hpp>

#include "sys-sage.hpp"

using namespace boost::ut;
using namespace sys_sage;

static suite<"data-path"> _ = []
{
    "Constructors"_test = []
    {
        "Constructor #1"_test = []
        {
            Component a, b;
            DataPath dp{&a, &b, DataPathOrientation::Oriented, DataPathType::Physical};
            expect(that % &a == dp.GetSource());
            expect(that % &b == dp.GetTarget());
            expect(that % DataPathOrientation::Oriented == dp.GetOrientation());
            expect(that % DataPathType::Physical == dp.GetDataPathType());
        };
        "Constructor #2"_test = []
        {
            Component a, b;
            DataPath dp{&a, &b, DataPathOrientation::Oriented, 5.0, 42.0};
            expect(that % &a == dp.GetSource());
            expect(that % &b == dp.GetTarget());
            expect(that % DataPathOrientation::Oriented == dp.GetOrientation());
            expect(that % DataPathType::None == dp.GetDataPathType());
            expect(that % 5.0 == dp.GetBandwidth());
            expect(that % 42.0 == dp.GetLatency());
        };
        "Constructor #3"_test = []
        {
            Component a, b;
            DataPath dp{&a, &b, DataPathOrientation::Oriented, DataPathType::Physical, 5.0, 42.0};
            expect(that % &a == dp.GetSource());
            expect(that % &b == dp.GetTarget());
            expect(that % DataPathOrientation::Oriented == dp.GetOrientation());
            expect(that % DataPathType::Physical == dp.GetDataPathType());
            expect(that % 5.0 == dp.GetBandwidth());
            expect(that % 42.0 == dp.GetLatency());
        };
    };

    "Unidirectional data path"_test = []
    {
        Component a, b;
        DataPath dp{&a, &b, DataPathOrientation::Oriented, DataPathType::Physical};

        expect(that % a.FindDataPaths(DataPathType::Any, DataPathDirection::Incoming).empty());
        expect(that % std::vector{&dp} == a.FindDataPaths(DataPathType::Any, DataPathDirection::Outgoing));

        expect(that % b.FindDataPaths(DataPathType::Any, DataPathDirection::Outgoing).empty());
        expect(that % std::vector{&dp} == b.FindDataPaths(DataPathType::Any, DataPathDirection::Incoming));
    };

    "Bidirectional data path"_test = []
    {
        Component a, b;
        DataPath dp{&a, &b, DataPathOrientation::Bidirectional, DataPathType::Physical};

        expect(that % std::vector{&dp} == a.FindDataPaths(DataPathType::Any, DataPathDirection::Incoming));
        expect(that % std::vector{&dp} == a.FindDataPaths(DataPathType::Any, DataPathDirection::Outgoing));

        expect(that % std::vector{&dp} == b.FindDataPaths(DataPathType::Any, DataPathDirection::Incoming));
        expect(that % std::vector{&dp} == b.FindDataPaths(DataPathType::Any, DataPathDirection::Outgoing));
    };

    "Get data path by type"_test = []
    {
        Component a, b;
        DataPath dp1{&a, &b, DataPathOrientation::Oriented, DataPathType::Logical};
        DataPath dp2{&a, &b, DataPathOrientation::Oriented, DataPathType::Physical};
        DataPath dp3{&a, &b, DataPathOrientation::Oriented, DataPathType::Physical};
        DataPath dp4{&b, &a, DataPathOrientation::Oriented, DataPathType::Physical};

        expect(that % &dp1 == a.GetDataPathByType(DataPathType::Logical, DataPathDirection::Outgoing));
        expect(that % &dp2 == a.GetDataPathByType(DataPathType::Physical, DataPathDirection::Outgoing));
        expect(that % nullptr == a.GetDataPathByType(DataPathType::L3CAT, DataPathDirection::Outgoing));
        expect(that % &dp4 == a.GetDataPathByType(DataPathType::Physical, DataPathDirection::Incoming));
        expect(that % &dp4 == b.GetDataPathByType(DataPathType::Physical, DataPathDirection::Outgoing));
        expect(that % &dp2 == b.GetDataPathByType(DataPathType::Physical, DataPathDirection::Any));
    };

    "Get all data paths by type"_test = []
    {
        Component a, b;
        DataPath dp1{&a, &b, DataPathOrientation::Oriented, DataPathType::Logical};
        DataPath dp2{&a, &b, DataPathOrientation::Oriented, DataPathType::Physical};
        DataPath dp3{&a, &b, DataPathOrientation::Oriented, DataPathType::Physical};
        DataPath dp4{&b, &a, DataPathOrientation::Oriented, DataPathType::Physical};

        "Get all incoming logical data paths"_test = [&]()
        {
            std::vector<DataPath *> v;
            a.FindDataPaths(&v, DataPathType::Logical, DataPathDirection::Incoming);
            expect(that % v.empty());
        };

        "Get all incoming physical data paths"_test = [&]()
        {
            std::vector<DataPath *> v;
            a.FindDataPaths(&v, DataPathType::Physical, DataPathDirection::Incoming);
            expect(std::vector{&dp4} == v);
        };

        "Get all outgoing logical data paths"_test = [&]()
        {
            std::vector<DataPath *> v;
            a.FindDataPaths(&v, DataPathType::Logical, DataPathDirection::Outgoing);
            expect(that % std::vector{&dp1} == v);
        };

        "Get all outgoing physical data paths"_test = [&]()
        {
            std::vector<DataPath *> v;
            a.FindDataPaths(&v, DataPathType::Physical, DataPathDirection::Outgoing);
            expect(that % std::vector{&dp2, &dp3} == v);
        };

        "Get all physical data paths"_test = [&]()
        {
            std::vector<DataPath *> v;

            a.FindDataPaths(&v, DataPathType::Physical, DataPathDirection::Any);
            expect(that % std::vector{&dp2, &dp3, &dp4} == v);
        };
    };
};
