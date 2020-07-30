//
//  bwt.hpp
//  pzip
//
//  Created by Phuc Nguyen on 07/21/20.
//  Copyright © 2020 Phuc Nguyen. All rights reserved.
//

#include <vector>
#include <unordered_map>
#include <algorithm>

using namespace std;

const int BWT_SENTINEL = 256;

bool Compare(const vector<int>& lhs, const vector<int>& rhs) {
    if (lhs[1] < rhs[1]) return true;
    else if (lhs[1] > rhs[1]) return false;
    return lhs[2] < rhs[2];
}

vector<uint8_t> bwt2(const vector<uint8_t>& input, int& index) {
    vector<int> v_input = vector<int>(input.begin(), input.end());
    v_input.push_back(BWT_SENTINEL);
    vector<int> copy = vector<int>(v_input);

    int size = (int)v_input.size();

    //
    sort(copy.begin(), copy.end());
    copy.erase(unique(copy.begin(), copy.end()), copy.end());

    unordered_map<int, int> inp_map;
    for (size_t i = 0; i < copy.size(); i++) {
        inp_map[copy[i]] = (int)i;
    }

    //
    vector<int> inp_idxs;
    for (int ch: v_input) {
        inp_idxs.push_back(inp_map[ch]);
    }
    inp_idxs.push_back(-1);

    //
    vector<vector<int>> suffix;
    for (size_t i = 0; i < size; i++) {
        suffix.push_back(vector<int>{static_cast<int>(i), inp_idxs[i], inp_idxs[i+1]});
    }

    // Had some bugs with unstable sorting
    stable_sort(suffix.begin(), suffix.end(), &Compare);
    vector<int> idxs(size, 0);

    int k = 2;
    while (k < size) {
        int r = 0, prev_r = suffix[0][1];
        for (size_t i = 0; i < size; i++) {
            size_t safe_idx = i == 0 ? size - 1 : i - 1;
            if (suffix[i][1] != prev_r || suffix[safe_idx][2] != suffix[i][2]) {
                r++;
            }
            prev_r = suffix[i][1];
            suffix[i][1] = r;
            idxs[suffix[i][0]] = (int)i;
        }
        for (size_t i = 0; i < size; i++) {
            int next = suffix[i][0] + k;
            suffix[i][2] = next < size ? suffix[idxs[next]][1] : -1;
        }
        stable_sort(suffix.begin(), suffix.end(), &Compare);
        k<<=1;
    }

    vector<int> ref;
    for (auto e: suffix) {
        ref.push_back(e[0] - 1);
    }

    vector<uint8_t> res;
    for (size_t i = 0; i < ref.size(); i++) {
        int idx = ref[i];
        int ch = idx >= 0 ? v_input[idx] : v_input[size-1];
        if (ch == BWT_SENTINEL) {
            index = (int)i;
        } else {
            res.push_back((uint8_t)ch);
        }
    }

//    index = (int)distance(ref.begin(), find(ref.begin(), ref.end(), -1));

    return res;
}

// A good observation is that the index == the position of SENTINEL

vector<uint8_t> ibwt(const vector<uint8_t>& input, int index) {
    vector<int> copy_input(input.begin(), input.end());
    copy_input.insert(copy_input.begin() + index, BWT_SENTINEL);
    vector<pair<int, int>> idxs;
    for (size_t i = 0; i < copy_input.size(); i++) {
        idxs.push_back(make_pair(copy_input[i], i));
    }
    stable_sort(idxs.begin(), idxs.end());
    vector<int> sorted_idxs;
    for (size_t i = 0; i < idxs.size(); i++) {
        sorted_idxs.push_back(idxs[i].second);
    }

    vector<uint8_t> res;
    for (size_t i = 0; i < copy_input.size(); i++) {
        index = sorted_idxs[index];
        int ch = copy_input[index];
        if (ch != BWT_SENTINEL) {
            res.push_back(copy_input[index]);
        }
    }

    return res;
}
