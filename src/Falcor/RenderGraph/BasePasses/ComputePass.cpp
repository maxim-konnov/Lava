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
#include "ComputePass.h"

namespace Falcor {

ComputePass::ComputePass(std::shared_ptr<Device> pDevice, const Program::Desc& desc, const Program::DefineList& defines, bool createVars): mpDevice(pDevice) {
    auto pProg = ComputeProgram::create(pDevice, desc, defines);
    mpState = ComputeState::create(pDevice);
    mpState->setProgram(pProg);
    if (createVars) mpVars = ComputeVars::create(pDevice, pProg.get());
    assert(pProg && mpState && (!createVars || mpVars));
}

ComputePass::SharedPtr ComputePass::create(std::shared_ptr<Device> pDevice, const std::string& filename, const std::string& csEntry, const Program::DefineList& defines, bool createVars) {
    assert(pDevice);
    Program::Desc d;
    d.addShaderLibrary(filename).csEntry(csEntry);
    return create(pDevice, d, defines, createVars);
}

ComputePass::SharedPtr ComputePass::create(std::shared_ptr<Device> pDevice, const Program::Desc& desc, const Program::DefineList& defines, bool createVars) {
    assert(pDevice);
    return SharedPtr(new ComputePass(pDevice, desc, defines, createVars));
}

void ComputePass::execute(ComputeContext* pContext, uint32_t nThreadX, uint32_t nThreadY, uint32_t nThreadZ) {
    LOG_DBG("ComputePass::execute");
    assert(pContext);
    uint3 threadGroupSize = mpState->getProgram()->getReflector()->getThreadGroupSize();
    uint3 groups = div_round_up(uint3(nThreadX, nThreadY, nThreadZ), threadGroupSize);
    pContext->dispatch(mpState.get(), mpVars.get(), groups);
}

void ComputePass::executeIndirect(ComputeContext* pContext, const Buffer* pArgBuffer, uint64_t argBufferOffset) {
    LOG_DBG("ComputePass::executeIndirect");
    assert(pContext);
    pContext->dispatchIndirect(mpState.get(), mpVars.get(), pArgBuffer, argBufferOffset);
}

void ComputePass::addDefine(const std::string& name, const std::string& value, bool updateVars) {
    mpState->getProgram()->addDefine(name, value);
    if (updateVars) mpVars = ComputeVars::create(mpDevice, mpState->getProgram().get());
}

void ComputePass::removeDefine(const std::string& name, bool updateVars) {
    mpState->getProgram()->removeDefine(name);
    if (updateVars) mpVars = ComputeVars::create(mpDevice, mpState->getProgram().get());
}

void ComputePass::setVars(const ComputeVars::SharedPtr& pVars) {
    mpVars = pVars ? pVars : ComputeVars::create(mpDevice, mpState->getProgram().get());
    assert(mpVars);
}

}  // namespace Falcor
