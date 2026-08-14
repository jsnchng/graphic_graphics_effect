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

#ifndef GRAPHICS_EFFECT_GE_SPIN_BLUR_SHADER_FILTER_H
#define GRAPHICS_EFFECT_GE_SPIN_BLUR_SHADER_FILTER_H

#include <vector>

#include "draw/canvas.h"
#include "effect/runtime_effect.h"
#include "effect/runtime_shader_builder.h"
#include "ge_filter_type_info.h"
#include "ge_shader_filter.h"
#include "ge_visual_effect.h"
#include "image/image.h"

namespace OHOS {
namespace Rosen {
class GESpinBlurShaderFilter : public GEShaderFilter {
public:
    GESpinBlurShaderFilter(const Drawing::GESpinBlurShaderFilterParams& params);
    ~GESpinBlurShaderFilter() override = default;
    DECLARE_GEFILTER_TYPEFUNC(GESpinBlurShaderFilter, Drawing::GESpinBlurShaderFilterParams);

    std::shared_ptr<Drawing::Image> OnProcessImage(Drawing::Canvas& canvas, const std::shared_ptr<Drawing::Image> image,
        const Drawing::Rect& src, const Drawing::Rect& dst) override;

    void Preprocess(Drawing::Canvas& canvas, const Drawing::Rect& src, const Drawing::Rect& dst) override;

    bool CheckInParams(float* color, float* position, float* strength, int tupleSize);
    std::shared_ptr<Drawing::RuntimeShaderBuilder> MakeSpinBlurBuilder();
    std::shared_ptr<Drawing::RuntimeShaderBuilder> MakeMaskSpinBlurBuilder();
    std::string GetDescription();

private:
    std::shared_ptr<Drawing::RuntimeShaderBuilder> PreProcessSpinBlurBuilder(float geoWidth, float geoHeight);

    std::vector<float> colors_;
    std::vector<float> positions_;
    std::vector<float> strengths_;
    std::shared_ptr<Drawing::GEShaderMask> mask_ = nullptr;
};

} // namespace Rosen
} // namespace OHOS

#endif // GRAPHICS_EFFECT_GE_SPIN_BLUR_SHADER_FILTER_H