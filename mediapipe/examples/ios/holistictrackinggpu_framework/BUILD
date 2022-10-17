# Copyright 2020 The MediaPipe Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Delete ios application and add ios framework configuration.
load("@build_bazel_rules_apple//apple:ios.bzl", "ios_framework")

licenses(["notice"])

MIN_IOS_VERSION = "15.0"

ios_framework(
    name = "HolisticTrackingGpu",
    hdrs = [
        "HolisticTrackingGpu.h",
    ],
    bundle_id = "com.example.HolisticTrackingGpu",
    families = [
        "iphone",
    ],
    infoplists = [
        "//mediapipe/examples/ios/common:Info.plist",
        "Info.plist",
    ],
    minimum_os_version = MIN_IOS_VERSION,
    deps = [
        ":HolisticTrackingGpuLibrary",
        "@ios_opencv//:OpencvFramework",
    ],
)

objc_library(
    name = "HolisticTrackingGpuLibrary",
    srcs = [
        "HolisticTrackingGpu.mm",
    ],
    hdrs = [
        "HolisticTrackingGpu.h",
    ],
    data = [
        "//mediapipe/graphs/holistic_tracking:holistic_tracking_gpu.binarypb",
        "//mediapipe/modules/face_detection:face_detection_short_range.tflite",
        "//mediapipe/modules/face_landmark:face_landmark.tflite",
        "//mediapipe/modules/hand_landmark:hand_landmark_full.tflite",
        "//mediapipe/modules/hand_landmark:hand_landmark_lite.tflite",
        "//mediapipe/modules/hand_landmark:handedness.txt",
        "//mediapipe/modules/holistic_landmark:hand_recrop.tflite",
        "//mediapipe/modules/pose_detection:pose_detection.tflite",
        "//mediapipe/modules/pose_landmark:pose_landmark_full.tflite",
    ],
    sdk_frameworks = [
        "AVFoundation",
        "CoreGraphics",
        "CoreMedia",
        "UIKit",
    ],
    deps = [
        "//mediapipe/objc:mediapipe_framework_ios",
        "//mediapipe/objc:mediapipe_input_sources_ios",
        "//mediapipe/objc:mediapipe_layer_renderer",
    ] + select({
        "//mediapipe:ios_i386": [],
        "//mediapipe:ios_x86_64": [],
        "//conditions:default": [
            "//mediapipe/graphs/holistic_tracking:holistic_tracking_gpu_deps",
        ],
    }),
)
