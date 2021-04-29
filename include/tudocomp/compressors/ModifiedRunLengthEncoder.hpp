#pragma once

#include <tudocomp/util.hpp>
#include <tudocomp/util/vbyte.hpp>
#include <tudocomp/Env.hpp>
#include <tudocomp/Compressor.hpp>

namespace tdc {

/**
 * Encode a byte-stream with modified binary run length encoding
 */
template<class char_type>
void mod_rle_encode(std::basic_istream<char_type>& is, std::basic_ostream<char_type>& os, size_t offset = 0) {
	char_type prev;
	if(tdc_unlikely(!is.get(prev))) return;
	os << prev;
	char_type c;
	std::string s(std::basic_istream<char_type>(is), {});
	int occurences[255];

	//count occurences of each char of the stream
	for (int i = 0; i < 255; i++)
	{
		occurences[i]=boost::count(s, static_cast<char>(i));
	}

	while(is.get(c)) {
		// construct the bitwise input stream
		BitIStream ibits(input); 
		bool bit = ibits.read_bit(); // read a single bit


		prev = c;
	}

	
}
/**
 * Decodes a modified run length encoded stream
 */
template<class char_type>
void mod_rle_decode(std::basic_istream<char_type>& is, std::basic_ostream<char_type>& os, size_t offset = 0) {
	//TODO
}

class RunLengthEncoder : public Compressor {
public:
    inline static Meta meta() {
        Meta m("compressor", "m-rle", "Modified Run Length Encoding Compressor");
        m.option("offset").dynamic(0);
        return m;
    }
	const size_t m_offset;
    inline RunLengthEncoder(Env&& env)
		: Compressor(std::move(env)), m_offset(this->env().option("offset").as_integer()) {
    }

    inline virtual void compress(Input& input, Output& output) override {
		auto is = input.as_stream();
		auto os = output.as_stream();
		rle_encode(is,os,m_offset);
	}
    inline virtual void decompress(Input& input, Output& output) override {
		auto is = input.as_stream();
		auto os = output.as_stream();
		rle_decode(is,os,m_offset);
	}
}

std::vector<BYTE> createMapping(){
    std::ifstream src("test.txt");
    std::vector<int> list(255);

    if (!src)
    {
        std::cerr << "Error opening file: ";
    }
    char ch = 0;
    while (src.get(ch))
    {
        list[(int)ch]++;
    }
    for (int i = 0; i < list.size(); i++)
    {
        std::cout
            << "char: " << i << ":" << (char)i
            << " frequency = " << list[i] << '\n';
    }
    src.close();

    std::vector<BYTE> mapping(255);
    for (int i = 0; i < mapping.size(); i++)
    {
        int current_max_pos = 0;
        for (int j = 0; j < mapping.size(); j++)
        {
            if (list[j] > list[current_max_pos])
            {
                current_max_pos = j;
            }
        }
        mapping[(BYTE)current_max_pos] = i;
        list[current_max_pos] = -1;
    }
    return mapping;
}

std::vector<BYTE> readFile(const char *filename)
{
    // open the file:
    std::streampos fileSize;
    std::ifstream file(filename, std::ios::binary);

    // get its size:
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // read the data:
    std::vector<BYTE> fileData(fileSize);
    file.read((char *)&fileData[0], fileSize);
    return fileData;
}


}//ns

