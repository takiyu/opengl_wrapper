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

}  // namespace

// ================================= Obj Loader ================================
void LoadObj(const std::string& filename, std::vector<GeometryPtr>& geoms) {
    geoms.clear();

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

    // Create attribute buffers
    auto vertex_buf = GpuArrayBuffer<float>::Create(attrib.vertices.size() / 3, 3);
    auto& vertices = attrib.vertices;
    float x_avg = 0.f;
    float y_avg = 0.f;
    float z_avg = 0.f;
    for (size_t i = 0; i < vertices.size() / 3; i++) {
        x_avg += vertices[3 * i + 0];
        y_avg += vertices[3 * i + 1];
        z_avg += vertices[3 * i + 2];
    }
    x_avg /= vertices.size() / 3;
    y_avg /= vertices.size() / 3;
    z_avg /= vertices.size() / 3;
    for (size_t i = 0; i < vertices.size() / 3; i++) {
        vertices[3 * i + 0] -= x_avg;
        vertices[3 * i + 1] -= y_avg;
//         vertices[3 * i + 2] -= z_avg;
    }
//     GpuArrayBuffer<float> normal_buf(attrib.normals.size() / 3, 3);
//     GpuArrayBuffer<float> texcoord_buf(attrib.texcoords.size() / 2, 2);

    vertex_buf->sendData(attrib.vertices.data());

    // Create geometries
    for (auto& shape : shapes) {
        // Parse indices
        auto tinyobj_indices = shape.mesh.indices;
        std::vector<unsigned int> indices(tinyobj_indices.size());
        for (size_t i = 0; i < tinyobj_indices.size(); i++) {
            indices[i] = static_cast<unsigned int>(tinyobj_indices[i].vertex_index);
        }

        auto index_buf = GpuIndexBuffer::Create(indices.size(), 1);
        index_buf->sendData(indices.data());

        // Parse to geometry
        auto geom = Geometry::Create();
        geom->setArrayBuffer(vertex_buf, 0);
        geom->setIndexBuffer(index_buf);
        geoms.push_back(geom);
    }
}

// -----------------------------------------------------------------------------

}  // namespace oglw
