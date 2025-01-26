﻿// learn_cpp_bitmap_2025.cpp
#include <cstdint>
#include <memory>

using std::uint16_t;
using std::uint32_t;

class PixArr {
private:
	std::unique_ptr<uint8_t[]> m_pix_arr;
	uint16_t m_padding;

public:
	PixArr(uint32_t image_width, uint32_t image_height) {
		uint32_t total_width{ image_width % 4 == 0 ? 
			image_width : image_width - (image_width % 4) + 4};
		m_padding = static_cast<uint16_t>(total_width - image_width);
		m_pix_arr = std::make_unique<uint8_t[]>(total_width * image_height);
	}
};

class Bitmap {
private:
	const char* sig{ "BM" };

	// File headers
	uint32_t m_file_size;
	uint16_t m_reserved_1;
	uint16_t m_reserved_2;
	uint32_t m_pix_arr_offset;

	// DIB header
	uint32_t m_dib_header_size;
	uint32_t m_image_width;
	uint32_t m_image_height;
	uint16_t m_colour_planes;
	uint16_t m_bits_per_pix;
	uint16_t m_compress_type;
	uint16_t m_image_size;
	uint16_t m_horizontal_res;
	uint16_t m_vertical_res;
	uint16_t m_pallete_colours;
	uint16_t m_important_colours;

	// Colour table
	std::unique_ptr<uint16_t> m_palette;

	// Pixel Array
	PixArr m_pix_arr;

public:
	Bitmap(uint32_t image_width, uint32_t image_height) :
		m_image_width(image_width), m_image_height(image_height), m_colour_planes(1),
		m_bits_per_pix(8), m_compress_type(0), m_horizontal_res(20), m_vertical_res(20),
		m_dib_header_size(40), m_pallete_colours(0), m_important_colours(0),
		m_pix_arr(PixArr(m_image_width, m_image_height)) {

		// TODO: Need to find:
			// m_file_size
			// m_pix_arr_offset
			// m_image_size
	}
};


int main()
{

}


