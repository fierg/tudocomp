#pragma once

#include <tudocomp/util.hpp>
#include <tudocomp/util/vbyte.hpp>
#include <tudocomp/Env.hpp>
#include <tudocomp/Compressor.hpp>
#include <tudocomp_stat/StatPhase.hpp>

typedef unsigned char BYTE;

#define myLog

namespace tdc
{
    const BYTE MAX_RUN_COUNT = 255;
    const unsigned int MAP_SIZE = MAX_RUN_COUNT + 1;

    class Literals : LiteralIterator
    {
    private:
        //const vector<vector<BYTE>>*   m_runs;
        len_t m_pos;

    public:
        std::vector<Literal> m_literals;
        inline Literals(const std::vector<std::vector<BYTE>> &runs)
            : m_pos(0)
        {
            std::cout
                << "[debug] methodEnter ModifiedRunLengthEncoder::Literals\n";

            for (unsigned int bitPos = 0; bitPos < 8; bitPos++)
            {
                for (unsigned int vPos = 0; vPos < runs[bitPos].size(); vPos++)
                {
                    Literal l;
                    l.c = runs[bitPos][vPos];
                    m_literals.push_back(l);
                }
                Literal l;
                l.c = MAX_RUN_COUNT;
                m_literals.push_back(l);
            }
        }

        inline bool has_next() const
        {
            return m_pos < m_literals.size();
        }

        inline Literal next()
        {
            assert(has_next());
            return m_literals[m_pos++];
        }
    };

    template <typename coder_t>
    class ModifiedRunLengthEncoder : public Compressor
    {

    public:
        inline static Meta meta()
        {
            Meta m("compressor", "mrle", "Modified Run Length Encoding Compressor");
            m.option("coder").templated<coder_t>("coder");
            return m;
        }
        inline ModifiedRunLengthEncoder(Env &&env)
            : Compressor(std::move(env))
        {
        }

        inline virtual void compress(Input &input, Output &output) override
        {
            // Stats
            StatPhase phase("Modified Run Length Compression");
            StatPhase::log("test startphase logging", 1);

            std::cout
                << "[debug] methodEnter ModifiedRunLengthEncoder::compress\n";
            auto istream = input.as_stream();
            auto iview = input.as_view();

            //define the mapping for all occuring characters
            StatPhase::wrap("Construct byte mapping", []{
            std::vector<BYTE> mapping = createByteMapping(istream);
            });

            std::cout << "[debug] parsing iview with mapping...\n";
            Range mappingRange(0, MAX_RUN_COUNT);

            std::vector<BYTE> run0;
            run0.push_back(0);
            std::vector<BYTE> run1;
            run1.push_back(0);
            std::vector<BYTE> run2;
            run2.push_back(0);
            std::vector<BYTE> run3;
            run3.push_back(0);
            std::vector<BYTE> run4;
            run4.push_back(0);
            std::vector<BYTE> run5;
            run5.push_back(0);
            std::vector<BYTE> run6;
            run6.push_back(0);
            std::vector<BYTE> run7;
            run7.push_back(0);

            std::vector<bool> bitBoolean;
            for (unsigned int i = 0; i < 8; i++)
            {
                bitBoolean.push_back(false);
            }

            std::vector<std::vector<BYTE>> allRuns;
            allRuns.push_back(run0);
            allRuns.push_back(run1);
            allRuns.push_back(run2);
            allRuns.push_back(run3);
            allRuns.push_back(run4);
            allRuns.push_back(run5);
            allRuns.push_back(run6);
            allRuns.push_back(run7);

            //std::cout << "[debug]  iView size: " << iview.size() << "\n";
            bool setZero = false;

            for (unsigned int i = 0; i < iview.size(); i++)
            {
                uliteral_t c = mapping[iview[i]];
                for (unsigned int bitPos = 0; bitPos < 8; bitPos++)
                {
                    //std::cout << "[debug] char: " << (unsigned int) c;
                    if (bitBoolean[bitPos] == ((c >> bitPos) & 1))
                    {

                        if (allRuns[bitPos][allRuns[bitPos].size() - 1] == 254)
                        {
                            allRuns[bitPos].push_back(0);
                            allRuns[bitPos].push_back(1);
                            setZero = true;
                        }
                        else
                        {
                            allRuns[bitPos][allRuns[bitPos].size() - 1] += 1;
                        }
                    }
                    else
                    {

                        bitBoolean[bitPos] = !bitBoolean[bitPos];
                        allRuns[bitPos].push_back(1);
                    }
                    //std::cout << "[debug] bitpos: " << bitPos << " runPos: " << allRuns[bitPos].size() << " val: " << (unsigned int) allRuns[bitPos][allRuns[bitPos].size() - 1] <<  "\n";
                }
            }

            std::cout
                << "[debug] all runs in vector\n";

            std::cout
                << "[debug] runs0:" << allRuns[0].size() << "\n"
                << "[debug] runs1:" << allRuns[1].size() << "\n"
                << "[debug] runs2:" << allRuns[2].size() << "\n"
                << "[debug] runs3:" << allRuns[3].size() << "\n"
                << "[debug] runs4:" << allRuns[4].size() << "\n"
                << "[debug] runs5:" << allRuns[5].size() << "\n"
                << "[debug] runs6:" << allRuns[6].size() << "\n"
                << "[debug] runs7:" << allRuns[7].size() << "\n";

            Literals literals(allRuns);

            typename coder_t::Encoder coder(env().env_for_option("coder"), output, literals);

            std::cout
                << "[debug] encoding static info\n";

            MinDistributedRange encodingRange(0, 255);

            std::cout
                << "[debug] start Encoding\n";

            for (BYTE mappingEntry : mapping)
            {
                //std::cout << "[debug] encoding mapping: " << (unsigned int)mappingEntry << "\n";
                coder.encode(mappingEntry, encodingRange);
            }

            // encode runs
            for (Literal run : literals.m_literals)
            {
                //std::cout << "[debug] encoding run: " << (unsigned int)run.c << "\n";
                coder.encode(run.c, encodingRange);
            }

            std::cout
                << "[debug] finished encoding.\n";
            phase.to_json().str(std::cout);
        }

