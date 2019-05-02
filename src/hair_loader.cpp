#include <oglw/hair_loader.h>

#include <cassert>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>

namespace oglw {

namespace {

using Strand = std::vector<Vec3>;

// -----------------------------------------------------------------------------

void LoadHairBinary(const std::string& filename, std::vector<Strand>& strands) {
    FILE* f = fopen(filename.c_str(), "rb");
    if (!f) {
        throw std::runtime_error("Hair error: Couldn't open : " + filename);
    }

    int nstrands = 0;
    if (!fread(&nstrands, 4, 1, f)) {
        fclose(f);
        throw std::runtime_error("Hair error: Invalid format (n_strands) : " +
                                 filename);
    }
    strands.resize(static_cast<size_t>(nstrands));

    for (size_t i = 0; i < static_cast<size_t>(nstrands); i++) {
        int nverts = 0;
        if (!fread(&nverts, 4, 1, f)) {
            fclose(f);
            throw std::runtime_error("Hair error: Invalid format (n_vtxs) : " +
                                     filename);
        }
        strands[i].resize(static_cast<size_t>(nverts));

        for (size_t j = 0; j < static_cast<size_t>(nverts); j++) {
            if (!fread(&strands[i][j][0], 12, 1, f)) {
                fclose(f);
                throw std::runtime_error("Hair error: Invalid format (vtx) : " +
                                         filename);
            }
        }
    }

    fclose(f);
}

// -----------------------------------------------------------------------------
void ComputeTangents(const std::vector<Strand>& strands,
                     std::vector<Strand>& strand_tans) {
    strand_tans.resize(strands.size());
    for (size_t s_idx = 0; s_idx < strands.size(); s_idx++) {
        const Strand& strand = strands[s_idx];
        Strand& strand_tan = strand_tans[s_idx];
        strand_tan.resize(strand.size());
        for (size_t v_idx = 0; v_idx < strand.size(); v_idx++) {
            Vec3 tan_sum(0.f, 0.f, 0.f);
            if (0 < v_idx) {
                tan_sum += (strand[v_idx] - strand[v_idx - 1]).normalized();
            }
            if (v_idx < strand.size() - 1) {
                tan_sum += (strand[v_idx + 1] - strand[v_idx]).normalized();
            }
            strand_tan[v_idx] = tan_sum.normalized();
        }
    }
}

// -----------------------------------------------------------------------------
void FlattenStrands(const std::vector<Strand>& strands,
                    std::vector<float>& vtxs) {
    // Count the number of line segments
    size_t n_seg = 0;
    for (size_t s_idx = 0; s_idx < strands.size(); s_idx++) {
        const auto& strand = strands[s_idx];
        if (strand.size() == 0) {
            continue;
        }
        n_seg += strand.size() - 1;
    }

    // Flatten
    vtxs.resize(n_seg * 2 * 3);
    size_t seg_idx = 0;
    for (size_t s_idx = 0; s_idx < strands.size(); s_idx++) {
        const auto& strand = strands[s_idx];
        for (size_t v_idx = 0; v_idx < strand.size() - 1; v_idx++) {
            for (size_t c_idx = 0; c_idx < 3; c_idx++) {
                vtxs[(seg_idx * 2 + 0) * 3 + c_idx] =
                    strand[v_idx + 0](static_cast<int>(c_idx));
                vtxs[(seg_idx * 2 + 1) * 3 + c_idx] =
                    strand[v_idx + 1](static_cast<int>(c_idx));
            }
            seg_idx += 1;
        }
    }
}

// -----------------------------------------------------------------------------
void ShiftVertices(std::vector<Strand>& strands, const Vec3& shift) {
    for (size_t s_idx = 0; s_idx < strands.size(); s_idx++) {
        Strand& strand = strands[s_idx];
        for (size_t v_idx = 0; v_idx < strand.size(); v_idx++) {
            strand[v_idx] += shift;
        }
    }
}

}  // namespace

// ================================= Hair Loader ===============================
GeometryPtr LoadHair(const std::string& filename, const Vec3& shift) {
    // Load basic informations
    std::vector<Strand> strands;
    LoadHairBinary(filename, strands);

    // Apply shift
    ShiftVertices(strands, shift);

    // Compute tangents
    std::vector<Strand> strand_tans;
    ComputeTangents(strands, strand_tans);

    // Flatten
    std::vector<float> vertices, tangents;
    FlattenStrands(strands, vertices);
    FlattenStrands(strand_tans, tangents);

    // Parse to geometry
    auto vertex_buf = GpuArrayBuffer<float>::Create(vertices.size() / 3, 3);
    vertex_buf->sendData(vertices.data());
    auto tangent_buf = GpuArrayBuffer<float>::Create(vertices.size() / 3, 3);
    tangent_buf->sendData(tangents.data());
    auto geom = Geometry::Create();
    geom->setArrayBuffer(vertex_buf, 0);
    geom->setArrayBuffer(tangent_buf, 1);

    // Set primitive as line
    geom->setPrimitive(oglw::PrimitiveType::LINE, 1.f);

    return geom;
}

// -----------------------------------------------------------------------------

}  // namespace oglw
