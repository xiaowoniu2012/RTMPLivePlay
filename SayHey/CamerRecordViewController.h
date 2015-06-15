//
//  CamerRecordViewController.h
//  SayHey
//
//  Created by zelong zou on 15/6/10.
//  Copyright (c) 2015年 Mingliang Chen. All rights reserved.
//

#import <UIKit/UIKit.h>


@class CamerRecordViewController;
@protocol CamerRecordViewControllerDelegate <NSObject>

- (void)videoOutPut:(uint8_t *)rawData;

@end
@interface CamerRecordViewController : UIViewController
@property (nonatomic,assign) id<CamerRecordViewControllerDelegate> delegate;
@end
