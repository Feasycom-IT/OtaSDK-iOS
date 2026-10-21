//
//  FEBluetoothSDK.h
//  FEBluetoothSDK
//
//  Created by ericj on 2020/5/20.
//  Copyright © 2020 Feasycom. All rights reserved.
//
//

#import <Foundation/Foundation.h>
@class FEBluetoothSDK, FEFilter, FEPeripheral, CBUUID;

NS_ASSUME_NONNULL_BEGIN

/// SDK 版本
extern NSString * const SDKVersion;

/// 中心蓝牙状态
typedef NS_ENUM(NSInteger, FEBluetoothSDKState) {
    FEBluetoothSDKState_Unknown = 0,
    FEBluetoothSDKState_Resetting,
    FEBluetoothSDKState_Unsupported,
    FEBluetoothSDKState_Unauthorized,
    FEBluetoothSDKState_PoweredOff,
    FEBluetoothSDKState_PoweredOn,
};

/// 系统 BLE 连接事件类型
typedef NS_ENUM(NSInteger, FEConnectionEvent) {
    FEConnectionEventConnected = 0,
    FEConnectionEventDisconnected,
};

/// 通用操作回调
typedef void(^FEOperationCompletion)(BOOL success, NSError * _Nullable error);

/// 扫描回调：高频回调。App 层不要在该回调中直接高频 reloadData，建议自行节流
typedef void(^ScanBlock)(FEPeripheral *peripheral, BOOL isNew);

/// 系统 BLE 连接事件回调
typedef void(^FEConnectionEventBlock)(FEPeripheral *peripheral, FEConnectionEvent event);

/// 查询外设数组回调
typedef void(^FEPeripheralListBlock)(NSArray<FEPeripheral *> *peripherals, BOOL isSuccess);

#pragma mark - Options

/// 扫描配置
@interface FEScanOptions : NSObject

/// 是否允许重复发现同一设备。默认 YES
@property (nonatomic, assign) BOOL allowDuplicates;

/// 对应 CBCentralManagerScanOptionSolicitedServiceUUIDsKey
@property (nonatomic, copy, nullable) NSArray<CBUUID *> *solicitedServiceUUIDs;

+ (instancetype)defaultOptions;

@end

/// 连接配置
@interface FEConnectOptions : NSObject

/// 默认 YES
@property (nonatomic, assign) BOOL notifyOnConnection;

/// 默认 YES
@property (nonatomic, assign) BOOL notifyOnDisconnection;

/// 默认 YES
@property (nonatomic, assign) BOOL notifyOnNotification;

/// 默认随机 0~2。若小于 0，则不设置 CBConnectPeripheralOptionStartDelayKey
@property (nonatomic, assign) NSInteger startDelay;

/// 默认 NO，仅 iOS 13+
@property (nonatomic, assign) BOOL enableTransportBridging;

/// 默认 NO，仅 iOS 17+
@property (nonatomic, assign) BOOL enableAutoReconnect;

/// 默认 NO，仅 iOS 13+。普通 BLE 设备建议 NO
@property (nonatomic, assign) BOOL requiresANCS;

+ (instancetype)defaultOptions;

@end

#pragma mark - Delegate

@protocol FEBluetoothSDKDelegate <NSObject>
@optional

/// 中心蓝牙状态发生变化
- (void)feBluetoothSDK:(FEBluetoothSDK *_Nullable)sdk
        didUpdateState:(FEBluetoothSDKState)state;

/// ANCS 授权状态发生变化，仅 iOS 13+
- (void)feBluetoothSDK:(FEBluetoothSDK *_Nullable)sdk
didUpdateANCSAuthorizationForPeripheral:(FEPeripheral *)peripheral
            authorized:(BOOL)authorized;

@end

#pragma mark - SDK

@interface FEBluetoothSDK : NSObject

/// 单例对象
+ (instancetype)sharedFEBluetoothSDK;

/// 中心蓝牙状态
@property (nonatomic, assign, readonly) FEBluetoothSDKState feBluetoothSDKState;

/// 扩展扫描与增强连接能力检测
+ (BOOL)isExtendedBLESupported;

/// 过滤
@property (nonatomic, strong) FEFilter *filter;

/// 搜索到的外设数组
/// 兼容旧客户保留。新代码建议使用 getScanPeripheralsSnapshot:。
@property (nonatomic, strong, readonly) NSMutableArray<FEPeripheral *> *scanPeripherals;

/// 检索到的外围设备数组
/// 兼容旧客户保留。新代码建议使用 getRetrievedPeripheralsSnapshot:。
@property (nonatomic, strong, readonly) NSMutableArray<FEPeripheral *> *retrievedPeripherals;

/// 已经连接的外设数组
/// 兼容旧客户保留。新代码建议使用 getConnectedPeripheralsSnapshot:。
@property (nonatomic, strong, readonly) NSMutableArray<FEPeripheral *> *peripherals;

/// 当前系统已连接 BLE 设备缓存
/// 这是最近一次 refresh/retrieveConnected 查询后的缓存。
@property (nonatomic, strong, readonly) NSArray<FEPeripheral *> *systemConnectedPeripherals;

/// FEBluetoothSDKDelegate 代理
@property (nonatomic, weak) id<FEBluetoothSDKDelegate> delegate;

