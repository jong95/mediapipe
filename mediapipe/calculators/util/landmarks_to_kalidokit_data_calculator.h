#ifndef MEDIAPIPE_CALCULATORS_UTIL_LANDMARKS_TO_KALIDOKIT_DATA_CALCULATOR_H_
#define MEDIAPIPE_CALCULATORS_UTIL_LANDMARKS_TO_KALIDOKIT_DATA_CALCULATOR_H_

#include <math.h>
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
