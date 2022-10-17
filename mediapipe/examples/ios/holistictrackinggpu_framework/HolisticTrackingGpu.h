#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

@class HolisticTrackingGpu;

@protocol TrackerDelegate <NSObject>
- (void)holisticTrackingGpu:(HolisticTrackingGpu *)holisticTrackingGpu
       didOutputPixelBuffer:(CVPixelBufferRef)pixelBuffer;
@end

@interface HolisticTrackingGpu : NSObject
- (instancetype)init;
- (void)startGraph;
- (void)processVideoFrame:(CVPixelBufferRef)imageBuffer
                timestamp:(CMTime)timestamp;
@property(weak, nonatomic) id<TrackerDelegate> delegate;
@end
