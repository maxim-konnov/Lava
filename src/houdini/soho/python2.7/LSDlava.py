"""
    This file was automatically generated.  Please do not modify by hand.

    The code provides a mapping from Houdini parameters to the parameters
    needed for rendering.
"""

def initializeFeatures(settings):
    Renderer = settings.addGlobal
    Camera = settings.addGlobal
    Image = settings.addGlobal
    Photon = settings.addGlobal
    Object = settings.addObject
    Light = settings.addLight
    Fog = settings.addFog
    Geometry = settings.addGeometry
    Deep = settings.addDeepResolver
    Measure = settings.addMeasure
    Bokeh = settings.addBokeh
    IPlane = settings.addImagePlane
    ImageOption = settings.addImageOption

    # -- Renderer --
    Renderer("renderer", "abortmissingtexture", "bool", "lv_abort_missing_texture")
    Renderer("renderer", "alfprogress", "bool", "lv_alfprogress")
    Renderer("renderer", "bake_alphadestplanes", "string", "lv_bake_alphadestplanes")
    Renderer("renderer", "bake_normalizep", "bool", "lv_bake_normalizep")
    Renderer("renderer", "bake_normalsfaceforward", "bool", "lv_bake_normalsfaceforward")
    Renderer("renderer", "bake_udimrange", "string", "lv_bake_udimrange")
    Renderer("renderer", "bake_usemikkt", "bool", "lv_bake_usemikkt")
    Renderer("renderer", "bboxenablelod", "bool", "lv_bboxenablelod")
    Renderer("renderer", "blurquality", "bool", "lv_blurquality")
    Renderer("renderer", "bvhoriented", "bool", "lv_bvhoriented")
    Renderer("renderer", "cacheratio", "float", "lv_cacheratio")
    Renderer("renderer", "cachesize", "int", "lv_cachesize")
    Renderer("renderer", "colorspace", "string", "lv_colorspace", skipdefault=False)
    Renderer("renderer", "constrainmaxrough", "bool", "lv_constrainmaxrough")
    Renderer("renderer", "curvebunchsize", "int", "lv_curvebunchsize")
    Renderer("renderer", "dicingquality", "bool", "lv_dicingquality")
    Renderer("renderer", "diffusecomponents", "string", "lv_diffusecomponents")
    Renderer("renderer", "dofquality", "bool", "lv_dofquality")
    Renderer("renderer", "exportcomponents", "string", "lv_exportcomponents")
    Renderer("renderer", "extractimageplanes", "bool", "lv_extractimageplanes")
    Renderer("renderer", "extractimageplanesformat", "string", "lv_extractimageplanesformat")
    Renderer("renderer", "extractlinearplanes", "string", "lv_extractlinearplanes")
    Renderer("renderer", "extractremoveintermediate", "bool", "lv_extractremoveintermediate")
    Renderer("renderer", "extractseparator", "string", "lv_extractseparator")
    Renderer("renderer", "fogbackground", "bool", "lv_fogbackground")
    Renderer("renderer", "fogbackgroundshader", "string", "lv_fogbackgroundshader")
    Renderer("renderer", "fogimports", "string", "lv_fogimports")
    Renderer("renderer", "giquality", "bool", "lv_giquality")
    Renderer("renderer", "hidden", "bool", "lv_hidden")
    Renderer("renderer", "imageblur", "bool", "lv_imageblur")
    Renderer("renderer", "iprfakerelight", "bool", "lv_iprfakerelight")
    Renderer("renderer", "iprfixsamples", "bool", "lv_iprfixsamples")
    Renderer("renderer", "iprpasssamples", "float", "lv_iprpasssamples")
    Renderer("renderer", "kdmemfactor", "float", "lv_kdmemfactor")
    Renderer("renderer", "logcolors", "int", "lv_logcolors", skipdefault=False)
    Renderer("renderer", "logtimestamps", "bool", "lv_logtimestamps")
    Renderer("renderer", "measuremedian", "bool", "lv_measuremedian")
    Renderer("renderer", "nesteddielectric", "bool", "lv_nesteddielectric")
    Renderer("renderer", "networkcompress", "bool", "lv_networkcompress")
    Renderer("renderer", "networkqsize", "int", "lv_networkqsize")
    Renderer("renderer", "objroot", "string", "lv_objroot")
    Renderer("renderer", "octreestyle", "string", "lv_octreestyle")
    Renderer("renderer", "pathcutoff", "bool", "lv_pathcutoff")
    Renderer("renderer", "pathcutoffdepth", "int", "lv_pathcutoffdepth")
    Renderer("renderer", "pathcutoffmethod", "int", "lv_pathcutoffmethod")
    Renderer("renderer", "pathcutoffthresh", "float", "lv_pathcutoffthresh")
    Renderer("renderer", "pbrpathtype", "string", "lv_pbrpathtype")
    Renderer("renderer", "pbrssssamplingmode", "int", "lv_pbrssssamplingmode")
    Renderer("renderer", "preview", "bool", "lv_preview")
    Renderer("renderer", "previewadaptive", "bool", "lv_previewadaptive")
    Renderer("renderer", "previewadaptivefactor", "float", "lv_previewadaptivefactor")
    Renderer("renderer", "previewmode", "string", "lv_previewmode")
    Renderer("renderer", "previewmouseradius", "float", "lv_previewmouseradius")
    Renderer("renderer", "previewtime", "float", "lv_previewtime")
    Renderer("renderer", "ptexmapmaxres", "int", "lv_ptexmapmaxres")
    Renderer("renderer", "ptexmapminres", "int", "lv_ptexmapminres")
    Renderer("renderer", "ptexmapscale", "float", "lv_ptexmapscale")
    Renderer("renderer", "ptexrelativescale", "bool", "lv_ptexrelativescale")
    Renderer("renderer", "ptexscaleattr", "string", "lv_ptexscaleattr")
    Renderer("renderer", "ptexwraporient", "bool", "lv_ptexwraporient")
    Renderer("renderer", "raybias", "float", "lv_raybias")
    Renderer("renderer", "rayblurquality", "bool", "lv_rayblurquality")
    Renderer("renderer", "rayderivbias", "float", "lv_rayderivbias")
    Renderer("renderer", "rayderivrandom", "bool", "lv_rayderivrandom")
    Renderer("renderer", "raylimitcomponents", "string", "lv_raylimitcomponents")
    Renderer("renderer", "raylimiteval", "string", "lv_raylimiteval")
    Renderer("renderer", "raylimitlightmask", "string", "lv_raylimitlightmask")
    Renderer("renderer", "rayquality", "bool", "lv_rayquality")
    Renderer("renderer", "refractcomponents", "string", "lv_refractcomponents")
    Renderer("renderer", "relightingbuffer", "bool", "lv_relightingbuffer")
    Renderer("renderer", "renderengine", "string", "lv_renderengine", skipdefault=False)
    Renderer("renderer", "renderinterrupt", "bool", "lv_renderinterrupt")
    Renderer("renderer", "samplecachesize", "int", "lv_samplecachesize")
    Renderer("renderer", "shadingfactor", "float", "lv_shadingfactor")
    Renderer("renderer", "shadingqueuesize", "int", "lv_shadingqueuesize")
    Renderer("renderer", "ssscomponents", "string", "lv_ssscomponents")
    Renderer("renderer", "stylesheets", "bool", "lv_stylesheets")
    Renderer("renderer", "threadcount", "int", "lv_threadcount")
    Renderer("renderer", "tilecallback", "string", "lv_tilecallback")
    Renderer("renderer", "timelimit", "float", "lv_timelimit")
    Renderer("renderer", "unshadedlimit", "int", "lv_unshadedlimit")
    Renderer("renderer", "usecacheratio", "bool", "lv_usecacheratio")
    Renderer("renderer", "usemaxthreads", "int", "lv_usemaxthreads")
    Renderer("renderer", "uvattribute", "string", "lv_uvattribute")
    Renderer("renderer", "uvborderpixels", "int", "lv_uvborderpixels")
    Renderer("renderer", "uvmkpath", "bool", "lv_uvmkpath")
    Renderer("renderer", "uvpostprocess", "string", "lv_uvpostprocess")
    Renderer("renderer", "uvshadingquality", "float", "lv_uvshadingquality")
    Renderer("renderer", "uvtype", "string", "lv_uvtype")
    Renderer("renderer", "verbose", "int", "lv_verbose", skipdefault=False)
    Renderer("renderer", "vexprofile", "int", "lv_vexprofile")
    Renderer("renderer", "volumecomponents", "string", "lv_volumecomponents")

    # -- Image --
    Image("image", "background", "string", "lv_background")
    Image("image", "backgroundchannels", "string", "lv_backgroundchannels")
    Image("image", "backgroundmatte", "bool", "lv_backgroundmatte")
    Image("image", "batchmode", "bool", "lv_imagebatchmode")
    Image("image", "bgenable", "bool", "lv_bgenable")
    Image("image", "bgscale", "bool", "lv_bgscale")
    Image("image", "bgzscale", "float", "lv_bgzscale")
    Image("image", "bucket", "int", "lv_bucketsize")
    Image("image", "bucketthreadcolor", "bool", "lv_bucketthreadcolor")
    Image("image", "checkpointcachesize", "int", "lv_checkpointcachesize")
    Image("image", "checkpointname", "string", "lv_checkpointname")
    Image("image", "checkpointperiod", "float", "lv_checkpointperiod")
    Image("image", "colorlimit", "float", "lv_colorlimit")
    Image("image", "colorlimitdecay", "float", "lv_colorlimitdecay")
    Image("image", "colorlimitdepth", "int", "lv_colorlimitdepth")
    Image("image", "decorrelatedof", "bool", "lv_decorrelatedof")
    Image("image", "exrdatawindowpadding", "int", "lv_exrdatawindowpadding")
    Image("image", "exrdatawindowplanes", "string", "lv_exrdatawindowplanes")
    Image("image", "exrdatawindowthreshold", "float", "lv_exrdatawindowthreshold")
    Image("image", "iprbucket", "int", "lv_iprbucketsize")
    Image("image", "jitter", "float", "lv_jitter")
    Image("image", "matchdeeppixelfilter", "bool", "lv_matchdeeppixelfilter")
    Image("image", "opacitylimit", "float", "lv_opacitylimit")
    Image("image", "opacitythresh", "float", "lv_opacitythresh")
    Image("image", "overridecheckpointname", "bool", "lv_overridecheckpointname")
    Image("image", "randomseed", "int", "lv_randomseed")
    Image("image", "readcheckpoint", "bool", "lv_readcheckpoint")
    Image("image", "resolution", "int", "lv_uvunwrapres")
    Image("image", "samplelock", "bool", "lv_samplelock")
    Image("image", "samples", "int", "lv_samples", skipdefault=False)
    Image("image", "saveoptions", "string", "lv_saveoptions")
    Image("image", "setexrdatawindow", "bool", "lv_setexrdatawindow")
    Image("image", "subpixel", "bool", "lv_subpixel")
    Image("image", "transparent", "bool", "lv_transparent")
    Image("image", "transparentsamples", "int", "lv_transparentsamples")
    Image("image", "writecheckpoint", "bool", "lv_writecheckpoint")

    # -- Photon --
    Photon("photon", "photoncfile", "string", "lv_photoncfile")
    Photon("photon", "photoncount", "int", "lv_photoncount")
    Photon("photon", "photongfile", "string", "lv_photongfile")
    Photon("photon", "photonminratio", "float", "lv_photonminratio")

    # -- Camera --
    Camera("camera", "curvature", "float", "lv_curvature")
    Camera("camera", "dofaspect", "float", "lv_dofaspect")
    Camera("camera", "field", "int", "lv_field")
    Camera("camera", "shuttershape", "string", "lv_shuttershape")

    # -- Object --
    Object("name", "string", "object:name")
    Object("biasnormal", "bool", "lv_biasnormal")
    Object("bumpraydisplace", "bool", "lv_bumpraydisplace")
    Object("categories", "string", "categories")
    Object("coving", "int", "lv_coving")
    Object("curvesurface", "bool", "lv_curvesurface")
    Object("curveunitst", "bool", "lv_curveunitst")
    Object("decoupleindirect", "bool", "lv_decoupleindirect")
    Object("diffuselimit", "int", "lv_pbrdiffuselimit")
    Object("diffuselimit", "int", "lv_diffuselimit")
    Object("diffusequality", "float", "lv_diffusequality")
    Object("displacebound", "float", "lv_displacebound")
    Object("displacespace", "string", "lv_displacespace")
    Object("dorayvariance", "bool", "lv_dorayvariance")
    Object("emitillum", "bool", "lv_emitillum")
    Object("filter", "string", "lv_volumefilter")
    Object("filterwidth", "float", "lv_volumefilterwidth")
    Object("flatness", "float", "lv_flatness")
    Object("flattenprocedural", "int", "lv_flattenprocedural")
    Object("flattentoroot", "int", "lv_flattentoroot")
    Object("geofilterwidth", "float", "lv_geofilterwidth")
    Object("geometrygroup", "string", "lv_geometrygroup")
    Object("gienable", "bool", "lv_gienable")
    Object("gierror", "float", "lv_gierror")
    Object("gifile", "string", "lv_gifile")
    Object("gifilemode", "string", "lv_gifilemode")
    Object("gimaxspacing", "float", "lv_gimaxspacing")
    Object("giminspacing", "float", "lv_giminspacing")
    Object("gisample", "int", "lv_gisample")
    Object("globalquality", "float", "lv_globalquality")
    Object("heightfieldtess", "int", "lv_heightfieldtess")
    Object("indirectvariance", "float", "lv_indirectvariance")
    Object("jitcurves", "bool", "lv_jitcurves")
    Object("lightcategories", "string", "lightcategories")
    Object("lightmask", "oplist", "lightmask")
    Object("lpetag", "string", "lv_lpetag")
    Object("matte", "bool", "lv_matte")
    Object("maxindirectraysamples", "int", "lv_maxindirectraysamples")
    Object("maxraysamples", "int", "lv_maxraysamples")
    Object("metavolume", "bool", "lv_metavolume")
    Object("minindirectraysamples", "int", "lv_minindirectraysamples")
    Object("minraysamples", "int", "lv_minraysamples")
    Object("motionfactor", "float", "lv_motionfactor")
    Object("osd_fvarinterp", "int", "lv_osd_fvarinterp")
    Object("osd_quality", "float", "lv_osd_quality")
    Object("osd_vtxinterp", "int", "lv_osd_vtxinterp")
    Object("overridedetail", "bool", "lv_overridedetail")
    Object("pack_mergefragments", "bool", "lv_pack_mergefragments")
    Object("pack_refineoptions", "string", "lv_pack_refineoptions")
    Object("pack_sequencesubsteps", "int", "lv_pack_sequencesubsteps")
    Object("packinstance", "bool", "lv_packinstance")
    Object("photonmodifier", "int", "lv_photonmodifier")
    Object("pointscale", "float", "lv_pointscale")
    Object("procuseroottransform", "bool", "lv_procuseroottransform")
    Object("pscalediameter", "bool", "lv_pscalediameter")
    Object("raypredice", "int", "lv_raypredice")
    Object("rayshade", "bool", "lv_rayshade")
    Object("rayweight", "float", "lv_rayweight")
    Object("redice", "bool", "lv_redice")
    Object("reflectcategories", "string", "reflectcategories")
    Object("reflectionquality", "float", "lv_reflectionquality")
    Object("reflectlimit", "int", "lv_pbrreflectlimit")
    Object("reflectlimit", "int", "lv_reflectlimit")
    Object("reflectmask", "oplist", "reflectmask")
    Object("refractcategories", "string", "refractcategories")
    Object("refractionquality", "float", "lv_refractionquality")
    Object("refractlimit", "int", "lv_pbrrefractlimit")
    Object("refractlimit", "int", "lv_refractlimit")
    Object("refractmask", "oplist", "refractmask")
    Object("renderable", "bool", "lv_renderable")
    Object("renderpoints", "int", "lv_renderpoints")
    Object("renderpointsas", "int", "lv_renderpointsas")
    Object("rendersubd", "bool", "lv_rendersubd")
    Object("rendersubdcurves", "bool", "lv_rendersubdcurves")
    Object("rendervisibility", "string", "lv_rendervisibility")
    Object("rmbackface", "bool", "lv_rmbackface")
    Object("samplecoving", "bool", "lv_samplecoving")
    Object("samplecovingexpansion", "float", "lv_samplecovingexpansion")
    Object("samplingquality", "float", "lv_samplingquality")
    Object("shadingquality", "float", "lv_shadingquality")
    Object("sharedisplace", "bool", "lv_sharedisplace")
    Object("smoothP", "bool", "lv_smoothP")
    Object("smoothcolor", "bool", "lv_smoothcolor")
    Object("ssslimit", "int", "lv_ssslimit")
    Object("sssquality", "float", "lv_sssquality")
    Object("subdgroup", "string", "lv_subdgroup")
    Object("subdstyle", "string", "lv_subdstyle")
    Object("traceblur", "bool", "lv_traceblur")
    Object("truedisplace", "bool", "lv_truedisplace")
    Object("usenforpoints", "bool", "lv_usenforpoints")
    Object("variance", "float", "lv_variance")
    Object("velocitybound", "float", "lv_velocitybound")
    Object("velocityquality", "float", "lv_velocityquality")
    Object("volumedensity", "float", "lv_volumedensity")
    Object("volumedpdzfov", "bool", "lv_volumedpdzfov")
    Object("volumeiso", "bool", "lv_volumeiso")
    Object("volumeisodefault", "bool", "lv_volumeisodefault")
    Object("volumelimit", "int", "lv_volumelimit")
    Object("volumenormal", "bool", "lv_volumenormal")
    Object("volumequality", "float", "lv_volumequality")
    Object("volumesamples", "int", "lv_volumesamples")
    Object("volumesamplingchannel", "string", "lv_volumesamplingchannel")
    Object("volumeshadowsteprate", "float", "lv_volumeshadowsteprate")
    Object("volumesteprate", "float", "lv_volumesteprate")
    Object("volumesurface", "bool", "lv_volumesurface")
    Object("volumeuniform", "bool", "lv_volumeuniform")
    Object("volumeuniformflipthresh", "float", "lv_volumeuniformflipthresh")
    Object("volumeuniformtype", "int", "lv_volumeuniformtype")
    Object("volumeupvector", "float", "lv_volumeupvector")
    Object("volumezerothresh", "float", "lv_volumezerothresh")

    # -- Light --
    Light("__nondiffuse", "bool", "lv_nondiffuse")
    Light("__nonspecular", "bool", "lv_nonspecular")
    
    Light("type", "string", "lv_light_type")
    
    Light("activeradius", "float", "lv_activeradius")
    Light("areafullsphere", "bool", "lv_areafullsphere")
    Light("areamap", "string", "lv_areamap")
    Light("areashape", "string", "lv_areashape")
    Light("areasize", "float", "lv_areasize")
    Light("envangle", "float", "lv_envangle")
    Light("envintensity", "float", "lv_envintensity")
    Light("importancelevels", "int", "lv_importancelevels")
    Light("lightcache", "bool", "lv_lightcache")
    Light("photontarget", "string", "lv_photontarget")
    Light("photonweight", "float", "lv_photonweight")
    Light("raybackground", "bool", "lv_raybackground")
    Light("shadowcategories", "string", "shadowcategories")
    Light("shadowmask", "oplist", "shadowmask")

    # -- Fog --
    Fog("name", "string", "object:name")
    Fog("categories", "string", "categories")
    Fog("lightcategories", "string", "lightcategories")
    Fog("lightmask", "oplist", "lightmask")

    # -- Geometry --
    Geometry("computeN", "bool", "lv_computeN")
    Geometry("segmentattrs", "string", "lv_segmentattrs")
    Geometry("timescale", "float", "lv_motiontimescale")
    Geometry("volumevelocitynames", "string", "lv_volumevelocitynames")

    # -- Deep --
    Deep("shadow", "OpenEXR.attributes", "string", "lv_image_exr_attributes")
    Deep("shadow", "compositing", "bool", "lv_dsmcompositing")
    Deep("shadow", "deepcompression", "int", "lv_dsmcompression")
    Deep("shadow", "depth_interp", "string", "lv_dsmdepthinterp")
    Deep("shadow", "depth_mode", "string", "lv_dsmdepthmode")
    Deep("shadow", "depth_planes", "string", "lv_dsmdepthplanes")
    Deep("shadow", "filename", "string", "lv_dsmfilename", ['$HIP/dsm.rat'])
    Deep("shadow", "mipmaps", "bool", "lv_dsmmipmaps")
    Deep("shadow", "ofsize", "int", "lv_dsmofsize")
    Deep("shadow", "ofstorage", "string", "lv_dsmofstorage")
    Deep("shadow", "pzstorage", "string", "lv_dsmpzstorage")
    Deep("shadow", "zbias", "float", "lv_dsmzbias")

    # -- Deep --
    Deep("camera", "OpenEXR.attributes", "string", "lv_image_exr_attributes")
    Deep("camera", "compositing", "bool", "lv_dcmcompositing")
    Deep("camera", "deepcompression", "int", "lv_dcmcompression")
    Deep("camera", "depth_interp", "string", "lv_dcmdepthinterp")
    Deep("camera", "depth_mode", "string", "lv_dcmdepthmode")
    Deep("camera", "depth_planes", "string", "lv_dcmdepthplanes")
    Deep("camera", "filename", "string", "lv_dcmfilename", ['$HIP/dcm.rat'])
    Deep("camera", "mipmaps", "bool", "lv_dcmmipmaps")
    Deep("camera", "ofsize", "int", "lv_dcmofsize")
    Deep("camera", "ofstorage", "string", "lv_dcmofstorage")
    Deep("camera", "pzstorage", "string", "lv_dcmpzstorage")
    Deep("camera", "zbias", "float", "lv_dcmzbias")

    # -- Measure --
    Measure("nonraster", "offscreenquality", "float", "lv_measureoffscreenquality", skipdefault=False)
    Measure("nonraster", "zimportance", "float", "lv_measurezimportance", skipdefault=False)

    # -- Bokeh --
    Bokeh("file", "-f", "string", "lv_bokehfile")
    Bokeh("file", "-r", "float", "lv_bokehrotation")

    # -- IPlane --
    IPlane("dither", "float", "lv_dither")
    IPlane("gain", "float", "lv_gain")
    IPlane("gamma", "float", "lv_gamma")
    IPlane("lightexport", "string", "lv_lightexport", mainimage=False)
    IPlane("pfilter", "string", "lv_pfilter")
    IPlane("quantize", "string", "lv_quantize")
    IPlane("sfilter", "string", "lv_sfilter")
    IPlane("whitepoint", "float", "lv_whitepoint")

    # -- ImageOption --
    ImageOption("Cineon.cingamma", "float", "lv_image_cineon_gamma")
    ImageOption("Cineon.cinlut", "string", "lv_image_cineon_lut")
    ImageOption("Cineon.cinspace", "string", "lv_image_cineon_space")
    ImageOption("Cineon.cinwhite", "int", "lv_image_cineon_whitepoint")
    ImageOption("IPlay.bgimage", "string", "lv_image_mplay_bgimage")
    ImageOption("IPlay.direction", "string", "lv_image_mplay_direction")
    ImageOption("IPlay.gamma", "float", "lv_image_mplay_gamma")
    ImageOption("IPlay.houdiniportnum", "int", "mplay:port")
    ImageOption("IPlay.label", "string", "lv_image_mplay_label")
    ImageOption("IPlay.lut", "string", "lv_image_mplay_lut")
    ImageOption("IPlay.sockethost", "string", "lv_image_mplay_sockethost")
    ImageOption("IPlay.socketport", "int", "lv_image_mplay_socketport")
    ImageOption("JPEG.quality", "int", "lv_image_jpeg_quality")
    ImageOption("OpenEXR.attributes", "string", "lv_image_exr_attributes")
    ImageOption("OpenEXR.compression", "string", "lv_image_exr_compression")
    ImageOption("OpenEXR.dwa_compression", "float", "lv_image_exr_dwa_level")
    ImageOption("OpenEXR.enablepostprocess", "bool", "lv_image_exr_postprocess")
    ImageOption("OpenEXR.storage", "string", "lv_image_exr_storage")
    ImageOption("OpenEXR.tilex", "int", "lv_image_exr_tilex")
    ImageOption("OpenEXR.tiley", "int", "lv_image_exr_tiley")
    ImageOption("PNG.frompremult", "string", "lv_image_png_frompremult")
    ImageOption("RAT.color", "string", "lv_image_rat_color")
    ImageOption("RAT.compression", "string", "lv_image_rat_compression")
    ImageOption("RAT.makemips", "bool", "lv_image_rat_makemips")
    ImageOption("TIFF.compression", "string", "lv_image_tiff_compression")
    ImageOption("TIFF.predictor", "string", "lv_image_tiff_predictor")
    ImageOption("artist", "string", "lv_image_artist")
    ImageOption("comment", "string", "lv_image_comment")
    ImageOption("hostname", "string", "lv_image_hostname")

