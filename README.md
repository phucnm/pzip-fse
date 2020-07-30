# pzip-fse
An implementation of bzip compression scheme with Finite State Entropy coding.

## Usage
This program actually contains separate compressor and decompressor. The two executables work with standard stream (std) instead of file similar to gzip or bzip2. To compress, use `pcompress`:

`./pcompress < inputfile > outputfile`

To decompress, use `pdecompress`:

`./pdecompress < inputfile > outputfile`

## Compression ratios

The table below compares compression ratios on two common datasets for data compression Calgary and Canterbury. 
It's worth noting that pzip is a simple and unoptimized program but has achieved a comparable peformance to a commercial standard like gzip.

| File	                        | gzip	    | pzip      |
| ------------------------------|-----------|-----------|
| CalgaryCorpus/bib	            | 34,896    | 29,961    |
| CalgaryCorpus/book1	          | 312,27    | 5256,017  |
| CalgaryCorpus/book2	          | 206,15    | 2173,002  |
| CalgaryCorpus/geo	            | 68,410    | 62,857    |
| CalgaryCorpus/news	          | 144,39    | 5126,670  |
| CalgaryCorpus/obj1	          | 10,315    | 11,159    |
| CalgaryCorpus/obj2	          | 81,082    | 80,373    |
| CalgaryCorpus/paper1	        | 18,536    | 17,934    |
| CalgaryCorpus/paper2	        | 29,660    | 27,334    |
| CalgaryCorpus/paper3	        | 18,067    | 17,064    |
| CalgaryCorpus/paper4	        | 5,527	    | 5,531     |
| CalgaryCorpus/paper5	        | 4,988	    | 5,135     |
| CalgaryCorpus/paper6	        | 13,206    | 13,393    |
| CalgaryCorpus/pic	            | 52,377    | 57,115    |
| CalgaryCorpus/progc	          | 13,255    | 13,489    |
| CalgaryCorpus/progl	          | 16,158    | 17,239    |
| CalgaryCorpus/progp	          | 11,180    | 11,841    |
| CalgaryCorpus/trans	          | 18,856    | 19,530    |
| CanterburyCorpus/alice29.txt  | 54,179	  | 47,333    |
| CanterburyCorpus/asyoulik.txt	| 48,816    | 43,331    |
| CanterburyCorpus/cp.html	    | 7,973	    | 8,394     |
| CanterburyCorpus/fields.c	    | 3,127	    | 3,502     |
| CanterburyCorpus/grammar.lsp	| 1,234	    | 1,473     |
| CanterburyCorpus/kennedy.xls	| 209,721   | 192,084   |
| CanterburyCorpus/lcet10.txt	  | 144,418	  | 119,094   |
| CanterburyCorpus/plrabn12.txt	| 194,26    | 4159,844  |
| CanterburyCorpus/ptt5	        | 52,377    | 57,115    |
| CanterburyCorpus/sum	        | 12,768    | 13,774    |
| CanterburyCorpus/xargs.1	    | 1,748	    | 1,948     |

## Documentation

#### 1. Compression scheme

The proposed scheme consists of 4 stages:
- BWT
- Move-to-front (MTF)
- Run length encoding (RLE)
- Finite State Entropy (FSE) 

The program runs with 1 default block size parameter 900K bytes. The first two steps do not change input size whereas the two last steps actually compress the input. The scheme here is similar to bzip2 except the last step where I experiment FSE which is a new promising entropy coder. I only had time to implement a naive version of each technique/algorithm without many optimizations so I could not beat bzip2 and lzma in term of compression ratio. However according to comparative results on connex, this basic scheme easily beats gzip in many test cases. This shows the power of FSE.

#### 2. Program implementation
2.1. BWT
- Raw input will be transformed by BWT. I implemented a linear version of BWT using suffix array, the time complexity is O(n.logn.logn). However the inverse process only runs in O(n) time. In order to correctly work on all test cases, I added a SENTINEL symbol whose value is 256 at the end of the input before feeding into BWT. An observation is that the index of the original input in the sorted matrix is the index of SENTINEL symbol. H
- Output of the BWT stage is an array of bytes of the same length, and an index indicates where's the original input for inverse BWT. I remove SENTINEL symbol from the output bytes because I can easily put it back at `index`. A hypothesis is it always has a frequency of 1, and might slightly affect RLE and Entropy Encoding step.
- Many online articles helped a lot with understanding the idea of using suffix array.
- https://www.labri.fr/perso/ruricaru/bioinfo_master2/cours3.pdf
- http://www.csbio.unc.edu/mcmillan/Comp555S18/Lecture13.pdf

