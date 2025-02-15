# A trip learning compression through https://akreson.github.io/en/posts/entropy-encoding-part1/

import numpy as np 
from collections import defaultdict

# Kraft-McMillan, Entropy, Huffman-encoding 
def part1():
    symbols = ['a', 'b', 'c', 'd']
    lengths = np.array([1, 2, 3, 3]) # li = -log2(pi)
    probs = 1/(2**lengths)           # pi = 1/(2^li)

    # print table 
    print('symb\tprob\tcode')
    for symbi, li, pi in zip(symbols, lengths, probs):
        print(f'{symbi}\t{li}\t{pi}')

    # Kraft-McMillan
    if probs.sum() <= 1: 
        print('We really can unambiguously decode a sequence of such codes')

    # Entropy 
    H = -(probs * np.log2(probs)).sum() 
    print(f'entropy: {H}')

# Implementing Arithmetic Coding
def part2_1(): 

    # NOTE(ziv): Arithmetic coding is an entropy encoding technique, in which 
    # the frequently seen symbols are encoded with fewer bits than rarely seen 
    # symbols

    message = np.array([c for c in 'AACBBCAAACCCAAABBBBBBBBBBBBBBBBBBBABAACBBBBBBBB'])
    unique, counts = np.unique(message, return_counts=True)
    probs = counts/counts.sum() # frequency
    
    # building of model 
    model = defaultdict(lambda: 0.0) # default is 0

    pi_sum = 0
    for ui, pi in zip(unique, probs):
        # 'c' : cdf(low, high)
        model[ui] = pi_sum, pi_sum+pi
        pi_sum += pi

    
    def encode(message):
        low, high = 0.0, 1.0
        for c in message: 
            cdf = model[c]
            r = high-low # range
            high = low + r*(cdf[1])
            low  = low + r*(cdf[0])
        return (high+low)/2 

    def decode(encoded):
        decoded = ''
        low, high = 0, 1
        for i in range(len(message)):
            # decode symbol 
            r = high - low
            for symb, cdf in model.items():
                upper = low+r*cdf[1]
                lower = low+r*cdf[0]
                if (lower <= encoded and encoded <= upper): 
                    low, high = lower, upper
                    decoded += symb
                    break
        return decoded

    encoded = encode(message)
    decoded = decode(encoded)
    print(f'{len(message)=} {encoded=}')

    # verify no mistakes in compression
    for m, d in zip(message, decoded): 
        assert(m == d)

def part2_2(): 
    # The building of 'real' Arithmetic Coding not bounded by 
    # precision problems of using 32/64bit floats

    message = np.array([c for c in 'AACBBCAAACCCAAABBBBBBBBBBBBBBBBBBBABAACBBBBBBBB'])
    unique, counts = np.unique(message, return_counts=True)
    probs = counts/counts.sum() # frequency
    
    # building of model 
    model = defaultdict(lambda: (0,0,10**9)) # default is 0

    pi_sum = 0
    for ui, pi in zip(unique, probs):
        # 'c' : cdf(low, high)
        denominator = 10**9
        integer_pi = int(pi * denominator)
        model[ui] = pi_sum, pi_sum+integer_pi, denominator
        pi_sum += integer_pi


    def encode(message):

        buffer = 0
        pending_bits = 0
        
        low, high = 0, 0xffffffff
        for c in message: 
            cdf_lower, cdf_upper, denom = model[c]
            r = high - low +1
            high = low + int(r*cdf_upper/denom)
            low  = low + int(r*cdf_lower/denom)

            if (high < 0x80000000):
              buffer <<= 1
              while (pending_bits > 0): 
                  pending_bits -= 1
                  buffer <<= 1

              low <<= 1
              high <<= 1
              high |= 1
              high &= 0xffffffff; low &= 0xffffffff # snapping back to 32 bit
            elif (low >= 0x80000000):
              buffer <<= 1
              buffer |= 1
              while (pending_bits > 0): 
                  pending_bits -= 1
                  buffer <<= 1
                  buffer |= 1

              low <<= 1
              high <<= 1
              high |= 1
              high &= 0xffffffff; low &= 0xffffffff # snapping back to 32 bit
            elif (low >= 0x40000000 and high < 0xC0000000):
              pending_bits += 1
              low <<= 1
              low &= 0x7FFFFFFF
              high <<= 1
              high |= 0x80000001
              high &= 0xffffffff; low &= 0xffffffff # snapping back to 32 bit
            else:
              break

        return int((low + high)/2)

    def decode(encoded):
        pass

    encoded = encode(message)
    print(encoded)
    decoded = decode(encoded)

# part1()
# part2_1()
part2_2()
