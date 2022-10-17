// Copyright 2019 The MediaPipe Authors.
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
#include "mediapipe/calculators/util/landmarks_to_kalidokit_data_calculator.h"

#include "absl/memory/memory.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "mediapipe/calculators/util/landmarks_to_kalidokit_data_calculator.pb.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/formats/location_data.pb.h"
#include "mediapipe/framework/port/ret_check.h"
#include "mediapipe/util/kalidokit_data.pb.h"
namespace mediapipe
{

  namespace
  {

    constexpr char kLandmarksTag[] = "LANDMARKS";
    constexpr char kNormLandmarksTag[] = "NORM_LANDMARKS";
    constexpr char kKalidokitDataTag[] = "KALIDOKIT_DATA";
    constexpr char kLandmarkLabel[] = "KEYPOINT";

    absl::Status LandmarksToKalidokitDataCalculator::GetContract(
        CalculatorContract *cc)
    {
      RET_CHECK(cc->Inputs().HasTag(kLandmarksTag) ||
                cc->Inputs().HasTag(kNormLandmarksTag))
          << "None of the input streams are provided.";
      RET_CHECK(!(cc->Inputs().HasTag(kLandmarksTag) &&
                  cc->Inputs().HasTag(kNormLandmarksTag)))
          << "Can only one type of landmark can be taken. Either absolute or "
             "normalized landmarks.";

      if (cc->Inputs().HasTag(kLandmarksTag))
      {
        cc->Inputs().Tag(kLandmarksTag).Set<LandmarkList>();
      }
      if (cc->Inputs().HasTag(kNormLandmarksTag))
      {
        cc->Inputs().Tag(kNormLandmarksTag).Set<NormalizedLandmarkList>();
      }
      cc->Outputs().Tag(kKalidokitDataTag).Set<KalidokitData>();
      return absl::OkStatus();
    }

    absl::Status LandmarksToKalidokitDataCalculator::Open(CalculatorContext *cc)
    {
      cc->SetOffset(TimestampDiff(0));
      options_ = cc->Options<LandmarksToKalidokitDataCalculatorOptions>();

      // Parse landmarks connections to a vector.
      RET_CHECK_EQ(options_.landmark_connections_size() % 2, 0)
          << "Number of entries in landmark connections must be a multiple of 2";

      return absl::OkStatus();
    }

    absl::Status LandmarksToKalidokitDataCalculator::Process(CalculatorContext *cc)
    {
      // Check that landmarks are not empty and skip rendering if so.
      // Don't emit an empty packet for this timestamp.
      if (cc->Inputs().HasTag(kLandmarksTag) &&
          cc->Inputs().Tag(kLandmarksTag).IsEmpty())
      {
        return absl::OkStatus();
      }
      if (cc->Inputs().HasTag(kNormLandmarksTag) &&
          cc->Inputs().Tag(kNormLandmarksTag).IsEmpty())
      {
        return absl::OkStatus();
      }

      if (cc->Inputs().HasTag(kLandmarksTag))
      {
        const LandmarkList &landmarks =
            cc->Inputs().Tag(kLandmarksTag).Get<LandmarkList>();
      }

      if (cc->Inputs().HasTag(kNormLandmarksTag))
      {
      }

      cc->Outputs()
          .Tag(kKalidokitDataTag)
          .Add(render_data.release(), cc->InputTimestamp());
      return absl::OkStatus();
    }

    REGISTER_CALCULATOR(LandmarksToKalidokitDataCalculator);
  } // namespace mediapipe
