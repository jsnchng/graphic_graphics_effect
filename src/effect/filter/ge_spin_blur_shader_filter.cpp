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

#include "ge_spin_blur_shader_filter.h"

#include "ge_log.h"
#include "ge_shader_diagnostics.h"
#include "ge_system_properties.h"
#include "ge_tone_mapping_helper.h"

namespace OHOS {
namespace Rosen {

namespace {
constexpr static uint8_t COLOR_CHANNEL = 4;    // 4 len of rgba
constexpr static uint8_t POSITION_CHANNEL = 2; // 2 len of rgba
constexpr static uint8_t ARRAY_SIZE = 12;      // 12 len of array
} // namespace

thread_local static std::shared_ptr<Drawing::RuntimeEffect> g_spinBlurShaderEffect_ = nullptr;
thread_local static std::shared_ptr<Drawing::RuntimeEffect> g_maskSpinBlurShaderEffect_ = nullptr;

GESpinBlurShaderFilter::GESpinBlurShaderFilter(const Drawing::GESpinBlurShaderFilterParams& params)
{
    colors_ = params.colors;
    positions_ = params.positions;
    strengths_ = params.strengths;
    mask_ = params.mask;
}

std::shared_ptr<Drawing::Image> GESpinBlurShaderFilter::OnProcessImage(Drawing::Canvas& canvas,
    const std::shared_ptr<Drawing::Image> image, const Drawing::Rect& src, const Drawing::Rect& dst)
{
    if (image == nullptr || image->GetWidth() < 1e-6 || image->GetHeight() < 1e-6) {
        LOGE("GESpinBlurShaderFilter::OnProcessImage input is invalid.");
        return nullptr;
    }

    float color[ARRAY_SIZE * COLOR_CHANNEL] = { 0.0 };       // 0.0 default
    float position[ARRAY_SIZE * POSITION_CHANNEL] = { 0.0 }; // 0.0 default
    float strength[ARRAY_SIZE] = { 0.0 };                    // 0.0 default
    if (!CheckInParams(color, position, strength, ARRAY_SIZE)) {
        return image;
    }

    Drawing::Matrix matrix = canvasInfo_.mat;
    matrix.PostTranslate(-canvasInfo_.tranX, -canvasInfo_.tranY);
    Drawing::Matrix invertMatrix;

    if (!matrix.Invert(invertMatrix)) {
        LOGE("GESpinBlurShaderFilter::ProcessImage Invert matrix failed");
        return image;
    }

    auto srcImageShader = Drawing::ShaderEffect::CreateImageShader(*image, Drawing::TileMode::CLAMP,
        Drawing::TileMode::CLAMP, Drawing::SamplingOptions(Drawing::FilterMode::LINEAR), invertMatrix);
    if (srcImageShader == nullptr) {
        LOGE("GESpinBlurShaderFilter::OnProcessImage srcImageShader is null");
        return image;
    }

    std::shared_ptr<Drawing::RuntimeShaderBuilder> builder =
        PreProcessSpinBlurBuilder(canvasInfo_.geoWidth, canvasInfo_.geoHeight);
    if (!builder) {
        LOGE("GESpinBlurShaderFilter::OnProcessImage mask builder error\n");
        return image;
    }

    builder->SetChild("srcImageShader", srcImageShader);
    builder->SetUniform("iResolution", canvasInfo_.geoWidth, canvasInfo_.geoHeight);
    builder->SetUniform("color", color, ARRAY_SIZE * COLOR_CHANNEL);
    builder->SetUniform("position", position, ARRAY_SIZE * POSITION_CHANNEL);
    builder->SetUniform("strength", strength, ARRAY_SIZE);
    auto resultImage = builder->MakeImage(canvas.GetGPUContext().get(), &(matrix), image->GetImageInfo(), false);
    if (resultImage == nullptr) {
        LOGE("GESpinBlurShaderFilter::OnProcessImage resultImage is null");
        return image;
    }

    return resultImage;
}

void GESpinBlurShaderFilter::Preprocess(Drawing::Canvas& canvas, const Drawing::Rect& src, const Drawing::Rect& dst)
{
    // Do tone mapping when enable edr effect
    if (!GEToneMappingHelper::NeedToneMapping(supportHeadroom_)) {
        return;
    }

    float highColor = 1.0f;
    for (size_t indexColors = 0; indexColors < colors_.size(); indexColors++) {
        if ((indexColors + 1) % COLOR_CHANNEL == 0) {
            continue;
        }
        if (ROSEN_GNE(colors_[indexColors], highColor)) {
            highColor = colors_[indexColors];
        }
    }
    float compressRatio = GEToneMappingHelper::GetBrightnessMapping(supportHeadroom_, highColor) / highColor;
    for (size_t indexColors = 0; indexColors < colors_.size(); indexColors++) {
        if ((indexColors + 1) % COLOR_CHANNEL == 0) {
            continue;
        }
        colors_[indexColors] *= compressRatio;
    }
}

bool GESpinBlurShaderFilter::CheckInParams(float* color, float* position, float* strength, int tupleSize)
{
    if (strengths_.size() <= 0 || strengths_.size() > ARRAY_SIZE ||
        strengths_.size() * COLOR_CHANNEL != colors_.size() ||
        strengths_.size() * POSITION_CHANNEL != positions_.size()) {
        LOGE("GESpinBlurShaderFilter::CheckInParams param size error\n");
        return false;
    }

    int arraySize = static_cast<int>(strengths_.size());
    if (!color || !position || !strength || tupleSize < arraySize) {
        LOGE("GESpinBlurShaderFilter::CheckInParams array size error\n");
        return false;
    }

    for (int i = 0; i < arraySize; i++) {
        color[i * COLOR_CHANNEL + 0] = colors_[i * COLOR_CHANNEL + 0]; // 0 red
        color[i * COLOR_CHANNEL + 1] = colors_[i * COLOR_CHANNEL + 1]; // 1 green
        color[i * COLOR_CHANNEL + 2] = colors_[i * COLOR_CHANNEL + 2]; // 2 blur
        color[i * COLOR_CHANNEL + 3] = colors_[i * COLOR_CHANNEL + 3]; // 3 alpha

        position[i * POSITION_CHANNEL + 0] = positions_[i * POSITION_CHANNEL + 0]; // 0 x
        position[i * POSITION_CHANNEL + 1] = positions_[i * POSITION_CHANNEL + 1]; // 1 y

        strength[i] = strengths_[i];
    }

    return true;
}

std::shared_ptr<Drawing::RuntimeShaderBuilder> GESpinBlurShaderFilter::MakeSpinBlurBuilder()
{
    if (g_spinBlurShaderEffect_ == nullptr) {
        static constexpr char prog[] = R"(
            uniform shader srcImageShader;
            uniform float2 iResolution;

            const int samples = 32;
            const float blurPixels = 125.0;
            const float2 center = float2(0.5, 0.5);

            float2 rotate2D(float2 p, float angle)
            {
                float cosine = cos(angle);
                float sine = sin(angle);
                return float2(cosine * p.x - sine * p.y, sine * p.x + cosine * p.y);
            }

            half4 main(float2 fragCoord)
            {
                float2 centerPixels = center * iResolution;
                float2 p = fragCoord - centerPixels;
                float radius = length(p);
                float maxAngle = blurPixels / max(radius, 1.0);
                half4 sum = half4(0.0);
                float weightSum = 0.0;

                for (int i = 0; i < samples; ++i) {
                    float t = (float(i) + 0.5) / float(samples) - 0.5;
                    float angle = t * maxAngle;
                    float2 sampleCoord = rotate2D(p, angle) + centerPixels;
                    float weight = exp(-t * t * 6.0);
                    sum += srcImageShader.eval(sampleCoord) * weight;
                    weightSum += weight;
                }

                return sum / weightSum;
            }
        )";

        g_spinBlurShaderEffect_ = GECreateRuntimeEffectForShader(prog);
        if (g_spinBlurShaderEffect_ == nullptr) {
            LOGD("GESpinBlurShaderFilter::MakeSpinBlurBuilder effect error\n");
            return nullptr;
        }
    }

