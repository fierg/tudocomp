#pragma once



#include <tudocomp/Compressor.hpp>
#include <tudocomp/Range.hpp>
#include <tudocomp/util.hpp>
#include <vector>
#include <tuple>

#include <tudocomp/io.hpp>


#include <tudocomp/io/BitIStream.hpp>
#include <tudocomp/io/BitOStream.hpp>
//#include <sdsl/bit_vectors.hpp>


#include <tudocomp/ds/IntVector.hpp>


#include <tudocomp/Literal.hpp>


//#include <tudocomp/tudocomp.hpp>


namespace tdc {
namespace lfs {



class STStrategy;

template<typename literal_coder_t, typename len_coder_t, typename strategy_t >
class LFSCompressor : public Compressor {
private:


    //(position in text, non_terminal_symbol_number, length_of_symbol);
    typedef std::tuple<uint,uint,uint> non_term;
    typedef std::vector<non_term> non_terminal_symbols;

    typedef std::vector<std::pair<uint,uint>> rules;

public:



    inline static Meta meta() {
        Meta m("compressor", "lfs_comp",
            "This is an implementation of the longest first substitution compression scheme.");
        m.option("lit_coder").templated<literal_coder_t>();
        m.option("len_coder").templated<len_coder_t>();
        m.option("sts").templated<strategy_t>();
        m.needs_sentinel_terminator();
        return m;
    }


    inline LFSCompressor(Env&& env):
        Compressor(std::move(env))
    {
        DLOG(INFO) << "Compressor instantiated";

    }
    inline virtual void compress(Input& input, Output& output) override {

        non_terminal_symbols nts_symbols = non_terminal_symbols();
        rules dictionary = rules();


        auto in = input.as_view();

        //
        strategy_t strategy(env().env_for_option("sts"));


        DLOG(INFO)<<"text size: "<<in.size();

        //compute dictionary and nts.
        strategy.compute_rules(in, dictionary, nts_symbols);

        DLOG(INFO)<<"dict size: "<<dictionary.size();
        DLOG(INFO)<<"symbols:"<<nts_symbols.size();



        // encode dictionary:
        DLOG(INFO) << "encoding dictionary symbol sizes ";

        std::shared_ptr<BitOStream> bitout = std::make_shared<BitOStream>(output);
        typename literal_coder_t::Encoder lit_coder(
            env().env_for_option("lit_coder"),
            bitout,
            NoLiterals()
        );
        typename len_coder_t::Encoder len_coder(
            env().env_for_option("len_coder"),
            bitout,
            NoLiterals()
        );

        auto it = dictionary.begin();
        //range int_r (0,MAX_INT);

        Range intrange (0, UINT_MAX);//uint32_r
        if(dictionary.size() >=1 ){

            std::pair<uint,uint> symbol = *it;
            uint last_length=symbol.second;
            Range s_length_r (0,last_length);
            len_coder.encode(last_length,intrange);
            it++;

            while (it != dictionary.end()){
                symbol=*it;
                len_coder.encode(last_length-symbol.second,s_length_r);
                last_length=symbol.second;
                it++;

            }
            len_coder.encode(symbol.second,s_length_r);
        }else {
            len_coder.encode(0,intrange);

        }


        DLOG(INFO) << "encoding dictionary symbols";
        // encode dictionary strings:
        if(dictionary.size() >=1 ){
            auto it = dictionary.begin();
            std::pair<uint,uint> symbol;// = *it;
            //Range slength_r (0, symbol.second);
            //coder.encode(symbol.second,intrange);//uint32_r

             while(it != dictionary.end()){
            //first length of non terminal symbol
                symbol = *it;
                //coder.encode(symbol.second,slength_r);

                for(uint k = 0; k<symbol.second; k++){
                    lit_coder.encode(in[symbol.first + k],literal_r);
                }
                it++;
            }
        }

        Range dict_r(0, dictionary.size());
        //encode string
        uint pos = 0;
        uint start_position;
        uint symbol_number ;
        uint symbol_length;
        bool first_char = true;
        for(auto it = nts_symbols.begin(); it!= nts_symbols.end(); it++){


            std::tuple<uint,uint,uint> next_position = *it;

            start_position = std::get<0>(next_position);
            symbol_number =std::get<1>(next_position);
            symbol_length = std::get<2>(next_position);

            while(pos< start_position){
                //get original text, because no symbol...
                lit_coder.encode(0, bit_r);
                lit_coder.encode(in[pos], literal_r);

                if(first_char){
                    first_char=false;
                }
                pos++;
            }

            //write symbol number

            lit_coder.encode(1, bit_r);
            lit_coder.encode(symbol_number, dict_r);

            pos += symbol_length;

        }
        //if no more terminals, write rest of text
        //one pos less, because ensure_null_ending adds a symbol
        while( pos<in.size()){

            lit_coder.encode(0, bit_r);
            lit_coder.encode(in[pos], literal_r);
            pos++;
        }

        DLOG(INFO) << "compression with lfs done";

    }

    inline virtual void decompress(Input& input, Output& output) override {
        DLOG(INFO) << "decompress lfs";
        std::shared_ptr<BitIStream> bitin = std::make_shared<BitIStream>(input);

        typename literal_coder_t::Decoder lit_decoder(
            env().env_for_option("lit_coder"),
            bitin
        );
        typename len_coder_t::Decoder len_decoder(
            env().env_for_option("len_coder"),
            bitin
        );
        Range int_r (0,UINT_MAX);

        uint symbol_length = len_decoder.template decode<uint>(int_r);
        Range slength_r (0, symbol_length);
        std::vector<uint> dict_lengths;
        dict_lengths.reserve(symbol_length);
        dict_lengths.push_back(symbol_length);
        while(symbol_length>0){

            uint current_delta = len_decoder.template decode<uint>(slength_r);
            symbol_length-=current_delta;
            dict_lengths.push_back(symbol_length);
        }
        dict_lengths.pop_back();

        std::vector<std::string> dictionary;
        uint dictionary_size = dict_lengths.size();

        Range dictionary_r (0, dictionary_size);


        uint length_of_symbol;
        std::string non_terminal_symbol;
        DLOG(INFO) << "reading dictionary";
        for(uint i = 0; i< dict_lengths.size();i++){
            non_terminal_symbol ="";
            char c1;
            length_of_symbol=dict_lengths[i];
            for(uint i =0; i< length_of_symbol;i++){
                c1 = lit_decoder.template decode<char>(literal_r);
                non_terminal_symbol += c1;
            }
            dictionary.push_back(non_terminal_symbol);
        }
        auto ostream = output.as_stream();

        while(!lit_decoder.eof()){
            //decode bit
            bool bit1 = lit_decoder.template decode<bool>(bit_r);
            char c1;
            uint symbol_number;
            // if bit = 0 its a literal
            if(!bit1){
                c1 = lit_decoder.template decode<char>(literal_r); // Dekodiere Literal

                ostream << c1;
            } else {
            //else its a non-terminal
                symbol_number = lit_decoder.template decode<uint>(dictionary_r); // Dekodiere Literal

                if(symbol_number < dictionary.size()){

                    ostream << dictionary.at(symbol_number);
                } else {
                    DLOG(INFO)<< "too large symbol: " << symbol_number;
                }

            }
        }
    }

};

}


}
