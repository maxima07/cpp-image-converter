#include <img_lib.h>
#include <bmp_image.h>
#include <jpeg_image.h>
#include <ppm_image.h>

#include <filesystem>
#include <string_view>
#include <iostream>

using namespace std;

enum class Format {
    BMP,
    JPEG,
    PPM,
    UNKNOWN
};

class ImageFormatInterface {
public:
    virtual bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const = 0;
    virtual img_lib::Image LoadImage(const img_lib::Path& file) const = 0;
};

namespace FormatInterfaces {

class BMPImageFormat : public ImageFormatInterface {
public:    
        bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override {
            return img_lib::SaveBMP (file, image);
        }
    
        img_lib::Image LoadImage(const img_lib::Path& file) const override {
            return img_lib::LoadBMP (file);
        }
    };

class JPEGImageFormat : public ImageFormatInterface {
public:    
    bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override {
        return img_lib::SaveJPEG (file, image);
    }

    img_lib::Image LoadImage(const img_lib::Path& file) const override {
        return img_lib::LoadJPEG (file);
    }
};

class PPMImageFormat : public ImageFormatInterface {
public:    
    bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override {
        return img_lib::SavePPM (file, image);
    }

    img_lib::Image LoadImage(const img_lib::Path& file) const override {
        return img_lib::LoadPPM (file);
    }
};

} //namespace FormatInterfaces

Format GetFormatByExtension(const img_lib::Path& input_file) {
    const string ext = input_file.extension().string();
    if (ext == ".bmp"sv) {
        return Format::BMP;
    }
    
    if (ext == ".jpg"sv || ext == ".jpeg"sv) {
        return Format::JPEG;
    }

    if (ext == ".ppm"sv) {
        return Format::PPM;
    }
    return Format::UNKNOWN;
}

const ImageFormatInterface* GetFormatInterface(const img_lib::Path& path) {
    Format image_format = GetFormatByExtension (path);
    static const FormatInterfaces::BMPImageFormat   bmp_format;
    static const FormatInterfaces::JPEGImageFormat  jpeg_format;
    static const FormatInterfaces::PPMImageFormat   ppm_format;

    switch (image_format) {
        case Format::BMP :
            return &bmp_format;
            break;
        case Format::JPEG :
            return &jpeg_format;
            break;
        case Format::PPM :
            return &ppm_format;
            break;
        default:
            return nullptr;
        }
    
}

int main(int argc, const char** argv) {
    if (argc != 3) {
        cerr << "Usage: "sv << argv[0] << " <in_file> <out_file>"sv << endl;
        return 1;
    }

    img_lib::Path in_path = argv[1];
    img_lib::Path out_path = argv[2];

    const ImageFormatInterface* input_interface_format = GetFormatInterface (in_path);
    if (!input_interface_format) {
        cerr << "Unknown format of the input file"sv;
        return 2;
    }

    const ImageFormatInterface* output_interface_format = GetFormatInterface (out_path);
    if (!output_interface_format) {
        cerr << "Unknown format of the output file"sv;
        return 3;
    }

    img_lib::Image image = input_interface_format->LoadImage (in_path);
    if (!image) {
        cerr << "Loading failed"sv << endl;
        return 4;
    }

    if (!output_interface_format->SaveImage(out_path, image)) {
        cerr << "Saving failed"sv << endl;
        return 5;
    }

    cout << "Successfully converted"sv << endl;
}