# FEBluetoothSDK Generic Edition — Integration Guide

SDK: 1.1.2 · Release: 2026-10-21 · Branch: DEV_JWH_Generic · Commit: 3f194cf

## 1. Contents and scope

This package exposes BLE scanning and connections, GATT data transfer, FACP, L2CAP, AT commands, Wi-Fi module provisioning, and generic FEOTA DFU. Devices must support the corresponding Feasycom protocols. Generic does not mean firmware updates for arbitrary BLE devices.

Model-specific OTA APIs for BT671, BT681, BT2094, BP106 and BT976 are not exposed. The library is compiled from the full branch and still contains their internal implementations. Public headers define the supported integration surface; this is not a symbol-stripped minimal library.

- `FEBluetoothSDK.xcframework`: arm64 iOS devices; arm64 and x86_64 simulators.
- `StaticLibrary/`: device-only `libFEBluetoothSDK.a` and `Headers/`.
- `Documentation/`: Chinese and English guides.
- `BUILD-INFO.json`: build metadata; `SHA256SUMS`: file checksums.

This binary requires **iOS 15.0 or later**. The source project targets iOS 11, which the installed Xcode 27.1 SDK no longer supports. This distribution overrides the build target to iOS 15.0 and must not be advertised as supporting iOS 11. The source version string remains `SDKVersion = 1.1.2`.

## 2. Install in Xcode

Recommended: drag `FEBluetoothSDK.xcframework` into the project and copy the files. Add it to the app target's Frameworks, Libraries, and Embedded Content, using **Do Not Embed** because it is static.

Alternatively, link `StaticLibrary/libFEBluetoothSDK.a` and add `StaticLibrary/Headers` to Header Search Paths. This alternative supports devices only. Choose one integration method to avoid duplicate symbols.

Link `Foundation.framework`, `CoreBluetooth.framework` and `UIKit.framework`. Add `$(inherited) -ObjC` to Other Linker Flags. Set the app deployment target to iOS 15.0 or later.

```objc
#import <CoreBluetooth/CoreBluetooth.h>
#import "FBlueSDKPublicHeader.h"
```

For Swift, add these imports to the Objective-C bridging header. The included module map may also be used with `import FEBluetoothSDK`. Examples below use the Objective-C API signatures.

Add `NSBluetoothAlwaysUsageDescription` to Info.plist with a clear explanation such as “Connect to devices, transfer data and update firmware.” Enable Background Modes → Uses Bluetooth LE accessories only if required; iOS still controls background execution time. Simulators can validate compilation and linking, but BLE integration requires physical devices.

## 3. Initialize and scan

Keep the SDK singleton and assign a retained delegate object; the delegate property itself is weak. Wait for PoweredOn before scanning.

```objc
self.sdk = [FEBluetoothSDK sharedFEBluetoothSDK];
self.sdk.delegate = self; // This class conforms to FEBluetoothSDKDelegate.

- (void)feBluetoothSDK:(FEBluetoothSDK *)sdk
        didUpdateState:(FEBluetoothSDKState)state {
    if (state != FEBluetoothSDKState_PoweredOn) return;
    FEScanOptions *options = [FEScanOptions defaultOptions];
    options.allowDuplicates = NO;
    [sdk scanForPeripheralsWithServices:nil options:options
                                 block:^(FEPeripheral *device, BOOL isNew) {
        NSLog(@"%@ %@ %@", device.displayName, device.identifier, device.RSSI);
        // Present results and connect after the user selects a device.
    }];
}
```

Pass known service UUIDs to filter results, or nil for no service restriction. Call `[self.sdk stopScan]` when finished. Throttle UI updates for frequent scan callbacks. Use snapshot methods such as `getScanPeripheralsSnapshot:` instead of modifying SDK-owned mutable arrays.

## 4. Connect and observe mode changes

Register callbacks before connecting. Declare strong properties for `sdk`, `device`, and the `atCommand` and `ota` objects used later. Here, `device` is a selected scan result.

```objc
self.device = device;
__weak typeof(self) weakSelf = self;
[device callBackConnectStatus:^{
    if (weakSelf.device.connectState == CONNECTSTATE_SUCCESS) {
        NSLog(@"Link connected; wait for the communication mode");
    }
} connectType:^{
    FEPeripheral *p = weakSelf.device;
    if (p.connectType == FEPeripheralConnectType_communication) {
        NSLog(@"Data channel ready");
    } else if (p.connectType == FEPeripheralConnectType_atCommand) {
        NSLog(@"AT channel ready");
    } else if (p.connectType == FEPeripheralConnectType_ota) {
        // Start the prepared FEOTA once here; see section 7.
    }
} deviceInfo:^(FEPeripheral *p, FEDeviceInfo *info) {
    NSLog(@"Model=%@ firmware=%@", info.modelName, info.system_version);
} RSSI:nil send:^(FEPeripheral *p, NSError *error, BOOL pause, BOOL finish) {
    NSLog(@"Send error=%@ pause=%d finish=%d", error, pause, finish);
} receive:^(FEPeripheral *p, CBCharacteristic *ch, NSError *error) {
    if (!error) NSLog(@"Received bytes: %@", ch.value);
}];
[self.sdk stopScan];
FEConnectOptions *options = [FEConnectOptions defaultOptions];
options.requiresANCS = NO;
options.enableAutoReconnect = NO;
[self.sdk connect:device options:options];
```

`connectState` describes the link, while `connectType` describes the operating mode. A connected link does not guarantee that the application channel is ready. The enhanced `enableAutoReconnect` capability requires iOS 17+; manage reconnection in the app on iOS 15/16 as needed. Dispatch UI work to the main queue. Callback registration replaces the peripheral's previous callbacks; AT/OTA helpers may also take ownership of them.

