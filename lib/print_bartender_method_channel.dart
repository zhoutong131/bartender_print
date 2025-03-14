import 'dart:io';
import 'dart:isolate';

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'print_bartender_platform_interface.dart';

/// An implementation of [PrintBartenderPlatform] that uses method channels.
class MethodChannelPrintBartender extends PrintBartenderPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('print_bartender');

  @override
  Future<String?> getPlatformVersion() async {
    final version = await methodChannel.invokeMethod<String>('getPlatformVersion');
    return version;
  }

  void startBackground (Map<String,dynamic> data) async {
    SendPort s = data['send'];
    BackgroundIsolateBinaryMessenger.ensureInitialized(data['instance'] as RootIsolateToken);
    final result = await methodChannel.invokeMethod<Map>("printLabel",{
      "btwPath": data['path'],
      "info": (data['info'] as List<dynamic>).map((ele) => (ele as Map<String,String>)).toList()
    });
    s.send(result);
  }

  @override
  Future startPrint(String path, List<Map<String, String>> info) async {
    // TODO: implement startPrint
    if (path.isEmpty) {
      return {
        "res": false,
        "info": "path is empty"
      };
    }
    var btwFile = File(path);
    if (!btwFile.existsSync()) {
      return {
        "res": false,
        "info": "btwFile is not exist"
      };
    }
    String type = btwFile.path.substring(btwFile.path.lastIndexOf(".") + 1);
    if (type != "btw") {
      return {
        "res": false,
        "info": "$type is an unqualified format"
      };
    }
    ReceivePort receivePort = ReceivePort();
    Isolate backgroundTask = await Isolate.spawn(startBackground, {
      "send": receivePort.sendPort,
      "instance": RootIsolateToken.instance!,
      "path": path,
      "info": info
    });
    var resInfo = await receivePort.first;
    backgroundTask.kill(priority: Isolate.immediate);
    receivePort.close();
    return resInfo;
  }


}
