#pragma once

#include <tudocomp/util.hpp>
#include <tudocomp/util/vbyte.hpp>
#include <tudocomp/Env.hpp>
#include <tudocomp/Compressor.hpp>

typedef unsigned char BYTE;

namespace tdc{

std::vector<BYTE> createMapping(std::istream& src){
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
    for (long unsigned int i = 0; i < list.size(); i++)
    {
        std::cout
            << "char: " << i << ":" << (char)i
            << " frequency = " << list[i] << '\n';
    }

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

class ModifiedRunLengthEncoder : public Compressor {
public:
    inline static Meta meta() {
        Meta m("compressor", "mrle", "Modified Run Length Encoding Compressor");
        m.option("offset").dynamic(0);
        return m;
    }
	const size_t m_offset;
    inline ModifiedRunLengthEncoder(Env&& env)
		: Compressor(std::move(env)), m_offset(this->env().option("offset").as_integer()) {
    }

    inline virtual void compress(Input& input, Output& output) override {
        std::cout
            << "[debug] methodEnter ModifiedRunLengthEncoder::compress\n";
    auto istream = input.as_stream(); 
    std::vector<BYTE> mapping = createMapping(istream);



	}
    inline virtual void decompress(Input& input, Output& output) override {


	}
};
}

