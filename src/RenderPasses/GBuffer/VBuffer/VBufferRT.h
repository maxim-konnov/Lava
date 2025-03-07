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
#pragma once

#ifdef FALCOR_D3D

#include "../GBufferBase.h"
#include "Utils/Sampling/SampleGenerator.h"

using namespace Falcor;

/** Ray traced V-buffer pass.

    This pass renders a visibility buffer using ray tracing.
    The visibility buffer encodes the mesh instance ID and primitive index,
    as well as the barycentrics at the hit point.
*/
class VBufferRT : public GBufferBase
{
public:
    using SharedPtr = std::shared_ptr<VBufferRT>;

    static SharedPtr create(RenderContext* pRenderContext, const Dictionary& dict);

    RenderPassReflection reflect(const CompileData& compileData) override;
    void setScene(RenderContext* pRenderContext, const Scene::SharedPtr& pScene) override;
    void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    std::string getDesc(void) override { return kDesc; }

private:
    VBufferRT(const Dictionary& dict);

    // Internal state
    SampleGenerator::SharedPtr mpSampleGenerator;
    uint32_t mFrameCount = 0;

    struct
    {
        RtProgram::SharedPtr pProgram;
        RtProgramVars::SharedPtr pVars;
    } mRaytrace;

    static const char* kDesc;
    friend void getPasses(Falcor::RenderPassLibrary& lib);
};

#endif  // FALCOR_D3D
