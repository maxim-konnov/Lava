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
#include <sstream>
#include <fstream>

#include "stdafx.h"
#include "Profiler.h"
#include "Falcor/Core/API/Device.h"
#include "Falcor/Core/API/GpuTimer.h"

#ifdef _WIN32
#define USE_PIX
#include "WinPixEventRuntime/Include/WinPixEventRuntime/pix3.h"
#endif

namespace Falcor {

    bool gProfileEnabled = false;  // TODO: make configurable

    using DeviceProfilerEvents = std::unordered_map<std::string, Profiler::EventData*>;

    std::unordered_map<Profiler::DeviceEventKey, Profiler::EventData*, Profiler::device_event_key_hash> Profiler::sProfilerEvents;
    std::vector<Profiler::EventData*> Profiler::sRegisteredEvents;
    std::string curEventName = "";
    uint32_t Profiler::sCurrentLevel = 0;
    uint32_t Profiler::sGpuTimerIndex = 0;

    void Profiler::initNewEvent(std::shared_ptr<Device> pDevice, EventData *pEvent, const std::string& name) {
        pEvent->name = name;
        sProfilerEvents[std::pair<uint8_t, std::string>(pDevice->uid(), curEventName)] = pEvent;
    }

    Profiler::EventData* Profiler::createNewEvent(std::shared_ptr<Device> pDevice, const std::string& name) {
        EventData *pData = new EventData;
        initNewEvent(pDevice, pData, name);
        return pData;
    }

    Profiler::EventData* Profiler::isEventRegistered(std::shared_ptr<Device> pDevice, const std::string& name) {
        auto event = sProfilerEvents.find(std::make_pair(pDevice->uid(), name));
        return (event == sProfilerEvents.end()) ? nullptr : event->second;
    }

    Profiler::EventData* Profiler::getEvent(std::shared_ptr<Device> pDevice, const std::string& name) {
        auto event = isEventRegistered(pDevice, name);
        return event ? event : createNewEvent(pDevice, name);
    }

    void Profiler::startEvent(std::shared_ptr<Device> pDevice, const std::string& name, Flags flags, bool showInMsg) {
        if (gProfileEnabled && is_set(flags, Flags::Internal)) {
            curEventName = curEventName + "#" + name;
            EventData* pData = getEvent(pDevice, curEventName);
            pData->triggered++;
            if (pData->triggered > 1) {
                logWarning("Profiler event `" + name + "` was triggered while it is already running. Nesting profiler events with the same name is disallowed and you should probably fix that. Ignoring the new call");
                return;
            }

            pData->showInMsg = showInMsg;
            pData->level = sCurrentLevel;
            pData->cpuStart = CpuTimer::getCurrentTimePoint();
            EventData::FrameData& frame = pData->frameData[sGpuTimerIndex];
            
            if (frame.currentTimer >= frame.pTimers.size()) {
                frame.pTimers.push_back(GpuTimer::create(pDevice));
            }
            
            frame.pTimers[frame.currentTimer]->begin();
            pData->callStack.push(frame.currentTimer);
            frame.currentTimer++;
            sCurrentLevel++;

            if (!pData->registered) {
                sRegisteredEvents.push_back(pData);
                pData->registered = true;
            }
        }
        #ifdef _WIN32
        if (is_set(flags, Flags::Pix)) {
            PIXBeginEvent((ID3D12GraphicsCommandList*)gpDevice->getRenderContext()->getLowLevelData()->getCommandList(), PIX_COLOR(0, 0, 0), name.c_str());
        }
        #endif
    }

    void Profiler::endEvent(std::shared_ptr<Device> pDevice, const std::string& name, Flags flags) {
        if (gProfileEnabled && is_set(flags, Flags::Internal)) {
            assert(isEventRegistered(pDevice, curEventName));
            EventData* pData = getEvent(pDevice, curEventName);
            pData->triggered--;
            if (pData->triggered != 0) return;

            pData->cpuEnd = CpuTimer::getCurrentTimePoint();
            pData->cpuTotal += CpuTimer::calcDuration(pData->cpuStart, pData->cpuEnd);

            pData->frameData[sGpuTimerIndex].pTimers[pData->callStack.top()]->end();
            pData->callStack.pop();

            sCurrentLevel--;
            curEventName.erase(curEventName.find_last_of("#"));
        }
        #ifdef _WIN32
        if (is_set(flags, Flags::Pix)) {
            PIXEndEvent((ID3D12GraphicsCommandList*)gpDevice->getRenderContext()->getLowLevelData()->getCommandList());
        }
        #endif
    }

    double Profiler::getEventGpuTime(std::shared_ptr<Device> pDevice, const std::string& name) {
        const auto& pEvent = getEvent(pDevice, name);
        return pEvent ? getGpuTime(pEvent) : 0;
    }

