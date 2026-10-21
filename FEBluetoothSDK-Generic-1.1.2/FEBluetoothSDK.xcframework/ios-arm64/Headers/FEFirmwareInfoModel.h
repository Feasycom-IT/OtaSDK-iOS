//
//  FEFirmwareInfoModel.h
//  FEBluetoothSDK
//
//  Created by chenchanghua on 2022/1/7.
//  Copyright © 2022 feasycom. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "FEDeviceInfo.h"

NS_ASSUME_NONNULL_BEGIN

/// 升级固件信息Model
@interface FEFirmwareInfoModel : NSObject

@property (nonatomic, copy) NSString *flagStart;
@property (nonatomic, copy) NSString *bootloader;
@property (nonatomic, copy) NSString *binCrc;
@property (nonatomic, assign) NSInteger length;
@property (nonatomic, copy) NSString *versionRange;
@property (nonatomic, copy) NSString *_Nullable modelType;
@property (nonatomic, assign) NSInteger modelTypeNum;
@property (nonatomic, copy) NSString *uploadModel;
@property (nonatomic, copy) NSString *reserve;
@property (nonatomic, copy) NSString *crc;
@property (nonatomic, strong) NSData *dfuData;
@property (nonatomic, copy) NSString *flagEnd;

+ (void)modelFromOTAData:(NSData *)data
                complete:(void(^)(FEFirmwareInfoModel *model))complete
                  option:(void(^)(FEFirmwareInfoModel *model))option
                  failed:(void(^)(void))failed;

- (BOOL)contrastDevideAndFirmwareForContinueUpgrading:(FEDeviceInfo *)deviceInfo;

@end

NS_ASSUME_NONNULL_END
