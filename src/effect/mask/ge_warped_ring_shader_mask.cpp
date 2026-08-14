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

#include "ge_warped_ring_shader_mask.h"

#include <chrono>

#include "ge_log.h"
#include "ge_shader_diagnostics.h"
#include "ge_trace.h"

namespace OHOS {
namespace Rosen {
namespace Drawing {

GEWarpedRingShaderMask::GEWarpedRingShaderMask(const GEWarpedRingShaderMaskParams& param) : param_(param) {}

std::shared_ptr<ShaderEffect> GEWarpedRingShaderMask::GenerateDrawingShader(float width, float height) const
{
    GE_TRACE_NAME_FMT("GEWarpedRingShaderMask::GenerateDrawingShader, Type: %s, Width: %g, Height: %g",
        Drawing::GE_MASK_WARPED_RING, width, height);
    std::shared_ptr<Drawing::RuntimeShaderBuilder> builder = nullptr;
    builder = GetWarpedRingShaderMaskBuilder();
    if (!builder) {
        LOGE("GEWarpedRingShaderMask::GenerateDrawingShaderHas builder error");
        return nullptr;
    }
    builder->SetUniform("iResolution", width, height);
    builder->SetUniform("centerPos", param_.center_.first, param_.center_.second);
    builder->SetUniform("warpedRingRadius", param_.radius_);
    builder->SetUniform("warpedRingWidth", param_.width_);
    builder->SetUniform("widthCenterOffset", param_.widthCenterOffset_);
    auto warpedRingMaskEffectShader = builder->MakeShader(nullptr, false);
    if (!warpedRingMaskEffectShader) {
        LOGE("GEWarpedRingShaderMask::GenerateDrawingShaderHas effect error");
    }
    return warpedRingMaskEffectShader;
}

std::shared_ptr<Drawing::RuntimeShaderBuilder> GEWarpedRingShaderMask::GetWarpedRingShaderMaskBuilder() const
{
    thread_local std::shared_ptr<Drawing::RuntimeShaderBuilder> warpedRingShaderMaskBuilder = nullptr;
    if (warpedRingShaderMaskBuilder) {
        return warpedRingShaderMaskBuilder;
    }

    static constexpr char prog[] = R"(
        uniform half2 iResolution;
        uniform half2 centerPos;
        uniform half warpedRingRadius;
        uniform half warpedRingWidth;
        uniform half widthCenterOffset;

        half4 main(vec2 fragCoord)
        {
            half2 uv = fragCoord.xy/iResolution.xy;
            half screenRatio = iResolution.x/iResolution.y;
            // Mask Info
            half2 MaskCenterUVs = uv - centerPos;
            MaskCenterUVs.x *= screenRatio;
            half offsetWidth = warpedRingWidth * widthCenterOffset;
            half uvDistance = length(MaskCenterUVs) - warpedRingRadius;
            half maskAlpha = smoothstep(warpedRingWidth, offsetWidth, uvDistance) *
                             smoothstep(-warpedRingWidth, offsetWidth, uvDistance);
            return half4(maskAlpha);
        }
    )";

    auto warpedRingShaderMaskEffect = GECreateRuntimeEffectForShader(prog);
    if (!warpedRingShaderMaskEffect) {
        LOGE("GEWarpedRingShaderMask::GetWarpedRingShaderMaskBuilder effect error");
        return nullptr;
    }

    warpedRingShaderMaskBuilder = std::make_shared<Drawing::RuntimeShaderBuilder>(warpedRingShaderMaskEffect);
    return warpedRingShaderMaskBuilder;
}

std::shared_ptr<ShaderEffect> GEWarpedRingShaderMask::GenerateDrawingShaderHasNormal(float width, float height) const
{
    GE_TRACE_NAME_FMT("GEWarpedRingShaderMask::GenerateDrawingShaderHasNormal, Type: %s, Width: %g, Height: %g",
        Drawing::GE_MASK_WARPED_RING, width, height);
    std::shared_ptr<Drawing::RuntimeShaderBuilder> builder = nullptr;
    builder = GetWarpedRingShaderNormalMaskBuilder();
    if (!builder) {
        LOGE("GEWarpedRingShaderMask::GenerateDrawingShaderHasNormal builder error");
        return nullptr;
    }
    builder->SetUniform("iResolution", width, height);
    builder->SetUniform("centerPos", param_.center_.first, param_.center_.second);
    builder->SetUniform("warpedRingRadius", param_.radius_);
    builder->SetUniform("warpedRingWidth", param_.width_);
    builder->SetUniform("widthCenterOffset", param_.widthCenterOffset_);
    auto warpedRingMaskEffectShader = builder->MakeShader(nullptr, false);
    if (!warpedRingMaskEffectShader) {
        LOGE("GEWarpedRingShaderMask::GenerateDrawingShaderHasNormal effect error");
    }
    return warpedRingMaskEffectShader;
}

std::shared_ptr<Drawing::RuntimeShaderBuilder> GEWarpedRingShaderMask::GetWarpedRingShaderNormalMaskBuilder() const
{
    thread_local std::shared_ptr<Drawing::RuntimeShaderBuilder> warpedRingShaderMaskNormalBuilder = nullptr;
    if (warpedRingShaderMaskNormalBuilder) {
        return warpedRingShaderMaskNormalBuilder;
    }

    static constexpr char prog[] = R"(
        uniform half2 iResolution;
        uniform half2 centerPos;
        uniform half warpedRingRadius;
        uniform half warpedRingWidth;
        uniform half widthCenterOffset;

        half4 main(vec2 fragCoord)
        {
            half2 uv = fragCoord.xy / iResolution.xy;
            half screenRatio = iResolution.x / iResolution.y;
            // Mask Info
            half2 MaskCenterUVs = uv - centerPos;
            MaskCenterUVs.x *= screenRatio;
            half dist = length(MaskCenterUVs);
            half offsetWidth = warpedRingWidth * widthCenterOffset;
            half uvDistance = dist - warpedRingRadius - offsetWidth;
            uvDistance = clamp(uvDistance, -1.0, 1.0);

            half mask = smoothstep(warpedRingWidth - offsetWidth, 0.0, uvDistance) *
                        smoothstep(-warpedRingWidth - offsetWidth, 0.0, uvDistance);

            half2 directionVector = MaskCenterUVs * (uvDistance * mask * 0.5 / (dist + 1e-4)) + 0.5;
            return half4(directionVector, 1.0, mask);
        }
    )";

    auto warpedRingShaderMaskNormalEffect = GECreateRuntimeEffectForShader(prog);
    if (!warpedRingShaderMaskNormalEffect) {
        LOGE("GEWarpedRingShaderMask::GetWarpedRingShaderNormalMaskBuilder effect error");
        return nullptr;
    }

    warpedRingShaderMaskNormalBuilder =
        std::make_shared<Drawing::RuntimeShaderBuilder>(warpedRingShaderMaskNormalEffect);
    return warpedRingShaderMaskNormalBuilder;
}

} // namespace Drawing
} // namespace Rosen
} // namespace OHOS