    double Profiler::getEventCpuTime(std::shared_ptr<Device> pDevice, const std::string& name) {
        const auto& pEvent = getEvent(pDevice, name);
        return pEvent ? getCpuTime(pEvent) : 0;
    }

    double Profiler::getGpuTime(const EventData* pData) {
        double gpuTime = 0;
        for (size_t i = 0; i < pData->frameData[1 - sGpuTimerIndex].currentTimer; i++) {
            gpuTime += pData->frameData[1 - sGpuTimerIndex].pTimers[i]->getElapsedTime();
        }
        return gpuTime;
    }

    double Profiler::getCpuTime(const EventData* pData) {
        return pData->cpuTotal;
    }

    std::string Profiler::getEventsString() {
        std::string results("Name\t\t\t\t\tCPU time(ms)\t\t  GPU time(ms)\n");

        for (EventData* pData : sRegisteredEvents) {
            assert(pData->triggered == 0);
            if(pData->showInMsg == false) continue;

            double gpuTime = getGpuTime(pData);
            assert(pData->callStack.empty());

            char event[1000];
            uint32_t nameIndent = pData->level * 2 + 1;
            uint32_t cpuIndent = 30 - (nameIndent + (uint32_t)pData->name.substr(pData->name.find_last_of("#") + 1).size());
            snprintf(event, 1000, "%*s%s %*.2f (%.2f) %14.2f (%.2f)\n", nameIndent, " ", pData->name.substr(pData->name.find_last_of("#") + 1).c_str(), cpuIndent, getCpuTime(pData),
                     pData->cpuRunningAverageMS, gpuTime, pData->gpuRunningAverageMS);
#if _PROFILING_LOG == 1
            pData->cpuMs[pData->stepNr] = (float)pData->cpuTotal;
            pData->gpuMs[pData->stepNr] = (float)gpuTime;
            pData->stepNr++;
            
            if (pData->stepNr == _PROFILING_LOG_BATCH_SIZE) {
                std::ostringstream logOss, fileOss;
                logOss << "dumping " << "profile_" << pData->name << "_" << pData->filesWritten;
                logInfo(logOss.str());
                fileOss << "profile_" << pData->name << "_" << pData->filesWritten++;
                std::ofstream out(fileOss.str().c_str());
                
                for (int i = 0; i < _PROFILING_LOG_BATCH_SIZE; ++i) {
                    out << pData->cpuMs[i] << " " << pData->gpuMs[i] << "\n";
                }
                pData->stepNr = 0;
            }
#endif
            results += event;
        }

        return results;
    }

    void Profiler::endFrame() {
        for (EventData* pData : sRegisteredEvents) {
            // Update CPU/GPU time running averages.
            const double cpuTime = getCpuTime(pData);
            const double gpuTime = getGpuTime(pData);
            // With sigma = 0.98, then after 100 frames, a given value's contribution is down to ~1.7% of
            // the running average, which seems to provide a reasonable trade-off of temporal smoothing
            // versus setting in to a new value when something has changed.
            const double sigma = .98;
            if (pData->cpuRunningAverageMS < 0.) pData->cpuRunningAverageMS = cpuTime;
            else pData->cpuRunningAverageMS = sigma * pData->cpuRunningAverageMS + (1. - sigma) * cpuTime;
            if (pData->gpuRunningAverageMS < 0.) pData->gpuRunningAverageMS = gpuTime;
            else pData->gpuRunningAverageMS = sigma * pData->gpuRunningAverageMS + (1. - sigma) * gpuTime;

            pData->showInMsg = false;
            pData->cpuTotal = 0;
            pData->triggered = 0;
            pData->frameData[1 - sGpuTimerIndex].currentTimer = 0;
            pData->registered = false;
        }
        sRegisteredEvents.clear();
        sGpuTimerIndex = 1 - sGpuTimerIndex;
    }

#if _PROFILING_LOG == 1
    void Profiler::flushLog() {
        for (EventData* pData : sRegisteredEvents) {
            std::ostringstream logOss, fileOss;
            logOss << "dumping " << "profile_" << pData->name << "_" << pData->filesWritten;
            logInfo(logOss.str());
            fileOss << "profile_" << pData->name << "_" << pData->filesWritten++;
            std::ofstream out(fileOss.str().c_str());
            
            for (int i = 0; i < pData->stepNr; ++i) {
                out << pData->cpuMs[i] << " " << pData->gpuMs[i] << "\n";
            }
            
            pData->stepNr = 0;
        }
    }
#endif

    void Profiler::clearEvents() {
        for (EventData* pData : sRegisteredEvents) {
            delete pData;
        }

        sProfilerEvents.clear();
        sRegisteredEvents.clear();
        sCurrentLevel = 0;
        sGpuTimerIndex = 0;
        curEventName = "";
    }
}
