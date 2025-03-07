#ifndef SRC_LAVA_LIB_RENDERER_H_
#define SRC_LAVA_LIB_RENDERER_H_

#include <cstddef>
#include <string>
#include <vector>
#include <memory>

#include "types.h"

#include "Falcor/Falcor.h"
#include "Falcor/FalcorExperimental.h"
#include "Falcor/Core/API/Device.h"
#include "Falcor/Core/API/DeviceManager.h"
#include "Falcor/Utils/Timing/FrameRate.h"
#include "Falcor/Core/Renderer.h"
#include "Falcor/Scene/Camera/Camera.h"

#include "Falcor/Utils/SampleGenerators/StratifiedSamplePattern.h"

#include "RenderPasses/AccumulatePass/AccumulatePass.h"
#include "RenderPasses/DepthPass/DepthPass.h"
#include "RenderPasses/SkyBox/SkyBox.h"
#include "RenderPasses/ForwardLightingPass/ForwardLightingPass.h"

#include "display.h"
#include "renderer_iface.h"
#include "scene_builder.h"

namespace lava {

class Renderer: public std::enable_shared_from_this<Renderer> {

	struct GraphData {
        Falcor::RenderGraph::SharedPtr pGraph;
        std::string mainOutput;
        bool showAllOutputs = false;
        std::vector<std::string> originalOutputs;
        std::unordered_map<std::string, uint32_t> graphOutputRefs;
	};
  public:
    enum class SamplePattern : uint32_t {
        Center,
        DirectX,
        Halton,
        Stratified,
    };

 public:
 	virtual ~Renderer();
    using SharedPtr = std::shared_ptr<Renderer>;
 	using UniquePtr = std::unique_ptr<Renderer>;

	std::unique_ptr<RendererIface> 	aquireInterface();
 	void						 	releaseInterface(std::unique_ptr<RendererIface> pInterface);

 public:
 	//static UniquePtr create(Device::SharedPtr pDevice);
    static SharedPtr create(Device::SharedPtr pDevice);

    Falcor::Device::SharedPtr device() { return mpDevice; };

 public:
 	bool init();
 	bool isInited() const { return mInited; }
 	
    Display::SharedPtr display() { return mpDisplay; };
    bool loadDisplay(Display::DisplayType display_type);
    bool openDisplay(const std::string& image_name, uint width, uint height);
    bool closeDisplay();

 	bool loadScript(const std::string& file_name);

 	void renderFrame(const RendererIface::FrameData frame_data);

#ifdef SCRIPTING
 	static void registerBindings(pybind11::module& m);
#endif

 private:
 	void addGraph(const Falcor::RenderGraph::SharedPtr& pGraph);
 	void initGraph(const Falcor::RenderGraph::SharedPtr& pGraph, GraphData* pData);
 	std::vector<std::string> getGraphOutputs(const Falcor::RenderGraph::SharedPtr& pGraph);

    void resolvePerFrameSparseResourcesForActiveGraph(Falcor::RenderContext* pRenderContext);
 	void executeActiveGraph(Falcor::RenderContext* pRenderContext);
 	
    void beginFrame(Falcor::RenderContext* pRenderContext, const Falcor::Fbo::SharedPtr& pTargetFbo);
 	void endFrame(Falcor::RenderContext* pRenderContext, const Falcor::Fbo::SharedPtr& pTargetFbo);

 private:
    /** This should be called before any graph execution
    */
    void finalizeScene(const RendererIface::FrameData& frame_data);

    void createRenderGraph(const RendererIface::FrameData& frame_data);

 private:
	Renderer(Device::SharedPtr pDevice);
    
 	std::vector<std::string> 	mErrorMessages;
 	Falcor::Device::SharedPtr 	mpDevice;

 	int mGpuId;
    
 	bool mIfaceAquired = false;

 	Display::SharedPtr 			mpDisplay;

    Falcor::Camera::SharedPtr   mpCamera;

 	Falcor::Fbo::SharedPtr 		mpTargetFBO;		///< The FBO available to renderers
 	Falcor::FrameRate*			mpFrameRate;
    Falcor::Clock* 	   			mpClock;
    Falcor::ArgList 			mArgList;

    lava::SceneBuilder::SharedPtr   mpSceneBuilder;
    Falcor::Sampler::SharedPtr      mpSampler;
    std::vector<GraphData>          mGraphs;
    uint32_t mActiveGraph = 0;

    ///
    float2 mInvFrameDim;
    CPUSampleGenerator::SharedPtr   mpSampleGenerator = nullptr;
    Falcor::RenderGraph::SharedPtr  mpRenderGraph = nullptr;

    AccumulatePass::SharedPtr       mpAccumulatePass = nullptr;
    DepthPass::SharedPtr            mpDepthPass = nullptr;
    SkyBox::SharedPtr               mpSkyBoxPass = nullptr;
    ForwardLightingPass::SharedPtr  mpLightingPass = nullptr;
    ///

    bool mInited = false;

    friend class RendererIface;
}; 

}  // namespace lava

#endif  // SRC_LAVA_LIB_RENDERER_H_
