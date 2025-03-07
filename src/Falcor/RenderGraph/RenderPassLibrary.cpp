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
#include <fstream>
#include <vector>
#include <map>

#ifdef _MSC_VER
#include <filesystem>
namespace fs = filesystem;
#else
#include "boost/filesystem.hpp"
namespace fs = boost::filesystem;
#endif

#include "Falcor/stdafx.h"
#include "RenderPassLibrary.h"
#include "RenderPasses/ResolvePass.h"
#include "Falcor/Core/API/Device.h"
#include "Falcor/Utils/Debug/debug.h"
#include "Falcor/Core/Platform/OS.h"
#include "RenderGraph.h"

namespace Falcor {

    extern std::vector<RenderGraph*> gRenderGraphs;

    #ifdef _MSC_VER
    static const std::string kPassLibExt = ".dll";
    #else
    static const std::string kPassLibExt = ".rpl";
    #endif

    static const std::string kPassTempLibSuffix = ".falcor";

    RenderPassLibrary* RenderPassLibrary::spInstance = nullptr;
    std::map<Device*, RenderPassLibrary*> RenderPassLibrary::spInstances = {};

    template<typename Pass>
    using PassFunc = typename Pass::SharedPtr(*)(RenderContext* pRenderContext, const Dictionary&);

#define addClass(c, desc) registerClass(#c, desc, (PassFunc<c>)c::create)

    static bool addBuiltinPasses(std::shared_ptr<Device> pDevice) {
        auto& lib = RenderPassLibrary::instance(pDevice);

        lib.addClass(ResolvePass, ResolvePass::kDesc);

        return true;
    };

    // static const bool b = addBuiltinPasses();

    RenderPassLibrary& RenderPassLibrary::instance(std::shared_ptr<Device> pDevice) {
        //if (!spInstance) spInstance = new RenderPassLibrary;
        
        auto it = spInstances.find(pDevice.get());
        if(it != spInstances.end()) {
            // found device bound pass library
            return *it->second;
        } else {
            // create pass library for a new device
            auto ret = spInstances.insert(std::pair<Device*, RenderPassLibrary*>(pDevice.get(), new RenderPassLibrary(pDevice)));
            if (ret.second == false) {
                logError("RenderPassLibrary for device " + pDevice->getPhysicalDeviceName() + " already created !!!");
            } else {
                // add built-in passes for provided device
                addBuiltinPasses(pDevice);
            }
            return *ret.first->second;
        }
        return *spInstance;
    }

    RenderPassLibrary::RenderPassLibrary(std::shared_ptr<Device> pDevice): mpDevice(pDevice) {}

    RenderPassLibrary::~RenderPassLibrary() {
        mPasses.clear();
        while (mLibs.size()) releaseLibrary(mLibs.begin()->first);
    }

    void RenderPassLibrary::shutdown() {
        safe_delete(spInstance);
    }

    RenderPassLibrary& RenderPassLibrary::registerClass(const char* className, const char* desc, CreateFunc func) {
        registerInternal(className, desc, func, nullptr);
        return *this;
    }

    void RenderPassLibrary::registerInternal(const char* className, const char* desc, CreateFunc func, DllHandle module) {
        if (mPasses.find(className) != mPasses.end()) {
            logWarning("Trying to register a render-pass `" + std::string(className) + "` to the render-passes library,  but a render-pass with the same name already exists. Ignoring the new definition");
        } else {
            mPasses[className] = ExtendedDesc(className, desc, func, module);
        }
    }

    std::shared_ptr<RenderPass> RenderPassLibrary::createPass(RenderContext* pRenderContext, const char* className, const Dictionary& dict) {
        if (mPasses.find(className) == mPasses.end()) {
            // See if we can load a DLL with the class's name and retry
            std::string libName = std::string(className) + kPassLibExt;
            logInfo("Can't find a render-pass named `" + std::string(className) + "`. Trying to load a render-pass library `" + libName + '`' + kPassLibExt);
            loadLibrary(libName);

            if (mPasses.find(className) == mPasses.end()) {
                logWarning("Trying to create a render-pass named `" + std::string(className) + "`, but no such class exists in the library");
                return nullptr;
            }
        }

        auto& renderPass = mPasses[className];
        return renderPass.func(pRenderContext, dict);
    }

    RenderPassLibrary::DescVec RenderPassLibrary::enumerateClasses() const {
        DescVec v;
        v.reserve(mPasses.size());
        for (const auto& p : mPasses) v.push_back(p.second);
        return v;
    }

    RenderPassLibrary::StrVec RenderPassLibrary::enumerateLibraries() {
        StrVec libNames;
        for (const auto& lib : spInstance->mLibs) {
            libNames.push_back(lib.first);
        }
        return libNames;
    }

    std::string RenderPassLibrary::getClassDescription(const std::string& className) {
        auto classDescIt = spInstance->mPasses.find(className);
        return std::string(classDescIt->second.desc);
    }

