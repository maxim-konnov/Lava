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
#ifndef SRC_FALCOR_CORE_API_LOWLEVELCONTEXTDATA_H_
#define SRC_FALCOR_CORE_API_LOWLEVELCONTEXTDATA_H_

#include <memory>

#include "Falcor/Core/Framework.h"
#include "GpuFence.h"

namespace Falcor {

class Device;    
struct LowLevelContextApiData;

class dlldecl LowLevelContextData : public std::enable_shared_from_this<LowLevelContextData> {
 public:
    using SharedPtr = std::shared_ptr<LowLevelContextData>;
    using SharedConstPtr = std::shared_ptr<const LowLevelContextData>;

    enum class CommandQueueType {
        Copy,
        Compute,
        Direct,
        Count
    };

    ~LowLevelContextData();

    /** Create a new low-level context data object.
        \param[in] type Command queue type.
        \param[in] queue Command queue handle. Can be nullptr.
        \return A new object, or throws an expception if creation failed.
    */
    static SharedPtr create(std::shared_ptr<Device> device, CommandQueueType type, CommandQueueHandle queue);

    void flush();

    const CommandListHandle& getCommandList() const { return mpList; }
    const CommandQueueHandle& getCommandQueue() const { return mpQueue; }
    const CommandAllocatorHandle& getCommandAllocator() const { return mpAllocator; }
    const GpuFence::SharedPtr& getFence() const { return mpFence; }
    LowLevelContextApiData* getApiData() const { return mpApiData; }

#ifdef FALCOR_D3D12
    // Used in DXR
    void setCommandList(CommandListHandle pList) { mpList = pList; }
#endif

 protected:
    LowLevelContextData(std::shared_ptr<Device> device, CommandQueueType type, CommandQueueHandle queue);

    LowLevelContextApiData* mpApiData = nullptr;
    CommandQueueType mType;
    CommandListHandle mpList;
    CommandQueueHandle mpQueue;  // Can be nullptr
    CommandAllocatorHandle mpAllocator;
    GpuFence::SharedPtr mpFence;
    std::shared_ptr<Device> mpDevice; 
};

inline std::string to_string(LowLevelContextData::CommandQueueType typ) {
    switch (typ) {
        case LowLevelContextData::CommandQueueType::Copy:
            return "Copy";
        case LowLevelContextData::CommandQueueType::Compute:
            return "Compute";
        case LowLevelContextData::CommandQueueType::Direct:
            return "Direct";
        default: 
            should_not_get_here(); 
            return "";
    }
}

}  // namespace Falcor

#endif  // SRC_FALCOR_CORE_API_LOWLEVELCONTEXTDATA_H_
