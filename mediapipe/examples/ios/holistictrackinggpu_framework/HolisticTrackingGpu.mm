#import "HolisticTrackingGpu.h"

#import "mediapipe/objc/MPPGraph.h"
#import "mediapipe/objc/MPPLayerRenderer.h"
#import "mediapipe/objc/MPPTimestampConverter.h"

#include <map>
#include <string>
#include <utility>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/formats/matrix_data.pb.h"

@interface HolisticTrackingGpu () <MPPGraphDelegate>
@property(nonatomic) MPPGraph *mediapipeGraph;
@property(nonatomic) MPPTimestampConverter *timestampConverter;
@property(nonatomic) NSString *graphName;
@property(nonatomic) const char *graphInputStream;
@property(nonatomic) const char *graphOutputStream;
@property(nonatomic) const char *kLandmarksOutputStream;
@end

@implementation HolisticTrackingGpu {
}

#pragma mark - Cleanup methods

- (void)dealloc {
  self.mediapipeGraph.delegate = nil;
  [self.mediapipeGraph cancel];
  // Ignore errors since we're cleaning up.
  [self.mediapipeGraph closeAllInputStreamsWithError:nil];
  [self.mediapipeGraph waitUntilDoneWithError:nil];
}

#pragma mark - HolisticTrackingGpu methods

- (instancetype)init {
  // 1. Call super function.
  self = [super init];

  // 2. Set graph name and graph input, output stream name.
  self.graphName = @"holistic_tracking_gpu";
  self.graphInputStream = "input_video";
  self.graphOutputStream = "output_video";
  self.kLandmarksOutputStream = "face_landmarks";

  // 3. Set time converter function.
  self.timestampConverter = [[MPPTimestampConverter alloc] init];

  // 4. Load graph.
  self.mediapipeGraph = [[self class] loadGraphFromResource:self.graphName];
  [self.mediapipeGraph addFrameOutputStream:self.graphOutputStream
                           outputPacketType:MPPPacketTypePixelBuffer];

  // 5. Add face landmark data output stream.
  [self.mediapipeGraph addFrameOutputStream:self.kLandmarksOutputStream
                           outputPacketType:MPPPacketTypePixelBuffer];

  // 6. Set delegate as self.
  self.mediapipeGraph.delegate = self;

  return self;
}

- (void)startGraph {
  // Start running self.graph.
  NSError *error;
  if (![self.mediapipeGraph startWithError:&error]) {
    NSLog(@"Failed to start graph: %@", error);
  } else if (![self.mediapipeGraph waitUntilIdleWithError:&error]) {
    NSLog(@"Failed to complete graph initial run: %@", error);
  }
}

#pragma mark - MediaPipe graph methods

+ (MPPGraph *)loadGraphFromResource:(NSString *)resource {
  // 1. Load the graph config resource.
  NSError *configLoadError = nil;
  NSBundle *bundle = [NSBundle bundleForClass:[self class]];
  if (!resource || resource.length == 0) {
    return nil;
  }
  NSURL *graphURL = [bundle URLForResource:resource withExtension:@"binarypb"];
  NSData *data = [NSData dataWithContentsOfURL:graphURL
                                       options:0
                                         error:&configLoadError];
  if (!data) {
    NSLog(@"Failed to load MediaPipe graph config: %@", configLoadError);
    return nil;
  }

  // 2. Parse the graph config resource into mediapipe::CalculatorGraphConfig
  // proto object.
  mediapipe::CalculatorGraphConfig config;
  config.ParseFromArray(data.bytes, data.length);

  // 3. Create MediaPipe graph with mediapipe::CalculatorGraphConfig proto
  // object.
  MPPGraph *newGraph = [[MPPGraph alloc] initWithGraphConfig:config];
  return newGraph;
}

#pragma mark - MPPInputSourceDelegate methods

- (void)processVideoFrame:(CVPixelBufferRef)imageBuffer
                timestamp:(CMTime)timestamp {
  [self.mediapipeGraph sendPixelBuffer:imageBuffer
                            intoStream:self.graphInputStream
                            packetType:MPPPacketTypePixelBuffer
                             timestamp:[self.timestampConverter
                                           timestampForMediaTime:timestamp]];
}

#pragma mark - MPPGraphDelegate methods

- (void)mediapipeGraph:(MPPGraph *)graph
    didOutputPixelBuffer:(CVPixelBufferRef)pixelBuffer
              fromStream:(const std::string &)streamName {
  if (streamName == self.graphOutputStream) {
    [_delegate holisticTrackingGpu:self didOutputPixelBuffer:pixelBuffer];
  }
}

- (void)mediapipeGraph:(MPPGraph *)graph
       didOutputPacket:(const ::mediapipe::Packet &)packet
            fromStream:(const std::string &)streamName {
  // TODO: Change kalidokit data type.
  if (streamName == self.kLandmarksOutputStream) {
    if (packet.IsEmpty()) {
      NSLog(@"[TS:%lld] No face landmarks", packet.Timestamp().Value());
      return;
    }
    const auto &face_landmarks =
        packet.Get<std::vector<::mediapipe::NormalizedLandmark>>();

    NSLog(@"\tNumber of landmarks: %d", face_landmarks.landmark_size());
    for (int i = 0; i < face_landmarks.landmark_size(); ++i) {
      NSLog(@"\t\tLandmark[%d]: (%f, %f, %f)", i,
            face_landmarks.landmark(i).x(), face_landmarks.landmark(i).y(),
            face_landmarks.landmark(i).z());
    }
  }
}

@end
