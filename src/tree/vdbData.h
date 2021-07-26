//
// Created by pekopeko on 20.07.2021.
//

#ifndef MB_CLOUDS_VDBDATA_H
#define MB_CLOUDS_VDBDATA_H

#include <glm/glm.hpp>

#include <algorithm>
#include <array>
#include <bitset>
#include <optional>
#include <unordered_set>

using dimType = glm::ivec3;
static constexpr uint32_t vdbBadIndex = std::numeric_limits<uint32_t>::max();

template<int rootLevel, int nodeLevel, int leafLevel>
class vdbAccessor {
    static constexpr uint32_t ones = std::numeric_limits<uint32_t>::max();

    static constexpr int voxelL = 0;
    static constexpr int leafL = leafLevel;
    static constexpr int nodeL = nodeLevel + leafL;
    static constexpr int rootL = rootLevel + nodeL;

    static constexpr int voxelH = leafL;
    static constexpr int leafH = nodeL;
    static constexpr int nodeH = rootL;
    static constexpr int rootH = 0;

    template<int l, int h>
    dimType _reduceDim(dimType pos) {
        constexpr auto pattern = [](int a, int b) {
            uint32_t p{};

            for (int i = a; i < b; i++) {
                p |= 1 << i;
            }

            return p;
        };

        for (int i = 0; i < dimType::length(); i++) {
            auto &n = pos[i];

            if constexpr(bool(h)) {
                n &= pattern(0, h);
            }

            if constexpr(bool(l)) {
                n >>= l;
            }
        }

        return pos;
    }

public:
    vdbAccessor() = default;

    explicit vdbAccessor(dimType pos) :
            rootPos(_reduceDim<rootL, rootH>(pos)),
            nodePos(_reduceDim<nodeL, nodeH>(pos)),
            leafPos(_reduceDim<leafL, leafH>(pos)),
            voxelPos(_reduceDim<voxelL, voxelH>(pos)) {}

    [[nodiscard]] dimType decodePosition() const {
        dimType pos{};
        pos += rootPos * (1 << rootL);
        pos += nodePos * (1 << nodeL);
        pos += leafPos * (1 << leafL);
        pos += voxelPos;
        return pos;
    }

    dimType rootPos{};
    dimType nodePos{};
    dimType leafPos{};
    dimType voxelPos{};
};

template<typename valueType, int level>
class vdbLeaf {
public:
    static constexpr uint32_t ARRAY_SIZE = 1 << (level * 3);
    static constexpr int dimSize = 1 << level;

    struct dataStruct {
        std::bitset<ARRAY_SIZE> bitset;
        std::array<valueType, ARRAY_SIZE> values;
    };

    vdbLeaf(dimType lowDim, dimType highDim)
            : _lowDim(lowDim), _highDim(highDim) {}

    [[nodiscard]] uint32_t countOnValues() const { return _data.bitset.count(); };

    [[nodiscard]] const dataStruct &getData() const {
        return _data;
    }

    [[nodiscard]] dimType getLowDim() const { return _lowDim; }

    [[nodiscard]] dimType getHighDim() const { return _highDim; }

    [[nodiscard]] dimType getSize() const { return _highDim - _lowDim; }

    [[nodiscard]] uint32_t getActiveVoxelsCount() const {
        uint32_t sum{};
        for (uint32_t i = 0; i < ARRAY_SIZE; ++i) {
            sum += _data.bitset[i] == true;
        }
        return sum;
    };

    inline static constexpr uint32_t getLevel() { return level; }

    void setValue(dimType pos, const valueType &value) {
        assert(pos[0] >= 0 && pos[0] < dimSize);
        assert(pos[1] >= 0 && pos[1] < dimSize);
        assert(pos[2] >= 0 && pos[2] < dimSize);

        uint32_t index = pos[0] | pos[1] << level | pos[2] << level * 2;

        assert(index < ARRAY_SIZE);

        _data.bitset[index] = true;
        _data.values[index] = value;
    }

    void unsetValue(dimType pos) {
        assert(pos[0] >= 0 && pos[0] < dimSize);
        assert(pos[1] >= 0 && pos[1] < dimSize);
        assert(pos[2] >= 0 && pos[2] < dimSize);

        uint32_t index = pos[0] | pos[1] << level | pos[2] << level * 2;

        assert(index < ARRAY_SIZE);

        _data.bitset[index] = false;
    }

    [[nodiscard]] const valueType &getValue(dimType pos) const {
        assert(pos[0] >= 0 && pos[0] < dimSize);
        assert(pos[1] >= 0 && pos[1] < dimSize);
        assert(pos[2] >= 0 && pos[2] < dimSize);

        uint32_t index = pos[0] | pos[1] << level | pos[2] << level * 2;

        assert(index < ARRAY_SIZE);
        assert(_data.bitset[index]);

        return _data.values[index];
    }

