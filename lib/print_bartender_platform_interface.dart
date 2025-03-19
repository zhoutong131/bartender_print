import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'print_bartender_method_channel.dart';

abstract class PrintBartenderPlatform extends PlatformInterface {
  /// Constructs a PrintBartenderPlatform.
  PrintBartenderPlatform() : super(token: _token);

  static final Object _token = Object();

  static PrintBartenderPlatform _instance = MethodChannelPrintBartender();

  /// The default instance of [PrintBartenderPlatform] to use.
  ///
  /// Defaults to [MethodChannelPrintBartender].
  static PrintBartenderPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [PrintBartenderPlatform] when
  /// they register themselves.
  static set instance(PrintBartenderPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }

  Future startPrint(String path,String info,{String? printerName}) {
    throw UnimplementedError('startPrint() has not been implemented.');
  }

  Future<String?> getDefaultPrinterName() {
    throw UnimplementedError('getDefaultPrinterName() has not been implemented.');
  }

  Future<List<String>?> getPrinterList () {
    throw UnimplementedError('getPrinterList() has not been implemented.');
  }
}
