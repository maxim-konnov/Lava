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
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY
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
#ifndef SRC_FALCOR_RENDERPASSES_GBUFFER_GBUFFERBASE_H_
#define SRC_FALCOR_RENDERPASSES_GBUFFER_GBUFFERBASE_H_

#include "Falcor/Falcor.h"

using namespace Falcor;

extern "C" falcorexport void getPasses(Falcor::RenderPassLibrary& lib);

/** Base class for the different types of G-buffer passes (including V-buffer).
*/
class GBufferBase : public RenderPass {
 public:
    enum class SamplePattern : uint32_t {
        Center,
        DirectX,
        Halton,
        Stratified,
    };

    virtual void renderUI(Gui::Widgets& widget) override;
    virtual void compile(RenderContext* pContext, const CompileData& compileData) override;
    virtual void resolvePerFrameSparseResources(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual Dictionary getScriptingDictionary() override;
    virtual void setScene(RenderContext* pRenderContext, const Scene::SharedPtr& pScene) override;

 private:
    void updateFlags(const RenderData& renderData);

 protected:
    GBufferBase(Device::SharedPtr pDevice);
    virtual void parseDictionary(const Dictionary& dict);
    void updateSamplePattern();

    // Internal state
    Scene::SharedPtr                mpScene;
    CPUSampleGenerator::SharedPtr   mpSampleGenerator;

    uint2                           mFrameDim = {};
    float2                          mInvFrameDim = {};

    // UI variables
    SamplePattern                   mSamplePattern = SamplePattern::Stratified; ///< Which camera jitter sample pattern to use.
    uint32_t                        mSampleCount = 1024;                        ///< Sample count for camera jitter.
    bool                            mDisableAlphaTest = false;                  ///< Disable alpha test.
    bool                            mOptionsChanged = false;

    static void registerBindings(pybind11::module& m);
    friend void getPasses(Falcor::RenderPassLibrary& lib);
};

#define str(a) case GBufferBase::SamplePattern::a: return #a
inline std::string to_string(GBufferBase::SamplePattern type) {
    switch (type) {
        str(Center);
        str(DirectX);
        str(Halton);
        str(Stratified);
        default:
            should_not_get_here();
            return "";
    }
}
#undef str

#endif  // SRC_FALCOR_RENDERPASSES_GBUFFER_GBUFFERBASE_H_
