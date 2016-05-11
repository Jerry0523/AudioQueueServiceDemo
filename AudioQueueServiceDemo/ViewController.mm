//
//  ViewController.m
//  AudioQueueServiceDemo
//
//  Created by Jerry Wong's Mac Mini on 15/11/12.
//  Copyright © 2015年 Jerry Wong. All rights reserved.
//

#import <AVFoundation/AVFoundation.h>
#import "ViewController.h"
#import "AudioRecorderInput.h"

#import "AudioPlayerOutput.h"
#import "AudioFileOutput.h"

@interface ViewController ()

@property (weak, nonatomic) IBOutlet UIView *microphoneView;

@property (nonatomic) JWAudioToolkit::AudioRecorderInput *recorder;
@property (nonatomic, strong) CAShapeLayer *volumeLayer;

@end

@implementation ViewController

static void refreshSoundPow(void* userData, float peak, float average) {
    dispatch_async(dispatch_get_main_queue(), ^{
        ViewController *controller = (__bridge ViewController*)userData;
        controller.volumeLayer.fillColor = [UIColor colorWithRed: 0 green: 122.0 / 255.0 blue: 1.0 alpha: peak].CGColor;
    });
}

- (void)awakeFromNib {
    
//    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES);
//    NSString *docDir = [paths objectAtIndex:0];
//    NSString *filePath = [docDir stringByAppendingPathComponent:@"test.wav"];
//    const char *cFilePath = [filePath UTF8String];
//    JWAudioToolkit::AudioFileOutput *output = new JWAudioToolkit::AudioFileOutput(cFilePath);
    
    JWAudioToolkit::AudioPlayerOutput *output = new JWAudioToolkit::AudioPlayerOutput();
    JWAudioToolkit::AudioRecorderInput *input = new JWAudioToolkit::AudioRecorderInput(output, refreshSoundPow, (__bridge void*)self);
    self.recorder = input;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    CAShapeLayer *microphoneLayer = [CAShapeLayer layer];
    microphoneLayer.frame = CGRectMake(0, 0, CGRectGetWidth(self.microphoneView.frame), CGRectGetHeight(self.microphoneView.frame));
    
    microphoneLayer.strokeColor = [UIColor colorWithRed: 0 green: 122.0 / 255.0 blue: 1.0 alpha: 1].CGColor;
    microphoneLayer.lineWidth = 4.0;
    microphoneLayer.fillColor = [UIColor clearColor].CGColor;
    microphoneLayer.lineCap = kCALineCapRound;
    
    UIBezierPath *bezierPath = UIBezierPath.bezierPath;
    
    [bezierPath moveToPoint:CGPointMake(28, 98)];
    [bezierPath addLineToPoint:CGPointMake(72, 98)];
    
    [bezierPath moveToPoint:CGPointMake(50, 98)];
    [bezierPath addLineToPoint:CGPointMake(50, 90)];
    
    [bezierPath moveToPoint:CGPointMake(20, 52)];
    [bezierPath addQuadCurveToPoint: CGPointMake(50, 90) controlPoint: CGPointMake(22, 88)];
    [bezierPath addQuadCurveToPoint: CGPointMake(80, 52) controlPoint: CGPointMake(78, 88)];
    
    UIBezierPath *roundPath = [UIBezierPath bezierPathWithRoundedRect:CGRectMake(32, 2, 36, 74) cornerRadius:18];
    [bezierPath appendPath:roundPath];
    
    microphoneLayer.path = bezierPath.CGPath;
    
    [self.microphoneView.layer addSublayer:microphoneLayer];
    
    self.volumeLayer = [CAShapeLayer layer];
    self.volumeLayer.frame = CGRectMake(0, 0, CGRectGetWidth(self.microphoneView.frame), CGRectGetHeight(self.microphoneView.frame));
    self.volumeLayer.fillColor = [UIColor clearColor].CGColor;
    
    roundPath = [UIBezierPath bezierPathWithRoundedRect:CGRectMake(36, 6, 28, 66) cornerRadius:18];
    
    self.volumeLayer.path = roundPath.CGPath;
    [self.microphoneView.layer addSublayer:self.volumeLayer];
}

- (void)dealloc {
    if (_recorder) {
        delete _recorder;
    }
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)recordButtonClicked:(UIButton*)sender {
    sender.selected = !sender.selected;
    if (self.recorder->isRunning()) {
        self.recorder->stop();
    } else {
        
        NSError *error;
        AVAudioSession *audioSession = [AVAudioSession sharedInstance];
        [audioSession setCategory:AVAudioSessionCategoryPlayAndRecord withOptions:AVAudioSessionCategoryOptionDefaultToSpeaker error:&error];
        if (error) {
            NSLog(@"%@", [error localizedDescription]);
            return;
        } else {
            [audioSession setActive:YES error:&error];
            if (error) {
                NSLog(@"%@", [error localizedDescription]);
                return;
            }
        }
        
        self.recorder->start();
    }
}

@end
