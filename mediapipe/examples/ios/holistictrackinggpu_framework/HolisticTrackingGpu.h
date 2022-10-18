#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

@class KalidokitData;
@class Head;
@class Position;
@class Normalized;
@class Degrees;

@class HolisticTrackingGpu;

@protocol TrackerDelegate <NSObject>
- (void)holisticTrackingGpu:(HolisticTrackingGpu *)holisticTrackingGpu
       didOutputPixelBuffer:(CVPixelBufferRef)pixelBuffer;
- (void)holisticTrackingGpu:(HolisticTrackingGpu *)holisticTrackingGpu
     didOutputKalidokitData:(KalidokitData *)kalidokitData;
@end

@interface KalidokitData : NSObject
@property(nonatomic) Head *head;
@end

@interface Head: NSObject
@property(nonatomic) float x;
@property(nonatomic) float y;
@property(nonatomic) float z;
@property(nonatomic) int width;
@property(nonatomic) int height;
@property(nonatomic) Position *position;
@property(nonatomic) Normalized *normalized;
@property(nonatomic) Degrees *degrees;
@end

@interface Position : NSObject
@property(nonatomic) float x;
@property(nonatomic) float y;
@property(nonatomic) float z;
@end

@interface Normalized : NSObject
@property(nonatomic) float x;
@property(nonatomic) float y;
@property(nonatomic) float z;
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
