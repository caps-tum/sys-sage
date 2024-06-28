#include <boost/ut.hpp>

#include "sys-sage.hpp"

using namespace boost::ut;

static suite<"topology"> _ = []
{
    "Node"_test = []
    {
        Node node{42};
        expect(that % 42 == node.GetId());
        expect(that % SYS_SAGE_COMPONENT_NODE == node.GetComponentType());
        expect(that % "Node"sv == node.GetComponentTypeStr());
        expect(that % nullptr == node.GetParent());

        Node root{0};
        node.SetParent(&root);
        expect(that % &root == node.GetParent());
    };

    "Topology"_test = []
    {
        Topology node;
        expect(that % 0 == node.GetId());
        expect(that % SYS_SAGE_COMPONENT_TOPOLOGY == node.GetComponentType());
        expect(that % "Topology"sv == node.GetComponentTypeStr());
    };

    "Thread"_test = []
    {
        Node root{0};
        Thread node{&root, 42, "foo"};
        expect(that % &root == node.GetParent());
        expect(that % 42 == node.GetId());
        expect(that % "foo"sv == node.GetName());
        expect(that % SYS_SAGE_COMPONENT_THREAD == node.GetComponentType());
        expect(that % "HW_thread"sv == node.GetComponentTypeStr());
    };

    "Core"_test = []
    {
        Node root{0};
        Core node{&root, 42, "foo"};
        expect(that % &root == node.GetParent());
        expect(that % 42 == node.GetId());
        expect(that % "foo"sv == node.GetName());
        expect(that % SYS_SAGE_COMPONENT_CORE == node.GetComponentType());
        expect(that % "Core"sv == node.GetComponentTypeStr());
    };

    "Cache"_test = []
    {
        Node root{0};
        Cache node{&root, 42, "3", 32, 2, 16};
        expect(that % &root == node.GetParent());
        expect(that % 42 == node.GetId());
        expect(that % "Cache"sv == node.GetName());
        expect(that % "3"sv == node.GetCacheName());
        expect(that % 3 == node.GetCacheLevel());
        expect(that % 32 == node.GetCacheSize());
        expect(that % 2 == node.GetCacheAssociativityWays());
        expect(that % 16 == node.GetCacheLineSize());
        expect(that % SYS_SAGE_COMPONENT_CACHE == node.GetComponentType());
        expect(that % "Cache"sv == node.GetComponentTypeStr());

        node.SetCacheSize(16);
        expect(that % 16 == node.GetCacheSize());

        node.SetCacheLineSize(8);
        expect(that % 8 == node.GetCacheLineSize());
    };

    "Subdivision"_test = []
    {
        Node root{0};
        Subdivision node{&root, 42, "foo"};
        expect(that % &root == node.GetParent());
        expect(that % 42 == node.GetId());
        expect(that % "foo"sv == node.GetName());
        expect(that % SYS_SAGE_COMPONENT_SUBDIVISION == node.GetComponentType());
        expect(that % "Subdivision"sv == node.GetComponentTypeStr());

        node.SetSubdivisionType(3);
        expect(that % 3 == node.GetSubdivisionType());
    };

    "Numa"_test = []
    {
        Node root{0};
        Numa node{&root, 42, 64};
        expect(that % &root == node.GetParent());
        expect(that % 42 == node.GetId());
        expect(that % "Numa"sv == node.GetName());
        expect(that % SYS_SAGE_COMPONENT_NUMA == node.GetComponentType());
        expect(that % "NUMA"sv == node.GetComponentTypeStr());

        node.SetSubdivisionType(3);
        expect(that % 3 == node.GetSubdivisionType());
    };

    "Chip"_test = []
    {
        Node root{0};
        Chip node{&root, 42, "foo", 5};
        expect(that % &root == node.GetParent());
        expect(that % 42 == node.GetId());
        expect(that % "foo"sv == node.GetName());
        expect(that % SYS_SAGE_COMPONENT_CHIP == node.GetComponentType());
        expect(that % "Chip"sv == node.GetComponentTypeStr());
        expect(that % 5 == node.GetChipType());

        node.SetModel("model");
        expect(that % "model"sv == node.GetModel());

        node.SetVendor("vendor");
        expect(that % "vendor"sv == node.GetVendor());

        node.SetChipType(6);
        expect(that % 6 == node.GetChipType());
    };

    "Memory"_test = []
    {
        Node root{0};
        Memory node{&root, 0, "foo", 32};
        expect(that % &root == node.GetParent());
        expect(that % 0 == node.GetId());
        expect(that % "foo"sv == node.GetName());
        expect(that % 32 == node.GetSize());
        expect(that % SYS_SAGE_COMPONENT_MEMORY == node.GetComponentType());
        expect(that % "Memory"sv == node.GetComponentTypeStr());

        node.SetSize(64);
        expect(that % 64 == node.GetSize());
    };

    "Storage"_test = []
    {
        Node root{0};
        Storage node{&root};
        expect(that % &root == node.GetParent());
        expect(that % SYS_SAGE_COMPONENT_STORAGE == node.GetComponentType());
        expect(that % "Storage"sv == node.GetComponentTypeStr());

        node.SetSize(64);
        expect(that % 64 == node.GetSize());
    };

    "Children insertion and removal"_test = []
    {
        Node a;
        Node b;
        Node c;
        Node d;

        a.InsertChild(&b);
        a.InsertChild(&c);
        a.InsertChild(&d);
        expect(that % 3_u == a.GetChildren()->size());

        expect(that % 1 == a.RemoveChild(&b));
        expect(that % (2_u == a.GetChildren()->size()) >> fatal);
        expect(that % (std::find(a.GetChildren()->begin(), a.GetChildren()->end(), &b) == a.GetChildren()->end()));
        expect(that % (std::find(a.GetChildren()->begin(), a.GetChildren()->end(), &c) != a.GetChildren()->end()));
        expect(that % (std::find(a.GetChildren()->begin(), a.GetChildren()->end(), &d) != a.GetChildren()->end()));
    };

    "Get child"_test = []
    {
        Node a{1};
        Node b{2};
        Node c{3};
        Node d{4};

        a.InsertChild(&b);
        a.InsertChild(&c);
        b.InsertChild(&d);

        expect(that % a.GetChild(2) == &b);
        expect(that % a.GetChild(3) == &c);
        expect(that % a.GetChild(4) == nullptr);
    };

    "Get child by type"_test = []
    {
        Node a;
        Memory b;
        Memory c;
        Chip d;

        a.InsertChild(&b);
        a.InsertChild(&c);
        a.InsertChild(&d);

        expect(that % a.GetChildByType(SYS_SAGE_COMPONENT_MEMORY) == &b);
        expect(that % a.GetChildByType(SYS_SAGE_COMPONENT_CHIP) == &d);
        expect(that % a.GetAllChildrenByType(SYS_SAGE_COMPONENT_MEMORY) == (std::vector<Component *>{&b, &c}));
    };

    "Get parent by type"_test = []
    {
        Cache a;
        Core b{&a};
        Thread c{&b};
        expect(that % &a == c.FindParentByType(SYS_SAGE_COMPONENT_CACHE));
    };

    "Component tree consistency"_test = []
    {
        {
            Node a;
            Node b;
            Node c;
            a.InsertChild(&b);
            b.InsertChild(&c);
            expect(that % 0 == a.CheckComponentTreeConsistency());
        }
        {
            Node a;
            Node b;
            Node c;
            a.InsertChild(&b);
            b.InsertChild(&c);
            c.SetParent(&a);
            expect(that % 1 == a.CheckComponentTreeConsistency());
        }
    };

    "Get deeper components"_test = []
    {
        Node a;
        Node b;
        Node c;
        Node d;

        a.InsertChild(&b);
        a.InsertChild(&c);
        c.InsertChild(&d);

        std::vector<Component *> array;
        a.GetNthDescendents(&array, 1);
        expect(that % 2_u == array.size());
    };

    "Get subcomponents by type"_test = []
    {
        Node a;
        Chip b;
        Memory c;
        Chip d;

        a.InsertChild(&b);
        a.InsertChild(&c);
        c.InsertChild(&d);

        std::vector<Component *> array;
        a.GetSubcomponentsByType(&array, SYS_SAGE_COMPONENT_CHIP);
        expect(that % 2_u == array.size());
    };

    "Get total number of threads"_test = []
    {
        Node a;
        Thread b;
        Thread c;
        Node d;
        Thread e;
        Node f;

        a.InsertChild(&b);
        a.InsertChild(&c);
        a.InsertChild(&d);
        d.InsertChild(&e);
        d.InsertChild(&f);

        expect(that % 3 == a.GetNumThreads());
    };

    "Linearize subtree"_test = []
    {
        Node a;
        Node b;
        Node c;
        Node d;

        a.InsertChild(&b);
        b.InsertChild(&d);
        a.InsertChild(&c);

        std::vector<Component *> array;
        a.GetComponentsInSubtree(&array);
        expect(that % array == (std::vector<Component *>{&a, &b, &d, &c}));
    };

    "Tree depth"_test = []
    {
        Node a;
        Node b;
        Node c;
        Node d;
        Node e;
        Node f;
        Node g;

        a.InsertChild(&b);
        b.InsertChild(&c);
        a.InsertChild(&d);
        d.InsertChild(&e);
        e.InsertChild(&f);
        a.InsertChild(&g);

        expect(that % 3 == a.GetSubtreeDepth());
    };
};
