//
//  FEPeripheral.h
//  FEBluetoothSDK
//
//  Created by ericj on 2021/1/22.
//  Copyright © 2021 Feasycom. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "FEDeviceInfo.h"
@class FEPeripheral;

/// 连接状态
typedef NS_ENUM(NSInteger, CONNECTSTATE) {
    CONNECTSTATE_DISCONNECT,    // 断开连接
    CONNECTSTATE_SUCCESS,       // 连接成功
    CONNECTSTATE_FAIL,          // 连接失败
};

/// 通讯模式(未知、普通通讯、AT指令、OTA)
typedef NS_ENUM(NSInteger, FEPeripheralConnectType) {
    FEPeripheralConnectType_none = 0, // 未知或初始状态
    
    FEPeripheralConnectType_communication_prepare,  // 普通通讯模式准备中
    FEPeripheralConnectType_communication, // 普通通讯模式
    
    FEPeripheralConnectType_atCommand_prepare, // AT指令模式准备中
    FEPeripheralConnectType_atCommand, // AT指令模式
    
    FEPeripheralConnectType_ota_prepare, // OTA模式准备中
    FEPeripheralConnectType_ota // OTA模式
};

/// Facp通讯协议版本（使用前先确定模块是否支持）
typedef NS_ENUM(NSInteger, FSCFacpType) {
    FSCFacpType_2_0 = 0, //
    FSCFacpType_2_1      // 支持报错重传
};

/// 连接状态的回调
typedef void(^_Nullable ConnectStatusUpdateBlock)(void);

/// 通讯模式的回调
typedef void(^_Nullable ConnectTypeUpdateBlock)(void);

/// 读取到设备信息的回调
typedef void(^_Nullable DeviceInfoBlock)(FEPeripheral * _Nonnull peripheral, FEDeviceInfo * _Nullable info);

/// 读取到RSSI信号的回调
typedef void(^_Nullable RssiBlock)(FEPeripheral * _Nonnull peripheral, NSNumber * _Nullable RSSI, NSError * _Nullable error);

/// 向蓝牙外设发送数据的回调
typedef void(^_Nullable SendBlock)(FEPeripheral * _Nonnull peripheral, NSError* _Nullable error, BOOL pause, BOOL finish);

/// 接收蓝牙外设返回数据的回调
typedef void(^_Nullable ReceiveBlock)(FEPeripheral * _Nonnull peripheral, CBCharacteristic * _Nonnull characteristic, NSError * _Nullable error);

typedef void(^_Nullable ReceiveL2CAPBlock)(FEPeripheral * _Nonnull peripheral, NSData * _Nonnull data, NSError * _Nullable error);

NS_ASSUME_NONNULL_BEGIN

@interface FEPeripheral : NSObject <CBPeripheralDelegate>

/// 当前连接状态
@property (nonatomic, assign) CONNECTSTATE connectState;
/// 当前连接模式(普通通讯、修改设备信息、空中升级)
@property (nonatomic, assign) FEPeripheralConnectType connectType;
/// 指定Facp通讯协议版本
@property (nonatomic, assign) FSCFacpType facpTpye;

/// 当前是否已打开 L2CAP Channel
@property (nonatomic, assign, readonly) BOOL isL2CAPOpened;
/// 当前 L2CAP 的 PSM（仅当 enterCommunicationModeWithL2CAPChannel: 后有值）
@property (nonatomic, assign, readonly) CBL2CAPPSM l2capPSM;

/// 是否获取完整GATT数据结构
@property (nonatomic, assign) BOOL isGetCompleteGATTStructure;

/// 信号值
@property (nonatomic, strong) NSNumber *RSSI;
/// 设备唯一标识UUID（由主端设备与从端设备共同生成，即对于同一外设被不同的主端设备扫描时的UUID是不一样的）
/// 主端设备不变情况下，在手机重启、蓝牙开关重启、不同AppID、不同中心蓝牙对象等操作下，同一蓝牙外设UUID不变
@property(readonly, nonatomic) NSUUID *identifier;
/// 广播内容对象
@property (nonatomic, strong) NSDictionary *advertisementData;
/// Local名称
@property (nonatomic, copy) NSString *displayName;
/// peripheralName
@property (nonatomic, copy) NSString *peripheralName;
/// 设备信息
@property (nonatomic, strong, nullable) FEDeviceInfo *deviceInfo;

// MARK: 设置回调 

