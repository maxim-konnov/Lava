/************************************************************************
Copyright 2020 Advanced Micro Devices, Inc
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
************************************************************************/

#include "lavaApiAov.h"
#include "lavaApi.h"
#include "lavaApiFramebuffer.h"

//#include "pxr/imaging/rprUsd/contextMetadata.h"
#include "error.h"

PXR_NAMESPACE_OPEN_SCOPE

namespace {

bool ReadRifImage(rif_image image, void* dstBuffer, size_t dstBufferSize) {
    if (!image || !dstBuffer) {
        return false;
    }

    size_t size;
    size_t dummy;
    auto rifStatus = rifImageGetInfo(image, RIF_IMAGE_DATA_SIZEBYTE, sizeof(size), &size, &dummy);
    if (rifStatus != RIF_SUCCESS || dstBufferSize < size) {
        return false;
    }

    void* data = nullptr;
    rifStatus = rifImageMap(image, RIF_IMAGE_MAP_READ, &data);
    if (rifStatus != RIF_SUCCESS) {
        return false;
    }

    std::memcpy(dstBuffer, data, size);

    rifStatus = rifImageUnmap(image, data);
    if (rifStatus != RIF_SUCCESS) {
        TF_WARN("Failed to unmap rif image");
    }

    return true;
}

} // namespace anonymous

HdLavaApiAov::HdLavaApiAov(rpr_aov rprAovType, int width, int height, HdFormat format,
                         rpr::Context* rprContext, RprUsdContextMetadata const& rprContextMetadata, std::unique_ptr<rif::Filter> filter)
    : m_aovDescriptor(HdLavaAovRegistry::GetInstance().GetAovDesc(rprAovType, false))
    , m_filter(std::move(filter))
    , m_format(format) {
    if (rif::Image::GetDesc(0, 0, format).type == 0) {
        RIF_THROW_ERROR_MSG("Unsupported format: " + TfEnum::GetName(format));
    }

    m_aov = pxr::make_unique<HdLavaApiFramebuffer>(rprContext, width, height);
    m_aov->AttachAs(rprAovType);

    // XXX (Hybrid): Hybrid plugin does not support framebuffer resolving (rprContextResolveFrameBuffer)
    if (rprContextMetadata.pluginType != kPluginHybrid) {
        m_resolved = pxr::make_unique<HdLavaApiFramebuffer>(rprContext, width, height);
    }
}

HdLavaApiAov::HdLavaApiAov(rpr_aov rprAovType, int width, int height, HdFormat format,
                         rpr::Context* rprContext, RprUsdContextMetadata const& rprContextMetadata, rif::Context* rifContext)
    : HdLavaApiAov(rprAovType, width, height, format, rprContext, rprContextMetadata, [format, rifContext]() -> std::unique_ptr<rif::Filter> {
        if (format == HdFormatFloat32Vec4) {
            // LAVA framebuffers by default with such format
            return nullptr;
        }

        auto filter = rif::Filter::CreateCustom(RIF_IMAGE_FILTER_RESAMPLE, rifContext);
        if (!filter) {
            LAVA_THROW_ERROR_MSG("Failed to create resample filter");
        }

        filter->SetParam("interpOperator", RIF_IMAGE_INTERPOLATION_NEAREST);
        return filter;
    }()) {

}

void HdLavaApiAov::Resolve() {
    if (m_aov) {
        m_aov->Resolve(m_resolved.get());
    }

    if (m_filter) {
        m_filter->Resolve();
    }
}

void HdLavaApiAov::Clear() {
    if (m_aov) {
        auto& v = m_aovDescriptor.clearValue;
        m_aov->Clear(v[0], v[1], v[2], v[3]);
    }
}

bool HdLavaApiAov::GetDataImpl(void* dstBuffer, size_t dstBufferSize) {
    if (m_filter) {
        return ReadRifImage(m_filter->GetOutput(), dstBuffer, dstBufferSize);
    }

    auto resolvedFb = GetResolvedFb();
    if (!resolvedFb) {
        return false;
    }

    return resolvedFb->GetData(dstBuffer, dstBufferSize);
}