        inline virtual void decompress(Input &input, Output &output) override
        {
            // retrieve an output stream
            auto ostream = output.as_stream();

            // instantiate the decoder using the whole input alphabet
            typename coder_t::Decoder decoder(
                env().env_for_option("coder"), input);

            // define the range for all occuring characters
            MinDistributedRange encodingRange(0, MAX_RUN_COUNT);

            std::vector<BYTE> parsed_mapping;

            for (int i = 0; i < MAP_SIZE; i++)
            {
                auto mappingEntry = decoder.template decode<uliteral_t>(encodingRange);
                std::cout << "[debug] decoded mapping entry: " << (unsigned int)mappingEntry << "\n";
                parsed_mapping.push_back(mappingEntry);
            }

            std::vector<BYTE> mapping(MAP_SIZE);

            for (int i = 0; i < parsed_mapping.size(); i++)
            {
                mapping[parsed_mapping[i]] = i;
            }

            std::vector<BYTE> run0;
            std::vector<BYTE> run1;
            std::vector<BYTE> run2;
            std::vector<BYTE> run3;
            std::vector<BYTE> run4;
            std::vector<BYTE> run5;
            std::vector<BYTE> run6;
            std::vector<BYTE> run7;

            std::vector<std::vector<BYTE>> allRuns;
            allRuns.push_back(run0);
            allRuns.push_back(run1);
            allRuns.push_back(run2);
            allRuns.push_back(run3);
            allRuns.push_back(run4);
            allRuns.push_back(run5);
            allRuns.push_back(run6);
            allRuns.push_back(run7);

            unsigned int bitPos = 0;
            unsigned int count = 0;

            // decode text
            while (!decoder.eof())
            {
                uliteral_t c = decoder.template decode<uliteral_t>(encodingRange);
                //std::cout << "[debug] decoded at " << count++ << (unsigned int)c << "\n";

                if (c == MAX_RUN_COUNT)
                    bitPos++;
                else
                    allRuns[bitPos].push_back(c);
            }

            std::cout
                << "[debug] finished decoding.\n";

            std::cout
                << "[debug] all runs in vector\n";

            std::cout
                << "[debug] runs0:" << allRuns[0].size() << "\n"
                << "[debug] runs1:" << allRuns[1].size() << "\n"
                << "[debug] runs2:" << allRuns[2].size() << "\n"
                << "[debug] runs3:" << allRuns[3].size() << "\n"
                << "[debug] runs4:" << allRuns[4].size() << "\n"
                << "[debug] runs5:" << allRuns[5].size() << "\n"
                << "[debug] runs6:" << allRuns[6].size() << "\n"
                << "[debug] runs7:" << allRuns[7].size() << "\n";

            // bitsetter init
            std::vector<int> bitBoolean;
            for (unsigned int i = 0; i < 8; i++)
            {
                bitBoolean.push_back(0);
            }

            // size view
            unsigned int sizeDecompressed = 0;
            for (unsigned int i = 0; i < allRuns[0].size(); i++)
            {
                sizeDecompressed += allRuns[0][i];
            }

            // preinit vector of char (view)
            std::vector<BYTE> myView;
            for (unsigned int i = 0; i < sizeDecompressed; i++)
            {
                myView.push_back((BYTE)0);
            }

            // payload
            for (unsigned int bitPos = 0; bitPos < 8; bitPos++)
            {
                unsigned int bytePos = 0;

                for (unsigned int i = 0; i < allRuns[bitPos].size(); i++)
                {
                    int count = allRuns[bitPos][i];

                    for (unsigned int j = 0; j < count; j++)
                    {
                        myView[bytePos] |= (bitBoolean[bitPos] << bitPos);
                        bytePos += 1;
                    }
                    bitBoolean[bitPos] = 1 - bitBoolean[bitPos];
                }
            }
            for (unsigned int i = 0; i < myView.size(); i++)
            {
                //std::cout << "writing char: " << mapping[myView[i]] << " as int " << (unsigned int)myView[i] << "\n";
                ostream << mapping[myView[i]];
            }
        }

        inline std::vector<BYTE> createByteMapping(std::istream &src)

        {
            std::cout << "[debug] analyzing file... \n";
            std::vector<BYTE> mapping(MAP_SIZE);
            std::vector<long long int> list(MAP_SIZE);

            if (!src)
            {
                std::cerr << "Error opening file: ";
            }
            char ch = 0;

            while (src.get(ch))
            {
                list[(unsigned char)ch]++;
            }

/*
            for (unsigned int i = 0; i < list.size(); i++)
            {
                std::cout
                    << "char: " << i << ":" << (unsigned char)i
                    << " frequency = " << list[i] << '\n';
            }
*/
            for (unsigned int i = 0; i < mapping.size(); i++)
            {
                int current_max_pos = 0;
                for (int j = 0; j < mapping.size(); j++)
                {
                    if (list[j] > list[current_max_pos])
                    {
                        current_max_pos = j;
                    }
                }
                mapping[current_max_pos] = (BYTE) i;
                list[current_max_pos] = -1;
            }

            // for (int i = 0; i < mapping.size(); i++)
            // {
            //     std::cout << "[debug] Mapping entry: " << i << " : " << (unsigned int)mapping[i] << "\n";
            // }
            return mapping;
        }
    };
}