    void copyDllFile(const std::string& fullpath) {
        std::ifstream src(fullpath, std::ios::binary);
        std::ofstream dst(fullpath + kPassTempLibSuffix, std::ios::binary);
        dst << src.rdbuf();
        dst.flush();
        dst.close();
    }

    void RenderPassLibrary::loadLibrary(const std::string& filename) {
        fs::path filePath = filename;
        
        // render-pass name was privided without an extension and that's fine
        if (filePath.extension() != kPassLibExt) filePath += kPassLibExt;

        //std::string fullpath = getExecutableDirectory() + "/render_passes/" + getFilenameFromPath(filePath.string());
        std::string fullpath;
        
        if (!findFileInRenderPassDirectories(filePath.string(), fullpath)) {
            logWarning("Can't load render-pass library `" + filePath.string() + "`. File not found");
            return;
        }

        if (mLibs.find(fullpath) != mLibs.end()) {
            logInfo("Render-pass library `" + fullpath + "` already loaded. Ignoring `loadLibrary()` call");
            return;
        }

        // Copy the library to a temp file
        copyDllFile(fullpath);

        DllHandle l = loadDll(fullpath + kPassTempLibSuffix);
        mLibs[fullpath] = { l, getFileModifiedTime(fullpath) };
        auto func = (LibraryFunc)getDllProcAddress(l, "getPasses");

        if(!func) {
            LOG_ERR("RenderPass library getPasses proc address is NULL !!!");
        }

        // Add the DLL project directory to the search paths
        if (isDevelopmentMode()) {
            auto libProjPath = (const char*(*)(void))getDllProcAddress(l, "getProjDir");
            if (libProjPath) {
                const char* projDir = libProjPath();
                addDataDirectory(std::string(projDir) + "/Data/");
            }
        }

        RenderPassLibrary lib(mpDevice);

        try {
            func(lib);
        } catch (...) {
            logError("Can't get passes from library " + fullpath);
            throw;
        }

        for (auto& p : lib.mPasses) {
            registerInternal(p.second.className, p.second.desc, p.second.func, l);
        }
    }

    void RenderPassLibrary::releaseLibrary(const std::string& filename) {
        std::string fullpath;// = getExecutableDirectory() + "/render_passes/" + getFilenameFromPath(filename);
        if(!findFileInRenderPassDirectories(filename, fullpath)) {
            should_not_get_here();
        }

        auto libIt = mLibs.find(fullpath);
        if (libIt == mLibs.end()) {
            logWarning("Can't unload render-pass library `" + fullpath + "`. The library wasn't loaded");
            return;
        }

        mpDevice->flushAndSync();

        // Delete all the classes that were owned by the module
        DllHandle module = libIt->second.module;
        for (auto it = mPasses.begin(); it != mPasses.end();) {
            if (it->second.module == module) it = mPasses.erase(it);
            else ++it;
        }

        // Remove the DLL project directory to the search paths
        if (isDevelopmentMode()) {
            auto libProjPath = (const char*(*)(void))getDllProcAddress(module, "getProjDir");
            if (libProjPath) {
                const char* projDir = libProjPath();
                removeDataDirectory(std::string(projDir) + "/Data/");
            }
        }

        releaseDll(module);
        std::remove((fullpath + kPassTempLibSuffix).c_str());
        mLibs.erase(libIt);
    }

    void RenderPassLibrary::reloadLibrary(RenderContext* pRenderContext, std::string name) {
        assert(pRenderContext);

        auto lastTime = getFileModifiedTime(name);
        if ((lastTime == mLibs[name].lastModified) || (lastTime == 0)) return;

        DllHandle module = mLibs[name].module;

        struct PassesToReplace {
            RenderGraph* pGraph;
            std::string className;
            uint32_t nodeId;
        };

        std::vector<PassesToReplace> passesToReplace;

        for (auto& passDesc : mPasses) {
            if (passDesc.second.module != module) continue;

            // Go over all the graphs and remove this pass
            for (auto& pGraph : gRenderGraphs) {
                // Loop over the passes
                for (auto& node : pGraph->mNodeData) {
                    if (getClassTypeName(node.second.pPass.get()) == passDesc.first) {
                        passesToReplace.push_back({ pGraph, passDesc.first, node.first });
                        node.second.pPass = nullptr;
                        pGraph->mpExe.reset();
                    }
                }
            }
        }

        // OK, we removed all the passes. Reload the library
        releaseLibrary(name);
        loadLibrary(name);

        // Recreate the passes
        for (auto& r : passesToReplace) {
            r.pGraph->mNodeData[r.nodeId].pPass = createPass(pRenderContext, r.className.c_str());
            r.pGraph->mpExe = nullptr;
        }
    }

    void RenderPassLibrary::reloadLibraries(RenderContext* pRenderContext) {
        // Copy the libs vector so we don't screw up the iterator
        auto libs = mLibs;
        for (const auto& l : libs) reloadLibrary(pRenderContext, l.first);
    }
}