    [[nodiscard]] bool isEmpty() const { return _data.bitset.none(); }

private:
    dataStruct _data;
    dimType _lowDim;
    dimType _highDim;

};

static_assert(vdbLeaf<float, 3>::ARRAY_SIZE == 512);
static_assert(sizeof(vdbLeaf<float, 3>::dataStruct::bitset) == 512 / 8);
static_assert(sizeof(vdbLeaf<float, 3>::dataStruct::values) == 512 * sizeof(float));

template<typename valueType, int level>
class vdbNode {
public:
    static constexpr uint32_t DIM_SIZE = 1 << level;
    static constexpr uint32_t ARRAY_SIZE = 1 << (level * 3);

    struct dataStruct {
        std::bitset<ARRAY_SIZE> bitset;
        std::array<uint32_t, ARRAY_SIZE> pointer;
    };

    vdbNode(dimType lowDim, dimType highDim)
            : _lowDim(lowDim), _highDim(highDim) {}

    [[nodiscard]] dimType getLowDim() const { return _lowDim; }

    [[nodiscard]] dimType getHighDim() const { return _highDim; }

    [[nodiscard]] dimType getSize() const { return _highDim - _lowDim; }

    void setIndex(dimType pos, uint32_t index) {
        uint32_t internalIndex = calculateIndex(pos);

        assert(internalIndex != vdbBadIndex);

        _data.bitset.set(internalIndex);
        _data.pointer[internalIndex] = index;
    }

    [[nodiscard]] std::optional<uint32_t> getIndex(dimType pos) const {
        uint32_t internalIndex = calculateIndex(pos);

        assert(internalIndex != vdbBadIndex);

        if (_data.bitset[internalIndex]) {
            return _data.pointer[internalIndex];
        }

        return std::nullopt;
    }

    [[nodiscard]] uint32_t calculateIndex(dimType pos) const {
        for (int i = 0; i < dimType::length(); i++) {
            if (pos[i] < 0 || pos[i] >= (1 << level)) {
                return vdbBadIndex;
            }
        }

        uint32_t index{};

        for (int i = 0; i < dimType::length(); i++) {
            index += pos[i] << (level * i);
        }

        return index;
    }

    void setBackgroundValue(const valueType &value) { backgroundValue = value; }

    const valueType &getBackgroundValue() const { return backgroundValue; }

    const dataStruct &getData() const { return _data; }

    [[nodiscard]] std::vector<uint32_t> getIndices() const {
        std::vector<uint32_t> indices;

        for (uint32_t i = 0; i < ARRAY_SIZE; i++) {
            if (_data.bitset[i]) {
                indices.push_back(_data.pointer[i]);
            }
        }

        return indices;
    }

    void pokeIndices(const std::vector<uint32_t> &newIndices) {
        auto iter = newIndices.begin();

        for (uint32_t i = 0; i < ARRAY_SIZE; i++) {
            if (_data.bitset[i]) {
                assert(iter != newIndices.end());
                _data.pointer[i] = *iter;
                ++iter;
            }
        }

        assert(iter == newIndices.end());
    }

private:
    dataStruct _data;

    dimType _lowDim;
    dimType _highDim;

    valueType backgroundValue{};
};

template<typename valueType, int rootLevel, int nodeLevel, int leafLevel>
class vdbDataset {
public:
    using leafType = vdbLeaf<valueType, leafLevel>;
    using nodeType = vdbNode<valueType, nodeLevel>;
    using rootType = vdbNode<valueType, rootLevel>;

    static constexpr int topLevel = rootLevel + nodeLevel + leafLevel;

    using accessorType = vdbAccessor<rootLevel, nodeLevel, leafLevel>;

    void setValue(dimType pos, const valueType &value) {
        accessorType accessor(pos);

        uint32_t rootIndex = _findRoot(accessor.rootPos);

        if (rootIndex == vdbBadIndex) {
            rootIndex = _createRoot(accessor.rootPos);
        }

        assert(rootIndex != vdbBadIndex);

        uint32_t nodeIndex = _findNode(accessor.nodePos, rootIndex);

        if (nodeIndex == vdbBadIndex) {
            nodeIndex = _createNode(accessor.nodePos, rootIndex);
        }

        assert(nodeIndex != vdbBadIndex);

        uint32_t leafIndex = _findLeaf(accessor.leafPos, nodeIndex);

        if (leafIndex == vdbBadIndex) {
            leafIndex = _createLeaf(accessor.leafPos, nodeIndex);
        }

        assert(leafIndex != vdbBadIndex);

        leaves[leafIndex].setValue(accessor.voxelPos, value);
    }

