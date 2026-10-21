//
//  FEDeviceInfo.h
//  FEBluetoothSDK
//
//  Created by chenchanghua on 2025/3/25.
//  Copyright © 2025 Feasycom. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

NS_ASSUME_NONNULL_BEGIN

@interface FEDeviceInfo : NSObject

#pragma mark - 180A

/// 模块名
@property (nonatomic, copy, nullable) NSString *modelName;
/// mac地址
@property (nonatomic, copy, nullable) NSString *mac;
/// 硬件版本
@property (nonatomic, copy, nullable) NSString *device_version;
/// 软件版本
@property (nonatomic, copy, nullable) NSString *system_version;
/// 设备厂家
@property (nonatomic, copy, nullable) NSString *owner;

/// 将特征值「写入」到对应的信息中
- (void)infoFromCharacteristic_180A_service:(CBCharacteristic *)characteristic;

#pragma mark - EnterDFU

/// 固件版本号
@property (nonatomic, copy, nullable) NSString *appVersion;
/// Boot Loader版本号
@property (nonatomic, copy, nullable) NSString *firmwareVersion;
/// 模块名称
@property (nonatomic, copy, nullable) NSString *moduleName;
/// 模块序列号
@property (nonatomic, assign) NSInteger moduleNumber;

/// 清除该类中所有属性的数据
- (void)clear;

@end

NS_ASSUME_NONNULL_END
