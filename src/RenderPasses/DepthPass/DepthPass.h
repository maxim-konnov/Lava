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
#ifndef SRC_FALCOR_RENDERPASSES_DEPTHPASS_DEPTHPASS_H_
#define SRC_FALCOR_RENDERPASSES_DEPTHPASS_DEPTHPASS_H_

#include "Falcor.h"
#include "FalcorExperimental.h"
#include "Falcor/Core/API/Device.h"

using namespace Falcor;

#ifdef BUILD_DEPTH_PASS
// #define dllpassdecl __declspec(dllexport)
#define dllpassdecl falcorexport
#else
// #define dllpassdecl __declspec(dllimport)
#define dllpassdecl falcorimport
#endif

class dllpassdecl DepthPass : public RenderPass, public inherit_shared_from_this<RenderPass, DepthPass> {
 public:
    using SharedPtr = std::shared_ptr<DepthPass>;
    using inherit_shared_from_this<RenderPass, DepthPass>::shared_from_this;
    static const char* kDesc;

    /** Create a new object
    */
    static SharedPtr create(RenderContext* pRenderContext = nullptr, const Dictionary& dict = {});

    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void execute(RenderContext* pContext, const RenderData& renderData) override;
    virtual void setScene(RenderContext* pRenderContext, const Scene::SharedPtr& pScene) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual Dictionary getScriptingDictionary() override;
    virtual std::string getDesc() override { return kDesc; };

    DepthPass& setDepthBufferFormat(ResourceFormat format);
    DepthPass& setDepthStencilState(const DepthStencilState::SharedPtr& pDsState);
    DepthPass& setRasterizerState(const RasterizerState::SharedPtr& pRsState);

 private:
    DepthPass(Device::SharedPtr pDevice, const Dictionary& dict);
    void parseDictionary(const Dictionary& dict);

    Fbo::SharedPtr mpFbo;
    GraphicsState::SharedPtr mpState;
    GraphicsVars::SharedPtr mpVars;
    RasterizerState::SharedPtr mpRsState;
    ResourceFormat mDepthFormat = ResourceFormat::D32Float;
    Scene::SharedPtr mpScene;
};

#endif  // SRC_FALCOR_RENDERPASSES_DEPTHPASS_DEPTHPASS_H_
