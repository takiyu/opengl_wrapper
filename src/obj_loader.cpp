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
void AccumulateNormalCounts(const std::vector<float>& vertices,
                            const std::vector<unsigned int>& indices,
                            std::vector<float>& normals,
                            std::vector<int>& normal_cnts) {
    for (size_t f_idx = 0; f_idx < indices.size() / 3; f_idx++) {
        const unsigned int v_idxs[3] = {indices[3 * f_idx + 0],
                                        indices[3 * f_idx + 1],
                                        indices[3 * f_idx + 2]};

        float vtxs[3][3];
        for (size_t i = 0; i < 3; i++) {
            for (size_t c_idx = 0; c_idx < 3; c_idx++) {
                vtxs[i][c_idx] = vertices[3 * v_idxs[i] + c_idx];
            }
        }

        float e0[3], e1[3];
        for (size_t c_idx = 0; c_idx < 3; c_idx++) {
            e0[c_idx] = vtxs[1][c_idx] - vtxs[0][c_idx];
            e1[c_idx] = vtxs[2][c_idx] - vtxs[0][c_idx];
        }

        float n[3] = {e0[1] * e1[2] - e0[2] * e1[1],
                      e0[2] * e1[0] - e0[0] * e1[2],
                      e0[0] * e1[1] - e0[1] * e1[0]};
        const float n_norm = std::sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
        for (size_t c_idx = 0; c_idx < 3; c_idx++) {
            n[c_idx] /= n_norm;
        }

        for (size_t i = 0; i < 3; i++) {
            normal_cnts[v_idxs[i]] += 1;
            for (size_t c_idx = 0; c_idx < 3; c_idx++) {
                normals[3 * v_idxs[i] + c_idx] += n[c_idx];
            }
        }
    }
}

void ComputeGeometricNormals(
        const std::vector<float>& vertices,
        const std::map<std::string, AttributeIndices>& indices,
        std::vector<float>& normals) {

    // Initialize
    normals.resize(vertices.size(), 0.f);
    std::vector<int> normal_cnts(vertices.size() / 3, 0);

    // Accumulate normals over shapes
    for (auto&& v : indices) {
        AccumulateNormalCounts(vertices, v.second.vertex, normals, normal_cnts);
    }

    // Normalize normals
    for (size_t i = 0; i < normals.size() / 3; i++) {
        if (normal_cnts[i] == 0) {
            continue;
        }
        // Average
        normals[3 * i + 0] /= static_cast<float>(normal_cnts[i]);
        normals[3 * i + 1] /= static_cast<float>(normal_cnts[i]);
        normals[3 * i + 2] /= static_cast<float>(normal_cnts[i]);

        // Normalize
        float n_norm = 0.f;
        for (size_t c_idx = 0; c_idx < 3; c_idx++) {
            const float v = normals[3 * i + c_idx];
            n_norm += v * v;
        }
        n_norm = std::sqrt(n_norm);
        for (size_t c_idx = 0; c_idx < 3; c_idx++) {
            normals[3 * i + c_idx] /= n_norm;
        }
    }
}

void CreateGeometryIndexingVtxOnly(
        const std::vector<float>& vertices,
        const std::map<std::string, AttributeIndices>& indices,
        std::map<std::string, GeometryPtr>& geoms) {
    const size_t n_vtxs = vertices.size() / 3;

    // Vertex buffer
    auto vertex_buf = GpuArrayBuffer<float>::Create(n_vtxs, 3);
    vertex_buf->sendData(vertices.data());

    // Geometric normals
    std::vector<float> normals;
    ComputeGeometricNormals(vertices, indices, normals);

    // Normal buffer
    auto normal_buf = GpuArrayBuffer<float>::Create(n_vtxs, 3);
    normal_buf->sendData(normals.data());

    for (auto& v : indices) {
        // Create index buffer
        auto vtx_idxs = v.second.vertex;
        auto index_buf = GpuIndexBuffer::Create(vtx_idxs.size(), 1);
        index_buf->sendData(vtx_idxs.data());

        // Create Geometry (no texcoords)
        auto geom = Geometry::Create();
        geom->setArrayBuffer(vertex_buf, 0);
        geom->setArrayBuffer(normal_buf, 1);
        geom->setIndexBuffer(index_buf);

        // Register
        geoms[v.first] = geom;
    }
}

// -----------------------------------------------------------------------------
void CreateGeometryNoIndexing(
        const std::vector<float>& vertices,
        const std::vector<float>& normals,
        const std::vector<float>& texcoords,
        const std::map<std::string, AttributeIndices>& indices,
        std::map<std::string, GeometryPtr>& geoms) {
    throw std::runtime_error("Not implemented");
}

// -----------------------------------------------------------------------------
void ShiftVertices(std::vector<float>& vtxs,
                   const Vec3& shift) {
    for (size_t v_idx = 0; v_idx < vtxs.size() / 3; v_idx++) {
        vtxs[3 * v_idx + 0] += shift(0);
        vtxs[3 * v_idx + 1] += shift(1);
        vtxs[3 * v_idx + 2] += shift(2);
    }
}

}  // namespace

// ================================= Obj Loader ================================
void LoadObj(const std::string& filename,
             std::map<std::string, GeometryPtr>& geoms, ObjLoaderMode mode,
             const Vec3& shift) {
    // Load basic informations
    std::vector<float> vertices, normals, texcoords;
    std::map<std::string, AttributeIndices> indices;
    LoadObj(filename, vertices, normals, texcoords, indices);

    // Apply shift
    ShiftVertices(vertices, shift);

    if (mode == ObjLoaderMode::INDEXING) {
        // Indexing
        throw std::runtime_error("Not implemented");
    } else if (mode == ObjLoaderMode::INDEXING_VTX_ONLY) {
        // Indexing with only vertices
        CreateGeometryIndexingVtxOnly(vertices, indices, geoms);
    } else if (mode == ObjLoaderMode::NO_INDICING) {
        // No indexing
        CreateGeometryNoIndexing(vertices, normals, texcoords, indices, geoms);
    } else {
        throw std::runtime_error("Invalid Obj loader mode");
    }

    // Set primitive as triangle
    for (auto& v : geoms) {
        v.second->setPrimitive(oglw::PrimitiveType::TRIANGLE);
    }
}

// -----------------------------------------------------------------------------

}  // namespace oglw
