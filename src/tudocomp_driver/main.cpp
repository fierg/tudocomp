#include <iostream>
#include <string>
#include <fstream>
#include <vector>
//#include <tudocomp/io/BitIStream.hpp>
//#include <tudocomp/io/BitOStream.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>

typedef unsigned char BYTE;

using namespace std;

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



/*
void rle_encode(std::basic_istream<char>& is, std::basic_ostream<char>& os, size_t offset = 0) {
	char prev;
	os << prev;
	char c;
	//std::string s(std::basic_istream<char>(is), {});
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

	
}*/


int main()
{
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("Current working dir: %s\n", cwd);
    }
    else
    {
        perror("getcwd() error");
        return 1;
    }
    

    std::vector<int> currentRuns(8);
    bool bit0;
    bool bit1;
    bool bit2;
    bool bit3;
    bool bit4;
    bool bit5;
    bool bit6;
    bool bit7;

    char ch = 0;
    std::vector<BYTE> mapping = createMapping();
    std::ifstream src2("test.txt");
    while (src2.get(ch))
    {
        bool bit0 = (mapping[(BYTE)ch] >> 0) & 1;
        bool bit1 = (mapping[(BYTE)ch] >> 1) & 1;
        bool bit2 = (mapping[(BYTE)ch] >> 2) & 1;
        bool bit3 = (mapping[(BYTE)ch] >> 3) & 1;
        bool bit4 = (mapping[(BYTE)ch] >> 4) & 1;
        bool bit5 = (mapping[(BYTE)ch] >> 5) & 1;
        bool bit6 = (mapping[(BYTE)ch] >> 6) & 1;
        bool bit7 = (mapping[(BYTE)ch] >> 7) & 1;

        std::cout << "char: " << ch << " bits: " << (bit7 ? "0" : "1") << (bit6 ? "0" : "1") << (bit5 ? "0" : "1") << (bit4 ? "0" : "1") << (bit3 ? "0" : "1") << (bit2 ? "0" : "1") << (bit1 ? "0" : "1") << (bit0 ? "0" : "1") << "\n";
    }
std:
    cout << "done.";

    return 0;
}
