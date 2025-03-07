/***************************************************************************
 # Copyright (c) 2020, NVIDIA CORPORATION. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions
 # are met:
 #  * Redistributions of source code must retain the above copyright
 #    notice, this list of conditions and the following disclaimer.
 #  * Redistributions in binary form must reproduce the above copyright
 #    notice, this list of conditions and the following disclaimer in the
 #    documentation and/or other materials provided with the distribution.
 #  * Neither the name of NVIDIA CORPORATION nor the names of its
 #    contributors may be used to endorse or promote products derived
 #    from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 # EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 # PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 # CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 # PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 # PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 # OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 # OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************/
#include <array>
#include <thread>

#include "Falcor/Core/API/Texture.h"
#include "Falcor/Scene/Lights/LightProbe.h"

#include "scene_builder.h"
#include "lava_utils_lib/logging.h"

#include "reader_bgeo/bgeo/Run.h"
#include "reader_bgeo/bgeo/Poly.h"
#include "reader_bgeo/bgeo/PrimType.h"
#include "reader_bgeo/bgeo/parser/Detail.h"

namespace lava {    

SceneBuilder::SceneBuilder(Falcor::Device::SharedPtr pDevice, Flags buildFlags): Falcor::SceneBuilder(pDevice, buildFlags), mUniqueTrianglesCount(0) {
    mpDefaultMaterial = Material::create(pDevice, "default");
    mpDefaultMaterial->setBaseColor({0.2, 0.2, 0.2, 1.0});
    mpDefaultMaterial->setRoughness(0.3);
    mpDefaultMaterial->setIndexOfRefraction(1.5);
    mpDefaultMaterial->setEmissiveFactor(0.0);
}

SceneBuilder::~SceneBuilder() {
    std::cout << "SceneBuilder stats:" << std::endl;
    std::cout << "\t Triangles count: " << std::to_string(mUniqueTrianglesCount);
    std::cout << std::endl << std::endl;
}

SceneBuilder::SharedPtr SceneBuilder::create(Falcor::Device::SharedPtr pDevice, Flags buildFlags) {
    return SharedPtr(new SceneBuilder(pDevice, buildFlags));
}

Falcor::Scene::SharedPtr SceneBuilder::getScene() {
    return Falcor::SceneBuilder::getScene();
}

uint32_t SceneBuilder::addGeometry(ika::bgeo::Bgeo::SharedConstPtr pBgeo, const std::string& name) {
    assert(pBgeo);

    const int64_t bgeo_point_count = pBgeo->getPointCount();
    const int64_t bgeo_vertex_count = pBgeo->getTotalVertexCount();

    LLOG_DBG << "bgeo point count: " << bgeo_point_count;
    LLOG_DBG << "bgeo total vertex count: " << bgeo_vertex_count;
    LLOG_DBG << "bgeo prim count: " << pBgeo->getPrimitiveCount();
    LLOG_DBG << "------------------------------------------------";

    // get basic bgeo data P, N, UV

    std::vector<float> P;
    pBgeo->getP(P);
    assert(P.size() / 3 == bgeo_point_count && "P positions count not equal to the bgeo points count !!!");
    LLOG_DBG << "P<float> size: " << P.size();

    std::vector<float> N;
    pBgeo->getPointN(N);
    LLOG_DBG << "N<float> size: " << N.size();
    
    std::vector<float> UV;
    pBgeo->getPointUV(UV);
    LLOG_DBG << "UV<float> size: " << UV.size();

    std::vector<float> vN;
    pBgeo->getVertexN(vN);
    LLOG_DBG << "vN<float> size: " << vN.size();
    
    std::vector<float> vUV;
    pBgeo->getVertexUV(vUV);
    LLOG_DBG << "vUV<float> size: " << vUV.size();

    bool unique_points = false; // separate points only if we have any vertex data present
    if(vN.size() || vUV.size()) unique_points = true; 

    // separated verices data arrays
    std::vector<float3> positions;
    std::vector<float3> normals;
    std::vector<float2> uv_coords;
    
    // build separated verices data
    if(unique_points) {
        auto pDetail = pBgeo->getDetail();
        auto const& vt_map = pDetail->getVertexMap();

        assert(vt_map.vertexCount == bgeo_vertex_count && "Bgeo detail vertices count not equal to the number of bgeo vertices count !!!");

        ika::bgeo::parser::int32 point_idx;
        const ika::bgeo::parser::int32* vt_idx_ptr = vt_map.getVertices();
        
        if( unique_points) {
            positions.resize(bgeo_vertex_count);
            // fill in vertex positions
            for( ika::bgeo::parser::int64 i = 0; i < vt_map.getVertexCount(); i++){
                point_idx = vt_idx_ptr[i] * 3;
                positions[i] = {P[point_idx], P[point_idx+1], P[point_idx+2]};
            }
        }

        // fill in normals
        if (vN.size() || N.size()) normals.resize(bgeo_vertex_count);
        if (vN.size()) {
            size_t ii = 0;
            assert(vN.size() / 3 == bgeo_vertex_count && "Vertex normals count not equal to the number of bgeo verices count !!!");
            // use vertex normals
            for( size_t i = 0; i < bgeo_vertex_count; i++){
                normals[i] = {vN[ii], vN[ii+1], vN[ii+2]};
                ii += 3;
            }
        } else if (N.size() && unique_points) {
            // use point normals
            assert(N.size() == P.size() && "Point normals count not equal to the number of bgeo points !!!");

            for( ika::bgeo::parser::int64 i = 0; i < vt_map.getVertexCount(); i++){
                point_idx = vt_idx_ptr[i] * 3;
                normals[i] = {N[point_idx], N[point_idx+1], N[point_idx+2]};
            }
        }

        // fill in texture coordinates
        if (vUV.size() || UV.size()) uv_coords.resize(bgeo_vertex_count);
        if (vUV.size()) {
            size_t ii = 0;
            assert(vUV.size()/2 == bgeo_vertex_count && "Vertex texture coordinates count not equal to the number of bgeo verices count !!!");
            // use vertex normals
            for( size_t i = 0; i < bgeo_vertex_count; i++){
                uv_coords[i] = {vUV[ii], 1.0 - vUV[ii+1]};
                ii += 2;
            }
        } else if (UV.size() && unique_points) {
            // use point normals
            assert(UV.size()/2 == P.size()/3 && "Point texture coordinates count not equal to the number of bgeo points !!!");

            for( ika::bgeo::parser::int64 i = 0; i < vt_map.getVertexCount(); i++){
                point_idx = vt_idx_ptr[i]*2;
                uv_coords[i] = {UV[point_idx], 1.0 - UV[point_idx+1]};
            }
        } else {
            // no coords provided from bgeo. fill with zeroes as this field is required
            for(auto& uv: uv_coords) {
                uv = {0.0, 0.0};
            }
        }
    }

    // process primitives and build indices

    uint32_t face_count = 0;
    std::vector<uint32_t> indices;
    std::vector<int32_t> vtx_list;
    std::vector<int32_t> prim_start_indices;

    for(uint32_t p_i=0; p_i < pBgeo->getPrimitiveCount(); p_i++) {
        std::cout << "Processing primitive number " << p_i << "\n";

        const auto& pPrim = pBgeo->getPrimitive(p_i);
        if(!pPrim) {
            LLOG_WRN << "Unable to get primitive number: " << p_i;
            continue;
        }

        const ika::bgeo::Poly* pPoly;
        prim_start_indices.clear();

        switch (pPrim->getType()) {
            case ika::bgeo::PrimType::PolyPrimType:
                pPoly = std::dynamic_pointer_cast<ika::bgeo::Poly>(pPrim).get();

                if (unique_points) {
                    
                    pPoly->getStartIndices(prim_start_indices);

                    // process faces
                    int32_t csi; // face current start index
                    int32_t nsi; // next face start index
                    for( int32_t i = 0; i < (prim_start_indices.size() - 1); i++){
                        csi = prim_start_indices[i];
                        nsi = prim_start_indices[i+1];
                        switch(nsi-csi) { // number of face sides literally
                            case 0:
                            case 1:
                            case 2:
                                LLOG_ERR << "Polygon sides count should be 3 or more !!!";
                                break;
                            case 3:
                                indices.push_back(csi+2);
                                indices.push_back(csi+1);
                                indices.push_back(csi);
                                face_count += 1;
                                break;
                            case 4:
                                indices.push_back(csi+2);
                                indices.push_back(csi+1);
                                indices.push_back(csi);

                                indices.push_back(csi+3);
                                indices.push_back(csi+2);
                                indices.push_back(csi);
                                face_count += 2;
                                break;
                            case 5:
                                indices.push_back(csi+2);
                                indices.push_back(csi+1);
                                indices.push_back(csi);

                                indices.push_back(csi+3);
                                indices.push_back(csi+2);
                                indices.push_back(csi);

                                indices.push_back(csi+4);
                                indices.push_back(csi+3);
                                indices.push_back(csi);
                                face_count += 3;
                                break;
                            default:
                                LLOG_WRN << "Poly sides more than 5 unsupported for now !";
                                break;
                        }
                    }
                    LLOG_DBG << "prim vertex count: " << pPoly->getVertexCount();
                    LLOG_DBG << "prim faces count: " << pPoly->getFaceCount();
                } else {
                    const auto& sides_list = pPoly->getSidesList();

                    // process faces
                    uint64_t cur_i = 0;
                    for(const auto& sides: sides_list){
                        switch(sides) {
                            case 0:
                            case 1:
                            case 2:
                                LLOG_ERR << "Polygon sides count should be 3 or more !!!";
                                break;
                            case 3:
                                indices.push_back(vtx_list[cur_i+2]);
                                indices.push_back(vtx_list[cur_i+1]);
                                indices.push_back(vtx_list[cur_i]);
                                face_count += 1;
                                break;
                            case 4:
                                indices.push_back(vtx_list[cur_i+2]);
                                indices.push_back(vtx_list[cur_i+1]);
                                indices.push_back(vtx_list[cur_i]);

                                indices.push_back(vtx_list[cur_i+3]);
                                indices.push_back(vtx_list[cur_i+2]);
                                indices.push_back(vtx_list[cur_i]);
                                face_count += 2;
                                break;
                            case 5:
                                indices.push_back(vtx_list[cur_i+2]);
                                indices.push_back(vtx_list[cur_i+1]);
                                indices.push_back(vtx_list[cur_i]);

                                indices.push_back(vtx_list[cur_i+3]);
                                indices.push_back(vtx_list[cur_i+2]);
                                indices.push_back(vtx_list[cur_i]);

                                indices.push_back(vtx_list[cur_i+4]);
                                indices.push_back(vtx_list[cur_i+3]);
                                indices.push_back(vtx_list[cur_i]);
                                face_count += 3;
                                break;
                            default:
                                LLOG_WRN << "Poly sides more than 5 unsupported for now !";
                                break;
                        }

                        cur_i += sides;
                    }
                }

                break;
            default:
                LLOG_WRN << "Unsupported prim type \"" + std::string(pPrim->getStrType()) + "\" !!!";
                break;
        }
    }

    Mesh mesh;
    mesh.faceCount = face_count;

    mesh.positions.frequency = Mesh::AttributeFrequency::Vertex;
    if(unique_points) {
        mesh.vertexCount = positions.size();
        mesh.positions.pData = (float3*)positions.data();
    } else {
        mesh.vertexCount = P.size() / 3;
        mesh.positions.pData = (float3*)P.data();
    }

    mesh.indexCount = indices.size();
    mesh.pIndices = (uint32_t*)indices.data();
    
    mesh.normals.frequency = Mesh::AttributeFrequency::Vertex;
    if(unique_points) {
        mesh.normals.pData = (float3*)normals.data();
    } else {
        mesh.normals.pData = (float3*)N.data();
    }

    mesh.texCrds.frequency = Mesh::AttributeFrequency::Vertex;
    if(unique_points) {
        mesh.texCrds.pData = (float2*)uv_coords.data();
    } else {
        mesh.texCrds.pData = (float2*)UV.data();
    }

    //mesh.pBoneIDs = nullptr;
    //mesh.pBoneWeights = nullptr;
    mesh.topology = Falcor::Vao::Topology::TriangleList;
    mesh.name = name;
    mesh.pMaterial = mpDefaultMaterial;

    mUniqueTrianglesCount += face_count;

    return Falcor::SceneBuilder::addMesh(mesh);
}

std::shared_future<uint32_t> SceneBuilder::addGeometryAsync(ika::bgeo::Bgeo::SharedConstPtr pBgeo, const std::string& name) {
    assert(pBgeo);

    std::packaged_task<uint32_t(ika::bgeo::Bgeo::SharedConstPtr, const std::string&)> task([this](ika::bgeo::Bgeo::SharedConstPtr p, const std::string& n) {
        return this->addGeometry(p, n); 
    });

    std::shared_future<uint32_t> result = task.get_future().share();

    // Pass the packaged_task to thread to run asynchronously
    std::thread(std::move(task), pBgeo, name).detach();
    return result;
}

void SceneBuilder::finalize() {
    mDirty = true;
    getScene();
}


}  // namespace lava