bool HdLavaApiAov::GetData(void* dstBuffer, size_t dstBufferSize) {
    if (GetDataImpl(dstBuffer, dstBufferSize)) {
        if (m_format == HdFormatInt32) {
            // LAVA store integer ID values to RGB images using such formula:
            // c[i].x = i;
            // c[i].y = i/256;
            // c[i].z = i/(256*256);
            // i.e. saving little endian int24 to uchar3
            // That's why we interpret the value as int and filling the alpha channel with zeros
            auto primIdData = reinterpret_cast<int*>(dstBuffer);
            for (size_t i = 0; i < dstBufferSize / sizeof(int); ++i) {
                primIdData[i] &= 0xFFFFFF;
            }
        }

        return true;
    }

    return false;
}

void HdLavaApiAov::Resize(int width, int height, HdFormat format) {
    if (m_format != format) {
        m_format = format;
        m_dirtyBits |= ChangeTracker::DirtyFormat;
    }

    if (m_aov && m_aov->Resize(width, height)) {
        m_dirtyBits |= ChangeTracker::DirtySize;
    }

    if (m_resolved && m_resolved->Resize(width, height)) {
        m_dirtyBits |= ChangeTracker::DirtyFormat;
    }
}

void HdLavaApiAov::Update(HdLavaApi const* rprApi, rif::Context* rifContext) {
    if (m_dirtyBits & ChangeTracker::DirtyFormat) {
        OnFormatChange(rifContext);
    }
    if (m_dirtyBits & ChangeTracker::DirtySize) {
        OnSizeChange(rifContext);
    }
    m_dirtyBits = ChangeTracker::Clean;

    if (m_filter) {
        m_filter->Update();
    }
}

HdLavaApiFramebuffer* HdLavaApiAov::GetResolvedFb() {
    return (m_resolved ? m_resolved : m_aov).get();
}

void HdLavaApiAov::OnFormatChange(rif::Context* rifContext) {
    m_filter = nullptr;
    if (rifContext && m_format != HdFormatFloat32Vec4) {
        m_filter = rif::Filter::CreateCustom(RIF_IMAGE_FILTER_RESAMPLE, rifContext);
        m_filter->SetParam("interpOperator", RIF_IMAGE_INTERPOLATION_NEAREST);

        // Reset inputs
        m_dirtyBits |= ChangeTracker::DirtySize;
    }
}

void HdLavaApiAov::OnSizeChange(rif::Context* rifContext) {
    if (m_filter) {
        auto fbDesc = m_aov->GetDesc();
        m_filter->Resize(fbDesc.fb_width, fbDesc.fb_height);
        m_filter->SetInput(rif::Color, GetResolvedFb());
        m_filter->SetOutput(rif::Image::GetDesc(fbDesc.fb_width, fbDesc.fb_height, m_format));
        m_filter->SetParam("outSize", GfVec2i(fbDesc.fb_width, fbDesc.fb_height));
    }
}

HdLavaApiColorAov::HdLavaApiColorAov(HdFormat format, std::shared_ptr<HdLavaApiAov> rawColorAov, rpr::Context* rprContext, RprUsdContextMetadata const& rprContextMetadata)
    : HdLavaApiAov(HdLavaAovRegistry::GetInstance().GetAovDesc(rpr::Aov(kColorAlpha), true), format)
    , m_retainedRawColor(std::move(rawColorAov)) {

}

void HdLavaApiColorAov::SetFilter(Filter filter, bool enable) {
    bool isFilterEnabled = m_enabledFilters & filter;
    if (enable != isFilterEnabled) {
        if (enable) {
            m_enabledFilters |= filter;
        } else {
            m_enabledFilters &= ~filter;
        }
        m_isEnabledFiltersDirty = true;
    }
}

void HdLavaApiColorAov::SetOpacityAov(std::shared_ptr<HdLavaApiAov> opacity) {
    if (m_retainedOpacity != opacity) {
        m_retainedOpacity = opacity;
        SetFilter(kFilterComposeOpacity, CanComposeAlpha());
    }
}