## 5. Data transfer and custom GATT

```objc
// Request transparent mode after connecting.
[self.device enterCommunicationModeWithFacp:NO];
// Execute only after the communication mode callback:
NSData *payload = [@"hello\r\n" dataUsingEncoding:NSUTF8StringEncoding];
[self.device send:payload withResponse:NO];
```

YES selects Write; NO selects WriteWithoutResponse. The selected characteristic must support the requested operation. Send completion describes the SDK send flow, not application-level acknowledgement. Process received bytes as NSData and implement framing; a notification may contain only part of a message or UTF-8 sequence.

For custom devices, select characteristics by the actual service/characteristic UUIDs using `allWriteCharacteristic`, `allWriteWithoutResponseCharacteristic` and `allNotifyCharacteristic`, then configure them using `setupWriteCharacteristic:`, `setupWriteWithoutResponseCharacteristic:` and `setupNotifyValue:forCharacteristic:`. Do not select the first characteristic arbitrarily.

`setPackageSize:` sets packet size; `sendFinishInterval:` sets the interval in milliseconds. Respect the negotiated GATT write limit. Control the queue with `pauseSend`, `continueSend` and `stopSend`. FACP requires matching firmware support. L2CAP requires the device's correct PSM: use `enterCommunicationModeWithL2CAPChannel:`, wait for `isL2CAPOpened`, and register `callBackL2CAPReceive:`.

## 6. AT commands and Wi-Fi

```objc
[self.device enterAtCommandMode];
// Execute after connectType becomes FEPeripheralConnectType_atCommand:
self.atCommand = [[FEAtCommand alloc] initWithPeripheral:self.device];
BOOL accepted = [self.atCommand send:@"AT\r\n"
                          complete:^(NSString *key, NSString *value, NSError *error) {
    NSLog(@"AT value=%@ error=%@", value, error);
}];
if (!accepted) NSLog(@"Not sent; check the mode and GATT channel");
```

Use the target firmware's AT manual for supported commands, casing and line endings. YES means the command was accepted for sending, not that the device executed it successfully. Use `check:complete:` for queries and `change:value:complete:` for updates. Follow the device protocol to leave AT mode; assigning `connectType` does not switch the physical device's mode.

For Wi-Fi modules supporting Bluetooth provisioning, use `FEWiFi`, a subclass of `FEAtCommand`. After entering AT mode, call `btConfigDynamicNetwork:password:reconnect:completeBlock:` or the static network API. Do not log Wi-Fi passwords. Standard BLE modules do not necessarily support these features.

## 7. Generic DFU

Use a complete DFU file matching the target module, not a raw BIN file. Retain FEOTA until the operation ends and stop ordinary business traffic during the update.

```objc
NSData *dfuData = [NSData dataWithContentsOfURL:firmwareURL];
if (dfuData.length == 0) return;
self.ota = [[FEOTA alloc] initWithPeripheral:self.device dfuData:dfuData];
[self.device enterOtaModeWithFactory:NO]; // YES requests factory reset.
// Run the following from the section 4 mode callback, once ota is ready:
[self.ota startProgress:^(float progress, NSInteger total, NSInteger sent) {
    NSLog(@"OTA %.1f%% %ld/%ld", progress * 100, (long)sent, (long)total);
} finish:^{
    NSLog(@"Transfer finished; reconnect after reboot and verify the version");
} abort:^{
    NSLog(@"Aborted; check the connection, mode and firmware");
} timeout:^{
    NSLog(@"Timed out; enter OTA mode again before retrying");
}];
```

These are sequential integration fragments: do not call `startProgress:` immediately after requesting OTA mode. Put it inside the mode callback and guard against duplicate starts. This branch exposes only the single-argument mode-entry API; it does not expose a completion overload or `cancelOtaModeEntry`. If the device disconnects while switching modes, the app must rediscover/reconnect and confirm OTA readiness. Do not assume an automatic mode-switch reconnection flow.

FEOTA takes over peripheral callbacks; do not re-register transparent-transfer callbacks during DFU. There is no dedicated public cancel API. If the app disconnects during an update, end the update UI explicitly: abort/timeout callbacks are not guaranteed for every disconnect path. Retry with the complete file. The finish callback is delayed by about three seconds after the last packet and indicates transfer completion. Verify the actual firmware version after reboot. Roughly ten seconds without valid progress triggers a timeout.

## 8. Disconnect and troubleshoot

Call `[self.sdk disconnect:self.device]` to disconnect. Stop scanning and release session objects as appropriate when leaving the screen.

- No scan results: check physical-device permissions, Bluetooth state, advertising, service filters and other hosts occupying the device.
- Connected but unable to send: check communication mode, characteristic UUIDs, write properties and notification subscriptions.
- Duplicate symbols: remove duplicate integration of the XCFramework, old static libraries or source files.
- Undefined symbols: check `-ObjC`, required system frameworks and platform slices, then clean the build.
- Immediate OTA abort: check the link, ota mode, complete DFU format and module compatibility.
- No terminal callback after OTA disconnect: the app must end its own update UI; this version does not guarantee terminal callbacks for all failure paths.

## 9. Validation

This distribution is checked for Release builds on device arm64 and simulator arm64/x86_64, public-header compilation, minimal consumer linking, archive extraction and checksums. BLE data transfer, AT, Wi-Fi provisioning and DFU have not been verified on physical modules and require acceptance testing against the target firmware.
