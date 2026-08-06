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

#include <memory>

#include "core/ge_effect_factory.h"
#include "core/ge_visual_effect_impl.h"
#include "effect/filter/ge_aibar_shader_filter.h"
#include "effect/filter/ge_bezier_warp_shader_filter.h"
#include "effect/filter/ge_blur_bubbles_rise_filter.h"
#include "effect/filter/ge_blur_shader_filter.h"
#include "effect/filter/ge_color_gradient_shader_filter.h"
#include "effect/filter/ge_content_light_shader_filter.h"
#include "effect/filter/ge_depth_occlusion_shader_filter.h"
#include "effect/filter/ge_direction_light_shader_filter.h"
#include "effect/filter/ge_dispersion_shader_filter.h"
#include "effect/filter/ge_displacement_distort_shader_filter.h"
#include "effect/filter/ge_distortion_collapse_filter.h"
#include "effect/filter/ge_edge_light_shader_filter.h"
#include "effect/filter/ge_frosted_glass_blur_shader_filter.h"
#include "effect/filter/ge_frosted_glass_shader_filter.h"
#include "effect/filter/ge_grey_shader_filter.h"
#include "effect/filter/ge_grid_warp_shader_filter.h"
#include "effect/filter/ge_heat_distortion_filter.h"
#include "effect/filter/ge_kawase_blur_shader_filter.h"
#include "effect/filter/ge_linear_gradient_blur_shader_filter.h"
#include "effect/filter/ge_magnifier_shader_filter.h"
#include "effect/filter/ge_mask_transition_shader_filter.h"
#include "effect/filter/ge_mesa_blur_shader_filter.h"
#include "effect/filter/ge_motion_blur_shader_filter.h"
#include "effect/filter/ge_sdf_edge_light.h"
#include "effect/filter/ge_sdf_from_image_filter.h"
#include "effect/filter/ge_sound_wave_filter.h"
#include "effect/filter/ge_variable_radius_blur_shader_filter.h"
#include "effect/filter/ge_water_ripple_filter.h"
#include "effect/ge_shader_filter_params.h"
#include "effect/mask/ge_double_ripple_shader_mask.h"
#include "effect/mask/ge_frame_gradient_shader_mask.h"
#include "effect/mask/ge_image_shader_mask.h"
#include "effect/mask/ge_linear_gradient_shader_mask.h"
#include "effect/mask/ge_pixel_map_shader_mask.h"
#include "effect/mask/ge_radial_gradient_shader_mask.h"
#include "effect/mask/ge_ripple_shader_mask.h"
#include "effect/mask/ge_use_effect_shader_mask.h"
#include "effect/mask/ge_wave_disturb_shader_mask.h"
#include "effect/mask/ge_wave_gradient_shader_mask.h"
#include "effect/shader/ge_aurora_noise_shader.h"
#include "effect/shader/ge_border_light_shader.h"
#include "effect/shader/ge_border_sdf_lg_color_shader.h"
#include "effect/shader/ge_border_sdf_shader.h"
#include "effect/shader/ge_circle_flowlight_effect.h"
#include "effect/shader/ge_color_gradient_effect.h"
#include "effect/shader/ge_contour_diagonal_flow_light_shader.h"
#include "effect/shader/ge_frosted_glass_effect.h"
#include "effect/shader/ge_particle_circular_halo_shader.h"
#include "effect/shader/ge_sdf_edge_light_shader.h"
#include "effect/shader/ge_spatial_glass_effect.h"
#include "effect/shader/ge_spatial_point_light.h"
#include "effect/shader/ge_wavy_ripple_light_shader.h"
#include "effect/shader/gex_dot_matrix_shader_params.h"
#include "effect/shape/ge_sdf_border_shader.h"
#include "effect/shape/ge_sdf_clip_shader.h"
#include "effect/shape/ge_sdf_color_shader.h"
#include "effect/shape/ge_sdf_distort_op_shader_shape.h"
#include "effect/shape/ge_sdf_empty_shader_shape.h"
#include "effect/shape/ge_sdf_ellipse_shader_shape.h"
#include "effect/shape/ge_sdf_path_shader_shape.h"
#include "effect/shape/ge_sdf_pixelmap_shader_shape.h"
#include "effect/shape/ge_sdf_rrect_shader_shape.h"
#include "effect/shape/ge_sdf_sub_op_shader_shape.h"
#include "effect/shape/ge_sdf_shadow_shader.h"
#include "effect/shape/ge_sdf_transform_shader_shape.h"
#include "effect/shape/ge_sdf_triangle_shader_shape.h"
#include "effect/shape/ge_sdf_union_op_shader_shape.h"
#include "ext/ge_external_dynamic_loader.h"
#include "ge_system_properties.h"
#include "pipeline/ge_render.h"

