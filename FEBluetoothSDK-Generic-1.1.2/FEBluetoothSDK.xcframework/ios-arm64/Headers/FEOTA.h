//
//  FEOTA.h
//  FEBluetoothSDK
//
//  Created by ericj on 2020/7/15.
//  Copyright © 2020 Feasycom. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "FEBluetoothSDK.h"

NS_ASSUME_NONNULL_BEGIN

@interface FEOTA : NSObject
@property (nonatomic, assign) BOOL isShowLog;

/// 初始化
/// @param peripheral 待升级设备
/// @param dfuData 待发送的数据（headerDate + 000……0 + binData）
- (instancetype)initWithPeripheral:(FEPeripheral *)peripheral dfuData:(NSData *)dfuData;

/// 替换待使用固件
- (void)updateDFUData:(NSData *)dfuData;

/// 开始升级
/// @param progress 进度
/// @param transferDidFinish 升级完成
/// @param abort 升级取消
/// @param timeout 升级超时
- (void)startProgress:(void(^)(float progress, NSInteger length, NSInteger finishLength))progress finish:(void(^)(void))transferDidFinish abort:(void(^)(void))abort timeout:(void(^)(void))timeout;

@end

NS_ASSUME_NONNULL_END