    valueType getValue(dimType pos) const {
        accessorType accessor(pos);

        uint32_t rootIndex = _findRoot(accessor.rootPos);

        if (rootIndex == vdbBadIndex) return backgroundValue;

        uint32_t nodeIndex = _findNode(accessor.nodePos, rootIndex);

        if (nodeIndex == vdbBadIndex) return roots[rootIndex].getBackgroundValue();

        uint32_t leafIndex = _findLeaf(accessor.leafPos, nodeIndex);

        if (leafIndex == vdbBadIndex) return nodes[rootIndex].getBackgroundValue();

        return leaves[leafIndex].getValue(accessor.voxelPos);
    }

private:
    [[nodiscard]] uint32_t _findRoot(dimType pos) const {
        for (uint32_t i = 0; i < roots.size(); i++) {
            auto rootPos = roots[i].getLowDim();

            if (rootPos == pos) {
                return i;
            }
        }

        return vdbBadIndex;
    }

    uint32_t _createRoot(dimType pos) {
        roots.emplace_back(pos, pos + 1);

        return roots.size() - 1;
    }

    [[nodiscard]] uint32_t _findNode(dimType pos, uint32_t index) const {
        assert(index < roots.size());

        const auto &root = roots[index];

        const auto optIndex = root.getIndex(pos);

        return optIndex ? *optIndex : vdbBadIndex;
    }

    uint32_t _createNode(dimType pos, uint32_t index) {
        nodes.emplace_back(pos, pos + 1);

        uint32_t newIndex = nodes.size() - 1;

        roots[index].setIndex(pos, newIndex);

        return newIndex;
    }

    [[nodiscard]] uint32_t _findLeaf(dimType pos, uint32_t index) const {
        assert(index < nodes.size());

        const auto &node = nodes[index];

        const auto optIndex = node.getIndex(pos);

        return optIndex ? *optIndex : vdbBadIndex;
    }

    uint32_t _createLeaf(dimType pos, uint32_t index) {
        leaves.emplace_back(pos, pos + 1);

        uint32_t newIndex = leaves.size() - 1;

        nodes[index].setIndex(pos, newIndex);

        return newIndex;
    }

public:
    [[nodiscard]] uint32_t countRoots() const {
        return roots.size();
    }

    [[nodiscard]] uint32_t countNodes() const {
        return nodes.size();
    }

    [[nodiscard]] uint32_t countLeaves() const {
        return leaves.size();
    }

    [[nodiscard]] uint32_t countVoxels() const {
        return leaves.size() * leafType::ARRAY_SIZE;
    }

    [[nodiscard]] uint32_t countActiveVoxels() const {
        uint32_t sum{};
        for (auto &leaf : leaves) {
            sum += leaf.getActiveVoxelsCount();
        }
        return sum;
    }

    [[nodiscard]] uint32_t countMemorySize() const {
        uint32_t memSize{};

        memSize += sizeof(*this);

        memSize += roots.size() * sizeof(rootType);
        memSize += nodes.size() * sizeof(nodeType);
        memSize += leaves.size() * sizeof(leafType);

        return memSize;
    }

    [[nodiscard]] dimType getLowDim() const {
        dimType dim = roots.front().getLowDim();

        for (auto &root : roots) {
            for (int i = 0; i < dimType::length(); i++) {
                dim[i] = std::min(dim[i], root.getLowDim()[i]);
            }
        }

        return dim * (1 << topLevel);
    }

    [[nodiscard]] dimType getHighDim() const {
        dimType dim = roots.front().getHighDim();

        for (auto &root : roots) {
            for (int i = 0; i < dimType::length(); i++) {
                dim[i] = std::max(dim[i], root.getHighDim()[i]);
            }
        }

        return dim * (1 << topLevel);
    }

    const auto &getRoots() const { return roots; }

    const auto &getNodes() const { return nodes; }

    const auto &getLeaves() const { return leaves; }

    void reorganizeData() {
        _reorganizeNodes();
        _reorganizeLeaves();

        roots.shrink_to_fit();
        nodes.shrink_to_fit();
        leaves.shrink_to_fit();
    }

private:
    void _reorganizeNodes() {
        std::vector<nodeType> newNodes;

        uint32_t index{};

        for (auto &root : roots) {
            std::vector<uint32_t> indices = root.getIndices();
            std::vector<uint32_t> newIndices(indices.size());

            for (auto &i : indices) {
                newNodes.push_back(nodes[i]);
            }

            for (auto &i : newIndices) {
                i = index++;
            }

            root.pokeIndices(newIndices);
        }

        nodes = newNodes;
    }

    void _reorganizeLeaves() {
        std::vector<leafType> newLeaves;

        uint32_t index{};

        for (auto &node : nodes) {
            std::vector<uint32_t> indices = node.getIndices();
            std::vector<uint32_t> newIndices(indices.size());

            for (auto &i : indices) {
                newLeaves.push_back(leaves[i]);
            }

            for (auto &i : newIndices) {
                i = index++;
            }

            node.pokeIndices(newIndices);
        }

        leaves = newLeaves;
    }

    std::vector<leafType> leaves;
    std::vector<nodeType> nodes;
    std::vector<rootType> roots;

    valueType backgroundValue{};
};

#endif //MB_CLOUDS_VDBDATA_H
