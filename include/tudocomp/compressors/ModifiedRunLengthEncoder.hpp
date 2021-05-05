#pragma once

#include <tudocomp/util.hpp>
#include <tudocomp/util/vbyte.hpp>
#include <tudocomp/Env.hpp>
#include <tudocomp/Compressor.hpp>

typedef unsigned char BYTE;

#define myLog

namespace tdc
{

    class Literals : LiteralIterator
    {
    private:
        //const vector<vector<BYTE>>*   m_runs;
        len_t m_pos;

    public:
        std::vector<Literal> m_literals;
        inline Literals(const std::vector<std::vector<BYTE>> &runs, unsigned int maxRun)
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
                l.c = (BYTE)(maxRun + 1);
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

            // old
            //
            // if(m_pos < m_text_size) {
            //     // from encoded text
            //     auto l = Literal { uliteral_t((*m_text)[m_pos]), m_pos };
            //     m_pos = m_next[m_pos];
            //     return l;
            // } else {
            //     // from grammar right sides
            //     auto l = Literal { m_g_literals[m_g_pos],
            //                        m_text_size + 2 * m_g_pos };
            //     ++m_g_pos;
            //     return l;
            // }
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
            std::cout
                << "[debug] methodEnter ModifiedRunLengthEncoder::compress\n";
            auto istream = input.as_stream();
            auto iview = input.as_view();

            //define the range for all occuring characters
            unsigned int min, max;
            std::vector<BYTE> mapping = createByteMapping(istream, min, max);
            MinDistributedRange mappingRange(min, max);
            int maxRun = 0;

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

            //std::cout  << "[debug]  iView size: " << iview.size() << "\n";
            bool setZero = false;

            for (unsigned int i = 0; i < iview.size(); i++)
            {
                uliteral_t c = mapping[iview[i]];
                for (unsigned int bitPos = 0; bitPos < 8; bitPos++)
                {
                    std::cout << "[debug] char: " << (unsigned int) c;
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
                            if (maxRun < allRuns[bitPos][allRuns[bitPos].size() - 1])
                                maxRun = allRuns[bitPos][allRuns[bitPos].size() - 1];
                        }
                    }
                    else
                    {

                        bitBoolean[bitPos] = !bitBoolean[bitPos];
                        allRuns[bitPos].push_back(1);
                    }
                     std::cout << "[debug] bitpos: " << bitPos << " runPos: " << allRuns[bitPos].size() << " val: " << (unsigned int) allRuns[bitPos][allRuns[bitPos].size() - 1] <<  "\n";
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

            Literals literals(allRuns, maxRun);

            typename coder_t::Encoder coder(env().env_for_option("coder"), output, literals);

            std::cout
                << "[debug] encoding static info\n";

            // encode the  largest characters
            coder.encode(maxRun + 1, uliteral_r);

            // encode the size of the mapping
            coder.encode(mapping.size(), uliteral_r);

            MinDistributedRange encodingRange(setZero ? 0 : 1, maxRun + 1);

            std::cout
                << "[debug] start Encoding\n";

            for (BYTE mappingEntry : mapping)
            {
                 std::cout
                << "[debug] encoding mapping" << (unsigned int) mappingEntry << "\n";
                coder.encode(mappingEntry, mappingRange);
            }

            // encode runs
            for (Literal run : literals.m_literals)
            {

                 std::cout
                << "[debug] encoding run" << (unsigned int) run.c << "\n";
                coder.encode(run.c, encodingRange);
            }

            std::cout
                << "[debug] finished encoding.\n";

        }

        inline virtual void decompress(Input &input, Output &output) override
        {
            // retrieve an output stream
            auto ostream = output.as_stream();

            // instantiate the decoder using the whole input alphabet
            typename coder_t::Decoder decoder(
                env().env_for_option("coder"), input);

            // encode the largest character
            auto maxRun = decoder.template decode<uliteral_t>(uliteral_r);
            std::cout << "[debug] decoded max run: " << (unsigned int)maxRun << "\n";

            // decode the mapping size
            auto mappingSize = decoder.template decode<uliteral_t>(uliteral_r);
            std::cout << "[debug] decoded mapping size: " << (unsigned int)maxRun << "\n";

            std::vector<BYTE> mapping; 

            for (int i = 0; i < mappingSize; i++)
            {
                auto mappingEntry = decoder.template decode<uliteral_t>(uliteral_r);
                std::cout << "[debug] decoded mapping entry: " << (unsigned int)maxRun << "\n";
                mapping.push_back(mappingEntry);
            }

            // define the range for all occuring characters
            Range range(maxRun);

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

            // decode text
            while (!decoder.eof())
            {
                uliteral_t c = decoder.template decode<uliteral_t>(range);
                std::cout << "[debug] decoded: " << (unsigned int)c << "\n";

                if (c == maxRun)
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
        }

        inline std::vector<BYTE> createByteMapping(std::istream &src, unsigned int& min, unsigned int& max)
        {
            std::vector<int> list(255);

            if (!src)
            {
                std::cerr << "Error opening file: ";
            }
            char ch = 0;
            max = 0;
            min = 255;
            while (src.get(ch))
            {
                list[(int)ch]++;
                min = (min > ch) ? ch : min;
                max = (max < ch) ? ch : max;
            }
             for (long unsigned int i = 0; i < list.size(); i++)
            {
                std::cout
                    << "char: " << i << ":" << (char)i
                    << " frequency = " << list[i] << '\n';
            }
            std::cout
                    << "min: " << min  << " max: " << max << "\n";

            std::vector<BYTE> mapping(max - min  + 1);
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

            for (int i = 0; i < mapping.size(); i++)
            {
                std::cout << "[debug] Mapping entry: " << i << " : " << (unsigned int)mapping[i] << "\n";
            }

            return mapping;
        }
    };
}

