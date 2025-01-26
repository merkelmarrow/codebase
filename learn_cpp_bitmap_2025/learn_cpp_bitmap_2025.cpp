// learn_cpp_bitmap_2025.cpp
#include <cstdint>
#include <memory>
#include <cmath>

using std::uint16_t;
using std::uint32_t;


class PixArr {
private:
	std::unique_ptr<uint8_t[]> m_pix_arr;
	int32_t m_image_width;
	int32_t m_image_height;
	uint16_t m_padding;

public:
	// constructor assumes bpp of 8
	PixArr(int32_t image_width, int32_t image_height) : 
		m_image_width(image_width), m_image_height(image_height) {
		int32_t total_width = image_width % 4 == 0 ?
			image_width : image_width - (image_width % 4) + 4;
		m_padding = static_cast<uint16_t>(total_width - image_width);
		m_pix_arr = std::make_unique<uint8_t[]>(total_width * image_height);
	}

	inline int32_t getTotalWidth() {
		return m_image_width + m_padding;
	}

	inline uint32_t getByteSize() {
		return getTotalWidth() * m_image_height;
	}
};

class Bitmap {
private:
	const char* sig{ "BM" };

	// File header
	uint32_t m_file_size;
	uint16_t m_reserved_1;
	uint16_t m_reserved_2;
	uint32_t m_pix_arr_offset;

	// DIB header (BITMAPINFOHEADER)
	uint32_t m_dib_header_size;

	// THE BELOW ARE STORED IN PixArr OBJECT
	// int32_t m_image_width;
	// int32_t m_image_height;

	uint16_t m_colour_planes;
	uint16_t m_bits_per_pix;
	uint32_t m_compress_type;
	uint32_t m_image_size;
	uint32_t m_horizontal_res;
	uint32_t m_vertical_res;
	uint32_t m_pallete_colours;
	uint32_t m_important_colours;

	// Colour table
	std::unique_ptr<uint8_t[]> m_palette;

	// Pixel Array
	PixArr m_pix_arr;

public:

	// This is a default constructor that assumes many parameters
	// -> bpp = 8
	// -> horizontal and vertical res = 20 pixels/m
	// -> full palette
	// and more
	Bitmap(int32_t image_width, int32_t image_height) :
		m_colour_planes(1), m_bits_per_pix(8), m_compress_type(0), m_horizontal_res(20), 
		m_vertical_res(20), m_dib_header_size(40), m_pallete_colours(0),
		m_important_colours(0), m_pix_arr(PixArr(image_width, image_height)),
		m_reserved_1(0), m_reserved_2(0) {

		const uint32_t file_header_size = 14; // FILEHEADER = 14
		const uint32_t dib_header_size = 40; // BITMAPINFOHEADER = 40
		m_pix_arr_offset = 14 + 40 + getPaletteSize();
		m_image_size = m_pix_arr.getByteSize();
		m_file_size = m_pix_arr_offset + m_image_size;
	}

	uint32_t getNumPaletteColours() {
		if (m_bits_per_pix <= 8 && m_pallete_colours == 0)
			return static_cast<uint16_t> (std::pow(2, m_bits_per_pix));
		else return m_pallete_colours;
	}

	uint32_t getPaletteSize() {
		return getNumPaletteColours() * 4;
	}
};


int main()
{

}


