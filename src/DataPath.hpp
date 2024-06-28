#ifndef DATAPATH
#define DATAPATH

#include <map>

#include "defines.hpp"
#include "Topology.hpp"

//Component pointing to a DataPath 
#define SYS_SAGE_DATAPATH_NONE 1 /**< Reserved for development purposes. */
#define SYS_SAGE_DATAPATH_OUTGOING 2 /**< This Component is the source DataPath. */
#define SYS_SAGE_DATAPATH_INCOMING 4 /**< This Component is the taerget DataPath. */

//int oriented
#define SYS_SAGE_DATAPATH_BIDIRECTIONAL 8 /**< DataPath has no direction. */
#define SYS_SAGE_DATAPATH_ORIENTED 16 /**< DataPath is directec from the source to the target. */

//dp_type
#define SYS_SAGE_DATAPATH_TYPE_NONE 32 /**< Generic type of DataPath */
#define SYS_SAGE_DATAPATH_TYPE_LOGICAL 64 /**< DataPath describes a logical connection/relation of two Components. */
#define SYS_SAGE_DATAPATH_TYPE_PHYSICAL 128 /**< DataPath describes a physical/hardware connection/relation of two Components. */
#define SYS_SAGE_DATAPATH_TYPE_L3CAT 256 /**< DataPath type describing Cache partitioning settings. */
#define SYS_SAGE_DATAPATH_TYPE_MIG 512 /**< DataPath type describing GPU partitioning settings. */
#define SYS_SAGE_DATAPATH_TYPE_DATATRANSFER 1024 /**< DataPath type describing data transfer attributes. */
#define SYS_SAGE_DATAPATH_TYPE_C2C 2048 /**< DataPath type describing cache-to-cache latencies (cccbench data source). */

using namespace std;
class Component;
class DataPath;

/**
 @private
Obsolete; use DataPath() constructors directly instead
@see DataPath()
*/
[[deprecated("Use DataPath() constructors directly instead.")]]
DataPath* NewDataPath(Component* _source, Component* _target, int _oriented, int _type = SYS_SAGE_DATAPATH_TYPE_NONE);
/**
 * @private
Obsolete; use DataPath() constructors directly instead
@see DataPath()
*/
[[deprecated("Use DataPath() constructors directly instead.")]]
DataPath* NewDataPath(Component* _source, Component* _target, int _oriented, double _bw, double _latency);
/**
 * @private
Obsolete; use DataPath() constructors directly instead
@see DataPath()
*/
[[deprecated("Use DataPath() constructors directly instead.")]]
DataPath* NewDataPath(Component* _source, Component* _target, int _oriented, int _type, double _bw, double _latency);