namespace OHOS {
namespace GraphicsEffectEngine {
namespace {

/*
 * 1. Filter effects
 *    1.1 Built-in effects
 */
GE_FACTORY_REGISTER(GEAIBarShaderFilter)
GE_FACTORY_REGISTER(GEBezierWarpShaderFilter)
GE_FACTORY_REGISTER(GEBlurBubblesRiseFilter)
GE_FACTORY_REGISTER(GEBlurShaderFilter)
GE_FACTORY_REGISTER(GEColorGradientShaderFilter)
GE_FACTORY_REGISTER(GEContentLightFilter)
GE_FACTORY_REGISTER(GEDepthOcclusionShaderFilter)
GE_FACTORY_REGISTER(GEDirectionLightShaderFilter)
GE_FACTORY_REGISTER(GEDisplacementDistortFilter)
GE_FACTORY_REGISTER(GEDistortionCollapseFilter)
GE_FACTORY_REGISTER(GEGridWarpShaderFilter)
GE_FACTORY_REGISTER(GEGreyShaderFilter)
GE_FACTORY_REGISTER(GEHeatDistortionFilter)
GE_FACTORY_REGISTER(GEMagnifierShaderFilter)
GE_FACTORY_REGISTER(GEMaskTransitionShaderFilter)
GE_FACTORY_REGISTER(GEMotionBlurShaderFilter)
GE_FACTORY_REGISTER(GESDFFromImageFilter)
GE_FACTORY_REGISTER(GESDFEdgeLight)
GE_FACTORY_REGISTER(GESoundWaveFilter)
GE_FACTORY_REGISTER(GEWaterRippleFilter)

// 1.2 Dynamically loaded effects
GE_FACTORY_REGISTER_EXTERNAL(GASIFY, ::OHOS::Rosen::Drawing::GEGasifyFilterParams)
GE_FACTORY_REGISTER_EXTERNAL(GASIFY_BLUR, ::OHOS::Rosen::Drawing::GEGasifyBlurFilterParams)
GE_FACTORY_REGISTER_EXTERNAL(GASIFY_SCALE_TWIST, ::OHOS::Rosen::Drawing::GEGasifyScaleTwistFilterParams)
GE_FACTORY_REGISTER_EXTERNAL(MAP_COLOR_BY_BRIGHTNESS, ::OHOS::Rosen::Drawing::GEMapColorByBrightnessFilterParams)
GE_FACTORY_REGISTER_EXTERNAL(PARTICLE_ABLATION, ::OHOS::Rosen::Drawing::GEParticleAblationFilterParams)
GE_FACTORY_REGISTER_EXTERNAL(WATER_DROPLET_TRANSITION, ::OHOS::Rosen::Drawing::GEWaterDropletTransitionFilterParams)

// 1.3 Dynamically loaded effects with fallback
GE_FACTORY_REGISTER_EXTERNAL_FALLBACK(DISPERSION,
    ::OHOS::Rosen::Drawing::GEDispersionShaderFilterParams,
    ::OHOS::Rosen::GEDispersionShaderFilter)

GE_FACTORY_REGISTER_EXTERNAL_FALLBACK(EDGE_LIGHT,
    ::OHOS::Rosen::Drawing::GEEdgeLightShaderFilterParams,
    ::OHOS::Rosen::GEEdgeLightShaderFilter)

GE_FACTORY_REGISTER_EXTERNAL_FALLBACK(FROSTED_GLASS,
    ::OHOS::Rosen::Drawing::GEFrostedGlassShaderFilterParams,
    ::OHOS::Rosen::GEFrostedGlassShaderFilter)

GE_FACTORY_REGISTER_EXTERNAL_FALLBACK(FROSTED_GLASS_BLUR,
    ::OHOS::Rosen::Drawing::GEFrostedGlassBlurShaderFilterParams,
    ::OHOS::Rosen::GEFrostedGlassBlurShaderFilter)

GE_FACTORY_REGISTER_EXTERNAL_FALLBACK(MESA_BLUR,
    ::OHOS::Rosen::Drawing::GEMESABlurShaderFilterParams,
    ::OHOS::Rosen::GEMESABlurShaderFilter)

GE_FACTORY_REGISTER_EXTERNAL_FALLBACK(VARIABLE_RADIUS_BLUR,
    ::OHOS::Rosen::Drawing::GEVariableRadiusBlurShaderFilterParams,
    ::OHOS::Rosen::GEVariableRadiusBlurShaderFilter)

// 1.4 Custom registration
GE_FACTORY_REGISTER_CUSTOM(KAWASE_BLUR,
    [](GEEffectFactory::VisualEffectImplPtr ve) -> std::shared_ptr<::OHOS::Rosen::Drawing::IGEFilterType> {
        if (FactoryCheckNullptr(ve.get(), "KAWASE_BLUR: VisualEffectImpl")) {
            return nullptr;
        }
        auto params = ve->template GetParams<::OHOS::Rosen::Drawing::GEKawaseBlurShaderFilterParams>();
        if (FactoryCheckNullptr(params.get(), "KAWASE_BLUR GetParams")) {
            return nullptr;
        }
        if (!GERender::IsMesablurAllEnabled()) {
            return std::make_shared<::OHOS::Rosen::GEKawaseBlurShaderFilter>(*params);
        }
        auto mesaParams = std::make_shared<::OHOS::Rosen::Drawing::GEMESABlurShaderFilterParams>();
        mesaParams->radius = params->radius;
        uint32_t type = static_cast<uint32_t>(::OHOS::Rosen::Drawing::GEFilterType::MESA_BLUR);
        void* impl = ::OHOS::Rosen::GEExternalDynamicLoader::GetInstance().CreateGEXObjectByType(
            type, sizeof(::OHOS::Rosen::Drawing::GEMESABlurShaderFilterParams), mesaParams.get());
        if (impl) {
            GE_LOGD("[GEEffectFactory] KAWASE_BLUR: Using MESA blur via external loader");
            return std::shared_ptr<::OHOS::Rosen::Drawing::IGEFilterType>(
                static_cast<::OHOS::Rosen::Drawing::IGEFilterType*>(impl));
        }
        GE_LOGW("[GEEffectFactory] KAWASE_BLUR: External loader failed, using built-in MESA blur");
        return std::make_shared<::OHOS::Rosen::GEMESABlurShaderFilter>(*mesaParams);
    })

GE_FACTORY_REGISTER_CUSTOM(LINEAR_GRADIENT_BLUR,
    [](GEEffectFactory::VisualEffectImplPtr ve) -> std::shared_ptr<::OHOS::Rosen::Drawing::IGEFilterType> {
        if (FactoryCheckNullptr(ve.get(), "LINEAR_GRADIENT_BLUR: VisualEffectImpl")) {
            return nullptr;
        }
        auto params = ve->template GetParams<::OHOS::Rosen::Drawing::GELinearGradientBlurShaderFilterParams>();
        if (FactoryCheckNullptr(params.get(), "LINEAR_GRADIENT_BLUR: GetParams")) {
            return nullptr;
        }
        if (params->isRadiusGradient) {
            uint32_t type = static_cast<uint32_t>(::OHOS::Rosen::Drawing::GEFilterType::LINEAR_GRADIENT_BLUR);
            void* impl = ::OHOS::Rosen::GEExternalDynamicLoader::GetInstance().CreateGEXObjectByType(
                type, sizeof(::OHOS::Rosen::Drawing::GELinearGradientBlurShaderFilterParams),
                static_cast<void*>(params.get()));
            if (impl) {
                GE_LOGD("[GEEffectFactory] LINEAR_GRADIENT_BLUR: Using external loader (isRadiusGradient=true)");
                return std::shared_ptr<::OHOS::Rosen::Drawing::IGEFilterType>(
                    static_cast<::OHOS::Rosen::Drawing::IGEFilterType*>(impl));
            }
            GE_LOGW("[GEEffectFactory] LINEAR_GRADIENT_BLUR: External loader failed, using built-in");
        }
        return std::make_shared<::OHOS::Rosen::GELinearGradientBlurShaderFilter>(*params);
    })


/*
 * 2. Shader effects
 *    2.1 Built-in effects
 */
GE_FACTORY_REGISTER(GEAuroraNoiseShader)
GE_FACTORY_REGISTER(GEBorderLightShader)
GE_FACTORY_REGISTER(GEBorderSDFShader)
GE_FACTORY_REGISTER(GEBorderSDFLGColorShader)
GE_FACTORY_REGISTER(GECircleFlowlightEffect)
GE_FACTORY_REGISTER(GEContourDiagonalFlowLightShader)
GE_FACTORY_REGISTER(GESDFBorderShader)
GE_FACTORY_REGISTER(GESDFClipShader)
GE_FACTORY_REGISTER(GESDFColorShader)
GE_FACTORY_REGISTER(GESDFEdgeLightShader)
GE_FACTORY_REGISTER(GESDFShadowShader)
GE_FACTORY_REGISTER(GEParticleCircularHaloShader)
GE_FACTORY_REGISTER(GESpatialPointLightShader)
GE_FACTORY_REGISTER(GEWavyRippleLightShader)

// 2.2 Dynamically loaded effects
GE_FACTORY_REGISTER_EXTERNAL(AIBAR_GLOW, ::OHOS::Rosen::Drawing::GEXAIBarGlowEffectParams)
GE_FACTORY_REGISTER_EXTERNAL(AIBAR_RECT_HALO, ::OHOS::Rosen::Drawing::GEXAIBarRectHaloEffectParams)
GE_FACTORY_REGISTER_EXTERNAL(DISTORT_CHROMA, ::OHOS::Rosen::Drawing::GEXDistortChromaEffectParams)
GE_FACTORY_REGISTER_EXTERNAL(DOT_MATRIX, ::OHOS::Rosen::Drawing::GEDotMatrixShaderParams)
GE_FACTORY_REGISTER_EXTERNAL(GRADIENT_FLOW_COLORS, ::OHOS::Rosen::Drawing::GEXGradientFlowColorsEffectParams)
GE_FACTORY_REGISTER_EXTERNAL(CELIA_BALL_EFFECT, ::OHOS::Rosen::Drawing::GECeliaBallEffectShaderParams)
GE_FACTORY_REGISTER_EXTERNAL(HARMONIUM_EFFECT, ::OHOS::Rosen::Drawing::GEHarmoniumEffectShaderParams)
GE_FACTORY_REGISTER_EXTERNAL(LIGHT_CAVE, ::OHOS::Rosen::Drawing::GEXLightCaveShaderParams)
GE_FACTORY_REGISTER_EXTERNAL(ROUNDED_RECT_FLOWLIGHT, ::OHOS::Rosen::Drawing::GEXRoundedRectFlowlightEffectParams)

// 2.3 Dynamically loaded effects with fallback
GE_FACTORY_REGISTER_EXTERNAL_FALLBACK(COLOR_GRADIENT_EFFECT,
    ::OHOS::Rosen::Drawing::GEXColorGradientEffectParams,
    ::OHOS::Rosen::GEColorGradientEffect)

GE_FACTORY_REGISTER_EXTERNAL_FALLBACK(FROSTED_GLASS_EFFECT,
    ::OHOS::Rosen::Drawing::GEFrostedGlassEffectParams,
    ::OHOS::Rosen::GEFrostedGlassEffect)

GE_FACTORY_REGISTER_EXTERNAL_FALLBACK(SPATIAL_GLASS_EFFECT,
    ::OHOS::Rosen::Drawing::GESpatialGlassEffectParams,
    ::OHOS::Rosen::GESpatialGlassEffect)


/*
 * 3. Mask effects
 *    3.1 Built-in effects
 */
GE_FACTORY_REGISTER_MASK(GEDoubleRippleShaderMask)
GE_FACTORY_REGISTER_MASK(GEFrameGradientShaderMask)
GE_FACTORY_REGISTER_MASK(GEImageShaderMask)
GE_FACTORY_REGISTER_MASK(GELinearGradientShaderMask)
GE_FACTORY_REGISTER_MASK(GEPixelMapShaderMask)
GE_FACTORY_REGISTER_MASK(GERadialGradientShaderMask)
GE_FACTORY_REGISTER_MASK(GERippleShaderMask)
GE_FACTORY_REGISTER_MASK(GEUseEffectShaderMask)
GE_FACTORY_REGISTER_MASK(GEWaveDisturbanceShaderMask)
GE_FACTORY_REGISTER_MASK(GEWaveGradientShaderMask)

// 3.2 Dynamically loaded effects
GE_FACTORY_REGISTER_EXTERNAL(DUPOLI_NOISE_MASK, ::OHOS::Rosen::Drawing::GEXDupoliNoiseMaskParams)
GE_FACTORY_REGISTER_EXTERNAL(NOISY_FRAME_GRADIENT_MASK, ::OHOS::Rosen::Drawing::GEXNoisyFrameGradientMaskParams)


/*
 * 4. Shape effects
 *    4.1 Built-in effects
 */
GE_FACTORY_REGISTER_SHAPE(GESDFDistortOpShaderShape)
GE_FACTORY_REGISTER_SHAPE(GESDFEllipseShaderShape)
GE_FACTORY_REGISTER_SHAPE(GESDFPathShaderShape)
GE_FACTORY_REGISTER_SHAPE(GESDFPixelmapShaderShape)
GE_FACTORY_REGISTER_SHAPE(GESDFTransformShaderShape)
GE_FACTORY_REGISTER_SHAPE(GESDFTriangleShaderShape)
GE_FACTORY_REGISTER_SHAPE(GESDFUnionOpShaderShape)
GE_FACTORY_REGISTER_SHAPE(GESDFSubOpShaderShape)
GE_FACTORY_REGISTER_SHAPE(GESDFSmoothSubOpShaderShape)

// 4.2 Custom registration
GE_FACTORY_REGISTER_CUSTOM(SDF_EMPTY_SHAPE,
    [](GEEffectFactory::VisualEffectImplPtr ve) -> std::shared_ptr<::OHOS::Rosen::Drawing::IGEFilterType> {
        if (FactoryCheckNullptr(ve.get(), "SDF_EMPTY_SHAPE")) {
            return nullptr;
        }
        return std::make_shared<::OHOS::Rosen::Drawing::GESDFEmptyShaderShape>();
    })

GE_FACTORY_REGISTER_CUSTOM(SDF_RRECT_SHAPE,
    [](GEEffectFactory::VisualEffectImplPtr ve) -> std::shared_ptr<::OHOS::Rosen::Drawing::IGEFilterType> {
        if (FactoryCheckNullptr(ve.get(), "SDF_RRECT_SHAPE")) {
            return nullptr;
        }
        auto params = ve->template GetParams<::OHOS::Rosen::Drawing::GESDFRRectShapeParams>();
        if (FactoryCheckNullptr(params.get(), "SDF_RRECT_SHAPE GetParams")) {
            return nullptr;
        }
        if (!::OHOS::Rosen::Drawing::GEVisualEffect::CanBeContinuous(params)) {
            return std::make_shared<::OHOS::Rosen::Drawing::GESDFRRectShaderShape>(*params);
        }
        auto object = ::OHOS::Rosen::GEExternalDynamicLoader::GetInstance().CreateGEXObjectByType(
            static_cast<uint32_t>(::OHOS::Rosen::Drawing::GEFilterType::SDF_RRECT_SHAPE),
            sizeof(::OHOS::Rosen::Drawing::GESDFRRectShapeParams),
            static_cast<void*>(params.get()));
        if (!object) {
            return std::make_shared<::OHOS::Rosen::Drawing::GESDFRRectShaderShape>(*params);
        }
        return std::shared_ptr<::OHOS::Rosen::Drawing::IGEFilterType>(
            static_cast<::OHOS::Rosen::Drawing::IGEFilterType*>(object));
    })

} // anonymous namespace
} // namespace GraphicsEffectEngine
} // namespace OHOS
