#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    return 4 * ((w * 3 + 3) / 4);
}

PACKED_STRUCT_BEGIN BitmapFileHeader {
    BitmapFileHeader () = default;
    BitmapFileHeader (const int width, const int height) {
        BMP_SIZE_ = BMP_IDENT_ + GetBMPStride (width) * height;
    }

    const char     BMP_SIGN_[2] = {'B','M'}; // Подпись BM (2 байта)
          uint32_t BMP_SIZE_ = 0;            // Суммарный размер заголовка и данных (4 байта)
    const uint32_t BMP_RESERV_SPACE_ = 0;    // Зарезервированное пространство (4 байта)
    const uint32_t BMP_IDENT_ = 54;          // Отступ данных от начала файла (4 байта)
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    BitmapInfoHeader() = default;
    BitmapInfoHeader (const int width, const int height)
        : BMP_IMAGE_WIDTH_(width)
        , BMP_IMAGE_HEIGHT_(height) {
            BMP_DATA_SIZE_ = GetBMPStride (width) * height;
        }
    
    const uint32_t BMP_HEADER_SIZE_ = 40;           // Размер заголовка. Учитывается только размер второй части заголовка. (4 байта)
          int32_t  BMP_IMAGE_WIDTH_ = 0;            // Ширина изображения в пикселях. (4 байта)
          int32_t  BMP_IMAGE_HEIGHT_ = 0;           // Высота изображения в пикселях. (4 байта)
    const int16_t  BMP_NUM_PLANES_ = 1;             // Количество плоскостей. В нашем случае всегда 1 — одна RGB плоскость. (2 байта)
    const int16_t  BMP_BIT_PER_PIX_ = 24;           // Количество бит на пиксель. В нашем случае всегда 24. (2 байта)
    const int32_t  BMP_COMPRES_TYPE_ = 0;           // Тип сжатия. В нашем случае всегда 0 — отсутствие сжатия. (4 байта)
          int32_t  BMP_DATA_SIZE_ = 0;              // Количество байт в данных. (4 байта)
    const int32_t  BMP_X_PIX_PER_METER_ = 11811;    // Горизонтальное разрешение, пикселей на метр. Нужно записать 11811, что примерно соответствует 300 DPI. (4 байта)
    const int32_t  BMP_Y_PIX_PER_METER_ = 11811;    // Вертикальное разрешение, пикселей на метр. Нужно записать 11811, что примерно соответствует 300 DPI. (4 байта)
    const int32_t  BMP_USE_COLOR_ = 0;              // Количество использованных цветов. Нужно записать 0 — значение не определено. (4 байта)
    const int32_t  BMP_IMPOTANT_COLOR_ = 0x1000000; // Количество значимых цветов. Нужно записать 0x1000000. (4 байта)
}
PACKED_STRUCT_END

// напишите эту функцию
bool SaveBMP(const Path& file, const Image& image) {
    // Открываем поток для записси записи в бинарном виде
    std::ofstream ofs(file, ios::binary);
    
    if (!ofs) {
        return false;
    }

    // Ширина высота изображения
    const int w = image.GetWidth();
    const int h = image.GetHeight();
    const size_t stride = GetBMPStride(w);

    // Создаем заголовки
    BitmapFileHeader file_header (w, h);
    BitmapInfoHeader info_header (w, h);

    // Записываем заголовки
    ofs.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
    ofs.write(reinterpret_cast<const char*>(&info_header), sizeof(info_header));

    // Колличество элементов цвета: 
    // R — красный,
    // G — зелёный,
    // B — синий
    const int pix_part_count = 3;

    // Временный буфер для сокращения количества операций ввода-вывода, равный отступу по ширине строки в пиксселях
    
    std::vector<char> buffer(stride);

    // Cтроки в BMP идут в другом порядке, начиная с нижней.
    for (int y = h-1; y >= 0; --y) {
        const Color* line = image.GetLine(y);

        // Порядок компонент в пикселе обратный: Blue, Green, Red
        for (int x = 0; x < w; ++x) {
            buffer[x * pix_part_count + 0] = static_cast<char>(line[x].b);
            buffer[x * pix_part_count + 1] = static_cast<char>(line[x].g);
            buffer[x * pix_part_count + 2] = static_cast<char>(line[x].r);
        }
        ofs.write(buffer.data(), buffer.size());
    }
    // Возвращает false, если не удалось записать изображение. В противном случае возвращает true
    return ofs.good();
}

// напишите эту функцию
Image LoadBMP(const Path& file) {
    // Открываем поток с флагом ios::binary поскольку будем читать данные в двоичном формате
    ifstream ifs(file, ios::binary);
    
    // Создаем заголовки
    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;

    // Читаем заголовки 
    ifs.read(reinterpret_cast<char*>(&file_header), sizeof(BitmapFileHeader));

    if (file_header.BMP_SIGN_[0] != 'B' || file_header.BMP_SIGN_[1] != 'M')  {
        return {};
    }

    ifs.read(reinterpret_cast<char*>(&info_header), sizeof(BitmapInfoHeader));

    // Создаем пустое изображение
    Image result(info_header.BMP_IMAGE_WIDTH_, info_header.BMP_IMAGE_HEIGHT_, Color::Black());

    // Колличество элементов цвета: 
    // R — красный,
    // G — зелёный,
    // B — синий
    const int pix_part_count = 3;

    // Ширина высота изображения
    const int w = result.GetWidth ();
    const int h = result.GetHeight ();

    // Задаем отступ по ширине
    const int stride = GetBMPStride(w);

    // Временный буфер для сокращения количества операций ввода-вывода, равный отступу по ширине строки в пиксселях
    std::vector<char> buffer(stride);
    
    // Cтроки в BMP идут в другом порядке, начиная с нижней.
    for (int y = h-1; y >= 0; --y) {
        Color* line = result.GetLine(y);
        ifs.read(buffer.data(), stride);

        // Порядок компонент в пикселе обратный: Blue, Green, Red
        for (int x = 0; x < w; ++x) {
            line[x].b = static_cast<byte>(buffer[x * pix_part_count + 0]);
            line[x].g = static_cast<byte>(buffer[x * pix_part_count + 1]);
            line[x].r = static_cast<byte>(buffer[x * pix_part_count + 2]);
        }
    }

    return result;
}

}  // namespace img_lib