## calculators/util/

- BUILD
  -- landmarks_to_kalidokit_data_calculator
  --- kalidokit_data_cc_proto
  -- kalidokit_data_cc_proto
  -- kalidokit_data_proto
  --- kalidokit_data.proto

- kalidokit_data.proto
- landmarks_to_kalidokit_data_calculator.h
- landmarks_to_kalidokit_data_calculator.cc

## examples/ios/holistictrackinggpu_framework/

- BUILD
  -- HolisticTrackingGpu
  -- HolisticTrackingGpuLibrary
  --- //mediapipe/graphs/holistic_tracking:holistic_tracking_gpu_deps"

- HolisticTrackingGpu.h
- HolisticTrackingGpu.mm

## graphs/holistic_tracking/

- BUILD
  -- holistic_tracking_gpu_deps
  --- holistic_tracking_to_kalidokit_data
  ---- //mediapipe/calculators/util:landmarks_to_kalidokit_data_calculator

- holistic_tracking_to_kalidokit_data.pbtxt
