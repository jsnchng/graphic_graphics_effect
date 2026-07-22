/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef GRAPHICS_EFFECT_GE_VISUAL_EFFECT_IMPL_H
#define GRAPHICS_EFFECT_GE_VISUAL_EFFECT_IMPL_H

#include <array>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "common/rs_vector2.h"
#include "common/rs_vector3.h"
#include "common/rs_vector4.h"
#include "effect/color_filter.h"
#include "effect/runtime_effect.h"
#include "effect/runtime_shader_builder.h"
#include "ge_filter_params.h"
#include "ge_filter_type.h"
#include "ge_log.h"
#include "ge_params_reflection.h"
#include "ge_shader.h"
#include "ge_shader_filter.h"
#include "ge_shader_filter_params.h"
#include "ge_visual_effect.h"

#include "utils/rect.h"

namespace OHOS {
namespace Rosen {
namespace Drawing {

/**
 * GEVisualEffectImpl v2 - Refactored implementation using GEFilterParams variant.
 *
 * Key differences from v1:
 * 1. Uses single `params_` member (type-erased GEFilterParams) instead of 60+ individual params
 * 2. SetParam uses reflection metadata (GEParamsMemberTag) for type-safe field access
 * 3. String-based SetParam uses generated GEParamsMemberTagFromString mapping
 */
class GE_EXPORT GEVisualEffectImpl {
public:
    // Type alias for compatiblilty. Many old call sites use GEVisualEffectImpl::FilterType
    using FilterType = GEFilterType;

    GEVisualEffectImpl(const std::string& name, const std::optional<Drawing::CanvasInfo>& canvasInfo = std::nullopt);
    ~GEVisualEffectImpl();

    // ========================================================================
    // Main SetParam overloads using reflection
    // ========================================================================

    /// Set parameter by tag enum (type-safe, recommended)
    template<typename T>
    void SetParam(GEParamsMemberTag tag, T value)
    {
        if (!params_) {
            GE_LOGE("GEVisualEffectImpl::SetParam: params_ is null");
            return;
        }

        // Check if tag is valid for current params type
        if (!IsTagValidForCurrentType(*params_, tag)) {
            GE_LOGE(
                "GEVisualEffectImpl::SetParam: tag %u not valid for current params type", static_cast<uint32_t>(tag));
            return;
        }

        // Use generated helper to set parameter by tag
        SetParamInternal(*params_, tag, value);
    }

    /// Set parameter by string tag (runtime lookup to enum, then dispatch)
    template<typename T>
    void SetParam(const std::string& tagStr, T value)
    {
        auto tag = GEParamsMemberHelper::GEParamsMemberTagFromString(tagStr);
        if (tag != GEParamsMemberTag::INVALID) {
            SetParam(tag, value);
        } else {
            // 2D effects may use string-based tags that are not in the enum for their specific use only
            // Log and ignore unknown tags
            GE_LOGD("GEVisualEffectImpl::SetParam: unknown tag '%s'", tagStr.c_str());
        }
    }

    // ========================================================================
    // Filter type management
    // ========================================================================
    // Not recommended to use
    void SetFilterType(FilterType type)
    {
        filterType_ = type;
    }

    const FilterType& GetFilterType() const
    {
        return filterType_;
    }

    // ========================================================================
    // Params management - using GEFilterParams variant
    // ========================================================================

    /// Create params of specific type and store in params_
    template<typename ParamsType>
    void MakeParams()
    {
        params_ = GEFilterParams::Box(std::make_shared<ParamsType>());
    }

    template<typename ParamsType>
    std::shared_ptr<ParamsType> GetParams() const
    {
        if (!params_) {
            return nullptr;
        }
        return GEFilterParams::Unbox<ParamsType>(*params_);
    }

    // ========================================================================
    // CanvasInfo management - Screen canvas geometry info
    // ========================================================================

    void SetCanvasInfo(Drawing::CanvasInfo info)
    {
        canvasInfo_ = info;
    }

