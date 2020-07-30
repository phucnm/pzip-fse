/* output_stream.hpp
   CSC 485B/CSC 578B/SENG 480B - Summer 2020

   Definition of a bitstream class which complies with the bit ordering
   required by the gzip format.

   (The member function definitions are all inline in this header file 
    for convenience, even though the use of such long inlined functions
    might be frowned upon under some style manuals)

   B. Bird - 05/13/2020
*/ 

#ifndef OUTPUT_STREAM_HPP
#define OUTPUT_STREAM_HPP

#include <iostream>
#include <cstdint>

/* These definitions are more reliable for fixed width types than using "int" and assuming its width */
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;



class OutputBitStream{
public:
    /* Constructor */
    OutputBitStream( std::ostream& output_stream ): bitvec{0}, numbits{0}, outfile{output_stream} {

    }

    /* Destructor (output any leftover bits) */
    virtual ~OutputBitStream(){
        if (numbits > 0)
            output_byte();
    }

    /* Push an entire byte into the stream, with the least significant bit pushed first */
    void push_byte(unsigned char b){
        push_bits(b,8);
    }


    /* These definitions allow for a notational convenience when pushing multiple bytes at a time
       You can write things like stream.push_bytes(0x01, 0x02, 0x03) for any number of bytes, 
       which will be pushed one at a time using the push_byte function above.*/
    void push_bytes(){
        //Base case
    }
    template<typename T, typename ...Ts>
    void push_bytes(T v1, Ts... rest){
        push_byte(v1);
        push_bytes(rest...);
    }

    /* Push a 32 bit unsigned integer value (LSB first) */
    void push_u32(u32 i){
        push_bits(i,32);
    }
    /* Push a 16 bit unsigned short value (LSB first) */
    void push_u16(u16 i){
        push_bits(i,16);
    }

    /* Push the lowest order num_bits bits from b into the stream
       with the least significant bit pushed first
    */
    void push_bits(unsigned int b, unsigned int num_bits){
        for (unsigned int i = 0; i < num_bits; i++)
            push_bit((b>>i)&1);
    }

    /* Push a single bit b (stored as the LSB of an unsigned int)
       into the stream */ 
    void push_bit(unsigned int b){
        bitvec |= (b&1)<<numbits;
        numbits++;
        if (numbits == 8)
            output_byte();
    }

    /* Flush the currently stored bits to the output stream */
    void flush_to_byte(){
        if (numbits > 0)
            output_byte();
    }


private:
    void output_byte(){
        outfile.put((unsigned char)bitvec);
        bitvec = 0;
        numbits = 0;
    }
    u32 bitvec;
    u32 numbits;
    std::ostream& outfile;
};


#endif 