//
//  mtf.hpp
//  pzip
//
//  Created by Phuc Nguyen on 07/21/20.
//  Copyright © 2020 Phuc Nguyen. All rights reserved.
//

#include "linked_list.hpp"
#include <deque>
#include <iostream>

using namespace std;

vector<uint8_t> MTF_encode(const vector<uint8_t>& input) {
    list dict;
    //Establish the list
    for (size_t i = 0; i < 256; i++) {
        dict.insert_back((uint8_t)i);
    }

    vector<uint8_t> res(input.size());
    for (size_t i = 0; i < input.size(); i++) {
        int dist = dict.move_to_front(input[i]);
        res[i] = dist;
    }

    return res;
}

vector<uint8_t> MTF_decode(const vector<uint8_t>& input) {
    deque<uint8_t> dict;
    //Establish the list
    for (size_t i = 0; i < 256; i++) {
        dict.push_back(i);
    }

    vector<uint8_t> res(input.size());
    for (size_t i = 0; i < input.size(); i++) {
        res[i] = dict.at(input[i]);
        dict.erase(dict.begin() + input[i]);
        dict.push_front(res[i]);
    }
    return res;
}

const int MIN_LENGTH = 4;
const int MAX_LENGTH = 255;

// I'm lazy so that I put RLE here
// This method only encode 0 symbols after the MTF stage
vector<uint8_t> RLE_encode(const vector<uint8_t>& input) {
    int zero_count = 0;
    int rle_length = 0;
    vector<uint8_t> encoded;
    size_t size = input.size();
    for (size_t i = 0; i < size; i++) {
        if (zero_count == MIN_LENGTH) {
            // in compression mode
            while (i < size && 0 == input[i] && rle_length < MAX_LENGTH) {
                rle_length++;
                i++;
            }
            // Move back a position
            --i;
            //encode this length
            encoded.push_back(rle_length);
            //reset length
            rle_length = 0;
            zero_count = 0;
        } else {
            if (input[i] == 0) {
                zero_count++;
            } else {
                zero_count = 0;
            }
            encoded.push_back(input[i]);
        }
    }
    return encoded;
}

vector<uint8_t> RLE_decode(const vector<uint8_t>& input) {
    int zero_count = 0;
    vector<uint8_t> decoded;
    for (size_t i = 0; i < input.size(); i++) {
        if (zero_count == MIN_LENGTH) {
            //decompression mode
            //read one byte only
            uint8_t length = input[i];
            while (length--) {
                decoded.push_back(0);
            }
            zero_count = 0;
        } else {
            if (input[i] == 0) {
                zero_count++;
            } else {
                zero_count = 0;
            }
            decoded.push_back(input[i]);
        }
    }
    return decoded;
}