    const Drawing::CanvasInfo& GetCanvasInfo() const
    {
        return canvasInfo_;
    }

    // ========================================================================
    // Per-effect cache management
    // ========================================================================

    void SetCache(std::shared_ptr<std::any> cacheData)
    {
        cacheAnyPtr_ = cacheData;
    }

    std::shared_ptr<std::any> GetCache() const
    {
        return cacheAnyPtr_;
    }

    // ========================================================================
    // Backward-compatible convenience methods
    // Warning: New types should use MakeParams<T> / GetParams<T> directly
    // ========================================================================

#define GE_DECLARE_MAKE_GET_PARAMS(MethodName, ParamsType) \
    void Make##MethodName()                                \
    {                                                      \
        MakeParams<ParamsType>();                          \
    }                                                      \
    auto Get##MethodName() const                           \
    {                                                      \
        return GetParams<ParamsType>();                    \
    }

    GE_DECLARE_MAKE_GET_PARAMS(MESAParams, GEMESABlurShaderFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(KawaseParams, GEKawaseBlurShaderFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(AIBarParams, GEAIBarShaderFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(GreyParams, GEGreyShaderFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(WaterRippleParams, GEWaterRippleFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(LinearGradientBlurParams, GELinearGradientBlurShaderFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(MagnifierParams, GEMagnifierShaderFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(ColorGradientParams, GEColorGradientShaderFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(DisplacementDistortParams, GEDisplacementDistortFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(SoundWaveParams, GESoundWaveFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(HeatDistortionParams, GEHeatDistortionFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(BlurBubblesRiseParams, GEBlurBubblesRiseFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(EdgeLightParams, GEEdgeLightShaderFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(BezierWarpParams, GEBezierWarpShaderFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(DispersionParams, GEDispersionShaderFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(DirectionLightParams, GEDirectionLightShaderFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(ContentLightParams, GEContentLightFilterParams)
    // intentional typo `Conten` instead of `Content` due to compatibility
    GE_DECLARE_MAKE_GET_PARAMS(ContenDiagonalParams, GEContentDiagonalFlowLightShaderParams)
    GE_DECLARE_MAKE_GET_PARAMS(DotMatrixShaderParams, GEDotMatrixShaderParams)
    GE_DECLARE_MAKE_GET_PARAMS(WavyRippleLightParams, GEWavyRippleLightShaderParams)
    GE_DECLARE_MAKE_GET_PARAMS(AuroraNoiseParams, GEAuroraNoiseShaderParams)
    GE_DECLARE_MAKE_GET_PARAMS(ParticleCircularHaloParams, GEParticleCircularHaloShaderParams)
    GE_DECLARE_MAKE_GET_PARAMS(VariableRadiusBlurParams, GEVariableRadiusBlurShaderFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(SDFEdgeLightParams, GESDFEdgeLightFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(SdfFromImageParams, GESDFFromImageFilterParams) // because original decl use 'Sdf'
    GE_DECLARE_MAKE_GET_PARAMS(SDFBorderShaderParams, GESDFBorderShaderParams)
    GE_DECLARE_MAKE_GET_PARAMS(SDFShadowShaderParams, GESDFShadowShaderParams)
    GE_DECLARE_MAKE_GET_PARAMS(RippleMaskParams, GERippleShaderMaskParams)
    GE_DECLARE_MAKE_GET_PARAMS(DoubleRippleMaskParams, GEDoubleRippleShaderMaskParams)
    GE_DECLARE_MAKE_GET_PARAMS(WaveDisturbanceMaskParams, GEWaveDisturbanceShaderMaskParams)
    GE_DECLARE_MAKE_GET_PARAMS(RadialGradientMaskParams, GERadialGradientShaderMaskParams)
    GE_DECLARE_MAKE_GET_PARAMS(PixelMapMaskParams, GEPixelMapMaskParams)
    GE_DECLARE_MAKE_GET_PARAMS(ImageMaskParams, GEImageMaskParams)
    GE_DECLARE_MAKE_GET_PARAMS(UseEffectMaskParams, GEUseEffectMaskParams)
    GE_DECLARE_MAKE_GET_PARAMS(WaveGradientMaskParams, GEWaveGradientShaderMaskParams)
    GE_DECLARE_MAKE_GET_PARAMS(MaskTransitionParams, GEMaskTransitionShaderFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(SDFUnionOpShapeParams, GESDFUnionOpShapeParams)
    GE_DECLARE_MAKE_GET_PARAMS(SDFSubOpShapeParams, GESDFSubOpShapeParams)
    GE_DECLARE_MAKE_GET_PARAMS(SDFSmoothSubOpShapeParams, GESDFSmoothSubOpShapeParams)
    GE_DECLARE_MAKE_GET_PARAMS(SDFRRectShapeParams, GESDFRRectShapeParams)
    GE_DECLARE_MAKE_GET_PARAMS(SDFPathShapeParams, GESDFPathShapeParams)
    GE_DECLARE_MAKE_GET_PARAMS(SDFPixelmapShapeParams, GESDFPixelmapShapeParams)
    GE_DECLARE_MAKE_GET_PARAMS(SDFTransformShapeParams, GESDFTransformShapeParams)
    GE_DECLARE_MAKE_GET_PARAMS(SDFClipParams, GESDFClipShaderParams)
    GE_DECLARE_MAKE_GET_PARAMS(SDFClipShaderParams, GESDFClipShaderParams)
    GE_DECLARE_MAKE_GET_PARAMS(SDFColorParams, GESDFColorShaderParams)
    GE_DECLARE_MAKE_GET_PARAMS(SDFColorShaderParams, GESDFColorShaderParams)
    GE_DECLARE_MAKE_GET_PARAMS(LightCaveParams, GEXLightCaveShaderParams)
    GE_DECLARE_MAKE_GET_PARAMS(DistortChromaParams, GEXDistortChromaEffectParams)
    GE_DECLARE_MAKE_GET_PARAMS(DupoliNoiseMaskParams, GEXDupoliNoiseMaskParams)
    GE_DECLARE_MAKE_GET_PARAMS(BorderLightParams, GEBorderLightShaderParams)
    GE_DECLARE_MAKE_GET_PARAMS(GasifyScaleTwistFilterParams, GEGasifyScaleTwistFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(GasifyBlurFilterParams, GEGasifyBlurFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(GasifyFilterParams, GEGasifyFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(ColorGradientEffectParams, GEXColorGradientEffectParams)
    GE_DECLARE_MAKE_GET_PARAMS(HarmoniumEffectParams, GEHarmoniumEffectShaderParams)
    GE_DECLARE_MAKE_GET_PARAMS(HaloEffectParams, GEHaloEffectShaderParams)
    GE_DECLARE_MAKE_GET_PARAMS(AIBarGlowEffectParams, GEXAIBarGlowEffectParams)
    GE_DECLARE_MAKE_GET_PARAMS(AIBarRectHaloEffectParams, GEXAIBarRectHaloEffectParams)
    GE_DECLARE_MAKE_GET_PARAMS(RoundedRectFlowlightEffectParams, GEXRoundedRectFlowlightEffectParams)
    GE_DECLARE_MAKE_GET_PARAMS(GradientFlowColorsEffectParams, GEXGradientFlowColorsEffectParams)
    GE_DECLARE_MAKE_GET_PARAMS(FrameGradientMaskParams, GEFrameGradientMaskParams)
    GE_DECLARE_MAKE_GET_PARAMS(CircleFlowlightEffectParams, GECircleFlowlightEffectParams)
    GE_DECLARE_MAKE_GET_PARAMS(FrostedGlassEffectParams, GEFrostedGlassEffectParams)
    GE_DECLARE_MAKE_GET_PARAMS(FrostedGlassBlurParams, GEFrostedGlassBlurShaderFilterParams);
    GE_DECLARE_MAKE_GET_PARAMS(FrostedGlassParams, GEFrostedGlassShaderFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(GridWarpFilterParams, GEGridWarpShaderFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(NoisyFrameGradientMaskParams, GEXNoisyFrameGradientMaskParams)
    GE_DECLARE_MAKE_GET_PARAMS(SDFEdgeLightEffectParams, GESDFEdgeLightEffectParams)
    GE_DECLARE_MAKE_GET_PARAMS(SDFTriangleShapeParams, GESDFTriangleShapeParams)
    GE_DECLARE_MAKE_GET_PARAMS(SDFEllipseShapeParams, GESDFEllipseShapeParams)
    GE_DECLARE_MAKE_GET_PARAMS(SDFDistortOpShapeParams, GESDFDistortOpShapeParams)
    GE_DECLARE_MAKE_GET_PARAMS(DistortionCollapseParams, GEDistortionCollapseFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(ParticleAblationFilterParams, GEParticleAblationFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(DepthOcclusionParams, GEDepthOcclusionShaderFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(SpatialGlassEffectParams, GESpatialGlassEffectParams)
    GE_DECLARE_MAKE_GET_PARAMS(MotionBlurParams, GEMotionBlurShaderFilterParams)
    GE_DECLARE_MAKE_GET_PARAMS(BlurShaderParams, GEBlurShaderFilterParams)
#undef GE_DECLARE_MAKE_GET_PARAMS

    // ========================================================================
    // Backward-compatible dirty methods
    // Warning: DO NOT ADD NEW METHODS or MODIFY EXISTING METHODS unless you have to
    //          These methods should be removed in the future
    // ========================================================================

    // Introduced by SDF Union Op
    void MakeSDFUnionOpShapeParams(const GESDFUnionOp& op);
    // Introduced by Frosted Glass
    const std::shared_ptr<Drawing::GEShaderShape> GetGEShaderShape(const std::string& tag) const;
    // Introduced by SDF Edge Light
    void SetSDFEdgeLightParams(const std::string& tag, float param);

    // 'Prams' Typo compatible: remove this method after fix all call sites including 2d dependencies
    void MakeSdfEdgeLightPrams()
    {
        MakeSDFEdgeLightParams();
    }

private:
    /// Check if tag belongs to current params type's tag range
    static bool IsTagValidForCurrentType(const GEFilterParams& params, GEParamsMemberTag tag)
    {
        auto tagType = GEParamsMemberHelper::GetFilterTypeFromTag(tag);
        return tagType == params.GetType();
    }

    /// Internal set param implementation - uses reflection metadata to set field value
    /// Non-template overloads for each type, generated from FOR_EACH_PARAM_TYPE
    /// Implementations in ge_visual_effect_impl.cpp
#define DECLARE_SET_PARAM_INTERNAL(Type) \
    void SetParamInternal(GEFilterParams& params, GEParamsMemberTag tag, const Type& value);

    FOR_EACH_PARAM_TYPE(DECLARE_SET_PARAM_INTERNAL)
#undef DECLARE_SET_PARAM_INTERNAL

    // Compatibility placeholders, remove these placeholders if future effects use them
    void SetParamInternal(GEFilterParams&, GEParamsMemberTag, const long&);
    void SetParamInternal(GEFilterParams&, GEParamsMemberTag, const long long&);
    void SetParamInternal(GEFilterParams&, GEParamsMemberTag, const double&);
    void SetParamInternal(GEFilterParams&, GEParamsMemberTag, const std::shared_ptr<Drawing::ColorFilter>);

private:
    FilterType filterType_ = FilterType::NONE;
    Drawing::CanvasInfo canvasInfo_;
    std::shared_ptr<std::any> cacheAnyPtr_ = nullptr;
    std::unique_ptr<GEFilterParams> params_ = nullptr;
};

} // namespace Drawing
} // namespace Rosen
} // namespace OHOS

#endif // GRAPHICS_EFFECT_GE_VISUAL_EFFECT_IMPL_H
