#ifndef _INCLUDED_ESP_COMPRESSOR_HPP
#define _INCLUDED_ESP_COMPRESSOR_HPP

#include <tudocomp/util.hpp>
#include <tudocomp/Env.hpp>
#include <tudocomp/Compressor.hpp>
#include <tudocomp/ds/GenericIntVector.hpp>

namespace tdc {

using int_vector::GenericIntVector;

class EspCompressor: public Compressor {

// Implementation that covers all of 64 bit
// TODO: Does the Paper mean base-e or base-2 ?
inline size_t iter_log(size_t n) {
    if (n < 3) return 1;
    if (n < 16) return 2;
    if (n < 3814280) return 3;
    return 4;
}

struct MetaBlock {
    size_t type;
    View view;
};

bool meta_blocks_debug(const std::vector<MetaBlock> meta_blocks, View in) {
    std::cout << "\nMeta blocks:\n";

    bool ok = false;
    {
        std::stringstream ss;
        for (auto& mb : meta_blocks) {
            ss << mb.view;
        }
        ok = (ss.str() == std::string(in));
    }

    std::cout << "|";
    for (auto& mb : meta_blocks) {
        size_t w = mb.view.size() / 2;
        std::cout << std::setw(w) << "";
        std::cout << std::left << std::setw(mb.view.size() - w) << mb.type;
        std::cout << "|";
    }
    std::cout << "\n";

    std::cout << "|";
    for (auto& mb : meta_blocks) {
        std::cout << mb.view << "|";
    }
    std::cout << "\n";
    return ok;
}

bool blocks_debug(const std::vector<View> blocks, View in) {
    std::cout << "\nBlocks:\n";

    bool ok = false;
    {
        std::stringstream ss;
        for (auto& b : blocks) {
            ss << b;
        }
        ok = (ss.str() == std::string(in));
    }

    std::cout << "|";
    for (auto& b : blocks) {
        size_t w = b.size();
        std::cout << std::setw(w) << "";
        std::cout << "|";
    }
    std::cout << "\n";

    std::cout << "|";
    for (auto& b : blocks) {
        std::cout << b << "|";
    }
    std::cout << "\n";
    return ok;
}

template<class T>
uint64_t calc_alphabet_size(const T& t) {
    Counter<typename T::value_type> c;
    for (auto v : t) {
        c.increase(v);
    }
    return c.getNumItems();
}

template<class T>
bool no_adjacent_identical(const T& t) {
    for(size_t i = 1; i < t.size(); i++) {
        if (t[i] == t[i - 1]) return false;
    }
    return true;
}

uint64_t label(uint64_t left, uint64_t right) {
    auto diff = left ^ right;

    //std::cout << "l: " << std::setbase(2) << left << "\n";
    //std::cout << "r: " << std::setbase(2) << right << "\n";
    //std::cout << "d: " << std::setbase(2) << diff << "\n";
    //std::cout << "\n";


    DCHECK(diff != 0);

    auto l = __builtin_ctz(diff);

    auto bit = [](uint8_t l, uint64_t v) {
        // TODO: test
        return (v >> l) & 1;
    };

    // form label(A[i])
    return 2*l + bit(l, right);
};

template<class T, class F>
void for_neigbors(T& t, F f) {
    for (size_t i = 0; i < t.size(); i++) {
        typename T::value_type neighbors[2];
        uint8_t neighbor_len = 0;

        if (i == 0 && i == t.size() - 1) {
            neighbor_len = 0;
        } else if (i == 0) {
            neighbor_len = 1;
            neighbors[0] = t[i + 1];
        } else if (i == t.size() - 1) {
            neighbor_len = 1;
            neighbors[0] = t[i - 1];
        } else {
            neighbor_len = 2;
            neighbors[0] = t[i - 1];
            neighbors[1] = t[i + 1];
        }

        f(i, neighbors, neighbor_len);
    }
}

template<class T>
bool check_landmarks(const T& t) {
    size_t last = 0;
    size_t i = 0;
    for(; i < t.size(); i++) {
        if (t[i] == 1u) {
            if (i > 1) return false;
            last = i;
            i++;
            break;
        }
    }
    for(; i < t.size(); i++) {
        if (t[i] == 1u) {
            if ((i - last) > 3 || (i - last) < 2) return false;
            last = i;
        }
    }
    return true;
}

// NB: Thuis assumes iter_log(alphabet_size) valid bytes before A
template<class G>
inline void handle_meta_block_2(View A,
                                uint64_t alphabet_size,
                                std::vector<uint8_t>& buf,
                                G push_block) {
    DCHECK(A.size() > 0);
    auto type_3_prefix = iter_log(alphabet_size);
    A = View(A.data() - type_3_prefix, A.size() + type_3_prefix);
    buf.clear();
    buf.insert(buf.cbegin(), A.cbegin(), A.cend());

    std::cout << "  " << vec_to_debug_string(buf) << "\n";
    std::cout << "  " << "Reduce to 6:\n";

    for (uint shrink_i = 0; shrink_i < iter_log(alphabet_size); shrink_i++) {
        for (size_t i = 1; i < buf.size(); i++) {
            auto left  = buf[i - 1];
            auto right = buf[i];
            buf[i - 1] = label(left, right);
        }
        buf.pop_back();

        std::cout << "  " << vec_to_debug_string(buf) << "\n";
    }

    DCHECK(calc_alphabet_size(buf) <= 6);

    std::cout << "  " << "Reduce to 3:\n";

    // TODO: This would benefit from a general, mutable, slice type

    // final pass: reduce to alphabet 3
    for(uint to_replace = 3; to_replace < 6; to_replace++) {
        for_neigbors(buf, [&](size_t i, uint8_t neighbors[], uint8_t neighbor_len) {
            auto& e = buf[i];
            if (e == to_replace) {
                e = 0;
                for (uint8_t j = 0; j < neighbor_len; j++) {
                    if (neighbors[j] == e) {
                        e++;
                    }
                }
                for (uint8_t j = 0; j < neighbor_len; j++) {
                    if (neighbors[j] == e) {
                        e++;
                    }
                }
            }
        });

        std::cout << "  " << vec_to_debug_string(buf) << "\n";
    }

    DCHECK(calc_alphabet_size(buf) <= 3);
    DCHECK(no_adjacent_identical(buf));

    // find landmarks:

    // TODO: Maybe store in high bits of buf to reduce memory?
    // buf gets reduced to 2 bit values anyway, and stays around long enough
    GenericIntVector<uint_t<1>> landmarks(buf.size());

    for_neigbors(buf, [&](size_t i, uint8_t neighbors[], uint8_t neighbor_len) {
        bool is_high_landmark = true;
        for (uint8_t j = 0; j < neighbor_len; j++) {
            if (neighbors[j] > buf[i]) {
                is_high_landmark = false;
            }
        }
        if (is_high_landmark) {
            landmarks[i] = 1;
        }
    });

    std::cout << "  High Landmarks:\n";
    std::cout << "  " << vec_to_debug_string(landmarks) << "\n";

    for_neigbors(buf, [&](size_t i, uint8_t neighbors[], uint8_t neighbor_len) {
        bool is_low_landmark = true;
        for (uint8_t j = 0; j < neighbor_len; j++) {
            if (neighbors[j] < buf[i]) {
                is_low_landmark = false;
            }
        }
        if (is_low_landmark) {
            if (   (!(i > 0)              || (landmarks[i - 1] == 0u))
                && (!(i < buf.size() - 1) || (landmarks[i + 1] == 0u))
            ) {
                landmarks[i] = 1;
            }
        }
    });

    std::cout << "  High and Low Landmarks:\n";
    std::cout << "  " << vec_to_debug_string(landmarks) << "\n";

    DCHECK(check_landmarks(landmarks));

    // assign blocks

    /*
    size_t last = 0;
    // we can cut iteration time in half by making use of the 2-3
    // difference between each landmark
    for(size_t i = 0; i < landmarks.size(); i += 2) {
        if (landmarks[i] == 0u) {
            i++;
        }
        DCHECK(i < landmarks.size());
        DCHECK(landmarks[i] == 1u);

        last = i;
    }

    */

    // TODO: An abstraction for iterating while
    // having special setup/teardown sections
    // for the first N and last M elements, as well if the array is only O long

    // TODO: debug only
    std::vector<size_t> debug_landmark_assoc(buf.size());

    size_t last_closes_landmark = 0;
    for (size_t i = 1; i < buf.size(); i++) {
        if (landmarks[i] == 1u) {
            last_closes_landmark = i;
        }
        debug_landmark_assoc[i - 1] = last_closes_landmark;
    }
    if (debug_landmark_assoc.size() > 0) {
        debug_landmark_assoc.back() = last_closes_landmark;
    }

    std::cout << "  Block-Landmark Assignment:\n";
    std::cout << "  " << vec_to_debug_string(debug_landmark_assoc) << "\n";

    if (debug_landmark_assoc.size() > 0) {
        auto block_range = [&](size_t a, size_t b) {
            //std::cout << a << " - " << b << "\n";
            push_block(A.substr(type_3_prefix).substr(a, b));
        };

        size_t last_pos = 0;
        for(size_t i = 0; i < debug_landmark_assoc.size() - 1; i++) {
            if (debug_landmark_assoc[i] != debug_landmark_assoc[i + 1]) {
                block_range(last_pos, i + 1);
                last_pos = i + 1;
            }
        }
        block_range(last_pos, debug_landmark_assoc.size());
    }

}

template<class G>
inline void handle_meta_block_13(View A,
                                 G push_block) {
    while (true) {
        auto s = A.size();
        DCHECK(s > 1);

        if (s == 2) {
            push_block(A);
            return;
        } else if (s == 3) {
            push_block(A);
            return;
        } else if (s == 4) {
            push_block(A.substr(0, 2));
            push_block(A.substr(2));
            return;
        } else {
            push_block(A.substr(0, 3));
            A = A.substr(3);
        }
    }
}

public:
    inline static Meta meta() {
        Meta m("compressor", "esp", "ESP based grammar compression");
        //m.option("coder").templated<coder_t>();
        //m.option("min_run").dynamic("3");
        return m;
    }

