#include <sys-sage.hpp>
#include <boost/ut.hpp>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>
#include <libxml/xpath.h>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <vector>

using namespace sys_sage;

using XmlStringView = std::basic_string_view<const xmlChar>;

/**
 * Create a string view over UTF-8 code points as used by libxml.
 */
XmlStringView operator""_xsv(const char *string, size_t len)
{
    return {BAD_CAST(string), len};
}

template <class T>
using raii = std::unique_ptr<T, void (*)(T *)>;

using namespace boost::ut;

/**
 * Validates XML output of sys-sage using an XML schema file.
 */
void validate(std::string_view path)
{
    auto parser = raii<xmlSchemaParserCtxt>{xmlSchemaNewParserCtxt(SYS_SAGE_TEST_RESOURCE_DIR "/schema.xml"), xmlSchemaFreeParserCtxt};
    expect(that % (parser != nullptr) >> fatal);

    auto schema = raii<xmlSchema>{xmlSchemaParse(parser.get()), xmlSchemaFree};
    expect(that % (schema != nullptr) >> fatal);

    auto validator = raii<xmlSchemaValidCtxt>{xmlSchemaNewValidCtxt(schema.get()), xmlSchemaFreeValidCtxt};

    xmlSchemaSetValidErrors(validator.get(), (xmlSchemaValidityErrorFunc)fprintf, (xmlSchemaValidityWarningFunc)fprintf, stdout);

    auto doc = raii<xmlDoc>{xmlParseFile(path.data()), xmlFreeDoc};
    expect(that % (doc != nullptr) >> fatal);

    int code = xmlSchemaValidateDoc(validator.get(), doc.get());
    expect(that % (code == 0) >> fatal);

    auto pathContext = raii<xmlXPathContext>{xmlXPathNewContext(doc.get()), xmlXPathFreeContext};
    expect(that % (pathContext != nullptr) >> fatal);

    "Attributes either have child nodes or a value attribute"_test = [&]
    {
        auto attributeNodes = raii<xmlXPathObject>{xmlXPathEvalExpression(BAD_CAST("//Attribute"), pathContext.get()), xmlXPathFreeObject};
        expect(that % (attributeNodes->nodesetval != nullptr) >> fatal);
        expect(that % (attributeNodes->type == XPATH_NODESET) >> fatal);
        for (int i = 0; i < attributeNodes->nodesetval->nodeNr; ++i)
        {
            xmlNode *node = attributeNodes->nodesetval->nodeTab[i];
            bool hasChildren = node->children != nullptr;

            auto attributes = raii<xmlXPathObject>{xmlXPathNodeEval(node, BAD_CAST("@value"), pathContext.get()), xmlXPathFreeObject};
            bool hasValueAttribute = attributes->nodesetval->nodeNr > 0;

            bool hasNeither = !(hasChildren || hasValueAttribute);
            bool hasEitherChildrenOrValueAttribute = hasChildren != hasValueAttribute;

            expect(hasEitherChildrenOrValueAttribute || hasNeither)
                << path << ':' << node->line << " <Attribute> must have either child nodes or a value attribute";
        }
    };

    "All component addresses are unique and that all data path endpoints exist"_test = [&]
    {
        auto componentAddrs = raii<xmlXPathObject>{xmlXPathEvalExpression(BAD_CAST("/sys-sage/components//*/@addr"), pathContext.get()), xmlXPathFreeObject};
        auto dataPathEndpoints = raii<xmlXPathObject>{
            xmlXPathEvalExpression(BAD_CAST("/sys-sage/data-paths/datapath/@source | /sys-sage/data-paths/datapath/@target"), pathContext.get()),
            xmlXPathFreeObject};

        std::set<XmlStringView> addrs;

        if (componentAddrs->nodesetval != nullptr && componentAddrs->nodesetval->nodeNr > 0)
        {
            for (int i = 0; i < componentAddrs->nodesetval->nodeNr; ++i)
            {
                auto node = componentAddrs->nodesetval->nodeTab[i];
                XmlStringView addr = node->children->content;
                expect(!addrs.contains(addr))
                    << path << ':' << node->children->line << " The addr attribute must be unique for each component";
                addrs.insert(addr);
            }
        }

        if (dataPathEndpoints->nodesetval != nullptr && dataPathEndpoints->nodesetval->nodeNr > 0)
        {
            for (int i = 0; i < dataPathEndpoints->nodesetval->nodeNr; ++i)
            {
                auto endpoint = dataPathEndpoints->nodesetval->nodeTab[i];
                XmlStringView addr = endpoint->children->content;
                expect(addrs.contains(addr))
                    << path << ':' << endpoint->children->line << " The data path endpoint must be an existing component address";
            }
        }
    };
}

xmlNode *getSingleNodeByPath(xmlChar *path, xmlXPathContext *context)
{
    auto result = raii<xmlXPathObject>{xmlXPathEvalExpression(path, context), xmlXPathFreeObject};
    expect(that % result->nodesetval != nullptr && result->type == XPATH_NODESET && result->nodesetval->nodeNr == 1)
        << "Invalid query path " << reinterpret_cast<const char *>(path);
    return result->nodesetval->nodeTab[0];
}

