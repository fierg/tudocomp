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
        std::vector<Literal> m_literals;
        len_t m_pos;

    public:
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
                    l.pos = bitPos + 8 * vPos;
                    m_literals.push_back(l);

                    // log
                    std::cout << "| l(" << (unsigned int)l.c << ", " << l.pos << ")"
                              << "\n";
                }
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
            std::vector<BYTE> mapping = createByteMapping(istream);

            std::vector<BYTE> run0; run0.push_back(0);
            std::vector<BYTE> run1; run1.push_back(0);
            std::vector<BYTE> run2; run2.push_back(0);
            std::vector<BYTE> run3; run3.push_back(0);
            std::vector<BYTE> run4; run4.push_back(0);
            std::vector<BYTE> run5; run5.push_back(0);
            std::vector<BYTE> run6; run6.push_back(0);
            std::vector<BYTE> run7; run7.push_back(0);

            bool bit0 = false;
            bool bit1 = false;
            bool bit2 = false;
            bool bit3 = false;
            bool bit4 = false;
            bool bit5 = false;
            bool bit6 = false;
            bool bit7 = false;

            std::cout
                << "[debug]  iView size: " << iview.size() << "\n";

            for (len_t i = 0; i < iview.size(); i++)
            {
                uliteral_t c = mapping[iview[i]];
                std::cout
                    << "[debug] i: " << i << " c: " << iview[i] << " m: " << int(c) << "\n";
                if (bit0 == ((c >> 0) & 1))
                    run0[run0.size() - 1] += 1;
                else
                {
                    bit0 = !bit0;
                    run0.push_back(1);
                }
                if (bit1 == ((c >> 1) & 1))
                    run1[run1.size() - 1] += 1;
                else
                {
                    bit1 = !bit1;
                    run1.push_back(1);
                }
                if (bit2 == ((c >> 2) & 1))
                    run2[run2.size() - 1] += 1;
                else
                {
                    bit2 = !bit2;
                    run2.push_back(1);
                }
                if (bit3 == ((c >> 3) & 1))
                    run3[run3.size() - 1] += 1;
                else
                {
                    bit3 = !bit3;
                    run3.push_back(1);
                }
                if (bit4 == ((c >> 4) & 1))
                    run4[run4.size() - 1] += 1;
                else
                {
                    bit4 = !bit4;
                    run4.push_back(1);
                }
                if (bit5 == ((c >> 5) & 1))
                    run5[run5.size() - 1] += 1;
                else
                {
                    bit5 = !bit5;
                    run5.push_back(1);
                }
                if (bit6 == ((c >> 6) & 1))
                    run6[run6.size() - 1] += 1;
                else
                {
                    bit6 = !bit6;
                    run6.push_back(1);
                }
                if (bit7 == ((c >> 7) & 1))
                    run7[run7.size() - 1] += 1;
                else
                {
                    bit7 = !bit7;
                    run7.push_back(1);
                }
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



                

            typename coder_t::Encoder coder(env().env_for_option("coder"), output, Literals(allRuns));
        }

        inline virtual void decompress(Input &input, Output &output) override
        {
            // retrieve an output stream
            auto ostream = output.as_stream();

            // instantiate the decoder using the whole input alphabet
            typename coder_t::Decoder decoder(
                env().env_for_option("coder"), input);

            // decode text
            while (!decoder.eof())
            {
                // decode text
                //uliteral_t c = c_min + decoder.template decode<uliteral_t>(occ_r);
            }
        }

        inline std::vector<BYTE> createByteMapping(std::istream &src)
        {
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