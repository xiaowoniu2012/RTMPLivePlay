//
//  ViewController.m
//  SayHey
//
//  Created by Mingliang Chen on 13-11-29.
//  Copyright (c) 2013年 Mingliang Chen. All rights reserved.
//

#import "ViewController.h"
#import "RTMPPublish.h"
#import "AudioRecoder.h"
#import "BOBOAACEncoder.h"
#import "CamerRecordViewController.h"
@interface ViewController ()<AudioRecordDelegate,CamerRecordViewControllerDelegate>
{
    RtmpClient *mRtmpClient;
    BOOL isStartRecord;
    BOOL isStartPlay;
    AudioRecoder *mAudioRecord;
    BOBOAACEncoder  *aacEncoder;
    NSMutableArray *queue;
    NSLock         *queueLock;
     dispatch_queue_t threadqueue;

}
@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    

    [self setup];
//    in_filename = "/private/var/mobile/Containers/Bundle/Application/A30AB20E-3D52-49CA-A976-090150EB94CC/SayHey.app/test1.flv";
//    out_filename = "rtmp://localhost/myapp/test";
    
    
    
//    mRtmpClient = [[RtmpClient alloc] initWithSampleRate:22050 withEncoder:0];
//    [mRtmpClient setOutDelegate:self];
	// Do any additional setup after loading the view, typically from a nib.
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)viewDidUnload {
    [self setStreamServerText:nil];
    [self setPubStreamNameText:nil];
    [self setPlayStreamNameText:nil];
    [self setPubBtn:nil];
    [self setPlayBtn:nil];
    [self setLogView:nil];
    [super viewDidUnload];
}

- (IBAction)clickPubBtn:(id)sender {
    NSString *path = [[NSBundle mainBundle] pathForResource:@"test1" ofType:@"flv"];
    NSLog(@"%@",path);
    
    
    push([path UTF8String],"rtmp://publish.live2.bn.netease.com/netease/yc04");
    
    /*
    if(isStartRecord){
        [mRtmpClient stopPublish];
    }else {
        NSString* rtmpUrl = [[NSString alloc] initWithFormat:@"%@/%@",_streamServerText.text,_pubStreamNameText.text];
        [mRtmpClient startPublishWithUrl:rtmpUrl];
        NSLog(@"Start publish with url %@",rtmpUrl);
        
    }
     */
}

- (IBAction)clickPlayBtn:(id)sender {
    if(isStartPlay)
    {
        [mRtmpClient stopPlay];
    }else
    {
        NSString* rtmpUrl = [[NSString alloc] initWithFormat:@"%@/%@",_streamServerText.text,_playStreamNameText.text];
        [mRtmpClient startPlayWithUrl:rtmpUrl];
        NSLog(@"Start play with url %@",rtmpUrl);
    }
}

-(void)updateLogs:(NSString*)text
{
    _logView.text = text;
}
-(void)updatePubBtn:(NSString*)text
{
    [_pubBtn setTitle:text forState:UIControlStateNormal];
}

-(void)updatePlayBtn:(NSString*)text
{
    [_playBtn setTitle:text forState:UIControlStateNormal];
}
-(void)EventCallback:(int)event{
    NSLog(@"EventCallback %d",event);
    NSString* viewText = _logView.text;
    switch (event) {
        case 1000:
            viewText =  [viewText stringByAppendingString:@"开始发布\r\n"];
            break;
        case 1001:
            viewText = [viewText stringByAppendingString:@"发布成功\r\n"];
            [self performSelectorOnMainThread:@selector(updatePubBtn:) withObject:@"Stop" waitUntilDone:YES];
            isStartRecord = YES;
            break;
        case 1002:
            viewText = [viewText stringByAppendingString:@"发布失败\r\n"];
            break;
        case 1004:
            viewText = [viewText stringByAppendingString:@"发布结束\r\n"];
            [self performSelectorOnMainThread:@selector(updatePubBtn:) withObject:@"Publish" waitUntilDone:YES];
            isStartRecord = NO;
            break;
        case 2000:
            viewText =  [viewText stringByAppendingString:@"开始播放\r\n"];
            break;
        case 2001:
            viewText = [viewText stringByAppendingString:@"播放成功\r\n"];
           [self performSelectorOnMainThread:@selector(updatePlayBtn:) withObject:@"Stop" waitUntilDone:YES];
            isStartPlay = YES;
            break;
        case 2002:
            viewText = [viewText stringByAppendingString:@"播放失败\r\n"];
            break;
        case 2004:
            viewText = [viewText stringByAppendingString:@"播放结束\r\n"];
            [self performSelectorOnMainThread:@selector(updatePlayBtn:) withObject:@"Play" waitUntilDone:YES];
            isStartPlay = NO;
            break;
        case 2005:
            viewText = [viewText stringByAppendingString:@"播放异常结束或发布端关闭\r\n"];
            break;
        default:
            break;
    }
    [self performSelectorOnMainThread:@selector(updateLogs:) withObject:viewText waitUntilDone:YES];
    
    
}


