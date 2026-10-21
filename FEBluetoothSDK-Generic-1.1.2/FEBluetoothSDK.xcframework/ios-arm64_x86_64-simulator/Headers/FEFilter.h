//
//  FEFilter.h
//  FEBluetoothSDK
//
//  Created by chenchanghua on 2025/3/25.
//  Copyright © 2025 Feasycom. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface FEFilter : NSObject

/// 是否过滤信号值
@property (nonatomic, assign) BOOL isFilterRSSI;

/// 是否过滤名称
@property (nonatomic, assign) BOOL isFilterName;

/// 过滤最小信号值
/// 范围：-100 ~ 0
/// 语义：低于该 RSSI 的设备会被过滤掉
@property (nonatomic, assign) NSInteger minRSSI;

/// 过滤最大信号值
/// 范围：-100 ~ 0
/// 语义：高于该 RSSI 的设备会被过滤掉
@property (nonatomic, assign) NSInteger maxRSSI;

/// 过滤设备名
/// 语义：设备名包含该字符串才通过过滤
@property (nonatomic, copy) NSString *filtrationName;

/// 判断某个设备是否符合当前过滤条件
/// @param RSSI 设备 RSSI
/// @param name 设备名称
- (BOOL)matchesRSSI:(NSInteger)RSSI name:(nullable NSString *)name;

@end

NS_ASSUME_NONNULL_END