/*
TEST(doc_compressor_impl, cycle)
{
    const string example = "aaabbaabab";

    // Run compression cycles using different encoders
    test::roundtrip<ModifiedRunLengthEncoder<HuffmanCoder>>(example);
    test::roundtrip<ModifiedRunLengthEncoder<ASCIICoder>>(example);
    test::roundtrip<ModifiedRunLengthEncoder<BitCoder>>(example);
    test::roundtrip<ModifiedRunLengthEncoder<EliasDeltaCoder>>(example);
}

TEST(doc_compressor_impl, helpers)
{
    // perform border case compression tests using different encoders
    test::roundtrip_batch(test::roundtrip<ModifiedRunLengthEncoder<HuffmanCoder>>);
    test::roundtrip_batch(test::roundtrip<ModifiedRunLengthEncoder<ASCIICoder>>);
    test::roundtrip_batch(test::roundtrip<ModifiedRunLengthEncoder<BitCoder>>);
    test::roundtrip_batch(test::roundtrip<ModifiedRunLengthEncoder<EliasDeltaCoder>>);

    // perform compression tests on generated strings using different encoders
    test::on_string_generators(test::roundtrip<ModifiedRunLengthEncoder<HuffmanCoder>>, 15);
    test::on_string_generators(test::roundtrip<ModifiedRunLengthEncoder<EliasDeltaCoder>>, 15);
    test::on_string_generators(test::roundtrip<ModifiedRunLengthEncoder<BitCoder>>, 15);
    test::on_string_generators(test::roundtrip<ModifiedRunLengthEncoder<ASCIICoder>>, 15);
}
*/

// old bit countings
/*
for (len_t i = 0; i < iview.size(); i++)
            {
                uliteral_t c = mapping[iview[i]];
                std::cout
                    << "[debug] i: " << i << " c: " << iview[i] << " m: " << int(c) << "\n";
                if (bit0 == ((c >> 0) & 1))
                {
                    if (maxRun < run0[run0.size() - 1])
                        maxRun = run0[run0.size() - 1];
                    if (run0[run0.size() - 1] == 254)
                    {
                        run0.push_back(245);
                        run0.push_back(1);
                    }
                    else
                        run0[run0.size() - 1] += 1;
                } else
                {
                    bit0 = !bit0;
                    run0.push_back(1);
                }
                if (bit1 == ((c >> 1) & 1)) {
                    if (maxRun < run1[run1.size() - 1])
                        maxRun = run1[run1.size() - 1];
                    if (run1[run1.size() - 1] == 254)
                    {
                        run1.push_back(245);
                        run1.push_back(1);
                    }
                    else
                        run1[run1.size() - 1] += 1;
                } else
                {
                    bit1 = !bit1;
                    run1.push_back(1);
                }
                if (bit2 == ((c >> 2) & 1)) {
                    if (maxRun < run2[run2.size() - 1])
                        maxRun = run2[run2.size() - 1];

                    if (run2[run2.size() - 1] == 254)
                    {
                        run2.push_back(245);
                        run2.push_back(1);
                    }
                    else
                        run2[run2.size() - 1] += 1;
                } else {
                    bit2 = !bit2;
                    run2.push_back(1);
                }
                if (bit3 == ((c >> 3) & 1)) {
                    if (maxRun < run3[run3.size() - 1])
                        maxRun = run3[run3.size() - 1];

                    if (run3[run3.size() - 1] == 254)
                    {
                        run3.push_back(245);
                        run3.push_back(1);
                    }
                    else
                        run3[run3.size() - 1] += 1;
                }
                else
                {
                    bit3 = !bit3;
                    run3.push_back(1);
                }
                if (bit4 == ((c >> 4) & 1)) {
                    if (maxRun < run4[run4.size() - 1])
                        maxRun = run4[run4.size() - 1];

                    if (run4[run4.size() - 1] == 254)
                    {
                        run4.push_back(245);
                        run4.push_back(1);
                    }
                    else
                        run4[run4.size() - 1] += 1;
                }
                else
                {
                    bit4 = !bit4;
                    run4.push_back(1);
                }
                if (bit5 == ((c >> 5) & 1)) {
                    if (maxRun < run5[run5.size() - 1])
                        maxRun = run5[run5.size() - 1];

                    if (run5[run5.size() - 1] == 254)
                    {
                        run5.push_back(245);
                        run5.push_back(1);
                    }
                    else
                        run5[run5.size() - 1] += 1;
                }
                else
                {
                    bit5 = !bit5;
                    run5.push_back(1);
                }
                if (bit6 == ((c >> 6) & 1)) {
                    if (maxRun < run6[run6.size() - 1])
                        maxRun = run6[run6.size() - 1];

                    if (run6[run6.size() - 1] == 254)
                    {
                        run6.push_back(245);
                        run6.push_back(1);
                    }
                    else
                        run6[run6.size() - 1] += 1;
                }
                else
                {
                    bit6 = !bit6;
                    run6.push_back(1);
                }
                if (bit7 == ((c >> 7) & 1)) {
                    if (maxRun < run7[run7.size() - 1])
                        maxRun = run7[run7.size() - 1];

                    if (run7[run7.size() - 1] == 254)
                    {
                        run7.push_back(245);
                        run7.push_back(1);
                    }
                    else
                        run7[run7.size() - 1] += 1;
                }
                else
                {
                    bit7 = !bit7;
                    run7.push_back(1);
                }
            }
*/