/// 是否开启详细的扫描外设广播日志
@property (nonatomic, assign) BOOL isPrintDiscoverPeripheralLog;

/// 历史 connect: / connect:enableAutoReconnect: 接口是否默认 RequiresANCS。
/// 若旧版 SDK 已经默认打开 RequiresANCS，为了兼容老客户可默认 YES。
/// 新项目建议使用 connect:enableAutoReconnect:requiresANCS: 或 connect:options: 精确控制。
@property (nonatomic, assign) BOOL legacyConnectRequiresANCS;

#pragma mark - Scan

/// 搜索普通通讯设备
/// 历史接口，保持旧行为：allowDuplicates = YES。
- (BOOL)scanForPeripheralsWithServices:(nullable NSArray<CBUUID *> *)serviceUUIDs
                                 block:(ScanBlock)scanBlock;

/// 搜索普通通讯设备，增强接口
- (BOOL)scanForPeripheralsWithServices:(nullable NSArray<CBUUID *> *)serviceUUIDs
                               options:(nullable FEScanOptions *)options
                                 block:(ScanBlock)scanBlock;

/// 停止搜索
- (void)stopScan;

/// 对“搜索到的外设数组 scanPeripherals”中的数据进行排序（按照 RSSI 从强到弱）
- (void)sort:(void (^)(BOOL isSuccess))sortedComplete;

/// 获取扫描设备快照，线程安全
- (void)getScanPeripheralsSnapshot:(void(^)(NSArray<FEPeripheral *> *peripherals))completion;

#pragma mark - Retrieve

/// 检索已知的外围设备
- (void)retrievePeripheralsWithIdentifiers:(NSArray<NSUUID *> *)identifiers
                                completion:(void(^)(BOOL isSuccess))completion;

/// 检索已知的外围设备，增强接口：返回具体设备数组
- (void)retrievePeripheralsWithIdentifiers:(NSArray<NSUUID *> *)identifiers
                          peripheralsBlock:(FEPeripheralListBlock)completion;

/// 系统当前 BLE 连接态查询（受 GATT Service 可见性限制）
- (void)refreshSystemConnectedPeripheralsForServiceUUIDs:(NSArray<CBUUID *> *)uuids;

/// 系统当前 BLE 连接态查询，增强接口：返回具体设备数组
- (void)retrieveConnectedPeripheralsWithServices:(NSArray<CBUUID *> *)serviceUUIDs
                                      completion:(nullable FEPeripheralListBlock)completion;

/// 获取已检索设备快照，线程安全
- (void)getRetrievedPeripheralsSnapshot:(void(^)(NSArray<FEPeripheral *> *peripherals))completion;

/// 获取已连接设备快照，线程安全
- (void)getConnectedPeripheralsSnapshot:(void(^)(NSArray<FEPeripheral *> *peripherals))completion;

/// 获取系统已连接设备快照，线程安全
- (void)getSystemConnectedPeripheralsSnapshot:(void(^)(NSArray<FEPeripheral *> *peripherals))completion;

#pragma mark - Connection Events

/// 注册系统层 BLE 连接/断开事件监听
///
/// 说明：
/// 1. 对应 iOS 13+ registerForConnectionEventsWithOptions:。
/// 2. 监听的是系统层面的 BLE 连接事件，不一定是当前 App 主动发起。
/// 3. serviceUUIDs 和 peripheralUUIDs 至少传一个。
/// 4. serviceUUIDs 最多建议传 4 个。
- (void)registerForConnectionEventsWithServiceUUIDs:(nullable NSArray<CBUUID *> *)serviceUUIDs
                                    peripheralUUIDs:(nullable NSArray<NSUUID *> *)peripheralUUIDs
                                         eventBlock:(nullable FEConnectionEventBlock)eventBlock;

/// 注册系统层 BLE 连接/断开事件监听，带注册结果回调
- (void)registerForConnectionEventsWithServiceUUIDs:(nullable NSArray<CBUUID *> *)serviceUUIDs
                                    peripheralUUIDs:(nullable NSArray<NSUUID *> *)peripheralUUIDs
                                         eventBlock:(nullable FEConnectionEventBlock)eventBlock
                                         completion:(nullable FEOperationCompletion)completion;

/// 取消系统层 BLE 连接/断开事件监听
- (void)unregisterForConnectionEvents;

#pragma mark - Manage

/// 清空搜索到的外设数组、检索到的外围设备数组
- (void)clear;

/// 设置连接成功后要检索的服务 UUID
- (void)setupDiscoverServicesUUIDs:(nullable NSArray<CBUUID *> *)serviceUUIDs;

#pragma mark - Connect

/// 连接设备，历史接口
- (void)connect:(FEPeripheral *)fePeripheral;

/// 连接设备，历史增强接口
- (void)connect:(FEPeripheral *)fePeripheral
enableAutoReconnect:(BOOL)enableAutoReconnect;

/// 连接设备，新增 ANCS 控制
- (void)connect:(FEPeripheral *)fePeripheral
enableAutoReconnect:(BOOL)enableAutoReconnect
   requiresANCS:(BOOL)requiresANCS;

/// 连接设备，完整配置接口
- (void)connect:(FEPeripheral *)fePeripheral
        options:(nullable FEConnectOptions *)options;

/// 断开设备
- (void)disconnect:(nullable FEPeripheral *)fePeripheral;

@end

NS_ASSUME_NONNULL_END
