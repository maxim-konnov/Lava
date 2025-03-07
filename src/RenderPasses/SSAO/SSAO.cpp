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
#include "SSAO.h"
#include "glm/gtc/random.hpp"

// Don't remove this. it's required for hot-reload to function properly
extern "C" falcorexport const char* getProjDir() {
    return PROJECT_DIR;
}

static void regSSAO(ScriptBindings::Module& m) {
    auto c = m.regClass(SSAO);
    c.property("kernelRadius", &SSAO::getKernelSize, &SSAO::setKernelSize);
    c.property("distribution", &SSAO::getDistribution, &SSAO::setDistribution);
    c.property("sampleRadius", &SSAO::getSampleRadius, &SSAO::setSampleRadius);

    auto sampleDistEnum = m.enum_<SSAO::SampleDistribution>("SampleDistribution");
    sampleDistEnum.regEnumVal(SSAO::SampleDistribution::Random);
    sampleDistEnum.regEnumVal(SSAO::SampleDistribution::UniformHammersley);
    sampleDistEnum.regEnumVal(SSAO::SampleDistribution::CosineHammersley);
}

extern "C" falcorexport void getPasses(Falcor::RenderPassLibrary& lib) {
    lib.registerClass("SSAO", "Screen-space ambient occlusion", SSAO::create);
    ScriptBindings::registerBinding(regSSAO);
}

const char* SSAO::kDesc = "Screen-space ambient occlusion. Can be used with and without a normal-map";

namespace {

const Gui::DropdownList kDistributionDropdown = {
    { (uint32_t)SSAO::SampleDistribution::Random, "Random" },
    { (uint32_t)SSAO::SampleDistribution::UniformHammersley, "Uniform Hammersley" },
    { (uint32_t)SSAO::SampleDistribution::CosineHammersley, "Cosine Hammersley" }
};

const std::string kAoMapSize = "aoMapSize";
const std::string kKernelSize = "kernelSize";
const std::string kNoiseSize = "noiseSize";
const std::string kDistribution = "distribution";
const std::string kRadius = "radius";
const std::string kBlurKernelWidth = "blurWidth";
const std::string kBlurSigma = "blurSigma";

const std::string kColorIn = "colorIn";
const std::string kColorOut = "colorOut";
const std::string kDepth = "depth";
const std::string kNormals = "normals";
const std::string kAoMap = "AoMap";

const std::string kSSAOShader = "RenderPasses/SSAO/SSAO.ps.slang";
const std::string kApplySSAOShader = "RenderPasses/SSAO/ApplyAO.ps.slang";

}

SSAO::SSAO(Device::SharedPtr pDevice): RenderPass(pDevice) {
    Sampler::Desc samplerDesc;
    samplerDesc.setFilterMode(Sampler::Filter::Point, Sampler::Filter::Point, Sampler::Filter::Point).setAddressingMode(Sampler::AddressMode::Wrap, Sampler::AddressMode::Wrap, Sampler::AddressMode::Wrap);
    mpNoiseSampler = Sampler::create(pDevice, samplerDesc);

    samplerDesc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Linear).setAddressingMode(Sampler::AddressMode::Clamp, Sampler::AddressMode::Clamp, Sampler::AddressMode::Clamp);
    mpTextureSampler = Sampler::create(pDevice, samplerDesc);

    mpSSAOPass = FullScreenPass::create(pDevice, kSSAOShader);
    mComposeData.pApplySSAOPass = FullScreenPass::create(pDevice, kApplySSAOShader);
    Sampler::Desc desc;
    desc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Linear);
    mComposeData.pApplySSAOPass["gSampler"] = Sampler::create(pDevice, desc);
    mComposeData.pFbo = Fbo::create(pDevice);
}

