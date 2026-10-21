# FEBluetoothSDK 通用版接入指南

SDK 版本：1.1.2 · 发行日期：2026-10-21 · 源码分支：DEV_JWH_Generic · 提交：3f194cf

## 1. 包内容与适用范围

本包公开 BLE 扫描、连接、GATT 透传、FACP、L2CAP、AT 指令、Wi-Fi 模组配网和通用 FEOTA DFU 接口。仅适用于支持对应协议的 Feasycom 模组；“通用”不代表可对任意 BLE 设备升级。

不公开 BT671、BT681、BT2094、BP106、BT976 等型号专用 OTA 接口。静态库从当前分支完整编译，内部仍保留这些实现；本包通过公开头文件限定接入范围，并非裁剪过内部符号的精简库。

- `FEBluetoothSDK.xcframework`：真机 arm64，模拟器 arm64 / x86_64。
- `StaticLibrary/`：仅真机的 `libFEBluetoothSDK.a` 与 `Headers/`。
- `Documentation/`：中文及英文指南。
- `BUILD-INFO.json`：构建信息；`SHA256SUMS`：文件校验值。

本次二进制最低支持 iOS 15.0。源码工程原目标为 iOS 11，本机 Xcode 27.1 的 SDK 已不支持该目标，因此本包使用 iOS 15.0 编译；不应宣称支持 iOS 11。版本号沿用源码中的 `SDKVersion = 1.1.2`。

## 2. Xcode 安装

推荐将 `FEBluetoothSDK.xcframework` 拖入工程，勾选 Copy items。加入 App target 的 Frameworks, Libraries, and Embedded Content，并选择 **Do Not Embed**（静态库）。

也可加入 `StaticLibrary/libFEBluetoothSDK.a`，将 `StaticLibrary/Headers` 加入 Header Search Paths。该方式只支持真机，不可用于模拟器。两种方式二选一，避免重复符号。

链接 `Foundation.framework`、`CoreBluetooth.framework` 和 `UIKit.framework`，在 Other Linker Flags 添加 `$(inherited) -ObjC`。App 最低部署版本设为 iOS 15.0 或更高。

Objective-C 引入：

```objc
#import <CoreBluetooth/CoreBluetooth.h>
#import "FBlueSDKPublicHeader.h"
```

Swift 工程在 Objective-C Bridging Header 加入上述 import；使用包中的模块定义时也可尝试 `import FEBluetoothSDK`。下文以 Objective-C 头文件签名为准。

在 Info.plist 配置 `NSBluetoothAlwaysUsageDescription`，例如“用于连接设备、传输数据和升级固件”。若确实需要后台 BLE，在 Signing & Capabilities 开启 Background Modes → Uses Bluetooth LE accessories；后台执行时间仍由 iOS 管理。模拟器只能验证编译链接，BLE 联调必须使用真机。

## 3. 初始化与扫描

持有 SDK 单例并设置代理；等待蓝牙 `PoweredOn` 后扫描。代理为 weak，需由 App 持有其对象。

```objc
self.sdk = [FEBluetoothSDK sharedFEBluetoothSDK];
self.sdk.delegate = self; // 当前类声明遵循 FEBluetoothSDKDelegate

- (void)feBluetoothSDK:(FEBluetoothSDK *)sdk
        didUpdateState:(FEBluetoothSDKState)state {
    if (state != FEBluetoothSDKState_PoweredOn) return;
    FEScanOptions *options = [FEScanOptions defaultOptions];
    options.allowDuplicates = NO;
    [sdk scanForPeripheralsWithServices:nil options:options
                                 block:^(FEPeripheral *device, BOOL isNew) {
        NSLog(@"%@ %@ %@", device.displayName, device.identifier, device.RSSI);
        // 显示设备列表，让用户选中后连接；不要在每次扫描回调中自动连接。
    }];
}
```

