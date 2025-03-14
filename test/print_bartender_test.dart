import 'package:flutter_test/flutter_test.dart';
import 'package:print_bartender/print_bartender.dart';
import 'package:print_bartender/print_bartender_platform_interface.dart';
import 'package:print_bartender/print_bartender_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockPrintBartenderPlatform
    with MockPlatformInterfaceMixin
    implements PrintBartenderPlatform {

  @override
  Future<String?> getPlatformVersion() => Future.value('42');

  @override
  Future startPrint(String path, List<Map<String, String>> info) {
    // TODO: implement startPrint
    throw UnimplementedError();
  }
}

void main() {
  final PrintBartenderPlatform initialPlatform = PrintBartenderPlatform.instance;

  test('$MethodChannelPrintBartender is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelPrintBartender>());
  });

  test('getPlatformVersion', () async {
    PrintBartender printBartenderPlugin = PrintBartender();
    MockPrintBartenderPlatform fakePlatform = MockPrintBartenderPlatform();
    PrintBartenderPlatform.instance = fakePlatform;

    expect(await printBartenderPlugin.getPlatformVersion(), '42');
  });
}
