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
#include "Falcor/stdafx.h"
#include "BlendState.h"
#include "FBO.h"

namespace Falcor {

BlendState::SharedPtr BlendState::create(const Desc& desc) {
    return SharedPtr(new BlendState(desc));
}

BlendState::Desc::Desc(std::shared_ptr<Device> device): mpDevice(device) {
    mRtDesc.resize(Fbo::getMaxColorTargetCount(device));
}

BlendState::~BlendState() = default;

BlendState::Desc& BlendState::Desc::setRtParams(uint32_t rtIndex, BlendOp rgbOp, BlendOp alphaOp, BlendFunc srcRgbFunc, BlendFunc dstRgbFunc, BlendFunc srcAlphaFunc, BlendFunc dstAlphaFunc) {
    if(rtIndex >= mRtDesc.size()) {
        logError("Error when setting blend state RT parameters. Invalid render-target index " + std::to_string(rtIndex) + ". Must be smaller than " + std::to_string(mRtDesc.size()) + ".");
        return *this;
    }
    mRtDesc[rtIndex].rgbBlendOp = rgbOp;
    mRtDesc[rtIndex].alphaBlendOp = alphaOp;
    mRtDesc[rtIndex].srcRgbFunc = srcRgbFunc;
    mRtDesc[rtIndex].dstRgbFunc = dstRgbFunc;
    mRtDesc[rtIndex].srcAlphaFunc = srcAlphaFunc;
    mRtDesc[rtIndex].dstAlphaFunc = dstAlphaFunc;
    return *this;
}

BlendState::Desc& BlendState::Desc::setRenderTargetWriteMask(uint32_t rtIndex, bool writeRed, bool writeGreen, bool writeBlue, bool writeAlpha) {
    if(rtIndex >= mRtDesc.size()) {
        logError("Error when setting blend state RT write-mask. Invalid render-target index " + std::to_string(rtIndex) + ". Must be smaller than " + std::to_string(mRtDesc.size()) + ".");
        return *this;
    }
    mRtDesc[rtIndex].writeMask.writeRed = writeRed;
    mRtDesc[rtIndex].writeMask.writeGreen = writeGreen;
    mRtDesc[rtIndex].writeMask.writeBlue = writeBlue;
    mRtDesc[rtIndex].writeMask.writeAlpha = writeAlpha;
    return *this;
}

#ifdef SCRIPTING
SCRIPT_BINDING(BlendState) {
    pybind11::class_<BlendState, BlendState::SharedPtr>(m, "BlendState");
}
#endif

}   // namespace Falcor