    return std::make_shared<Drawing::RuntimeShaderBuilder>(g_spinBlurShaderEffect_);
}

std::shared_ptr<Drawing::RuntimeShaderBuilder> GESpinBlurShaderFilter::MakeMaskSpinBlurBuilder()
{
    if (g_maskSpinBlurShaderEffect_ == nullptr) {
        static constexpr char withMaskProg[] = R"(
            uniform shader srcImageShader;
            uniform shader maskImageShader;
            uniform half2 iResolution;
            uniform half4 color[12];
            uniform half2 position[12];
            uniform half strength[12];

            half blendMultipleColorsByDistance(half2 uv, half2 positions, half strength)
            {
                positions.x *= iResolution.x / iResolution.y;
                half2 dist = uv - positions;
                half weight = strength / (dot(dist, dist) + 0.0001);
                return weight;
            }

            half4 main(vec2 fragCoord)
            {
                half2 uv = fragCoord / iResolution.xy;
                half screenRatio = iResolution.x / iResolution.y;
                uv.x *= screenRatio;
                half totalWeight = 0.0;
                half4 blendColor = half4(0.0);
                half4 finalColor = half4(0.0);
                half maskValue = maskImageShader.eval(fragCoord).a;
                if (maskValue > 0.0) {
                    half colorSphereWeight = blendMultipleColorsByDistance(uv, position[0], strength[0]);
                    totalWeight += colorSphereWeight;
                    blendColor += color[0] * colorSphereWeight;

                    colorSphereWeight = blendMultipleColorsByDistance(uv, position[1], strength[1]);
                    totalWeight += colorSphereWeight;
                    blendColor += color[1] * colorSphereWeight;

                    colorSphereWeight = blendMultipleColorsByDistance(uv, position[2], strength[2]);
                    totalWeight += colorSphereWeight;
                    blendColor += color[2] * colorSphereWeight;

                    colorSphereWeight = blendMultipleColorsByDistance(uv, position[3], strength[3]);
                    totalWeight += colorSphereWeight;
                    blendColor += color[3] * colorSphereWeight;

                    colorSphereWeight = blendMultipleColorsByDistance(uv, position[4], strength[4]);
                    totalWeight += colorSphereWeight;
                    blendColor += color[4] * colorSphereWeight;

                    colorSphereWeight = blendMultipleColorsByDistance(uv, position[5], strength[5]);
                    totalWeight += colorSphereWeight;
                    blendColor += color[5] * colorSphereWeight;

                    colorSphereWeight = blendMultipleColorsByDistance(uv, position[6], strength[6]);
                    totalWeight += colorSphereWeight;
                    blendColor += color[6] * colorSphereWeight;

                    colorSphereWeight = blendMultipleColorsByDistance(uv, position[7], strength[7]);
                    totalWeight += colorSphereWeight;
                    blendColor += color[7] * colorSphereWeight;

                    colorSphereWeight = blendMultipleColorsByDistance(uv, position[8], strength[8]);
                    totalWeight += colorSphereWeight;
                    blendColor += color[8] * colorSphereWeight;

                    colorSphereWeight = blendMultipleColorsByDistance(uv, position[9], strength[9]);
                    totalWeight += colorSphereWeight;
                    blendColor += color[9] * colorSphereWeight;

                    colorSphereWeight = blendMultipleColorsByDistance(uv, position[10], strength[10]);
                    totalWeight += colorSphereWeight;
                    blendColor += color[10] * colorSphereWeight;

                    colorSphereWeight = blendMultipleColorsByDistance(uv, position[11], strength[11]);
                    totalWeight += colorSphereWeight;
                    blendColor += color[11] * colorSphereWeight;

                    finalColor = blendColor / totalWeight;
                    finalColor.a *= maskValue;
                }

                finalColor.rgb = mix(srcImageShader.eval(fragCoord).rgb, finalColor.rgb, finalColor.a);
                return half4(finalColor.rgb, 1.0);
            }
        )";

        g_maskSpinBlurShaderEffect_ = GECreateRuntimeEffectForShader(withMaskProg);
        if (g_maskSpinBlurShaderEffect_ == nullptr) {
            LOGD("GESpinBlurShaderFilter::MakeMaskSpinBlurBuilder effect error\n");
            return nullptr;
        }
    }

