//
//  pdecompress.hpp
//  pzip
//
//  Created by Phuc Nguyen on 07/21/20.
//  Copyright © 2020 Phuc Nguyen. All rights reserved.
//

#include <iostream>
#include "bwt.hpp"
#include "mtf.hpp"
#include "input_stream.hpp"
#include "fse.hpp"
#include <cassert>

#define FSE_MODE 0
#define RLE_MODE 1

using namespace std;

int main(){

    InputBitStream stream{cin};

    while (1) {
        u8 last_block = stream.read_byte();
        u8 mode = stream.read_byte();
        assert(mode == FSE_MODE || mode == RLE_MODE);
        if (mode == FSE_MODE) {
            //Read BWT meta
            u32 index = stream.read_u32();

            u16 num_symbols = stream.read_u16();
            u32 rle_block_size = stream.read_u32();
            vector<int> freqs(num_symbols);
            for (u16 i = 0; i < num_symbols; i++) {
                freqs[i] = stream.read_u16();
            }
            int byte_offset = stream.read_byte();
            int state = stream.read_u32();
            int encoded_size = stream.read_u32();
            vector<u8> encoded_stream(encoded_size);
            for (int i = 0; i < encoded_size; i++) {
                encoded_stream[i] = stream.read_byte();
            }
            FSE fse;
            vector<u8> decoded_stream(rle_block_size);
            fse.Decompress(encoded_stream, freqs, decoded_stream, byte_offset, state, num_symbols);

            vector<u8> decompressed = ibwt(MTF_decode(RLE_decode(decoded_stream)), index);
            for (auto b: decompressed) {
                cout.put(b);
            }

//            for (auto b: decoded_stream) {
//                cout.put(b);
//            }
        } else {
            //RLE Mode
            u32 block_size = stream.read_u32();
            u32 index = stream.read_u32();
            vector<u8> block(block_size);
            for (size_t i = 0; i < block_size; i++) {
                block[i] = stream.read_byte();
            }
            vector<u8> decompressed = ibwt(MTF_decode(RLE_decode(block)), index);
            for (auto b: decompressed) {
                cout.put(b);
            }
        }

        if (last_block == 1) {
            break;
        }
    }
//    char c;
//    while(std::cin.get(c)){
//        std::cout.put(c);
//    }

    return 0;
}
