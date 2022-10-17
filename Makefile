holistictrackinggpu:
	bazel build --config=ios_arm64 //mediapipe/examples/ios/holistictrackinggpu:HolisticTrackingGpuApp

holistictrackinggpu_framework:
	bazel build --config=ios_arm64 //mediapipe/examples/ios/holistictrackinggpu_framework:HolisticTrackingGpu