    return std::make_shared<Drawing::RuntimeShaderBuilder>(g_maskSpinBlurShaderEffect_);
}

std::string GESpinBlurShaderFilter::GetDescription()
{
    return "GESpinBlurShaderFilter";
}

std::shared_ptr<Drawing::RuntimeShaderBuilder> GESpinBlurShaderFilter::PreProcessSpinBlurBuilder(
    float geoWidth, float geoHeight)
{
    std::shared_ptr<Drawing::RuntimeShaderBuilder> builder = nullptr;
    if (mask_) {
        builder = MakeMaskSpinBlurBuilder();
        if (!builder) {
            LOGE("GESpinBlurShaderFilter::PreProcessSpinBlurBuilder mask builder error\n");
            return nullptr;
        }
        auto maskImageShader = mask_->GenerateDrawingShader(geoWidth, geoHeight);
        if (!maskImageShader) {
            LOGE("GESpinBlurShaderFilter::PreProcessSpinBlurBuilder maskImageShader is null");
            return nullptr;
        }
        builder->SetChild("maskImageShader", maskImageShader);
    } else {
        builder = MakeSpinBlurBuilder();
        if (!builder) {
            LOGE("GESpinBlurShaderFilter::PreProcessSpinBlurBuilder builder error\n");
            return nullptr;
        }
    }
    return builder;
}
} // namespace Rosen
} // namespace OHOS