void interruptionListener(	void *	inClientData,  UInt32	inInterruptionState)
{
    
}


void propListener(	void *                  inClientData,
                  AudioSessionPropertyID	inID,
                  UInt32                  inDataSize,
                  const void *            inData)
{
    
}


- (void)setup
{
    queue = [NSMutableArray array];
   
    threadqueue = dispatch_queue_create("com.example.MyQueue", NULL);
    queueLock = [[NSLock alloc]init];
    mAudioRecord = [[AudioRecoder alloc] initWIthSampleRate:44100];
    [mAudioRecord setOutDelegate:self];
//    aacEncoder = encoderAlloc("rtmp://publish.live2.bn.netease.com/netease/yc04");
    
    aacEncoder = encoderAlloc("rtmp://192.168.1.100/myapp/test88");
    OSStatus error = AudioSessionInitialize(NULL, NULL, interruptionListener, (__bridge void *)(self));
    if (error) printf("ERROR INITIALIZING AUDIO SESSION! %d\n", (int)error);
    else
    {
        UInt32 category = kAudioSessionCategory_PlayAndRecord;
        error = AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(category), &category);
        if (error) printf("couldn't set audio category!");
        
        error = AudioSessionAddPropertyListener(kAudioSessionProperty_AudioRouteChange, propListener, (__bridge void *)self);
        if (error) printf("ERROR ADDING AUDIO SESSION PROP LISTENER! %d\n", (int)error);
        UInt32 inputAvailable = 0;
        UInt32 size = sizeof(inputAvailable);
        
        // we do not want to allow recording if input is not available
        error = AudioSessionGetProperty(kAudioSessionProperty_AudioInputAvailable, &size, &inputAvailable);
        if (error) printf("ERROR GETTING INPUT AVAILABILITY! %d\n", (int)error);
        
        // we also need to listen to see if input availability changes
        error = AudioSessionAddPropertyListener(kAudioSessionProperty_AudioInputAvailable, propListener, (__bridge void *)self);
        if (error) printf("ERROR ADDING AUDIO SESSION PROP LISTENER! %d\n", (int)error);
        
        error = AudioSessionSetActive(true);
        if (error) printf("AudioSessionSetActive (true) failed");
    }
}


- (IBAction)actionRecord:(UIButton *)sender {
    

    
    [mAudioRecord startRecord];
//    dispatch_async(threadqueue, ^{
//    
//        
//        while (1) {
//            
//            if (queue.count>0) {
//                [queueLock lock];
//                NSMutableData *data = [queue firstObject];
//                uint8_t *buffer = data.mutableBytes;
//                if (buffer != NULL) {
//                    encoderAAC(aacEncoder, buffer, 0, NULL, 0);
//                    
//                }
//                [queue removeObjectAtIndex:0];
//                [queueLock unlock];
//            }
//            
////            encoderAAC(aacEncoder, audioBuffer, 0, NULL, 0);
//        }
//    });
    

    
    
}

- (void)AudioDataOutputBuffer:(uint8_t *)audioBuffer bufferSize:(int)size
{
    

        //    queue addObject:[NSMutableData dataWithBytes:audi length:<#(NSUInteger)#>]
        encoderAAC(aacEncoder, audioBuffer, 0, NULL, 0);
    
    

    
}
- (IBAction)actionCameraRecrod:(UIButton *)sender {
    
    CamerRecordViewController *vc = [[CamerRecordViewController alloc]initWithNibName:@"CamerRecordViewController" bundle:nil];
    vc.delegate = self;
    [self presentViewController:vc animated:NO completion:^{
        
    }];
}

- (void)videoOutPut:(uint8_t *)rawData
{
    encoderH264(aacEncoder, rawData, 0, NULL, 0);
}
@end