扫描可按设备服务 UUID 过滤；nil 表示不限定服务。结束搜索时调用 `[self.sdk stopScan]`。高频扫描回调中刷新 UI 应节流。使用 `getScanPeripheralsSnapshot:` 等快照接口读取设备列表，不直接修改 SDK 的可变数组。

## 4. 连接、模式与接收

连接前注册回调。App 需要声明并持有 `sdk`、`device` 和后文的 `atCommand`、`ota` 属性。以下 `device` 是用户选中的扫描结果。

```objc
self.device = device;
__weak typeof(self) weakSelf = self;
[device callBackConnectStatus:^{
    if (weakSelf.device.connectState == CONNECTSTATE_SUCCESS) {
        NSLog(@"链路已连接；继续等待通讯模式就绪");
    }
} connectType:^{
    FEPeripheral *p = weakSelf.device;
    if (p.connectType == FEPeripheralConnectType_communication) {
        NSLog(@"透传通道已就绪");
    } else if (p.connectType == FEPeripheralConnectType_atCommand) {
        NSLog(@"AT 通道已就绪");
    } else if (p.connectType == FEPeripheralConnectType_ota) {
        // 在此启动已准备好的 FEOTA，见第 7 节。
    }
} deviceInfo:^(FEPeripheral *p, FEDeviceInfo *info) {
    NSLog(@"型号=%@ 固件=%@", info.modelName, info.system_version);
} RSSI:nil send:^(FEPeripheral *p, NSError *error, BOOL pause, BOOL finish) {
    NSLog(@"发送 error=%@ pause=%d finish=%d", error, pause, finish);
} receive:^(FEPeripheral *p, CBCharacteristic *ch, NSError *error) {
    if (!error) NSLog(@"接收原始字节：%@", ch.value);
}];
[self.sdk stopScan];
FEConnectOptions *options = [FEConnectOptions defaultOptions];
options.requiresANCS = NO;
options.enableAutoReconnect = NO;
[self.sdk connect:device options:options];
```

`connectState` 是链路状态，`connectType` 是通讯模式；不要把连接成功等同于可立即发送。`enableAutoReconnect` 对应的增强能力要求 iOS 17+，iOS 15/16 应由 App 按业务需要管理重连。UI 更新统一切回主线程。一次注册会替换该外设原有回调，AT/OTA 辅助对象也可能接管回调，避免重复注册覆盖当前操作。

## 5. 透传与自定义 GATT

```objc
// 已连接后请求普通透传；等待 communication 模式回调再发送。
[self.device enterCommunicationModeWithFacp:NO];
// 仅在 communication 就绪时执行：
NSData *payload = [@"hello\r\n" dataUsingEncoding:NSUTF8StringEncoding];
[self.device send:payload withResponse:NO];
```

`withResponse:YES` 使用 Write，NO 使用 WriteWithoutResponse，必须与选定特征的属性匹配。发送完成仅表示 SDK 发送流程完成，业务确认应由应用协议提供。二进制接收使用 `NSData`；不要假设每次通知都是完整 UTF-8 文本或完整业务帧。

默认通道不符合设备协议时，从 `allWriteCharacteristic`、`allWriteWithoutResponseCharacteristic`、`allNotifyCharacteristic` 中按实际 UUID 选择，再调用 `setupWriteCharacteristic:`、`setupWriteWithoutResponseCharacteristic:`、`setupNotifyValue:forCharacteristic:`。不要随意选择第一个特征。

可通过 `setPackageSize:` 设置分包、`sendFinishInterval:` 设置包间隔（毫秒），包长需符合实际 GATT 写入上限。使用 `pauseSend`、`continueSend`、`stopSend` 控制队列。FACP 需设备固件支持；L2CAP 需设备提供正确 PSM，调用 `enterCommunicationModeWithL2CAPChannel:` 并等待 `isL2CAPOpened`，接收注册 `callBackL2CAPReceive:`。

## 6. AT 与 Wi-Fi

