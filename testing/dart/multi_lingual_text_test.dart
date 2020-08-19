import 'dart:io';
import 'dart:typed_data';
import 'dart:ui';

import 'package:test/test.dart';

void main() {
  test('Test fallback fonts', () async {
    final ParagraphBuilder builder = ParagraphBuilder(ParagraphStyle());
    builder.addText('''
          Arabic:                        Start ضور المؤتمر end
          Bengali:                       Start অধিকার নিয়ে end
          Burmese:                       Start အခွင့်အရေ end
          Cambodian (also called Khmer): Start សេចក្ដីប្រកាសនេះ end
          Chinese:                       Start 情劇木質島治車実 end
          Gujarati:                      Start તેમનામાં વિચારશક્તિ end
          Hindi:                         Start प्राप्त करने end
          Japanese:                      Start ドフッ同迫まえべ主 end
          Kannada:                       Start ಸ್ಯಾಂಪಲ್ end
          Korean:                        Start 모여 다음과 같은 end
          Malayalam:                     Start മനുഷ്യന്നു end
          Marathi:                       Start गुन्ह्यात माफी end
          Nepali:                        Start वर्णको साझा end
          Sinhala:                       Start භාවය, භාෂාව end
          Tamil:                         Start அரசியல் அல்லது end
          Telugu:                        Start చేయు పని end
          Thai:                          Start รติศักดิ์]และสิท end
          Urdu:                          Start ئے ہی انہیںضمیر اور end
    ''');

    final Paragraph paragraph = builder.build();
    paragraph.layout(const ParagraphConstraints(width: double.maxFinite));
    final PictureRecorder recorder = PictureRecorder();
    final Canvas canvas = Canvas(recorder);
    canvas.drawPaint(Paint()..color = const Color(0xFF000000));
    canvas.drawParagraph(paragraph, const Offset(40, 40));

    final Picture picture = recorder.endRecording();
    final Image image = await picture.toImage(600, 600);
    final ByteData? rawImage = await image.toByteData(format: ImageByteFormat.png);

    expect(rawImage, isNotNull);
    final File file = File('output.png');
    file.writeAsBytesSync(rawImage!.buffer.asUint8List());
  });
}
