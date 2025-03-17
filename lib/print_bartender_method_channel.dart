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
    var transferInfo = {
      "btwPath": data['path'],
      "info": (data['info'] as List<dynamic>).map((ele) => (ele as Map<String,String>)).toList()
    };
    if (data['printerName'] != null && data['printerName'].toString().isNotEmpty) {
      transferInfo['printerName'] = data['printerName'];
    }
    final result = await methodChannel.invokeMethod<Map>("printLabel",transferInfo);
    s.send(result);
  }

  @override
  Future startPrint(String path, List<Map<String, String>> info,{String? printerName}) async {
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
      "info": info,
      "printerName": printerName
    });
    var resInfo = await receivePort.first;
    backgroundTask.kill(priority: Isolate.immediate);
    receivePort.close();
    return resInfo;
  }

  @override
  Future<String?> getDefaultPrinterName() async {
    // TODO: implement getDefaultPrinterName
    final result = await methodChannel.invokeMethod<String?>("getDefaultPrinter");
    return result;
  }

  @override
  Future<List<String>?> getPrinterList() async {
    // TODO: implement getPrinterList
    final result = await methodChannel.invokeListMethod<String>("getPrintList");
    return result;
  }


}
