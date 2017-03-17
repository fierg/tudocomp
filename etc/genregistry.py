#!/usr/bin/python3

import re
import sys
import itertools

if not len(sys.argv[1:]) == 3:
    print(str.format("Usage {} [config.h] [selection] [outfile]", sys.argv[0]))
    sys.exit(1)

config_path = sys.argv[1]
selection = sys.argv[2]
outfile = sys.argv[3]

def config_match(pattern):
    textfile = open(config_path, 'r')
    filetext = textfile.read()
    textfile.close()
    pattern = re.compile(pattern)
    for line in filetext.splitlines():
        for match in re.finditer(pattern, line):
            return True
    return False

context_free_coder = [
    ("ASCIICoder",      "coders/ASCIICoder.hpp",      []),
    ("BitCoder",        "coders/BitCoder.hpp",        []),
    ("EliasGammaCoder", "coders/EliasGammaCoder.hpp", []),
    ("EliasDeltaCoder", "coders/EliasDeltaCoder.hpp", []),
]

# TODO: Fix bad interaction between sle and lz78u code and remove this distinction
tmp_lz78u_string_coder = context_free_coder + [
    ("HuffmanCoder", "coders/HuffmanCoder.hpp", []),
]

coder = tmp_lz78u_string_coder + [
    ("SLECoder",   "coders/SLECoder.hpp",   []),
] + [
    ("ArithmeticCoder", "coders/ArithmeticCoder.hpp", []),
]


lz78_trie = [
    ("lz78::BinarySortedTrie", "compressors/lz78/BinarySortedTrie.hpp", []),
    ("lz78::BinaryTrie",       "compressors/lz78/BinaryTrie.hpp",       []),
    ("lz78::HashTrie",         "compressors/lz78/HashTrie.hpp",         []),
    ("lz78::MyHashTrie",       "compressors/lz78/MyHashTrie.hpp",       []),
    ("lz78::TernaryTrie",      "compressors/lz78/TernaryTrie.hpp",      []),
    ("lz78::CedarTrie",        "compressors/lz78/CedarTrie.hpp",        []),
]

if config_match("^#define JUDY_H_AVAILABLE 1"): lz78_trie += [
    ("lz78::JudyTrie",         "compressors/lz78/JudyTrie.hpp",         []),
]

lcpc_strat = [
    ("lcpcomp::MaxHeapStrategy",  "compressors/lcpcomp/compress/MaxHeapStrategy.hpp",   []),
    ("lcpcomp::MaxLCPStrategy",   "compressors/lcpcomp/compress/MaxLCPStrategy.hpp",    []),
    ("lcpcomp::ArraysComp", "compressors/lcpcomp/compress/ArraysComp.hpp",  []),
    ("lcpcomp::PLCPPeaksStrategy","compressors/lcpcomp/compress/PLCPPeaksStrategy.hpp", []),
]

if config_match("^#define Boost_FOUND 1"): lcpc_strat += [
    ("lcpcomp::BoostHeap",  "compressors/lcpcomp/compress/BoostHeap.hpp",   []),
    ("lcpcomp::PLCPStrategy",     "compressors/lcpcomp/compress/PLCPStrategy.hpp",      [])
    ]

lcpc_buffer = [
    ("lcpcomp::ScanDec",       "compressors/lcpcomp/decompress/ScanDec.hpp", []),
    ("lcpcomp::DecodeForwardQueueListBuffer", "compressors/lcpcomp/decompress/DecodeQueueListBuffer.hpp",  []),
    ("lcpcomp::CompactDec",           "compressors/lcpcomp/decompress/CompactDec.hpp",     []),
    ("lcpcomp::MyMapBuffer",                  "compressors/lcpcomp/decompress/MyMapBuffer.hpp",            []),
    ("lcpcomp::MultimapBuffer",               "compressors/lcpcomp/decompress/MultiMapBuffer.hpp",         []),
]

lcpc_coder = [
    ("ASCIICoder", "coders/ASCIICoder.hpp", []),
    ("SLECoder", "coders/SLECoder.hpp", []),
]

lz78u_strategy = [
    ("lz78u::StreamingStrategy", "compressors/lz78u/StreamingStrategy.hpp", [context_free_coder]),
    ("lz78u::BufferingStrategy", "compressors/lz78u/BufferingStrategy.hpp", [tmp_lz78u_string_coder]),
]

textds = [
    ("TextDS<>", "ds/TextDS.hpp", [])
]

compressors = [
    ("LCPCompressor",               "compressors/LCPCompressor.hpp",               [lcpc_coder, lcpc_strat, lcpc_buffer, textds]),
    ("LZ78UCompressor",             "compressors/LZ78UCompressor.hpp",             [lz78u_strategy, context_free_coder]),
    ("RunLengthEncoder",            "compressors/RunLengthEncoder.hpp",            []),
    ("LiteralEncoder",              "compressors/LiteralEncoder.hpp",              [coder]),
    ("LZ78Compressor",              "compressors/LZ78Compressor.hpp",              [context_free_coder, lz78_trie]),
    ("LZWCompressor",               "compressors/LZWCompressor.hpp",               [context_free_coder, lz78_trie]),
    ("RePairCompressor",            "compressors/RePairCompressor.hpp",            [coder]),
    ("LZSSLCPCompressor",           "compressors/LZSSLCPCompressor.hpp",           [coder, textds]),
    ("LZSSSlidingWindowCompressor", "compressors/LZSSSlidingWindowCompressor.hpp", [context_free_coder]),
    ("MTFCompressor",               "compressors/MTFCompressor.hpp",               []),
    ("NoopCompressor",              "compressors/NoopCompressor.hpp",              []),
    ("BWTCompressor",               "compressors/BWTCompressor.hpp",               [textds]),
    ("ChainCompressor",             "../tudocomp_driver/ChainCompressor.hpp",      []),
]

