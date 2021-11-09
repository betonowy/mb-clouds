//
// Created by pekopeko on 24.07.2021.
//

#ifndef MB_CLOUDS_VDBDATAEXTRACT_H
#define MB_CLOUDS_VDBDATAEXTRACT_H

#include "vdbData.h"

#include <string>
#include <sstream>
#include <iomanip>

template<typename valueType, int rootLevel, int nodeLevel, int leafLevel>
class vdbGl {
public:
    using sourceDatasetType = vdbDataset<valueType, rootLevel, nodeLevel, leafLevel>;

    using sourceRootType = typename sourceDatasetType::rootType;
    using sourceNodeType = typename sourceDatasetType::nodeType;
    using sourceLeafType = typename sourceDatasetType::leafType;

    using sourceRootDataType = typename sourceRootType::dataStruct;
    using sourceNodeDataType = typename sourceNodeType::dataStruct;
    using sourceLeafDataType = typename sourceLeafType::dataStruct;

    struct Description {
        std::string toString() {
            constexpr uint32_t w = 10;

            auto dimToStr = [](dimType dim) {
                std::stringstream str;
                str << "{ " << dim.x << ", " << dim.y << ", " << dim.z << " }";
                return str.str();
            };

            std::stringstream str;
            str << "lowDimBB: " << std::setw(w) << dimToStr(lowDimBB) << "\n"
                << "highDimBB:" << std::setw(w) << dimToStr(highDimBB) << "\n"
                << "rootCount:" << std::setw(w) << rootCount << "\n"
                << "nodeCount:" << std::setw(w) << nodeCount << "\n"
                << "leafCount:" << std::setw(w) << leafCount << "\n";
            return str.str();
        }

        dimType lowDimBB;
        uint32_t rootCount;

        dimType highDimBB;
        uint32_t nodeCount;

        uint32_t leafCount;
    };

    struct RootDescription {
        static constexpr int topLevel = rootLevel + nodeLevel + leafLevel;

        explicit RootDescription(const sourceRootType &source) {
            lowDimBB = source.getLowDim() * (1 << topLevel);
            highDimBB = source.getHighDim() * (1 << topLevel);
            data = source.getData();
        };

        dimType lowDimBB{};
        uint32_t index{};

        dimType highDimBB{};
        uint32_t _padding_1{};

        sourceRootDataType data;
    };

    struct NodeDescription {
        static constexpr int topLevel = nodeLevel + leafLevel;

        explicit NodeDescription(const sourceNodeType &source) {
            lowDimBB = source.getLowDim() * (1 << topLevel);
            highDimBB = source.getHighDim() * (1 << topLevel);
            data = source.getData();
        };

        dimType lowDimBB{};
        uint32_t index{};

        dimType highDimBB{};
        uint32_t _padding_1{};

        sourceNodeDataType data;
    };

    struct LeafDescription {
        static constexpr int topLevel = leafLevel;

        explicit LeafDescription(const sourceLeafType &source) {
            lowDimBB = source.getLowDim() * (1 << topLevel);
            highDimBB = source.getHighDim() * (1 << topLevel);
            data = source.getData();
        };

        dimType lowDimBB{};
        uint32_t index{};

        dimType highDimBB{};
        uint32_t _padding_1{};

        sourceLeafDataType data;

        uint32_t _padding_2[1024];
    };

    // types end

    explicit vdbGl(const sourceDatasetType &source) {
        for (const auto &sourceRoot : source.getRoots()) {
            _roots.emplace_back(sourceRoot);
        }

        for (const auto &sourceNode : source.getNodes()) {
            _nodes.emplace_back(sourceNode);
        }

        for (const auto &sourceLeave : source.getLeaves()) {
            _leaves.emplace_back(sourceLeave);
        }

        _desc.lowDimBB = source.getLowDim();
        _desc.highDimBB = source.getHighDim();
        _desc.rootCount = source.getRoots().size();
        _desc.nodeCount = source.getNodes().size();
        _desc.leafCount = source.getLeaves().size();
    }

    Description *getDescriptionPtr() { return &_desc; }

    uint32_t getDescriptionSize() { return sizeof(_desc); }

    RootDescription *getRootsPtr() { return _roots.data(); }

    uint32_t getRootsSize() { return sizeof(RootDescription) * _roots.size(); }

    NodeDescription *getNodesPtr() { return _nodes.data(); }

    uint32_t getNodesSize() { return sizeof(NodeDescription) * _nodes.size(); }

    LeafDescription *getLeavesPtr() { return _leaves.data(); }

    uint32_t getLeavesSize() { return sizeof(LeafDescription) * _leaves.size(); }

private:
    std::vector<RootDescription> _roots;
    std::vector<NodeDescription> _nodes;
    std::vector<LeafDescription> _leaves;

    Description _desc;
};


#endif //MB_CLOUDS_VDBDATAEXTRACT_H