void HdLavaApiColorAov::EnableAIDenoise(
    std::shared_ptr<HdLavaApiAov> albedo,
    std::shared_ptr<HdLavaApiAov> normal,
    std::shared_ptr<HdLavaApiAov> linearDepth) {
    if (m_enabledFilters & kFilterAIDenoise) {
        return;
    }
    if (!albedo || !normal || !linearDepth) {
        TF_RUNTIME_ERROR("Failed to enable AI denoise: invalid parameters");
        return;
    }

    for (auto& retainedInput : m_retainedDenoiseInputs) {
        retainedInput = nullptr;
    }
    m_retainedDenoiseInputs[rif::Normal] = normal;
    m_retainedDenoiseInputs[rif::LinearDepth] = linearDepth;
    m_retainedDenoiseInputs[rif::Albedo] = albedo;

    SetFilter(kFilterAIDenoise, true);
    SetFilter(kFilterEAWDenoise, false);
}

void HdLavaApiColorAov::EnableEAWDenoise(
    std::shared_ptr<HdLavaApiAov> albedo,
    std::shared_ptr<HdLavaApiAov> normal,
    std::shared_ptr<HdLavaApiAov> linearDepth,
    std::shared_ptr<HdLavaApiAov> objectId,
    std::shared_ptr<HdLavaApiAov> worldCoordinate) {
    if (m_enabledFilters & kFilterEAWDenoise) {
        return;
    }
    if (!albedo || !normal || !linearDepth || !objectId || !worldCoordinate) {
        TF_RUNTIME_ERROR("Failed to enable EAW denoise: invalid parameters");
        return;
    }

    for (auto& retainedInput : m_retainedDenoiseInputs) {
        retainedInput = nullptr;
    }
    m_retainedDenoiseInputs[rif::Normal] = normal;
    m_retainedDenoiseInputs[rif::LinearDepth] = linearDepth;
    m_retainedDenoiseInputs[rif::ObjectId] = objectId;
    m_retainedDenoiseInputs[rif::Albedo] = albedo;
    m_retainedDenoiseInputs[rif::WorldCoordinate] = worldCoordinate;

    SetFilter(kFilterEAWDenoise, true);
    SetFilter(kFilterAIDenoise, false);
}

void HdLavaApiColorAov::DisableDenoise(rif::Context* rifContext) {
    SetFilter(kFilterEAWDenoise, false);
    SetFilter(kFilterAIDenoise, false);
    SetFilter(kFilterResample, m_format != HdFormatFloat32Vec4);

    for (auto& retainedInput : m_retainedDenoiseInputs) {
        retainedInput = nullptr;
    }
}

void HdLavaApiColorAov::SetTonemap(TonemapParams const& params) {
    bool isTonemapEnabled = m_enabledFilters & kFilterTonemap;
    bool tonemapEnableDirty = params.enable != isTonemapEnabled;

    SetFilter(kFilterTonemap, params.enable);

    if (m_tonemap != params) {
        m_tonemap = params;

        if (!tonemapEnableDirty && isTonemapEnabled) {
            if (m_mainFilterType == kFilterTonemap) {
                SetTonemapFilterParams(m_filter.get());
            } else {
                for (auto& entry : m_auxFilters) {
                    if (entry.first == kFilterTonemap) {
                        SetTonemapFilterParams(entry.second.get());
                        break;
                    }
                }
            }
        }
    }
}

void HdLavaApiColorAov::SetTonemapFilterParams(rif::Filter* filter) {
    filter->SetParam("exposure", m_tonemap.exposure);
    filter->SetParam("sensitivity", m_tonemap.sensitivity);
    filter->SetParam("fstop", m_tonemap.fstop);
    filter->SetParam("gamma", m_tonemap.gamma);
}

bool HdLavaApiColorAov::CanComposeAlpha() {
    // Compositing alpha into framebuffer with less than 4 components is a no-op
    return HdGetComponentCount(m_format) == 4 && m_retainedOpacity;
}

