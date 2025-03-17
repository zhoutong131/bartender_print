
import 'print_bartender_platform_interface.dart';

class PrintBartender {
  Future<String?> getPlatformVersion() {
    return PrintBartenderPlatform.instance.getPlatformVersion();
  }

  Future startPrint(String path,List<Map<String,String>> info,{String? printerName}) {
    return PrintBartenderPlatform.instance.startPrint(path, info,printerName: printerName);
  }

  Future<String?> getDefaultPrinterName () {
    return PrintBartenderPlatform.instance.getDefaultPrinterName();
  }

  Future<List<String>?> getPrinterList () {
    return PrintBartenderPlatform.instance.getPrinterList();
  }
}