static suite<"export"> _ = []
{
    "Minimal topology"_test = []
    {
        auto topo = new Component(42, "a name");
        exportToXml(topo, "test.xml");
        delete topo;
        validate("test.xml");
    };

    "Sample output"_test = []
    {
        validate(SYS_SAGE_TEST_RESOURCE_DIR "/sys-sage_sample_output.xml");
    };

    "Sample output with attributes"_test = []
    {
        validate(SYS_SAGE_TEST_RESOURCE_DIR "/sys-sage_custom_attributes.xml");
    };

    "Single component"_test = []
    {
        {
            auto topo = new Topology;
            new Memory{topo, 56, "A single memory component", 16};
            exportToXml(topo, "test.xml");
        }

        {
            validate("test.xml");

            auto doc = raii<xmlDoc>{xmlParseFile("test.xml"), xmlFreeDoc};
            expect(that % (doc != nullptr) >> fatal);

            auto pathContext = raii<xmlXPathContext>{xmlXPathNewContext(doc.get()), xmlXPathFreeContext};
            expect(that % (pathContext != nullptr) >> fatal);

            xmlNode *topo = getSingleNodeByPath(BAD_CAST("/sys-sage/Components/Topology"), pathContext.get());
            xmlNode *memory = getSingleNodeByPath(BAD_CAST("/sys-sage/Components/Topology/Memory"), pathContext.get());

            for (const auto &[node, xpath, value] : std::vector{
                     std::tuple{topo, "string(@id)", "0"},
                     std::tuple{topo, "string(@name)", "sys-sage Topology"},
                     std::tuple{memory, "string(@id)", "56"},
                     std::tuple{memory, "string(@name)", "A single memory component"},
                     std::tuple{memory, "string(@size)", "16"},
                 })
            {
                auto result = raii<xmlXPathObject>{xmlXPathNodeEval(node, BAD_CAST(xpath), pathContext.get()), xmlXPathFreeObject};
                expect((result != nullptr) and that % XmlStringView{BAD_CAST(value)} == XmlStringView{result->stringval});
            }
        }
    };

    "Custom attributes"_test = []
    {
        {
            Topology topo;
            Node node{&topo, 1};

            std::string codename = "marsupial";
            std::string foo = "foo";
            int rackNo = 15;
            node.attrib["codename"] = reinterpret_cast<void *>(&codename);
            node.attrib["rack_no"] = reinterpret_cast<void *>(&rackNo);
            node.attrib["unknown_will_not_be_printed"] = reinterpret_cast<void *>(&foo);

            struct My_core_attributes
            {
                double temperature;
                int frequency;
            };

            My_core_attributes attrib{.temperature = 38.222, .frequency = 2000000000};
            Core c1{&node, 1};
            c1.attrib["my_core_info"] = reinterpret_cast<void *>(&attrib);

            auto print_my_attribs = [](std::string key, void *value, std::string *ret_value_str) -> int
            {
                if (key == "codename" || key == "info")
                {
                    *ret_value_str = *(std::string *)value;
                    return 1;
                }
                else if (key == "rack_no")
                {
                    *ret_value_str = std::to_string(*(int *)value);
                    return 1;
                }

                return 0;
            };

            auto print_my_custom_attribs = [](std::string key, void *value, xmlNode *xml) -> int
            {
                if (key != "my_core_info")
                {
                    return 0;
                }

                auto custom = reinterpret_cast<My_core_attributes *>(value);
                auto temperatur = std::to_string(custom->temperature);
                auto frequency = std::to_string(custom->frequency);
                xmlNode *attribNode = xmlNewNode(nullptr, BAD_CAST(key.c_str()));
                xmlNewProp(attribNode, BAD_CAST("temperature"), BAD_CAST(temperatur.c_str()));
                xmlNewProp(attribNode, BAD_CAST("temp_unit"), BAD_CAST("C"));
                xmlNewProp(attribNode, BAD_CAST("frequency"), BAD_CAST(frequency.c_str()));
                xmlNewProp(attribNode, BAD_CAST("freq_unit"), BAD_CAST("Hz"));

                xmlNode *attrib = xmlNewNode(nullptr, BAD_CAST("Attribute"));
                xmlNewProp(attrib, BAD_CAST("name"), BAD_CAST(key.c_str()));
                xmlAddChild(xml, attrib);
                xmlAddChild(attrib, attribNode);

                return 1;
            };

            std::string output_name = "test.xml";
            exportToXml(&topo, output_name, print_my_attribs, print_my_custom_attribs);
        }

        {
            validate("test.xml");

            auto doc = raii<xmlDoc>{xmlParseFile("test.xml"), xmlFreeDoc};
            expect(that % (doc != nullptr) >> fatal);

            auto pathContext = raii<xmlXPathContext>{xmlXPathNewContext(doc.get()), xmlXPathFreeContext};
            expect(that % (pathContext != nullptr) >> fatal);

            xmlNode *node = getSingleNodeByPath(BAD_CAST("/sys-sage/Components/Topology/Node"), pathContext.get());
            xmlNode *core = getSingleNodeByPath(BAD_CAST("/sys-sage/Components/Topology/Node/Core"), pathContext.get());

            for (const auto &[node, xpath, value] : std::vector{
                     std::tuple{node, "string(Attribute[1]/@name)", "codename"},
                     std::tuple{node, "string(Attribute[1]/@value)", "marsupial"},
                     std::tuple{node, "string(Attribute[2]/@name)", "rack_no"},
                     std::tuple{node, "string(Attribute[2]/@value)", "15"},
                     std::tuple{core, "string(Attribute/@name)", "my_core_info"},
                     std::tuple{core, "string(Attribute/my_core_info/@temperature)", "38.222000"},
                     std::tuple{core, "string(Attribute/my_core_info/@temp_unit)", "C"},
                     std::tuple{core, "string(Attribute/my_core_info/@frequency)", "2000000000"},
                     std::tuple{core, "string(Attribute/my_core_info/@freq_unit)", "Hz"},
                 })
            {
                auto result = raii<xmlXPathObject>{xmlXPathNodeEval(node, BAD_CAST(xpath), pathContext.get()), xmlXPathFreeObject};
                expect((result != nullptr) and that % XmlStringView{BAD_CAST(value)} == XmlStringView{result->stringval});
            }
        }
    };
};