    using Compressor::Compressor;

    inline virtual void compress(Input& input, Output& output) override {
        auto in = input.as_view();
        DCHECK(in.size() > 0 /* 0-byte input not covered by paper */);

        size_t alphabet_size = 256;

        std::vector<View> blocks;

        {
            std::vector<MetaBlock> meta_blocks;

            std::vector<uint8_t> buf;

            auto push_block = [&](View A) {
                blocks.push_back(A);
            };

            auto type_23_prefix = iter_log(alphabet_size);

            // Merge length-one-block with adjacent one
            auto middle_meta_block = [&](uint8_t type, View A) {
                // Attach to right meta block if not attached to left one
                if (meta_blocks.size() > 0 && meta_blocks.back().view.size() == 1) {
                    meta_blocks.pop_back();
                    A = View(A.data() - 1, A.size() + 1);
                }

                // Attach to left meta block before attempting to attach to right one
                if (type != 2 && A.size() == 1) {
                    if (meta_blocks.size() > 0 && meta_blocks.back().type == 1) {
                        auto& v = meta_blocks.back().view;
                        v = View(v.data(), v.size() + 1);
                    } else {
                        meta_blocks.push_back(MetaBlock { type, A });
                    }
                } else {
                    meta_blocks.push_back(MetaBlock { type, A });
                }
            };

            // Split long into short and long
            auto initial_meta_block = [&](uint8_t type, View A) {
                if (type == 2) {
                    middle_meta_block(3, A.substr(0, type_23_prefix));
                    A = A.substr(type_23_prefix);
                    if (A.size() > 0) {
                        middle_meta_block(2, A);
                    }
                } else {
                    middle_meta_block(type, A);
                }
            };

            // Split into repeating, long and short
            size_t i = 0;
            while(i < in.size()) {
                auto type_1_start = i;
                while ((i < (in.size() - 1)) && (in[i] == in[i + 1])) {
                    i++;
                }
                if ((i - type_1_start) > 0) {
                    View A = in.substr(type_1_start, i + 1);
                    {
                        initial_meta_block(1, A);
                    }
                    i++;
                }

                auto type_23_start = i;
                while (i < (in.size() - 1) && in[i] != in[i + 1]) {
                    i++;
                }

                if (i == in.size() - 1) {
                    i++;
                }
                size_t type_23_len = i - type_23_start;
                if (type_23_len > 0) {
                    View A = in.substr(type_23_start, i);

                    if (type_23_len >= iter_log(alphabet_size)) {
                        initial_meta_block(2, A);
                    } else {
                        initial_meta_block(3, A);
                    }
                }
            }

            // Final processing, without any more meta block splitting
            auto real_meta_block = [&](uint8_t type, View A) {
                if (type == 2) {
                    handle_meta_block_2(A, alphabet_size, buf, push_block);
                } else {
                    DCHECK(A.size() > 1 /* 1-byte input not covered by paper */);
                    handle_meta_block_13(A, push_block);
                }

                std::cout << "  ---\n";
            };

            DCHECK(meta_blocks_debug(meta_blocks, in));

            for (auto mb : meta_blocks) {
                real_meta_block(mb.type, mb.view);
            }
        }

        DCHECK(blocks_debug(blocks, in));
    }

    inline virtual void decompress(Input& input, Output& output) override {

    }
};

}

#endif
