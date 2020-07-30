//
//  fse.hpp
//  pzip
//
//  Created by Phuc Nguyen on 07/21/20.
//  Copyright © 2020 Phuc Nguyen. All rights reserved.
//


#include <vector>
#include <numeric>

using namespace std;

class FSE {
private:
    const uint8_t PRECISION = 4;

    int bitlen(unsigned int n) {
        int len = 0;
        while (n) {
            len++;
            n >>= 1;
        }
        return len;
    }

    // Count the frequencies and normalize them
    vector<int> NormalizeCount(const vector<uint8_t>& data, int num_symbols, int PROBABILITY_PRECISION) {
        int data_size = (int)data.size();
        double scale_factor = (double)(1<<PROBABILITY_PRECISION) / data_size;

        vector<int> freqs(num_symbols);
        for (int i = 0; i < data_size; i++)
            freqs[data[i]]++;

        for (int i = 0; i < num_symbols; i++) {
            freqs[i] = (int)(freqs[i] * scale_factor);
        }

        int sum = accumulate(freqs.begin(), freqs.end(), 0);
        int diff = (1<<PROBABILITY_PRECISION) - sum;

        int argmax = (int)distance(freqs.begin(), max_element(freqs.begin(), freqs.end()));
        freqs[argmax] += diff;

        // A symbol must have freq at least 1
        for (int i = 0; i < num_symbols; i++) {
            if (freqs[i] == 0) {
                freqs[i] = 1;
                freqs[argmax]--;
            }
        }

        return freqs;
    }

    vector<vector<int>> CreateEncodingTable(const vector<int>& freqs, int num_symbols, int PROBABILITY_PRECISION, int MAX_STATE) {
        //The encoding table is a matrix of size for efficient jumping between states
        //e.g enc_table[0] is a vector of states assigned to symbol 0
        vector<vector<int>> enc_table;
        for (int i = 0; i < num_symbols; i++) {
            vector<int> v{0};
            enc_table.push_back(v);
        }

        //Sum of scaled up freqs, no symbol can reach this value
        int sum_freqs = 1<<PROBABILITY_PRECISION;
        vector<int> table = vector<int>(MAX_STATE+1, sum_freqs);

        int state;
        // The result of this loop is a list of consecutive states,
        // the value of each state is a symbol

        // also spread symbols at same time
        for (int j = 1; j <= MAX_STATE; j++) {
            for (int i = 0; i < num_symbols; i++) {
                //making the input state to output state ratio (x'/x) as close to (1/P) as possible.
                //http://cbloomrants.blogspot.com/2014/02/02-06-14-understanding-ans-8.html
                state = max((j << PROBABILITY_PRECISION)/freqs[i], 2);
                while (state <= MAX_STATE) {
                    if (table[state] == sum_freqs) {
                        table[state] = i;
                        enc_table[i].push_back(state);
                        break;
                    }
                    state++;
                }
            }
        }
        return enc_table;
    }

    void CreateDecodingTable(const vector<vector<int>>& encoding_table, vector<int>& rev_sym_table, vector<int>& rev_sym_state, int num_symbols) {
        for (int i = 0; i < num_symbols; i++) {
            int j = 1;
            int t_size = (int)encoding_table[i].size();
            for (int k = 0; k < t_size - 1; k++) {
                rev_sym_table[encoding_table[i][j]] = i;
                rev_sym_state[encoding_table[i][j]] = k+1;
                j++;
            }
        }
    }

public:
    FSE() {

    }

    bool Compress(const vector<uint8_t>& data, int num_symbols, vector<u8>& encoded_stream,
                    vector<int>& freqs, int& byte_offset, int& final_state) {
        int data_size = (int)data.size();
        int PROBABILITY_PRECISION = bitlen(num_symbols) + PRECISION;
        int STATE_PRECISION = PROBABILITY_PRECISION + 1;
        int MAX_STATE = (1<<STATE_PRECISION)-1;

        // Collect prob distribution and scale up the freqs
        freqs = NormalizeCount(data, num_symbols, PROBABILITY_PRECISION);

        // Create encoding table
        vector<vector<int>> encoding_table = CreateEncodingTable(freqs, num_symbols, PROBABILITY_PRECISION, MAX_STATE);

        bool success = true;

        // Start encoding
        int state = MAX_STATE;
        unsigned char byte = 0;
        unsigned char bit  = 0;
        int control_MASK = 1 << (STATE_PRECISION-1);
        for (int i=0; i<data_size; ++i) {
            int t_size = (int)encoding_table[data[i]].size();
            // shift and output bits while finding the range
            while (state > t_size - 1) {
                ++bit;
                byte |= state & 1;
                if (bit == 8) {
                    encoded_stream.push_back(byte);
                    bit = 0;
                    byte = 0;
                }
                else {
                    byte <<=1;
                }
                state >>= 1;
            }
            // Move to next state
            state = encoding_table[data[i]][state];
            if (state < control_MASK) {
                cerr<<"FSE output error"<<endl;
                success = false;
                break;
            }
        }

        // Remark where we stopped
        if (bit == 0) {
            byte_offset = 0;
        }
        else {
            // Flush out this last byte
            byte_offset = 8 - bit;
            encoded_stream.push_back(byte << (7 - bit));
        }

        // State of the whole message
        final_state = state;

        return success;
    }

    void Decompress(const vector<u8>& encoded_stream, const vector<int>& freqs, vector<u8>& decoded_stream,
                    int byte_offset, int state, int num_symbols) {

        int PROBABILITY_PRECISION = bitlen(num_symbols) + PRECISION;
        int STATE_PRECISION = PROBABILITY_PRECISION + 1;
        int MAX_STATE = (1<<STATE_PRECISION)-1;

        vector<vector<int>> encoding_table = CreateEncodingTable(freqs, num_symbols, PROBABILITY_PRECISION, MAX_STATE);

        // Creating a decoding table
        vector<int> rev_symbol_table(MAX_STATE+1);
        vector<int> rev_symbol_state(MAX_STATE+1);
        CreateDecodingTable(encoding_table, rev_symbol_table, rev_symbol_state, num_symbols);

        // Start decoding
        int MASK = 1<<(STATE_PRECISION-1);
        unsigned char shift = byte_offset;
        // Counter is the index of current processing encoded byte,
        // it goes backward from the end to the beginning.
        int counter = (int)encoded_stream.size() - 1;
        for (int i = (int)decoded_stream.size() - 1; i >= 0; --i) {
            decoded_stream[i] = rev_symbol_table[state];
            state = rev_symbol_state[state];
            // Renorm to the range
            while (state < MASK) {
                state <<= 1;
                // Keep shifting and consuming bits until we are in range
                state |= (encoded_stream[counter] & (1<<shift))>>shift;
                ++shift;
                if (shift == 8) {
                    shift = 0;
                    --counter;
                }
            }
        }
    }
};
