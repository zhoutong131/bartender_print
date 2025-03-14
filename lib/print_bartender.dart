
import 'print_bartender_platform_interface.dart';

class PrintBartender {
  Future<String?> getPlatformVersion() {
    return PrintBartenderPlatform.instance.getPlatformVersion();
  }

  Future startPrint(String path,List<Map<String,String>> info) {
    return PrintBartenderPlatform.instance.startPrint(path, info);
  }
}
