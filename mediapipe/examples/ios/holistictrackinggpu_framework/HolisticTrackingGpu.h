#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

@class KalidokitData;
@class HeadData;
@class Degrees;

@class HolisticTrackingGpu;

@protocol TrackerDelegate <NSObject>
- (void)holisticTrackingGpu:(HolisticTrackingGpu *)holisticTrackingGpu
       didOutputPixelBuffer:(CVPixelBufferRef)pixelBuffer;
- (void)holisticTrackingGpu:(HolisticTrackingGpu *)holisticTrackingGpu
     didOutputKalidokitData:(KalidokitData *)kalidokitData;
@end

@interface KalidokitData : NSObject
@property(nonatomic) HeadData *head_data;
@end

@interface HeadData : NSObject
@property(nonatomic) float x;
@property(nonatomic) float y;
@property(nonatomic) float z;
@property(nonatomic) int width;
@property(nonatomic) int height;
@property(nonatomic) Degrees *degrees;
@end

@interface Degrees : NSObject
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
