202309102114
Status: #idea
Tags: #topic

# Information Theory

#### Source-Channel separation
The general model for the types of problems we want to solve in information theory is introduced:

Source -> `Encoder` -> (noisy)channel -> `Decoder` -> Destination

In this model we have limits to the amount of data we can transfer at a given time along with corruption that might happen due to noise.

In essence what we care about is Efficiency & reliability by compression & error-correction
1) Transferring as few bytes of information as possible 
2) Recovering original data even when it has noise from corruption

Assumptions: The model can only flow in one way. Meaning messages can not be resent.
FEC - Forward Error Correction

It seems like Shannon proved that you can decouple the source and the channel without loss of efficiency (TODO(Ziv): DO THIS). This means that the system in which you have a Source-Channel encoder or decoder in a combined form, will do just as fine as one that separates them. 

Source Encoder/Decoder handles the compression side
Channel Encoder/Decoder handles the error-correction 

Source -> `Source Encoder` -> `Channel Encoder` -> noisy channel -> `Channel Decoder` -> `Source Decoder` -> Destination 

What Shannon proved is the Source-Channel separation theorem.

#### Source-Channel separation theorem

#### Different notion of "information"
The notion of information commonly used by us is to describe the utility of something. For example an article that contains information about cats, will contain more information if it has more unique things to say about cats. 

The notion of information that we have information theory is that of randomness in data. If we take a picture of your screen right now and compare it to a picture containing noise from an old tv, we will see that your screen has more regularities of pixels, and a probabilistic model would be able to show "structure" inside it better than the random noise where there is no "structure". This effectively means that random noise data contains more information than more structured, uniform data. 


the exact definition seems to be inside the shannon 1948 paper. 

Source codes (also called variable-codes) aabbc
Code words 001010110 

def: C is uniquely decodable if $C^*$ is 1-1
(i.e if $x_1 \dots x_n \ne y_1 \dots y_m$ then $C^*(x_1 \dots x_n) \ne C^*(y_1 \dots y_m))$

To compress a file losslessly we must have a uniquely decodable codewords


[[Kraft-McMillan inequality]]


---
# References

