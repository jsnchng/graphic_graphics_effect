/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef GRAPHICS_EFFECT_GE_FILTER_TYPE_H
#define GRAPHICS_EFFECT_GE_FILTER_TYPE_H
#include <cstdint>
#include <string_view>

namespace OHOS {
namespace Rosen {
namespace Drawing {

enum class GEFilterType : int32_t {
    NONE,
    KAWASE_BLUR,
    MESA_BLUR,
    GREY,
    AIBAR,
    LINEAR_GRADIENT_BLUR,
    MAGNIFIER,
    WATER_RIPPLE,
    DISPLACEMENT_DISTORT_FILTER,
    COLOR_GRADIENT,
    MASK_TRANSITION,
    WATER_DROPLET_TRANSITION,
    GRID_WARP,
    MAP_COLOR_BY_BRIGHTNESS,
    GAMMA_CORRECTION,
    WATER_GLASS,
    REEDED_GLASS,
    DEPTH_OCCLUSION,
    // Mask
    RIPPLE_MASK,
    DOUBLE_RIPPLE_MASK,
    PIXEL_MAP_MASK,
    RADIAL_GRADIENT_MASK,
    WAVE_GRADIENT_MASK,
    FRAME_GRADIENT_MASK,
    LINEAR_GRADIENT_MASK,
    IMAGE_MASK,
    USE_EFFECT_MASK,
    DUPOLI_NOISE_MASK,
    NOISY_FRAME_GRADIENT_MASK,
    WAVE_DISTURBANCE_MASK,
    // Shape
    SDF_UNION_OP,
    SDF_RRECT_SHAPE,
    SDF_PATH_SHAPE,
    SDF_PIXELMAP_SHAPE,
    SDF_TRIANGLE_SHAPE,
    SDF_ELLIPSE_SHAPE,
    SDF_TRANSFORM_SHAPE,
    SDF_DISTORT_OP_SHAPE,
    SDF_EMPTY_SHAPE,
    // Shader
    DOT_MATRIX,
    FLOW_LIGHT_SWEEP,
    COMPLEX_SHADER,
    SOUND_WAVE,
    EDGE_LIGHT,
    BEZIER_WARP,
    DISPERSION,
    CONTENT_LIGHT,
    BORDER_LIGHT,
    CONTOUR_DIAGONAL_FLOW_LIGHT,
    WAVY_RIPPLE_LIGHT,
    AURORA_NOISE,
    PARTICLE_CIRCULAR_HALO,
    DIRECTION_LIGHT,
    VARIABLE_RADIUS_BLUR,
    GASIFY_SCALE_TWIST,
    GASIFY_BLUR,
    GASIFY,
    PARTICLE_ABLATION,
    COLOR_GRADIENT_EFFECT,
    CELIA_EFFECT,
    HARMONIUM_EFFECT,
    LIGHT_CAVE,
    AIBAR_GLOW,
    AIBAR_RECT_HALO,
    ROUNDED_RECT_FLOWLIGHT,
    GRADIENT_FLOW_COLORS,
    FROSTED_GLASS,
    SDF_BORDER,
    SDF_SHADOW,
    SDF_CLIP,
    CIRCLE_FLOWLIGHT,
    FROSTED_GLASS_BLUR,
    FROSTED_GLASS_EFFECT,
    DISTORT_CHROMA,
    SDF_EDGE_LIGHT,
    SDF_FROM_IMAGE,
    SDF_COLOR,
    SDF_EDGE_LIGHT_EFFECT,
    DISTORTION_COLLAPSE,
    HEAT_DISTORTION,
    BLUR_BUBBLES_RISE,
    SPATIAL_POINT_LIGHT,
    SPATIAL_GLASS_EFFECT,
    BORDER_SDF_SHADER,
    BORDER_SDF_LG_COLOR,
    MOTION_BLUR,
    BLUR,
    SDF_SUB_OP_SHAPE,
    SDF_SMOOTH_SUB_OP_SHAPE,
    MAX,
};

// Virtual interface for runtime type identification of registered GEFilter types and type-erasured class GEFilterParams
// Currently, you SHOULD NOT add member varaibles or any stateful data members to this class.
struct IGEFilterType {
    // Safeguard for type-safety, but generally you should not cast derived into IGEFilterType
    // since this interface acts only as a contract/trait for type queries
    virtual ~IGEFilterType() = default;

    // For runtime-type identification
    virtual GEFilterType Type() const
    {
        return Drawing::GEFilterType::NONE;
    }

    // Readonly type string for tracing/logging only, NOT INTENTED for serialization
    virtual const std::string_view TypeName() const
    {
        return "IGEFilterType";
    }
};

} // namespace Drawing
} // namespace Rosen
} // namespace OHOS

#endif // GRAPHICS_EFFECT_GE_FILTER_TYPE_H