void HdLavaApiColorAov::Resize(int width, int height, HdFormat format) {
    if (m_width != width || m_height != height) {
        m_width = width;
        m_height = height;
        m_dirtyBits |= ChangeTracker::DirtySize;
    }

    HdLavaApiAov::Resize(width, height, format);
}

void HdLavaApiColorAov::Update(HdLavaApi const* rprApi, rif::Context* rifContext) {
    if (m_dirtyBits & ChangeTracker::DirtyFormat) {
        OnFormatChange(rifContext);
    }

    if (m_isEnabledFiltersDirty) {
        m_isEnabledFiltersDirty = false;
        if (!rifContext && m_enabledFilters != kFilterNone) {
            TF_WARN("Can not enable %#x filters: rifContext required", m_enabledFilters);
            m_enabledFilters = kFilterNone;
        }

        m_filter = nullptr;
        m_auxFilters.clear();

        if ((m_enabledFilters & kFilterAIDenoise) ||
            (m_enabledFilters & kFilterEAWDenoise) ||
            (m_enabledFilters & kFilterComposeOpacity) ||
            (m_enabledFilters & kFilterTonemap)) {

            auto addFilter = [this](Filter type, std::unique_ptr<rif::Filter> filter) {
                if (m_filter) {
                    m_auxFilters.emplace_back(m_mainFilterType, std::move(m_filter));
                }

                m_filter = std::move(filter);
                m_mainFilterType = type;
            };

            if (m_enabledFilters & kFilterTonemap) {
                addFilter(kFilterTonemap, rif::Filter::CreateCustom(RIF_IMAGE_FILTER_PHOTO_LINEAR_TONEMAP, rifContext));
            }

            if ((m_enabledFilters & kFilterAIDenoise) ||
                (m_enabledFilters & kFilterEAWDenoise)) {
                auto denoiseFilterType = (m_enabledFilters & kFilterAIDenoise) ? rif::FilterType::AIDenoise : rif::FilterType::EawDenoise;
                auto fbDesc = m_retainedRawColor->GetAovFb()->GetDesc();

                auto type = (m_enabledFilters & kFilterAIDenoise) ? kFilterAIDenoise : kFilterEAWDenoise;
                auto filter = rif::Filter::Create(denoiseFilterType, rifContext, fbDesc.fb_width, fbDesc.fb_height);
                addFilter(type, std::move(filter));
            }

            if (m_enabledFilters & kFilterComposeOpacity) {
                auto filter = rif::Filter::CreateCustom(RIF_IMAGE_FILTER_USER_DEFINED, rifContext);
                auto opacityComposingKernelCode = std::string(R"(
                    int2 coord;
                    GET_COORD_OR_RETURN(coord, GET_BUFFER_SIZE(inputImage));
                    vec4 alpha = ReadPixelTyped(alphaImage, coord.x, coord.y);
                    vec4 color = ReadPixelTyped(inputImage, coord.x, coord.y) * alpha.x;
                    WritePixelTyped(outputImage, coord.x, coord.y, make_vec4(color.x, color.y, color.z, alpha.x));
                )");
                filter->SetParam("code", opacityComposingKernelCode);
                addFilter(kFilterComposeOpacity, std::move(filter));
            }
        } else if (m_enabledFilters & kFilterResample) {
            m_filter = rif::Filter::CreateCustom(RIF_IMAGE_FILTER_RESAMPLE, rifContext);
            m_filter->SetParam("interpOperator", RIF_IMAGE_INTERPOLATION_NEAREST);
            m_mainFilterType = kFilterResample;
        }

        // Signal to update inputs
        m_dirtyBits |= ChangeTracker::DirtySize;
    }

    if (m_dirtyBits & ChangeTracker::DirtySize) {
        OnSizeChange(rifContext);
    }
    m_dirtyBits = ChangeTracker::Clean;

    for (auto& auxFilter : m_auxFilters) {
        auxFilter.second->Update();
    }
    if (m_filter) {
        m_filter->Update();
    }
}

