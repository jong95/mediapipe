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

namespace mediapipe
{
  constexpr char kFaceLandmarksTag[] = "FACE_LANDMARKS";
  constexpr char kKalidokitDataTag[] = "KALIDOKIT_DATA";

  absl::Status LandmarksToKalidokitDataCalculator::GetContract(
      CalculatorContract *cc)
  {
    RET_CHECK(cc->Inputs().HasTag(kFaceLandmarksTag));
    RET_CHECK(cc->Outputs().HasTag(kKalidokitDataTag));

    if (cc->Inputs().HasTag(kFaceLandmarksTag))
    {
      cc->Inputs().Tag(kFaceLandmarksTag).Set<NormalizedLandmarkList>();
    }

    // cc->Outputs().Tag(kKalidokitDataTag).Set<RenderData>();
    cc->Outputs().Tag(kKalidokitDataTag).Set<KalidokitData>();
    return absl::OkStatus();
  }

  absl::Status LandmarksToKalidokitDataCalculator::Open(CalculatorContext *cc)
  {
    cc->SetOffset(TimestampDiff(0));

    return absl::OkStatus();
  }

  absl::Status LandmarksToKalidokitDataCalculator::Process(CalculatorContext *cc)
  {
    if (cc->Inputs().HasTag(kFaceLandmarksTag) &&
        cc->Inputs().Tag(kFaceLandmarksTag).IsEmpty())
    {
      return absl::OkStatus();
    }

    const NormalizedLandmarkList &landmarks =
        cc->Inputs().Tag(kFaceLandmarksTag).Get<NormalizedLandmarkList>();
    // std::cout << "landmarks size: " << landmarks.landmark_size() << std::endl;
    // for (int i = 0; i < landmarks.landmark_size(); ++i)
    // {
    //   const NormalizedLandmark &landmark = landmarks.landmark(i);
    //   std::cout << "landmark[" << i << "].x: " << landmark.x() << std::endl;
    // }

    auto kalidokit_data = absl::make_unique<KalidokitData>();
    auto head_data = absl::make_unique<HeadData>();
    auto head_data_degrees = absl::make_unique<HeadData_Degrees>();

    head_data_degrees->set_x(0.5);
    head_data_degrees->set_y(0.4);
    head_data_degrees->set_z(0.3);
    ::mediapipe::HeadData_Degrees *head_data_degrees_pointer = head_data_degrees.release();

    head_data->set_allocated_degrees(head_data_degrees_pointer);
    head_data->set_x(0.8);
    head_data->set_y(0.7);
    head_data->set_z(0.6);
    head_data->set_width(100);
    head_data->set_height(200);
    ::mediapipe::HeadData *head_data_pointer = head_data.release();

    kalidokit_data->set_allocated_head_data(head_data_pointer);

    cc->Outputs()
        .Tag(kKalidokitDataTag)
        .Add(kalidokit_data.release(), cc->InputTimestamp());
    return absl::OkStatus();
  }

  REGISTER_CALCULATOR(LandmarksToKalidokitDataCalculator);
} // namespace mediapipe
