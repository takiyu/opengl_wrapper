#include <oglw/obj_loader.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

#include <cassert>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>

namespace oglw {

namespace {

// -----------------------------------------------------------------------------
template <typename T>
std::string EscapeDuplicatedKey(const std::string& name,
                                const std::map<std::string, T>& map) {
    // No duplication
    if (map.count(name) == 0) {
        return name;
    }

    // Add suffix
    size_t idx = 1;
    while (true) {
        std::stringstream ss;
        if (!name.empty()) {
            ss << name << "_";
        }
        ss << idx;
        const std::string& new_name = ss.str();
        if (map.count(new_name) == 0) {
            return new_name;
        }
    }
}

// -----------------------------------------------------------------------------
struct AttributeIndices {
    std::vector<unsigned int> vertex;
    std::vector<unsigned int> normal;
    std::vector<unsigned int> texcoord;
};

void LoadObj(const std::string& filename, std::vector<float>& vertices,
             std::vector<float>& normals, std::vector<float>& texcoords,
             std::map<std::string, AttributeIndices>& indices) {

    // Load with tinyobj
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                                filename.c_str(), nullptr, true, true);
    if (!ret) {
        throw std::runtime_error("Loading obj error: " + err);
    }

    // Set attribute outputs
    vertices = std::move(attrib.vertices);
    normals = std::move(attrib.normals);
    texcoords = std::move(attrib.texcoords);

    for (auto& shape : shapes) {
        // Repack indices
        AttributeIndices idxs;
        auto& s_idxs = shape.mesh.indices;
        idxs.vertex.resize(s_idxs.size());
        idxs.normal.resize(s_idxs.size());
        idxs.texcoord.resize(s_idxs.size());
        for (size_t i = 0; i < s_idxs.size(); i++) {
            auto& s_idx = s_idxs[i];
            idxs.vertex[i] = static_cast<unsigned int>(s_idx.vertex_index);
            idxs.normal[i] = static_cast<unsigned int>(s_idx.normal_index);
            idxs.texcoord[i] = static_cast<unsigned int>(s_idx.texcoord_index);
        }

        indices[EscapeDuplicatedKey(shape.name, indices)] = idxs;
    }
}

// -----------------------------------------------------------------------------
void ShiftVertices(std::vector<float>& vtxs,
                   const std::array<float, 3>& shift) {
    for (size_t v_idx = 0; v_idx < vtxs.size() / 3; v_idx++) {
        vtxs[3 * v_idx + 0] += shift[0];
        vtxs[3 * v_idx + 1] += shift[1];
        vtxs[3 * v_idx + 2] += shift[2];
    }
}

}  // namespace

// ================================= Obj Loader ================================
void LoadObj(const std::string& filename, std::map<std::string, GeometryPtr>& geoms,
             ObjLoaderMode mode, const std::array<float, 3>& shift) {
    // Load basic informations
    std::vector<float> vertices, normals, texcoords;
    std::map<std::string, AttributeIndices> indices;
    LoadObj(filename, vertices, normals, texcoords, indices);

    // Apply shift
    ShiftVertices(vertices, shift);

    const size_t n_vtxs = vertices.size() / 3;

    if (mode == ObjLoaderMode::INDEXING) {
        // Indexing
        throw std::runtime_error("Not implemented");

    } else if (mode == ObjLoaderMode::INDEXING_VTX_ONLY) {
        // Indexing with only vertices
        auto vertex_buf = GpuArrayBuffer<float>::Create(n_vtxs, 3);
        vertex_buf->sendData(vertices.data());

        for (auto& v : indices) {
            // Create index buffer
            auto vtx_idxs = v.second.vertex;
            auto index_buf = GpuIndexBuffer::Create(vtx_idxs.size(), 1);
            index_buf->sendData(vtx_idxs.data());

            // Create Geometry
            auto geom = Geometry::Create();
            geom->setArrayBuffer(vertex_buf, 0);
            geom->setIndexBuffer(index_buf);

            // Register
            geoms[v.first] = geom;
        }
    } else if (mode == ObjLoaderMode::NO_INDICING) {
        // No indexing
        throw std::runtime_error("Not implemented");
    } else {
        throw std::runtime_error("Invalid Obj loader mode");
    }
}

// -----------------------------------------------------------------------------

}  // namespace oglw