bool HdLavaApiColorAov::GetData(void* dstBuffer, size_t dstBufferSize) {
    if (!m_filter) {
        if (auto resolvedRawColorFb = m_retainedRawColor->GetResolvedFb()) {
            return resolvedRawColorFb->GetData(dstBuffer, dstBufferSize);
        } else {
            return false;
        }
    } else {
        return HdLavaApiAov::GetData(dstBuffer, dstBufferSize);
    }
}

void HdLavaApiColorAov::Resolve() {
    HdLavaApiAov::Resolve();

    for (auto& auxFilter : m_auxFilters) {
        auxFilter.second->Resolve();
    }
}

void HdLavaApiColorAov::OnFormatChange(rif::Context* rifContext) {
    SetFilter(kFilterResample, m_format != HdFormatFloat32Vec4);
    SetFilter(kFilterComposeOpacity, CanComposeAlpha());
    m_dirtyBits |= ChangeTracker::DirtySize;
}

template <typename T>
void HdLavaApiColorAov::ResizeFilter(int width, int height, Filter filterType, rif::Filter* filter, T input) {
    filter->Resize(width, height);
    filter->SetInput(rif::Color, input);
    filter->SetOutput(rif::Image::GetDesc(width, height, m_format));

    if (filterType == kFilterAIDenoise || filterType == kFilterEAWDenoise) {
        for (int i = 0; i < rif::MaxInput; ++i) {
            if (auto retainedInput = m_retainedDenoiseInputs[i].get()) {
                filter->SetInput(static_cast<rif::FilterInputType>(i), retainedInput->GetResolvedFb());
            }
        }
    } else if (filterType == kFilterComposeOpacity) {
        filter->SetInput("alphaImage", m_retainedOpacity->GetResolvedFb());
    } else if (filterType == kFilterResample) {
        filter->SetParam("outSize", GfVec2i(width, height));
    } else if (filterType == kFilterTonemap) {
        SetTonemapFilterParams(filter);
    }
}

void HdLavaApiColorAov::OnSizeChange(rif::Context* rifContext) {
    if (!m_filter) {
        return;
    }

    auto fbDesc = m_retainedRawColor->GetAovFb()->GetDesc();
    if (m_auxFilters.empty()) {
        ResizeFilter(fbDesc.fb_width, fbDesc.fb_height, m_mainFilterType, m_filter.get(), m_retainedRawColor->GetResolvedFb());
    } else {
        // Ideally we would use "Filter combining" functionality, but it does not work with user-defined filter
        // So we attach each filter separately

        auto filter = m_auxFilters.front().second.get();
        ResizeFilter(fbDesc.fb_width, fbDesc.fb_height, m_auxFilters.front().first, filter, m_retainedRawColor->GetResolvedFb());
        for (int i = 1; i < m_auxFilters.size(); ++i) {
            auto filterInput = m_auxFilters[i - 1].second->GetOutput();
            ResizeFilter(fbDesc.fb_width, fbDesc.fb_height, m_auxFilters[i].first, m_auxFilters[i].second.get(), filterInput);
        }
        ResizeFilter(fbDesc.fb_width, fbDesc.fb_height, m_mainFilterType, m_filter.get(), m_auxFilters.back().second->GetOutput());
    }
}

HdLavaApiNormalAov::HdLavaApiNormalAov(
    int width, int height, HdFormat format,
    rpr::Context* rprContext, RprUsdContextMetadata const& rprContextMetadata, rif::Context* rifContext)
    : HdLavaApiAov(LAVA_AOV_SHADING_NORMAL, width, height, format, rprContext, rprContextMetadata, rif::Filter::CreateCustom(RIF_IMAGE_FILTER_REMAP_RANGE, rifContext)) {
    if (!rifContext) {
        LAVA_THROW_ERROR_MSG("Can not create normal AOV: RIF context required");
    }

    m_filter->SetParam("srcRangeAuto", 0);
    m_filter->SetParam("dstLo", -1.0f);
    m_filter->SetParam("dstHi", 1.0f);
}

