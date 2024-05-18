202405101743
Status: #idea
Tags: #topic

# Meaning

1. meaning is contained both in message sent and in the interperter who uses that information to interpert it. 
	1. expand on that 
	2. what happens when the interperter is what contains the meaning and not the content
2. Compression is the art of distilling meaning within content
3. isomorphism is the equivalence of meaning
4. 

Life is built around signaling. Messages are sent all the time with some intent for others to understand. When you ask someone "What do you mean?" was the lack of understanding due to partial messaging? or due to lack of ability to interpret the message? In his seminal text Godel Escher Bach, Hofstadter states that meaning itself is a strange dance between content and interpretation. For meaning to be found, content, which by itself meaningless is interpreted. When standing on their own, content and interpretation are interchangeable. GEB illustrates this by imagining a world where DNA is similar to all organisms, and the decoding mechanism in the environment differs. For example a cat and a monkey would contain the same DNA, but the decoding mechanisms within which made them grow is what differentiates between them. The genetic information would then not be contained in the DNA but in the decoding mechanisms themselves. The fact that the decoding mechanisms for life are similar while DNA varies, makes DNA the content and decoding mechanisms the interpretation.

Compression is in-fact an art of distilling information. Standard model for a lossless compressor is an Encoder->Message->Decoder scheme where, the content is the message and the interpreter is the decoder. In such scheme the total size of 

In his book Godel Escher Bach, Hofstadter introduces the term `isomorphism`. The term, taken from mathematics, is used to describe the similarity of two different formal systems. Meaning found in one formal system can be used for another, allowing us to use what we know of one to describe and analyze another. 



Meaning dances between information and interpertation. 


The difficult part about understanding where the information lays in a given system is that it can be split across the message passed and the decoder. This means that some information will stay hidden not allowing us to understand some messages regardless of how much we try. 

Another example of this split would be a image compressor. A lossless compressor can have many ways in which it can encode an image to compress it's "meaning". One way would be to give them a smaller representation based on statistical frequencies. So a pixel whos color is more like to show up will take up less bits to represent. This scheme would allow any one who wants to decode the message simply decode the compressed message by replacing compressed form of pixels with their colors. Another scheme for compressing the message could be to do a simple counting of similar pixels and grouping of them together. This would in-fact make the algorithm on the decoding end, contain information it did not contain before. It now knows that pictures contain many similar colored pixels one after the other. This type of information we can not deduce from the message itself. It is unique to the encoder/decoder scheme which we propose here. This unique information is exactly why the information of a message might be contained in the message passed as well as the decoder which decodes it. 



---
# References
[^1]: Antifragile
[^2]: The book GEB - Godel, Escher, Bach 