SSAO::SharedPtr SSAO::create(RenderContext* pRenderContext, const Dictionary& dict) {
    SharedPtr pSSAO = SharedPtr(new SSAO(pRenderContext->device()));
    Dictionary blurDict;
    for (const auto& v : dict) {
        if (v.key() == kAoMapSize) pSSAO->mAoMapSize = (uint2)v.val();
        else if (v.key() == kKernelSize) pSSAO->mData.kernelSize = v.val();
        else if (v.key() == kNoiseSize) pSSAO->mNoiseSize = (uint2)v.val();
        else if (v.key() == kDistribution) pSSAO->mHemisphereDistribution = (SampleDistribution)v.val();
        else if (v.key() == kRadius) pSSAO->mData.radius = v.val();
        else if (v.key() == kBlurKernelWidth) pSSAO->mBlurDict["kernelWidth"] = (uint32_t)v.val();
        else if (v.key() == kBlurSigma) pSSAO->mBlurDict["sigma"] = (float)v.val();
        else logWarning("Unknown field '" + v.key() + "' in a SSAO dictionary");
    }
    return pSSAO;
}

Dictionary SSAO::getScriptingDictionary() {
    Dictionary dict;
    dict[kAoMapSize] = mAoMapSize;
    dict[kKernelSize] = mData.kernelSize;
    dict[kNoiseSize] = mNoiseSize;
    dict[kRadius] = mData.radius;
    dict[kDistribution] = mHemisphereDistribution;

    auto blurDict = mpBlurGraph->getPass("GaussianBlur")->getScriptingDictionary();
    dict[kBlurKernelWidth] = (uint32_t)blurDict["kernelWidth"];
    dict[kBlurSigma] = (float)blurDict["sigma"];
    return dict;
}

RenderPassReflection SSAO::reflect(const CompileData& compileData) {
    RenderPassReflection reflector;
    reflector.addInput(kColorIn, "Color buffer");
    reflector.addOutput(kColorOut, "Color-buffer with AO applied to it");
    reflector.addInput(kDepth, "Depth-buffer");
    reflector.addInput(kNormals, "World space normals, [0, 1] range").flags(RenderPassReflection::Field::Flags::Optional);
    reflector.addInternal(kAoMap, "AO Map");
    return reflector;
}

void SSAO::compile(RenderContext* pRenderContext, const CompileData& compileData) {
    auto pDevice = pRenderContext->device();
    Fbo::Desc fboDesc(pDevice);
    fboDesc.setColorTarget(0, Falcor::ResourceFormat::R8Unorm);
    mpAOFbo = Fbo::create2D(pDevice, mAoMapSize.x, mAoMapSize.y, fboDesc);

    setKernel();
    setNoiseTexture(mNoiseSize.x, mNoiseSize.y);

    mpBlurGraph = RenderGraph::create(pDevice, "Gaussian Blur");
    GaussianBlur::SharedPtr pBlurPass = GaussianBlur::create(pRenderContext, mBlurDict);
    mpBlurGraph->addPass(pBlurPass, "GaussianBlur");
    mpBlurGraph->markOutput("GaussianBlur.dst");
}

void SSAO::execute(RenderContext* pRenderContext, const RenderData& renderData) {
    if (!mpScene) return;

    // Run the AO pass
    auto pDepth = renderData[kDepth]->asTexture();
    auto pNormals = renderData[kNormals]->asTexture();
    auto pColorOut = renderData[kColorOut]->asTexture();
    auto pColorIn = renderData[kColorIn]->asTexture();
    auto pAoMap = renderData[kAoMap]->asTexture();

    assert(pColorOut != pColorIn);
    pAoMap = generateAOMap(pRenderContext, mpScene->getCamera().get(), pDepth, pNormals);

    if (mApplyBlur) {
        mpBlurGraph->setInput("GaussianBlur.src", pAoMap);
        mpBlurGraph->execute(pRenderContext);
        pAoMap = mpBlurGraph->getOutput("GaussianBlur.dst")->asTexture();
    }

    mComposeData.pApplySSAOPass["gColor"] = pColorIn;
    mComposeData.pApplySSAOPass["gAOMap"] = pAoMap;
    mComposeData.pFbo->attachColorTarget(pColorOut, 0);
    mComposeData.pApplySSAOPass->execute(pRenderContext, mComposeData.pFbo);
}