void HdLavaApiNormalAov::OnFormatChange(rif::Context* rifContext) {
    m_dirtyBits |= ChangeTracker::DirtySize;
}

void HdLavaApiNormalAov::OnSizeChange(rif::Context* rifContext) {
    auto fbDesc = m_aov->GetDesc();
    m_filter->Resize(fbDesc.fb_width, fbDesc.fb_height);
    m_filter->SetInput(rif::Color, GetResolvedFb());
    m_filter->SetOutput(rif::Image::GetDesc(fbDesc.fb_width, fbDesc.fb_height, m_format));
}

HdLavaApiDepthAov::HdLavaApiDepthAov(
    HdFormat format,
    std::shared_ptr<HdLavaApiAov> worldCoordinateAov,
    rpr::Context* rprContext, RprUsdContextMetadata const& rprContextMetadata, rif::Context* rifContext)
    : HdLavaApiAov(HdLavaAovRegistry::GetInstance().GetAovDesc(rpr::Aov(kNdcDepth), true), format)
    , m_retainedWorldCoordinateAov(worldCoordinateAov) {
    if (!rifContext) {
        LAVA_THROW_ERROR_MSG("Can not create depth AOV: RIF context required");
    }

    m_filter = rif::Filter::CreateCustom(RIF_IMAGE_FILTER_NDC_DEPTH, rifContext);
    m_ndcFilter = m_filter.get();
    m_remapFilter = nullptr;

#if PXR_VERSION >= 2002
    m_retainedFilter = std::move(m_filter);

    m_filter = rif::Filter::CreateCustom(RIF_IMAGE_FILTER_REMAP_RANGE, rifContext);
    m_filter->SetParam("srcRangeAuto", 0);
    m_filter->SetParam("srcLo", -1.0f);
    m_filter->SetParam("srcHi", 1.0f);
    m_filter->SetParam("dstLo", 0.0f);
    m_filter->SetParam("dstHi", 1.0f);
    m_remapFilter = m_filter.get();
#endif

    auto fbDesc = m_retainedWorldCoordinateAov->GetAovFb()->GetDesc();
    m_width = fbDesc.fb_width;
    m_height = fbDesc.fb_height;
}

void HdLavaApiDepthAov::Update(HdLavaApi const* rprApi, rif::Context* rifContext) {
    if (m_dirtyBits & ChangeTracker::DirtyFormat ||
        m_dirtyBits & ChangeTracker::DirtySize) {

        if (m_remapFilter) {
            m_ndcFilter->SetInput(rif::Color, m_retainedWorldCoordinateAov->GetResolvedFb());
            m_ndcFilter->SetOutput(rif::Image::GetDesc(m_width, m_height, m_format));
            m_remapFilter->SetInput(rif::Color, m_ndcFilter->GetOutput());
            m_remapFilter->SetOutput(rif::Image::GetDesc(m_width, m_height, m_format));
        } else {
            m_ndcFilter->SetInput(rif::Color, m_retainedWorldCoordinateAov->GetResolvedFb());
            m_ndcFilter->SetOutput(rif::Image::GetDesc(m_width, m_height, m_format));
        }
    }
    m_dirtyBits = ChangeTracker::Clean;

    auto viewProjectionMatrix = rprApi->GetCameraViewMatrix() * rprApi->GetCameraProjectionMatrix();
    m_ndcFilter->SetParam("viewProjMatrix", GfMatrix4f(viewProjectionMatrix.GetTranspose()));

    if (m_remapFilter) {
        m_remapFilter->Update();
    }
    m_ndcFilter->Update();
}

void HdLavaApiDepthAov::Resize(int width, int height, HdFormat format) {
    if (m_format != format) {
        m_format = format;
        m_dirtyBits |= ChangeTracker::DirtyFormat;
    }

    if (m_width != width || m_height != height) {
        m_width = width;
        m_height = height;
        m_dirtyBits |= ChangeTracker::DirtySize;
    }
}

PXR_NAMESPACE_CLOSE_SCOPE