generators = [
    ("FibonacciGenerator",     "generators/FibonacciGenerator.hpp", []),
    ("ThueMorseGenerator",     "generators/ThueMorseGenerator.hpp", []),
    ("RandomUniformGenerator", "generators/RandomUniformGenerator.hpp", []),
    ("RunRichGenerator",       "generators/RunRichGenerator.hpp", []),
]

algorithms_cpp_template = '''
/* Autogenerated file by genregistry.py */
#include <tudocomp/tudocomp.hpp>
#include <tudocomp_driver/Registry.hpp>

namespace tdc_algorithms {

using namespace tdc;

void register_compressors(Registry<Compressor>& r);
void register_generators(Registry<Generator>& r);

// One global instance for the registry
Registry<Compressor> COMPRESSOR_REGISTRY = Registry<Compressor>::with_all_from(register_compressors, "compressor");
Registry<Generator> GENERATOR_REGISTRY = Registry<Generator>::with_all_from(register_generators, "generator");

void register_compressors(Registry<Compressor>& r) {
$COMPRESSORS
}//function register_compressors

void register_generators(Registry<Generator>& r) {
$GENERATORS
}//function register_generators

}//ns
'''

tudocomp_hpp_template = '''
/*
    Autogenerated file by genregistry.py
    Include this to include practically all of tudocomp.

    This header also contains the Doxygen documentation of the main namespaces.
*/

#pragma once

/// \\brief Contains the text compression and encoding framework.
///
/// This is the framework's central namespace in which text compression and
/// coding algorithms are contained, as well as utilities needed for text
/// compression and coding (e.g. I/O). Families of compressors and encoders
/// or utility groups are contained in the respective sub-namespaces. The
/// namespace \c tudocomp itself contains types important for all of the
/// framework and its communication.
namespace tdc {

/// \\brief Contains I/O abstractions and utilities.
///
/// All I/O done by compressors and encoders is routed through the \\ref Input
/// and \\ref Output abstractions over the underlying file, memory or stream
/// I/O.
///
/// \sa
/// \\ref Input for the input interface and \\ref Output for the output
/// interface.
namespace io {
}

/// \\brief Contains compressors and encoders that work with Lempel-Ziv-78-like
/// dictionaries.
///
/// The LZ78 family works with bottom-up dictionaries containing indexed
/// entries to achieve compression. Each entry points to a \e prefix (another
/// dictionary entry) and a follow-up symbol.
namespace lz78 {
}

/// \\brief Contains compressors and encoders that work with
/// Lempel-Ziv-Storer-Szymansky-like factors.
///
/// The LZSS family works with factors representing references to positions
/// within the original text that replace parts of the same text, effectively
/// using the input text itself as a dictionary. They consist of a \e source
/// text position and a \e length.
namespace lzss {
}

/// \\brief Contains compressors and encoders that work with
/// Lempel-Ziv-Welch-like dictionaries.
///
/// The LZW family works with bottom-up dictionaries containing indexed entries
/// to achieve compression. Other than \\ref lz78, the dictionary entries do not
/// explicitly store the follow-up symbol. Instead, they are re-generated on
/// the fly by the decoder.
namespace lzw {
}
}//ns tdc
'''

# Generates carthesian product of template params
def gen_list(ls):
    def expand_deps(algorithm):
        name = algorithm[0]
        deps = algorithm[2]

        deps_lists = [gen_list(x) for x in deps]
        deps_lists_prod = itertools.product(*deps_lists)

        return_list = []
        for deps_tuple in deps_lists_prod:
            if len(deps_tuple) == 0:
                return_list += [str.format("{}", name)]
            else:
                return_list += [str.format("{}<{}>", name, ",".join(deps_tuple))]

        assert len(return_list) != 0

        return return_list

    return_list = []
    for algorithm in ls:
        return_list += expand_deps(algorithm)
    return return_list

# Generates list of all includes
def gather_header(ls):
    headers = set()
    for e in ls:
        headers |= { e[1] }
        for a in e[2]:
            headers |= gather_header(a)
    return headers

# Output tudocomp.hpp
def gen_tudocomp_hpp():
    tudocomp_hpp = tudocomp_hpp_template + "\n"
    for header in sorted(gather_header(compressors + generators)):
        tudocomp_hpp += str.format("#include <tudocomp/{}>\n", header)
    return tudocomp_hpp

# Output algorithm.cpp
def gen_algorithm_cpp():
    algorithms_cpp = algorithms_cpp_template
    l_compressors = []
    for line in gen_list(compressors):
        l_compressors += [str.format("    r.register_algorithm<{}>();", line)]

    l_generators = []
    for line in gen_list(generators):
        l_generators += [str.format("    r.register_algorithm<{}>();", line)]

    return algorithms_cpp.replace(
                "$COMPRESSORS", "\n".join(l_compressors)).replace(
                "$GENERATORS", "\n".join(l_generators))+ "\n"

if selection == "tudocomp.hpp":
    file2 = open(outfile, 'w+')
    file2.write(gen_tudocomp_hpp())
    file2.close()
elif selection == "tudocomp_algorithms.cpp":
    file1 = open(outfile, 'w+')
    file1.write(gen_algorithm_cpp())
    file1.close()
else:
    sys.exit(1)
