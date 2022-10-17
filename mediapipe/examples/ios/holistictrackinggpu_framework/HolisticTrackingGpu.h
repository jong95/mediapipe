#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

@class KalidokitData;
@class HolisticTrackingGpu;

@protocol TrackerDelegate <NSObject>
- (void)holisticTrackingGpu:(HolisticTrackingGpu *)holisticTrackingGpu
       didOutputPixelBuffer:(CVPixelBufferRef)pixelBuffer;
- (void)holisticTrackingGpu:(HolisticTrackingGpu *)holisticTrackingGpu
     didOutputKalidokitData:(KalidokitData *)kalidokitData;
@end

@interface KalidokitData : NSObject
@property(nonatomic) float x;
@property(nonatomic) float y;
@property(nonatomic) float z;
@end

@interface HolisticTrackingGpu : NSObject
- (instancetype)init;
- (void)startGraph;
- (void)processVideoFrame:(CVPixelBufferRef)imageBuffer
                timestamp:(CMTime)timestamp;
@property(weak, nonatomic) id<TrackerDelegate> delegate;
@end
