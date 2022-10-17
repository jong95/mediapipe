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

  constexpr char kFaceLandmarksTag[] = "LANDMARKS";
  constexpr char kKalidokitDataTag[] = "KALIDOKIT_DATA";

  absl::Status LandmarksToKalidokitDataCalculator::GetContract(
      CalculatorContract *cc)
  {
    // cc->Inputs().NumEntries() returns the number of input streams
    // for the PacketClonerCalculator
    const int input_num_entries = cc->Inputs().NumEntries();
    std::cout << "input count: " << input_num_entries << std::endl;
    bool has_landmarks_tag = cc->Inputs().HasTag("LANDMARKS");
    std::cout << "has_landmarks_tag: " << has_landmarks_tag << std::endl;

    if (cc->Inputs().HasTag(kFaceLandmarksTag))
    {
      std::cout << "Has " << kFaceLandmarksTag << " tag name in input." << std::endl;
      cc->Inputs().Tag(kFaceLandmarksTag).Set<LandmarkList>();
    }
    else
    {
      std::cout << "Not has " << kFaceLandmarksTag << " tag name in input." << std::endl;
    }

    cc->Outputs().Tag(kKalidokitDataTag).Set<KalidokitData>();

    std::cout << "Success to process GetContract function." << std::endl;

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

    if (cc->Inputs().HasTag(kFaceLandmarksTag))
    {
      // 1. Get face landmark list.
      const LandmarkList &landmarks =
          cc->Inputs().Tag(kFaceLandmarksTag).Get<LandmarkList>();

      // 2. Get each face landmark point.
      for (int i = 0; i < landmarks.landmark_size(); ++i)
      {
        const Landmark &landmark = landmarks.landmark(i);
        printf("landmark[%d]: x(%f) y(%f) z(%f)\n",
               i, landmark.x(), landmark.y(), landmark.z());
      }

      // 3. Set kalidokit data.
      std::unique_ptr<KalidokitData>
          kalidokit_data = absl::make_unique<KalidokitData>();
      std::unique_ptr<mediapipe::HeadData>
          kalidokit_head_data = absl::make_unique<mediapipe::HeadData>();
      std::unique_ptr<mediapipe::HeadData_Degrees>
          kalidokit_head_data_degree = absl::make_unique<mediapipe::HeadData_Degrees>();

      kalidokit_head_data_degree->set_x(0.5);
      kalidokit_head_data->set_allocated_degrees(kalidokit_head_data_degree.get());
      kalidokit_head_data->set_x(1.0);
      kalidokit_data->set_allocated_head_data(kalidokit_head_data.get());

      // 4. Return kalidokit data.
      cc->Outputs()
          .Tag(kKalidokitDataTag)
          .Add(kalidokit_data.release(), cc->InputTimestamp());
    }

    return absl::OkStatus();
  }

  REGISTER_CALCULATOR(LandmarksToKalidokitDataCalculator);
} // namespace mediapipe