```objc
[self.device enterAtCommandMode];
// 等待 connectType == FEPeripheralConnectType_atCommand 后执行：
self.atCommand = [[FEAtCommand alloc] initWithPeripheral:self.device];
BOOL accepted = [self.atCommand send:@"AT\r\n"
                          complete:^(NSString *key, NSString *value, NSError *error) {
    NSLog(@"AT value=%@ error=%@", value, error);
}];
if (!accepted) NSLog(@"指令未发送，请检查模式和 GATT 写通道");
```

指令、大小写及结束符应以目标固件 AT 手册为准；返回 YES 表示已接受发送，不代表设备执行成功。查询和修改分别使用 `check:complete:`、`change:value:complete:`。退出 AT 的方式也以设备协议为准，不能仅修改 `connectType` 属性来切换设备模式。

支持蓝牙配网的 Wi-Fi 模组可使用 `FEWiFi`（继承 `FEAtCommand`），进入 AT 模式后调用 `btConfigDynamicNetwork:password:reconnect:completeBlock:` 或静态配网接口。不要记录 Wi-Fi 密码；普通 BLE 模组不具备此能力。

## 7. 通用 DFU 升级

仅使用目标模组匹配的完整 DFU 文件，不能替换为裸 BIN。App 持有 `FEOTA` 到流程结束，升级期间禁止普通业务发送。

```objc
NSData *dfuData = [NSData dataWithContentsOfURL:firmwareURL];
if (dfuData.length == 0) return;
self.ota = [[FEOTA alloc] initWithPeripheral:self.device dfuData:dfuData];
[self.device enterOtaModeWithFactory:NO]; // YES 表示请求恢复出厂配置
// 在第 4 节 connectType 回调中，确认 ota 模式后仅启动一次：
[self.ota startProgress:^(float progress, NSInteger total, NSInteger sent) {
    NSLog(@"OTA %.1f%% %ld/%ld", progress * 100, (long)sent, (long)total);
} finish:^{
    NSLog(@"传输结束；设备重启后重连检查版本");
} abort:^{
    NSLog(@"升级中止，请重新检查连接、模式和固件");
} timeout:^{
    NSLog(@"升级超时，请重新进入 OTA 模式后重试");
}];
```

以上两段是顺序示意：不能在调用 `enterOtaModeWithFactory:` 后立即调用 `startProgress:`，应将启动部分放入模式回调并用 App 状态防止重复启动。本分支仅提供单参数进入接口，不提供 completion 或 `cancelOtaModeEntry`。模式切换中若设备断连，App 需重新发现、连接并确认 OTA 模式；不要假设有自动切换重连闭环。

`FEOTA` 会接管外设回调。开始后不要再次注册透传回调。当前公开 API 无专用取消方法；中途主动断开后，App 应自行结束 UI 状态，不能依赖一定收到 abort/timeout。重试从完整文件开始。finish 在末包发送后约延迟 3 秒，表示传输流程完成，最终升级成功必须通过设备重启后的版本确认。连续无有效进展约 10 秒会超时。

## 8. 断开与排查

主动断开使用 `[self.sdk disconnect:self.device]`，退出页面时停止扫描并按业务清理对象。

- 扫描不到：检查真机权限、蓝牙状态、广播和服务过滤，确认设备未被其他主机占用。
- 已连接但发不出：确认 communication 模式、特征 UUID、写属性和通知订阅。
- 重复符号：确认没有同时链接 XCFramework、旧静态库或源码。
- Undefined symbols：检查 `-ObjC`、系统框架及当前平台 slice，重新清理构建。
- OTA 立即 abort：检查连接、ota 模式、完整 DFU 格式和模组型号。
- OTA 断连无结束回调：App 应监听业务连接状态并自行结束升级 UI；当前版本不能保证所有异常路径都有结束回调。

## 9. 验证范围

本包验证真机 arm64、模拟器 arm64/x86_64 Release 编译、公开头文件编译和最小消费端链接、归档解压及校验值。未进行真实蓝牙设备的透传、AT、配网或 DFU 验证；这些功能需在目标模组固件上验收。
