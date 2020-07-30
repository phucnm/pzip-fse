//
//  pcompress.hpp
//  pzip
//
//  Created by Phuc Nguyen on 07/21/20.
//  Copyright © 2020 Phuc Nguyen. All rights reserved.
//

#include <iostream>
#include <iomanip>
#include <vector>
#include <set>
#include <unordered_map>
#include <array>
#include <cassert>
#include "bwt.hpp"
#include "mtf.hpp"
#include "output_stream.hpp"
#include "CRC.h"
#include "fse.hpp"

#define CHUNK_SIZE 900000
#define FSE_MODE 0
#define RLE_MODE 1

using namespace std;

bool check_fse(array<u8, CHUNK_SIZE> block, int block_size) {
    vector<u8> data(block.begin(), block.begin() + block_size);

    int data_size = block_size;
    int max_val = *max_element(data.begin(), data.end());
    int nSymbols = max_val + 1;

    int byte_offset, state;
    vector<u8> encoded_stream;
    vector<int> freq;

    FSE fse;

    bool success = fse.Compress(data, nSymbols, encoded_stream, freq, byte_offset, state);
    assert(success);

    vector<u8> decoded_stream(data_size);
    fse.Decompress(encoded_stream, freq, decoded_stream, byte_offset, state, nSymbols);

    return data == decoded_stream;
}

bool check_bwt(array<u8, CHUNK_SIZE> block, int block_size) {
    cerr<<"Checking block with size "<<block_size<<endl;
    int index;
    vector<u8> v_block(block.begin(), block.begin() + block_size);
    vector<u8> encoded = bwt2(v_block, index);
    cerr<<"BWT done, index "<<index<<endl;
    vector<u8> decoded = ibwt(encoded, index);
    bool res = v_block == decoded;
    if (res) {
        cerr<<"BWT inversion success"<<endl;
    }
    return res;
}

void compress(OutputBitStream stream, array<u8, CHUNK_SIZE> block, u32 block_size, bool last_block) {
    int index;
    vector<u8> v_block(block.begin(), block.begin() + block_size);
    vector<u8> bwt = bwt2(v_block, index);
    auto mtf = MTF_encode(bwt);
    auto rle = RLE_encode(mtf);
    float ratio = (float)rle.size() / (float)block_size;
    cerr<<"Original size: "<<block_size<<", rle size: "<<rle.size()<<", ratio: "<<std::setprecision(2)<<ratio<<endl;

//    vector<u8> decoded = ibwt(MTF_decode(RLE_decode(rle)), index);
//    assert(decoded == v_block);
//    cerr<<"Decoded successfully"<<endl;

    // ===========================
    // Do FSE coding
    int max_val = *max_element(rle.begin(), rle.end());
    int nSymbols = max_val + 1;
    int byte_offset, state;
    vector<u8> encoded_stream;
    vector<int> freq;
    FSE fse;
    bool fse_success = fse.Compress(rle, nSymbols, encoded_stream, freq, byte_offset, state);
    assert((int)freq.size() == nSymbols);

    // ===========================
    // Output stream
    stream.push_byte((u8)last_block);
    if (fse_success) {
        // Output FSE bitstream
        cerr<<"Encoding FSE"<<endl;

        stream.push_byte(FSE_MODE);
        // Output RLE and BWT meta first.
        stream.push_u32((u32)index);

        stream.push_u16(nSymbols);
        stream.push_u32((u32)rle.size());
        for (int f: freq) {
            stream.push_u16((u16)f);
        }
        stream.push_byte(byte_offset);
        stream.push_u32(state);
        stream.push_u32((u32)encoded_stream.size());
        for (u8 b: encoded_stream) {
            stream.push_byte(b);
        }
    } else {
        // fall over RLE bitsream
        cerr<<"Encoding RLE"<<endl;
        stream.push_byte(RLE_MODE);
        stream.push_u32((u32)rle.size());
        stream.push_u32((u32)index);
        for (auto byte: rle) {
            stream.push_byte(byte);
        }
    }
}

int main(){

    array<u8, CHUNK_SIZE> a{0, 1, 2, 1, 1, 3, 2, 3, 3, 1};
    assert(check_fse(a, 10));

    OutputBitStream stream {cout};

    //Pre-cache the CRC table
    auto crc_table = CRC::CRC_32().MakeTable();

    array< u8, CHUNK_SIZE > block_contents {};
    u32 block_size {0};
    u32 bytes_read {0};

    char next_byte {};

    u32 crc {};

    if (!cin.get(next_byte)){
        //Empty input?
    }else{
        bytes_read++;
        //Update the CRC as we read each byte (there are faster ways to do this)
        crc = CRC::Calculate(&next_byte, 1, crc_table); //This call creates the initial CRC value from the first byte read.
        //Read through the input
        while(1){
            block_contents.at(block_size++) = next_byte;

            if (!cin.get(next_byte))
                break;

            bytes_read++;
            crc = CRC::Calculate(&next_byte,1, crc_table, crc); //Add the character we just read to the CRC (even though it is not in a block yet)

            //If we get to this point, we just added a byte to the block AND there is at least one more byte in the input waiting to be written.
            if (block_size == block_contents.size()){
//                assert(check_fse(block_contents, block_size));
//                assert(check_bwt(block_contents, block_size));
                compress(stream, block_contents, block_size, false);
                block_size = 0;
            }
        }
    }
    //At this point, we've finished reading the input (no new characters remain), and we may have an incomplete block to write.
    if (block_size > 0){
//        assert(check_fse(block_contents, block_size));
//        assert(check_bwt(block_contents, block_size));
        compress(stream, block_contents, block_size, true);
        block_size = 0;
    }

    return 0;
}