/// @param connectStatusUpdateBlock 连接状态发生改变
/// @param deviceInfoBlock 读取模块信息
/// @param rssiBlock 信号值
/// @param sendBlock 发送回调（error：发送错误信息，发送成功为nil。pause：表示是否暂停。finish：表示是否发送完毕）
/// @param receiveBlock 接收数据
- (void)callBackConnectStatus:(ConnectStatusUpdateBlock)connectStatusUpdateBlock
                  connectType:(ConnectTypeUpdateBlock)connectTypeUpdateBlock
                   deviceInfo:(DeviceInfoBlock)deviceInfoBlock
                         RSSI:(RssiBlock)rssiBlock
                         send:(SendBlock)sendBlock
                      receive:(ReceiveBlock)receiveBlock;

/// 单独注册 L2CAP 接收回调（不影响原 receiveBlock）
- (void)callBackL2CAPReceive:(ReceiveL2CAPBlock)receiveL2CAPBlock;

// MARK: 设置通讯模式

/// 进入数据透传模式
/// - Parameter isUseFacp: 是否使用Facp协议
- (void)enterCommunicationModeWithFacp:(BOOL)isUseFacp;

/// 进入透传模式，并尝试打开指定 PSM 的 L2CAP Channel
- (void)enterCommunicationModeWithL2CAPChannel:(CBL2CAPPSM)PSM;

/// 关闭当前 L2CAP Channel
- (void)closeL2CAPChannel;

/// 进入AT指令模式
- (void)enterAtCommandMode;

/// 进入OTA升级模式
/// - Parameter isFactory: 升级后模块是否恢复出厂配置
- (void)enterOtaModeWithFactory:(BOOL)isFactory;

// MARK: 设置通讯通道

/// 获取所有服务
- (NSArray<CBService *> *)allServices;

/// 获取所有带响应写特征
- (NSArray<CBCharacteristic *> *)allWriteCharacteristic;
/// 获取当前带响应写的特征
- (CBCharacteristic*)writeCharacteristic;
/// 设置带响应写的特征
- (BOOL)setupWriteCharacteristic:(CBCharacteristic *)characteristic;

/// 获取所有不带响应写特征
- (NSArray<CBCharacteristic *> *)allWriteWithoutResponseCharacteristic;
/// 获取当前不带响应写的特征
- (CBCharacteristic*)writeWithoutResponseCharacteristic;
/// 设置不带响应写的特征
- (BOOL)setupWriteWithoutResponseCharacteristic:(CBCharacteristic *)characteristic;

/// 获取所有Notify特征
- (NSArray<CBCharacteristic *> *)allNotifyCharacteristic;
/// Notify特征
- (void)setupNotifyValue:(BOOL)value forCharacteristic:(CBCharacteristic *)characteristic;

// MARK: 读取特征值

/// 获取所有读特征
- (NSArray<CBCharacteristic *> *)allReadCharacteristic;
/// Read特征
- (void)readValueForCharacteristic:(CBCharacteristic *)characteristic;
/// 读取RSSI值
- (void)readRSSI;

// MARK: 描述符

/// 更新描述符值
- (void)updateDescriptors;

// MARK: 发送相关

/// 设置分包大小
- (void)setPackageSize:(NSInteger)size;

/// 分包发送完成后间隔时间（毫秒）发下一分包
- (void)sendFinishInterval:(NSInteger)mSeconds;

/// 发送数据
/// @param data 发送的数据
/// @param isResponse YES-Write（带响应的写）；NO-WriteWithoutResponse（不带响应的写）
- (void)send:(NSData * _Nullable)data withResponse:(BOOL)isResponse;

/// 清除记录(不清除辅助变量)
- (void)clear;
/// 清除单次记录(不清除辅助变量)
- (void)clearSingleRecord;
/// 清除所有记录(不清除辅助变量)
- (void)clearAllRecord;

/// 已发送数量
@property (nonatomic, readonly) NSInteger sendCount;
/// 已接收数量
@property (nonatomic, readonly) NSInteger receiveCount;
/// 已发送包数
@property (nonatomic, readonly) NSInteger sePackageCount;
/// 已接收包数
@property (nonatomic, readonly) NSInteger rePackageCount;
/// 已发送时间
@property (nonatomic, readonly) NSTimeInterval sendTimeInterval;
/// 已接收crc32
@property (nonatomic, readonly) NSInteger crc32_re;
/// 已发送crc32
@property (nonatomic, readonly) NSInteger crc32_se;
/// 已接收的参考数据
@property (nonatomic, strong, readonly) NSMutableData *reData;
/// 已累计发送crc32
@property (nonatomic, assign, readonly) NSInteger all_seCRC32;

/// 剩余待发数量
- (NSInteger)countOfReadyToSend;
/// 暂停发送
- (void)pauseSend;
/// 继续发送
- (void)continueSend;
/// 停止发送
- (void)stopSend;

@end

NS_ASSUME_NONNULL_END