2.2. MTF
- This step basically takes advantage from BWT where identical characters tend to stay together. The idea of MTF is to assign a symbol to its position in the current alphabet stack, then pop the symbol and push it to front of the alphabet stack. I used singly linked list to store the alphabet stack, so that at each symbol, I only need 1 for loop to find the symbol, remove the symbol and push it to the front at the same time. Deleting an element and pushing it to front of a linked list only cost O(1).
- This step remains the same block length.

2.3. RLE
- I implemented a simple version of RLE where the minimum length must be 4 and the maximum is 255 to fit in 1 byte. After seeing 4 consecutive identical characters, the next symbol will be the run length rainging from 0-255, hence fit in 1 byte to store the length. Therefore the worst case is there are only 4 consecutive identical characters per run, e.g. "baaaabaaaa". This would expand the input by 1.25x.
- However the decoding is easy, whenever I see 4 identical characters, I know the only next byte is the run length.

2.4. FSE
- This is an experiment I tried to implement FSE which can encode a message with average code length close to Shannon Entropy value at a speed relatively comparable with Huffman coding. The implement is so challenging and I had a hard time understand papers and articles about it. I tended to switch back to implement a PPM variant but furtunately on the last day I partly figured out the conception and was able to implement a FSE coder worked correctly on most of the test cases. FSE (or tANS - tabled Asymetric Numeral System) is a variant of Asymetric Numeral System which is an idea proposed by Jarek Duda to combine the precision of Arithmetic Coding (AC) and the speed of Huffman coding. FSE is a state machine which stores all the infomation needed to encode and decode a message. According to the author, Huffman coding is a specical degenerated state of FSE where the number of bits needed to encode each symbol is an integer. The basic idea of ANS is similar to AC, but it generates larger range after each step whereas AC makes the range more and more narrower. There's also range renormalization in ANS and in FSE the renormalization is computed during creating the encoding table (so it's fast with tradeoff to store the encoding table in memory and transmit the table to the decoder). It's worth noting that FSE decoding works backward i.e. it decodes the last symbol to the first symbol. 
- There are not a lot papers and articles about it on the internet, but all of them are useful. I started with Duda's updated paper to partly understand what is ANS. One of the most famous implementation of FSE (https://github.com/Cyan4973/FiniteStateEntropy) is by Yann Collet (creator of zstd of Facebook). But that code was highly optimized and not easy to follow. 
- There's also a cool blog by Charles Bloom (http://cbloomrants.blogspot.com/2014/02/02-18-14-understanding-ans-conclusion.html) where he explains and discusses lots of compression concepts in an easier way.
- Another down to earth explanation on how the encoder and decoder work step by step: http://www.ezcodesample.com/abs/abs_article.html
- Blog series of Yann Collet is also helpful: http://fastcompression.blogspot.com/2013/12/finite-state-entropy-new-breed-of.html
- Papers: https://arxiv.org/abs/1311.2540, http://www2.ift.ulaval.ca/~dadub100/files/ISIT19.pdf.
- 
#### 3. Bitstream format
- Due to limited time left after FSE implementation, I only come up with a simple bistream format. The bitstream contains blocks.
- The block format is as follows
    - last_block (1 byte): indicates if this is the last block
    - compression_mode (1 byte): indicates if the block is compressed by FSE or just a RLE stream (in case FSE fails).
    - compressed block (variable bytes)
- Here I define the format of each compressed block. Firstly, the RLE mode:
    - encoded length N (4 bytes): the length of encoded block by RLE
    - the index returned by BWT step (4 bytes)
    - byte stream (N bytes): the encoded block by RLE
- The FSE mode format is as follows
   - the index returned by BWT step (4 bytes)
   - number of symbols of the RLE encoded stream N (2 bytes)
   - RLE encoded length (4 bytes)
   - frequency of the symbols (N * 2 bytes)
   - FSE byte offset (1 byte): indicates where the last bit of the FSE encoded stream is in the last byte.
   - FSE state (4 byte): the state of the FSE encoding step
   - FSE encoded stream length FSE_N (4 bytes)
   - byte stream of FSE encoded block (FSE_N bytes)
