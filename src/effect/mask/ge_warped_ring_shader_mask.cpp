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
        uniform float2 iResolution;
        uniform float2 centerPos;
        uniform float warpedRingRadius;
        uniform float warpedRingWidth;
        uniform float widthCenterOffset;

        const int displacementType = 4;
        const float amount = 25.0;
        const float size = 125.0;
        const float complexity = 1.0;
        const float evolution = 0.0;
        const float rotationSpeed = 0.08;
        const bool animate = true;
        const float animationSpeed = 1.0;
        const bool cycleEvolution = false;
        const float cycleRevolutions = 1.0;
        const int randomSeed = 0;
        const float frameRate = 60.0;
        const float iTime = 0.0;
        const float PI2 = 6.28318530718;

        float2 rotate2D(float2 p, float angle)
        {
            float cosine = cos(angle);
            float sine = sin(angle);
            return float2(cosine * p.x - sine * p.y, sine * p.x + cosine * p.y);
        }

        float hash13(float3 p)
        {
            p = fract(p * 0.1031);
            p += dot(p, p.yzx + 33.33);
            return fract((p.x + p.y) * p.z);
        }

        float valueNoise(float3 p)
        {
            float3 cell = floor(p);
            float3 local = fract(p);
            local = local * local * local * (local * (local * 6.0 - 15.0) + 10.0);

            float n000 = hash13(cell + float3(0.0, 0.0, 0.0));
            float n100 = hash13(cell + float3(1.0, 0.0, 0.0));
            float n010 = hash13(cell + float3(0.0, 1.0, 0.0));
            float n110 = hash13(cell + float3(1.0, 1.0, 0.0));
            float n001 = hash13(cell + float3(0.0, 0.0, 1.0));
            float n101 = hash13(cell + float3(1.0, 0.0, 1.0));
            float n011 = hash13(cell + float3(0.0, 1.0, 1.0));
            float n111 = hash13(cell + float3(1.0, 1.0, 1.0));

            float nx00 = mix(n000, n100, local.x);
            float nx10 = mix(n010, n110, local.x);
            float nx01 = mix(n001, n101, local.x);
            float nx11 = mix(n011, n111, local.x);
            float nxy0 = mix(nx00, nx10, local.y);
            float nxy1 = mix(nx01, nx11, local.y);
            return mix(nxy0, nxy1, local.z);
        }

        float3 evolutionPoint(float2 p, float octave, float seedOffset)
        {
            float seed = float(randomSeed) * 13.371 + seedOffset;
            float currentEvolution = evolution;
            if (animate) {
                float frameTime = iTime * frameRate;
                currentEvolution += frameTime * animationSpeed / 100.0;
            }
            if (cycleEvolution) {
                float turns = max(cycleRevolutions, 0.001);
                float phase = (currentEvolution / turns) * PI2;
                float2 orbit = float2(cos(phase), sin(phase));
                return float3(p + orbit * (0.83 + octave * 0.07), orbit.x * 1.37 + seed);
            }
            return float3(p + float2(currentEvolution * 0.071, -currentEvolution * 0.043),
                currentEvolution * 0.63 + seed);
        }

        float fractalNoise(float2 p, float seedOffset, bool smoother)
        {
            float requested = clamp(complexity - (smoother ? 0.35 : 0.0), 1.0, 6.0);
            float frequency = smoother ? 0.72 : 1.0;
            float amplitudeValue = 0.5;
            float sum = 0.0;
            float total = 0.0;
            for (int octaveIndex = 0; octaveIndex < 6; ++octaveIndex) {
                float octave = float(octaveIndex);
                float octaveWeight = clamp(requested - octave, 0.0, 1.0);
                float3 noisePoint = evolutionPoint(
                    p * frequency, octave, seedOffset + octave * 19.19);
                sum += valueNoise(noisePoint) * amplitudeValue * octaveWeight;
                total += amplitudeValue * octaveWeight;
                frequency *= 2.03;
                amplitudeValue *= 0.5;
            }
            return (sum / max(total, 0.0001)) * 2.0 - 1.0;
        }

        float2 turbulentField(float2 p, bool smoother)
        {
            float xNoise = fractalNoise(p, 17.0, smoother);
            float yNoise = fractalNoise(p + float2(31.73, -12.41), 83.0, smoother);
            return float2(xNoise, yNoise);
        }

        float2 gradientField(float2 p, bool twistMode, bool smoother)
        {
            float epsilon = smoother ? 0.16 : 0.09;
            float leftValue = fractalNoise(p - float2(epsilon, 0.0), 43.0, smoother);
            float rightValue = fractalNoise(p + float2(epsilon, 0.0), 43.0, smoother);
            float downValue = fractalNoise(p - float2(0.0, epsilon), 43.0, smoother);
            float upValue = fractalNoise(p + float2(0.0, epsilon), 43.0, smoother);
            float2 gradient = float2(rightValue - leftValue, upValue - downValue) / (2.0 * epsilon);
            gradient /= max(1.0, length(gradient));
            if (twistMode) {
                gradient = float2(-gradient.y, gradient.x);
            }
            return gradient;
        }

        float2 displacementField(float2 noisePosition)
        {
            bool smoother = displacementType == 1 || displacementType == 3 || displacementType == 5;
            float2 field;
            if (displacementType == 2 || displacementType == 3) {
                field = gradientField(noisePosition, false, smoother);
            } else if (displacementType == 4 || displacementType == 5) {
                field = gradientField(noisePosition, true, smoother);
            } else {
                field = turbulentField(noisePosition, smoother);
            }
            if (displacementType == 6) {
                field.x = 0.0;
            } else if (displacementType == 7) {
                field.y = 0.0;
            } else if (displacementType == 8) {
                float crossed = (field.x + field.y) * 0.5;
                field = float2(crossed, -crossed);
            }
            return field;
        }

        float sdRing(float2 p, float radius, float halfWidth)
        {
            return abs(length(p) - radius) - halfWidth;
        }

        half4 main(float2 fragCoord)
        {
            float2 center = iResolution * centerPos;
            float2 p = fragCoord - center;
            float angle = (evolution + iTime * rotationSpeed) * PI2;
            float2 noisePosition = p / max(size, 1.0);
            float2 field = displacementField(rotate2D(noisePosition, -angle));
            field = rotate2D(field, angle);
            float2 displacedP = p + field * amount;

            float minResolution = min(iResolution.x, iResolution.y);
            float radius = minResolution *
                (warpedRingRadius + warpedRingWidth * widthCenterOffset);
            float halfWidth = minResolution * warpedRingWidth;
            float distance = sdRing(displacedP, radius, halfWidth);
            float antiAlias = max(fwidth(distance), 0.75);
            float alpha = 1.0 - smoothstep(-antiAlias, antiAlias, distance);
            return half4(alpha, alpha, alpha, alpha);
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