Texture::SharedPtr SSAO::generateAOMap(RenderContext* pContext, const Camera* pCamera, const Texture::SharedPtr& pDepthTexture, const Texture::SharedPtr& pNormalTexture) {
    if (mDirty) {
        ShaderVar var = mpSSAOPass["StaticCB"];
        if (var.isValid()) var.setBlob(mData);
        mDirty = false;
    }

    {
        ShaderVar var = mpSSAOPass["PerFrameCB"];
        pCamera->setShaderData(var["gCamera"]);
    }

    // Update state/vars
    mpSSAOPass["gNoiseSampler"] = mpNoiseSampler;
    mpSSAOPass["gTextureSampler"] = mpTextureSampler;
    mpSSAOPass["gDepthTex"] = pDepthTexture;
    mpSSAOPass["gNoiseTex"] = mpNoiseTexture;
    mpSSAOPass["gNormalTex"] = pNormalTexture;

    // Generate AO
    mpSSAOPass->execute(pContext, mpAOFbo);
    return mpAOFbo->getColorTexture(0);
}

void SSAO::renderUI(Gui::Widgets& widget) {
    uint32_t distribution = (uint32_t)mHemisphereDistribution;
    if (widget.dropdown("Kernel Distribution", kDistributionDropdown, distribution)) setDistribution(distribution);

    uint32_t size = mData.kernelSize;
    if (widget.var("Kernel Size", size, 1u, kMaxSamples)) setKernelSize(size);

    float radius = mData.radius;
    if (widget.var("Sample Radius", radius, 0.001f, FLT_MAX, 0.001f)) setSampleRadius(radius);

    widget.checkbox("Apply Blur", mApplyBlur);
    if (mApplyBlur) {
        auto blurGroup = Gui::Group(widget, "Blur Settings");
        if (blurGroup.open()) {
            mpBlurGraph->getPass("GaussianBlur")->renderUI(blurGroup);
            blurGroup.release();
        }
    }
}

void SSAO::setSampleRadius(float radius) {
    mData.radius = radius;
    mDirty = true;
}

void SSAO::setKernelSize(uint32_t kernelSize) {
    kernelSize = glm::clamp(kernelSize, 1u, kMaxSamples);
    mData.kernelSize = kernelSize;
    setKernel();
}

void SSAO::setDistribution(uint32_t distribution) {
    mHemisphereDistribution = (SampleDistribution)distribution;
    setKernel();
}

void SSAO::setKernel() {
    for (uint32_t i = 0; i < mData.kernelSize; i++) {
        // Hemisphere in the Z+ direction
        float3 p;
        switch (mHemisphereDistribution) {
            case SampleDistribution::Random:
                p = glm::normalize(glm::linearRand(float3(-1.0f, -1.0f, 0.0f), float3(1.0f, 1.0f, 1.0f)));
                break;

            case SampleDistribution::UniformHammersley:
                p = hammersleyUniform(i, mData.kernelSize);
                break;

            case SampleDistribution::CosineHammersley:
                p = hammersleyCosine(i, mData.kernelSize);
                break;
        }

        mData.sampleKernel[i] = float4(p, 0.0f);

        // Skew sample point distance on a curve so more cluster around the origin
        float dist = (float)i / (float)mData.kernelSize;
        dist = glm::mix(0.1f, 1.0f, dist * dist);
        mData.sampleKernel[i] *= dist;
    }

    mDirty = true;
}

void SSAO::setNoiseTexture(uint32_t width, uint32_t height) {
    std::vector<uint32_t> data;
    data.resize(width * height);

    for (uint32_t i = 0; i < width * height; i++) {
        // Random directions on the XY plane
        float2 dir = glm::normalize(glm::linearRand(float2(-1), float2(1))) * 0.5f + 0.5f;
        data[i] = glm::packUnorm4x8(float4(dir, 0.0f, 1.0f));
    }

    mpNoiseTexture = Texture::create2D(mpDevice, width, height, ResourceFormat::RGBA8Unorm, 1, Texture::kMaxPossible, data.data());

    mData.noiseScale = float2(mpAOFbo->getWidth(), mpAOFbo->getHeight()) / float2(width, height);

    mDirty = true;
}
