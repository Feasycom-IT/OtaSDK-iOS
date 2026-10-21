# FEBluetoothSDK for iOS — Generic Edition (OTA SDK)

飞易通 iOS 通用蓝牙 SDK，提供 BLE 扫描、连接、GATT 透传、FACP、L2CAP、AT 指令、Wi-Fi 模组配网和通用 FEOTA DFU 固件升级接口。SDK 以预编译静态库和公开头文件交付，**不包含实现源码**。

> Feasycom generic Bluetooth SDK for iOS: BLE scanning and connections, GATT data transfer, FACP, L2CAP, AT commands, Wi-Fi module provisioning and generic FEOTA DFU. Delivered as a prebuilt static library with public headers; implementation source is not included.

| 项目 / Item | 值 / Value |
| --- | --- |
| SDK 版本 / Version | 1.1.2 |
| 发行日期 / Release date | 2026-10-21 |
| 最低系统 / Minimum iOS | iOS 15.0 |
| 真机 / Device | arm64 |
| 模拟器 / Simulator | arm64, x86_64（仅 XCFramework / XCFramework only） |

## 1. 文档 / Documentation

- [中文接入指南](FEBluetoothSDK-Generic-1.1.2/Documentation/Generic-SDK-Guide.zh-CN.md)
- [English integration guide](FEBluetoothSDK-Generic-1.1.2/Documentation/Generic-SDK-Guide.en.md)

## 2. 目录 / Contents

```text
FEBluetoothSDK-Generic-1.1.2/
├── FEBluetoothSDK.xcframework/   # 推荐：真机 + 模拟器 / Recommended: device + simulator
├── StaticLibrary/                # 仅真机：libFEBluetoothSDK.a + Headers/ / Device only
├── Documentation/                # 中英文接入指南 / Guides (zh-CN, en)
├── BUILD-INFO.json               # 构建信息 / Build metadata
└── SHA256SUMS                    # 文件校验值 / Checksums
```

## 3. 快速接入 / Quick start

1. 将 `FEBluetoothSDK.xcframework` 拖入工程，在 App target 的 Frameworks, Libraries, and Embedded Content 中选择 **Do Not Embed**（静态库）。
   Drag `FEBluetoothSDK.xcframework` into the project and set it to **Do Not Embed** (static library).
2. 链接 `Foundation`、`CoreBluetooth`、`UIKit`，在 Other Linker Flags 添加 `$(inherited) -ObjC`。
   Link `Foundation`, `CoreBluetooth`, `UIKit` and add `$(inherited) -ObjC` to Other Linker Flags.
3. 在 Info.plist 配置 `NSBluetoothAlwaysUsageDescription`。
   Add `NSBluetoothAlwaysUsageDescription` to Info.plist.
4. 导入头文件 / Import:

```objc
#import <CoreBluetooth/CoreBluetooth.h>
#import "FBlueSDKPublicHeader.h"
```

XCFramework 与 `StaticLibrary/` 二选一，不要同时链接。BLE 联调必须使用真机。
Use either the XCFramework or `StaticLibrary/`, not both. BLE features must be tested on a real device.

## 4. 获取 / Download

```bash
git clone https://github.com/Feasycom-IT/OtaSDK-iOS.git
```

校验文件完整性 / Verify checksums:

```bash
cd FEBluetoothSDK-Generic-1.1.2 && shasum -a 256 -c SHA256SUMS
```

## 5. 适用范围 / Scope

- 仅适用于支持对应协议的 Feasycom 模组；“通用”不代表可对任意 BLE 设备升级。
  Only for Feasycom modules that support the corresponding protocols.
- 不公开 BT671、BT681、BT2094、BP106、BT976 等型号专用 OTA 接口。
  Model-specific OTA APIs (BT671, BT681, BT2094, BP106, BT976, etc.) are not exposed.
- 本版本已验证 Release 编译与链接，未进行真实蓝牙设备的透传、AT、配网或 DFU 验收。
  Release builds and linking were validated; no hardware tests (data transfer, AT, provisioning, DFU) were performed.

## 6. 技术支持 / Support

飞易通 Feasycom：<https://www.feasycom.com>
