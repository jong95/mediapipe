#include "mediapipe/calculators/util/landmarks_to_kalidokit_data_calculator.h"

namespace mediapipe
{
  const auto TWO_PI = M_PI * 2;
  const auto PI = M_PI;

  float normalizeAngle(float radians)
  {
    float angle = fmod(radians, TWO_PI);

    if (angle > PI)
    {
      angle = angle - TWO_PI;
    }
    else
    {
      if (angle < -PI)
      {
        angle = TWO_PI + angle;
      }
      else
      {
        angle = angle;
      }
    }

    return angle / PI;
  }

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
    // 1. Check the data is empty.
    if (cc->Inputs().HasTag(kFaceLandmarksTag) &&
        cc->Inputs().Tag(kFaceLandmarksTag).IsEmpty())
    {
      return absl::OkStatus();
    }

    // 2. Get face landmark data.
    const NormalizedLandmarkList &landmarks =
        cc->Inputs().Tag(kFaceLandmarksTag).Get<NormalizedLandmarkList>();
    // std::cout << "landmarks size: " << landmarks.landmark_size() << std::endl;
    // for (int i = 0; i < landmarks.landmark_size(); ++i)
    // {
    //   const NormalizedLandmark &landmark = landmarks.landmark(i);
    //   std::cout << "landmark[" << i << "].x: " << landmark.x() << std::endl;
    // }

    // 3. Mimic createEulerPlane function in kalidokit solution.
    Eigen::Vector3f p1, p2, p3, p4, p3mid;
    p1 << landmarks.landmark(21).x(), landmarks.landmark(21).y(), landmarks.landmark(21).z();
    p2 << landmarks.landmark(251).x(), landmarks.landmark(251).y(), landmarks.landmark(251).z();
    p3 << landmarks.landmark(397).x(), landmarks.landmark(397).y(), landmarks.landmark(397).z();
    p4 << landmarks.landmark(172).x(), landmarks.landmark(172).y(), landmarks.landmark(172).z();
    p3mid = (p3 + p4) / 2;

    // 4. Calculate roll, pitch, and yaw.
    Eigen::Vector3f plane0, plane1, plane2, a, b, c;
    a = plane0 = p1;
    b = plane1 = p2;
    c = plane2 = p3mid;

    Eigen::Vector3f qb = b - a;
    Eigen::Vector3f qc = c - a;
    Eigen::Vector3f n = qb.cross(qc);

    Eigen::Vector3f unitZ = n.normalized();
    Eigen::Vector3f unitX = qb.normalized();
    Eigen::Vector3f unitY = unitZ.cross(unitX);

    auto beta = std::asin(unitZ.x());
    auto alpha = std::atan2(-unitZ.y(), unitZ.z());
    auto gamma = std::atan2(-unitY.x(), unitX.x());

    Eigen::Vector3f rotate;
    rotate << normalizeAngle(alpha), normalizeAngle(beta), normalizeAngle(gamma);

    Eigen::Vector3f midPoint = (plane0 + plane1) / 2;
    Eigen::Vector3f widthVector = plane0 - plane1;
    auto width = widthVector.norm();
    Eigen::Vector3f heightVector = midPoint - plane2;
    auto height = heightVector.norm();

    rotate.x() *= -1;
    rotate.z() *= -1;

    // Set kalidokit data.
    auto kalidokit_data = absl::make_unique<KalidokitData>();
    
    // Set head data.
    auto head = absl::make_unique<Head>();
    auto position = absl::make_unique<Position>();
    auto normalized = absl::make_unique<Normalized>();
    auto degrees = absl::make_unique<Degrees>();

    // Set degree data.
    degrees->set_x(rotate.x() * 180);
    degrees->set_y(rotate.y() * 180);
    degrees->set_z(rotate.z() * 180);
    ::mediapipe::Degrees *degrees_pointer = degrees.release();

    // Set normalized data.
    normalized->set_x(rotate.x());
    normalized->set_y(rotate.y());
    normalized->set_z(rotate.z());
    ::mediapipe::Normalized *normalized_pointer = normalized.release();

    // Set position data.
    Eigen::Vector3f position_vector = (midPoint + plane1) / 2;
    position->set_x(position_vector.x());
    position->set_y(position_vector.y());
    position->set_z(position_vector.z());
    ::mediapipe::Position *position_pointer = position.release();

    // Set head data.
    head->set_allocated_degrees(degrees_pointer);
    head->set_allocated_normalized(normalized_pointer);
    head->set_allocated_position(position_pointer);
    head->set_x(rotate.x() * PI);
    head->set_y(rotate.y() * PI);
    head->set_z(rotate.z() * PI);
    head->set_width(width);
    head->set_height(height);
    ::mediapipe::Head *head_pointer = head.release();

    // Set kalidokit data.
    kalidokit_data->set_allocated_head(head_pointer);

    // Send return data to ouput packet.
    cc->Outputs()
        .Tag(kKalidokitDataTag)
        .Add(kalidokit_data.release(), cc->InputTimestamp());
    return absl::OkStatus();
  }

  REGISTER_CALCULATOR(LandmarksToKalidokitDataCalculator);
} // namespace mediapipe
