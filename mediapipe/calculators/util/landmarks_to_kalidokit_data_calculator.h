// Copyright 2020 The MediaPipe Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef MEDIAPIPE_CALCULATORS_UTIL_LANDMARKS_TO_KALIDOKIT_DATA_CALCULATOR_H_
#define MEDIAPIPE_CALCULATORS_UTIL_LANDMARKS_TO_KALIDOKIT_DATA_CALCULATOR_H_

#include "Eigen/Dense"
#include "absl/memory/memory.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/port/ret_check.h"
#include "mediapipe/calculators/util/kalidokit_data.pb.h"
#include "mediapipe/util/render_data.pb.h"

namespace mediapipe
{
    class LandmarksToKalidokitDataCalculator : public CalculatorBase
    {
    public:
        LandmarksToKalidokitDataCalculator() {}
        ~LandmarksToKalidokitDataCalculator() override {}
        LandmarksToKalidokitDataCalculator(const LandmarksToKalidokitDataCalculator &) =
            delete;
        LandmarksToKalidokitDataCalculator &operator=(
            const LandmarksToKalidokitDataCalculator &) = delete;

        static absl::Status GetContract(CalculatorContract *cc);
        absl::Status Open(CalculatorContext *cc) override;
        absl::Status Process(CalculatorContext *cc) override;
    };
} // namespace mediapipe

#endif // MEDIAPIPE_CALCULATORS_UTIL_LANDMARKS_TO_KALIDOKIT_DATA_CALCULATOR_H_
