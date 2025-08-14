#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <cmath>
#include <unistd.h>
#include <png.h>

#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->png_jmpbuf)
#endif
const std::string grayramp = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/|()1{}[]?-_+~<>i!lI;:,\"^`. ";
const size_t grayramp_length = grayramp.size();
int width, height;
png_byte color_type;
png_byte bit_depth;
png_byte** row_pointers = nullptr;

void usage()
{
    std::cout << "Usage: imgtoascii -i [input file] -s [downscale factor]";
    std::cout << std::endl;
    exit(-1);
}

void read_png(std::string filename)
{
    FILE *fp = fopen(filename.c_str(), "rb");
    if (!fp)
    {
        std::cout << "unable to open source file" << std::endl;
        exit(-1);
    }
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png)
    {
        std::cout << "unable to create png struct" << std::endl;
        exit(-1);
    }
    png_infop info = png_create_info_struct(png);
    if (setjmp(png_jmpbuf(png)))
    {
        std::cout << "png_jmpbuf" << std::endl;
        exit(-2);
    }
    png_init_io(png, fp);
    png_read_info(png, info);
    width = png_get_image_width(png, info);
    height = png_get_image_height(png, info);
    color_type = png_get_color_type(png, info);
    bit_depth = png_get_bit_depth(png, info);
    if (bit_depth == 16)
    {
        png_set_strip_16(png);
    }
    if (color_type == PNG_COLOR_TYPE_PALETTE)
    {
        png_set_palette_to_rgb(png);
    }

    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    {
        png_set_expand_gray_1_2_4_to_8(png);
    }
    if (png_get_valid(png, info, PNG_INFO_tRNS))
    {
        png_set_tRNS_to_alpha(png);
    }
    // These color_type don't have an alpha channel then fill it with 0xff.
    if (color_type == PNG_COLOR_TYPE_RGB ||
       color_type == PNG_COLOR_TYPE_GRAY ||
       color_type == PNG_COLOR_TYPE_PALETTE)
    {
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY ||
       color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        png_set_gray_to_rgb(png);
    }
    png_read_update_info(png, info);

    row_pointers = (png_byte**) malloc(sizeof(png_byte*) * height);
    for (int y = 0; y < height; ++y)
    {
        row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png, info));
    }
    png_read_image(png, row_pointers);
    fclose(fp);
    png_destroy_read_struct(&png, &info, nullptr);
}

const char pixel_to_char(png_byte* block)
{
    return grayramp.at(ceil(grayramp_length - 1) * block[0] / 255);
}

png_byte process_pixel_block(int scale, int startx, int starty)
{
    int grayscale_block_sum = 0;
    int count = 0;
    for (int y = starty; y < starty + scale && y < height; ++y)
    {
        png_byte* row = row_pointers[y];
        for (int x = startx; x < startx + scale && x < width; ++x)
        {
            png_byte* pixel = &(row[x * 4]);
            grayscale_block_sum += (0.21*pixel[0] + 0.72*pixel[1] + 0.07*pixel[2]);
            ++count;
        }
    }
    png_byte avg_pixel[4];
    avg_pixel[0] = grayscale_block_sum / count;
    avg_pixel[1] = grayscale_block_sum / count;
    avg_pixel[2] = grayscale_block_sum / count;
    avg_pixel[3] = 255;
    return pixel_to_char(avg_pixel);
}

std::string process_png_file(int scale)
{
    std::string ascii_out;
    //to compensate for characters typically being taller than wide, sample only half as often for height
    // y += scale*2
    for (int y = 0; y < (height + scale); y += scale*2)
    {
        png_byte* row = row_pointers[y];
        for (int x = 0; x < width + scale; x += scale)
        {
            ascii_out.push_back(process_pixel_block(scale, x, y));
        }
        ascii_out.push_back('\n');
    }
    return ascii_out;
}

std::string change_filename(std::string filename)
{
    size_t pos = filename.find_first_of(".", 0);
    const std::string grayscale = "grayscale";
    std::string first = filename.substr(0, pos);
    std::string second = filename.substr(pos+1, filename.size());
    first.append(grayscale);
    first.append(".");
    first.append(second);
    return first;
}

void write_png(std::string filename)
{
    std::string out_name = change_filename(filename);
    FILE* fp = fopen(out_name.c_str(), "wb");
    if (!fp)
    {
        std::cout << "unable to open file for writing" << std::endl;
        exit(-3);
    }
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
    if (setjmp(png_jmpbuf(png)))
    {
        std::cout << "png_jmpbuf" << std::endl;
        exit(-4);
    }
    png_init_io(png, fp);
    png_set_IHDR(
      png,
      info,
      width, height,
      8,
      PNG_COLOR_TYPE_RGBA,
      PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_DEFAULT,
      PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);

    if (!row_pointers)
    {
        std::cout << "unable to get reference to grid for writing" << std::endl;
        exit(-5);
    }
    png_write_image(png, row_pointers);
    png_write_end(png, nullptr);

    for (int y = 0; y < height; y++)
    {
        free(row_pointers[y]);
    }
    free(row_pointers);
    fclose(fp);
    png_destroy_write_struct(&png, &info);
}

int main(int argc, char** argv)
{
    int opt;
    std::string input("");
    int scale(0);
    while ((opt = getopt(argc, argv, "i:s:")) != -1)
    {
        switch (opt)
        {
            case 'i':
                //input file
                input = optarg;
                break;
            case 's':
                //factor to scale down by
                //e.g. 360x360 img becomes 60x60
                scale = atoi(optarg);
                break;
            default:
                usage();
                break;
        }
    }
    if (input == "" || scale == 0)
    {
        usage();
    }
    read_png(input);
    std::string ascii = process_png_file(scale);
    write_png(input);
    std::cout << ascii << std::endl;
}