/**
Class DataPath represents Data Paths in the topology -- Data Paths represent an arbitrary relation (or data movement) between two Components from the Component Tree.
\n Data Paths create a Data-Path graph, which is a structure orthogonal to the Component Tree.
\n Each Component contains a reference to all Data Paths going to or from this components (as well as parents and children in the Component Tree). Using these references, it is possible to navigate between the Components and to view the data stores in the Components or the Data Paths.
*/
class DataPath {

public:
    /**
    DataPath constructor.
    @param _source - pointer to the source Component. (If _oriented == SYS_SAGE_DATAPATH_BIDIRECTIONAL, there is no difference between _source and _target)
    @param _target - pointer to the target Component.
    @param _oriented - is the Data Path oriented? (SYS_SAGE_DATAPATH_ORIENTED = oriented DP; SYS_SAGE_DATAPATH_BIDIRECTIONAL = NOT oriented DP)
    @param _type - (optional parameter) Denotes type of the Data Path -- helps to distinguish Data Paths carrying different type of information. The value is user-defined. If not specified, the Data Path will have type SYS_SAGE_DATAPATH_TYPE_NONE.
        \n Predefined types: SYS_SAGE_DATAPATH_TYPE_NONE, SYS_SAGE_DATAPATH_TYPE_LOGICAL, SYS_SAGE_DATAPATH_TYPE_PHYSICAL, SYS_SAGE_DATAPATH_TYPE_L3CAT. Each user can define an arbitrary type as an integer value > 1024
    */
    DataPath(Component* _source, Component* _target, int _oriented, int _type = SYS_SAGE_DATAPATH_TYPE_NONE);
    /**
    DataPath constructor. DataPath type is set to SYS_SAGE_DATAPATH_TYPE_NONE.
    @param _source - pointer to the source Component. (If _oriented == SYS_SAGE_DATAPATH_BIDIRECTIONAL, there is no difference between _source and _target)
    @param _target - pointer to the target Component.
    @param _oriented - is the Data Path oriented? (SYS_SAGE_DATAPATH_ORIENTED = oriented DP; SYS_SAGE_DATAPATH_BIDIRECTIONAL = NOT oriented DP)
    @param _bw - bandwidth from the source(provides the data) to the target(requests the data)
    @param _latency - Data load latency from the source(provides the data) to the target(requests the data)
    */
    DataPath(Component* _source, Component* _target, int _oriented, double _bw, double _latency);
    /**
    DataPath constructor.
    @param _source - pointer to the source Component. (If _oriented == SYS_SAGE_DATAPATH_BIDIRECTIONAL, there is no difference between _source and _target)
    @param _target - pointer to the target Component.
    @param _oriented - is the Data Path oriented? (SYS_SAGE_DATAPATH_ORIENTED = oriented DP; SYS_SAGE_DATAPATH_BIDIRECTIONAL = NOT oriented DP)
    @param _type - (optional parameter) Denotes type of the Data Path -- helps to distinguish Data Paths carrying different type of information. The value is user-defined. If not specified, the Data Path will have type SYS_SAGE_DATAPATH_TYPE_NONE.
        \n Predefined types: SYS_SAGE_DATAPATH_TYPE_NONE, SYS_SAGE_DATAPATH_TYPE_LOGICAL, SYS_SAGE_DATAPATH_TYPE_PHYSICAL, SYS_SAGE_DATAPATH_TYPE_L3CAT. Each user can define an arbitrary type as an integer value > 1024
    @param _bw - Bandwidth from the source(provides the data) to the target(requests the data)
    @param _latency - Data load latency from the source(provides the data) to the target(requests the data)
    */
    DataPath(Component* _source, Component* _target, int _oriented, int _type, double _bw, double _latency);

    /**
    @returns Pointer to the source Component
    */
    Component* GetSource();
    /**
    @returns Pointer to the target Component
    */
    Component* GetTarget();
    /**
    @returns Bandwidth from the source(provides the data) to the target(requests the data)
    */
    double GetBandwidth();
    
    /**
     * Sets the bandwidth from the source to the target.
    @param _bandwidth - value for bw
    @see bw
    */
    void SetBandwidth(double _bandwidth);
    /**
    @returns Data load latency from the source(provides the data) to the target(requests the data)
    */
    double GetLatency();

    /**
     * Sets the data load latency from the source to the target.
    @param _latency - value for latency
    @see latency
    */
    void SetLatency(double _latency);
    /**
    @returns Type of the Data Path.
    @see dp_type
    */
    int GetDataPathType();
    /**
     * Retrieves the orientation of the datapath (SYS_SAGE_DATAPATH_ORIENTED or SYS_SAGE_DATAPATH_BIDIRECTIONAL)
     * @returns orientation 
     * @see oriented
     * 
    */
    int GetOrientation();

    /**
    Prints basic information about the Data Path to stdout. Prints componentType and Id of the source and target Components, the bandwidth, load latency, and the attributes; for each attribute, the name and value are printed, however the value is only retyped to uint64_t (therefore will print nonsensical values for other data types).
    */
    void Print();

    /**
    Deletes and de-allocates the DataPath pointer from the list(std::vector) of outgoing and incoming DataPaths of source and target Components.
    @see dp_incoming
    @see dp_outgoing
    */
    void DeleteDataPath();
    
    /**
    For storing arbitrary pieces of information or data. key denotes the name of the attribute,
    and the value points to the data.
    */
    map<string,void*> attrib;
private:
    Component * source; /**< source component of the datapath */
    Component * target; /**< taget component of the datapath */

    const int oriented; /**< orientation of the datapath (SYS_SAGE_DATAPATH_ORIENTED or SYS_SAGE_DATAPATH_BIDIRECTIONAL) */
    const int dp_type; /**< type of the datapath */

    double bw; /**< Bandwidth from the source(provides the data) to the target(requests the data) */
    double latency; /**< Data load latency from the source(provides the data) to the target(requests the data) */

};

#endif
