#include "ppm_image.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

static const string_view PPM_SIG = "P6"sv;
static const int PPM_MAX = 255;

bool SavePPM(const Path& file, const Image& image) {
    // Открываем поток для записси записи в бинарном виде
    std::ofstream ofs(file, ios::binary);
    
    if (!ofs) {
        return false;
    }

    // Ширина высота изображения
    const int w = image.GetWidth();
    const int h = image.GetHeight();
    
    // Запись заголовка
    ofs << PPM_SIG << "\n" 
        << w << ' ' << h << "\n"
        << PPM_MAX << "\n";
    
    // Колличество элементов цвета: 
    // R — красный,
    // G — зелёный,
    // B — синий
    const int pix_part_count = 3;

    // Временный буфер для сокращения количества операций ввода-вывода, равный ширине строки в пиксселях
    const size_t buff_size = w*pix_part_count;
    std::vector<char> buffer(buff_size);

    // Читаем изображение построчно
    for (size_t y = 0; y < h; ++y) {
        const Color* line = image.GetLine(y);
        
        // Заполняем буфер попиксельно
        for (size_t x = 0; x < w; ++x) {
            buffer[x * pix_part_count + 0] = static_cast<char>(line[x].r);
            buffer[x * pix_part_count + 1] = static_cast<char>(line[x].g);
            buffer[x * pix_part_count + 2] = static_cast<char>(line[x].b);
        }
        ofs.write(buffer.data(), buff_size);
    }
    // Возвращает false, если не удалось записать изображение. В противном случае возвращает true
    return ofs.good();
}

Image LoadPPM(const Path& file) {
    // Открываем поток с флагом ios::binary поскольку будем читать данные в двоичном формате
    ifstream ifs(file, ios::binary);
    std::string sign;
    int w, h, color_max;

    // Заголовок: он содержит формат, размеры изображения и максимальное значение цвета
    ifs >> sign >> w >> h >> color_max;

    // Мы поддерживаем изображения только формата P6 с максимальным значением цвета 255
    if (sign != PPM_SIG || color_max != PPM_MAX) {
        return {};
    }

    // пропускаем один байт - это конец строки
    const char next = ifs.get();
    if (next != '\n') {
        return {};
    }

    // Колличество элементов цвета: 
    // R — красный,
    // G — зелёный,
    // B — синий
    const int pix_part_count = 3;

    // Создаем пустое изображение
    Image result(w, h, Color::Black());

    // Временный буфер для сокращения количества операций ввода-вывода, равный ширине строки в пиксселях
    std::vector<char> buff(w * pix_part_count);

    for (int y = 0; y < h; ++y) {
        Color* line = result.GetLine(y);
        ifs.read(buff.data(), w * pix_part_count);

        for (int x = 0; x < w; ++x) {
            line[x].r = static_cast<byte>(buff[x * pix_part_count + 0]);
            line[x].g = static_cast<byte>(buff[x * pix_part_count + 1]);
            line[x].b = static_cast<byte>(buff[x * pix_part_count + 2]);
        }
    }

    return result;
}

}  // namespace img_lib