202405101743
Status: #idea
Tags: #topic

# Meaning

Life is built around signaling, this text is itself a signal with some meaning I try to convey, reading only half of this text will not allow you to comprehend it's full meaning, just like when encountering many words one doesn't know will result in confusion. So where does meaning lay?

In his seminal text Godel Escher Bach, Hofstadter states that meaning itself is a strange dance between content and interpretation. For meaning to be found, content, which by itself meaningless must be interpreted. When standing on their own, content and interpretation are interchangeable. GEB illustrates this by imagining a world where DNA is similar to all organisms and the decoding mechanism is what makes them unique. For example a cat and a fish would contain the same DNA, but the decoding mechanisms which made them grow is what differentiates between them. The genetic information would then not be contained in the DNA but in the decoding mechanisms themselves. The fact that the decoding mechanisms for life are similar while DNA varies is what makes DNA the content and decoding mechanisms the interpreter. 

Because of this strange dance between content and interpretation, when looking at the meaning something has you must take into account both what you want to.

, compression is an art of distilling information into it's essence therefore condensing meaning. 
Language itself is a compact description for the set of all valid sentences. You have letters which are meaningless, words which contain meaning and yet have limited meaning, rules for how to form sentences and you have sentences built out of both words and rules which are meaninful on their own. Infact, language has many interesting properties because of the huge set they create verses the small description required. The explosion of the set of meaningful valid sentences which you can get from a relatively small set of words and rules, is what makes language so powerful. 

Compression will forever stay an art because it is uncomputable. The problem can be restated as finding the smallest program that can reconstruct a given file. This problem, better known as the Kolomogorov complexity of a program, has been proven uncomputable. It would require running all possible programs that produce the file and picking the shortest one. In doing so we might encounter programs that don't halt, making our imagineary program never halt, thus never produce a result. This problem is called the Halting problem which is known to be undecideable or in other words uncomputable, making our first problem of finding the Kolomogorov complexity also uncomputable.




One more interesting thought on compression. If we know that taking some initial state and some rules of inference are infact the shortest program for a problem, and it is the compressed form of all the states possible by that program, we can by state that it is isomorphic to a formal system. Formal systems are known to have truths which are not inside the formal system. 

If we know that the most compressed form of anything is the shortest content and a interpreter which generates the wanted result, we can say that anything that is compressed that way is a formal system. 

On a philosophical note, Stephen Wolfarm published A new kind of science in which he directly states that his new science is looking at the universe as a formal system. In much the same way that a formal system has truths which it can not state, the universe does not seem to ever reach a state which can be true but will never happen. This might be like looking at a green sun. The chemicals which will enable the sun to produce green radiation are likely never to occur in large quantities in the sun to produce the result. But the rules which the universe seems to be based on, suggest that a green sun can happen in the universe it is a "truth" and yet we will never see it. This, along with my first line of thought, suggests to me that the reason why Stephen has found the concept of irreducability when studying cellular automuta is because he when trying to compress the results from his experiments, has found himself stuck at a seemingly impossible problem. The same same level of difficulty as finding the answer to the Halting Problem. 



Another concept introduced is isomorphism. The term refers to the equivalence of meaning in two formal systems given by their consistancy when compared. e.g. Imagine you want to cook a couple eggs for breakfast. You first go to your refrigerator, take 2 eggs and put them on the counter. Since 2 eggs are no enough you return to your refrigerator and take 3 more. You then count how many there are on the counter and you find 5 eggs on the counter. Now you have probably already known that taking 2 eggs and adding 3 more eggs would result in 5 eggs. How did you know that? The answer is that you knew that the formal expression 2+3=5 is equivalent in meaning to the counting of eggs. Since you have learned this from a young age you are capable of computing the result easily. This equivalence or should I say isomorphism is a powerful concept which allows us to take the meaning of one and apply it onto the other. The real world is the only thing that is meaninful, since we live in it we give some "inherit" meaning to things in the world. In our example, the counting of eggs being a real physical action has meaning, while some expression we made up has to "gain" meaning by isomorphism as we say that the action of counting 2 eggs and 3 eggs is equivalent to addition of 2 and 3.


Compression is in-fact an art of distilling information. Standard model for a lossless compressor is an Encoder->Message->Decoder scheme where, the content is the message and the interpreter is the decoder. In such scheme the total size of 

Another example of this split would be a image compressor. A lossless compressor can have many ways in which it can encode an image to compress it's "meaning". One way would be to give them a smaller representation based on statistical frequencies. So a pixel whos color is more like to show up will take up less bits to represent. This scheme would allow any one who wants to decode the message simply decode the compressed message by replacing compressed form of pixels with their colors. Another scheme for compressing the message could be to do a simple counting of similar pixels and grouping of them together. This would in-fact make the algorithm on the decoding end, contain information it did not contain before. It now knows that pictures contain many similar colored pixels one after the other. This type of information we can not deduce from the message itself. It is unique to the encoder/decoder scheme which we propose here. This unique information is exactly why the information of a message might be contained in the message passed as well as the decoder which decodes it. 

Compression has a strange yet beautiful connection to intelligence. With the advance of modern machine learning techniques, research seems to suggest that there is a correlation between how well an algorithm can compress information and how well it can make intelligent predictions. In a recent research paper, researchers demonstrated a new technique for word similarity test. They built a metric based on the compressed length of strings A and B along with their concatenation A + B. Doing so is roughly equivalent of doing a cross-entropy comparison of the two strings. This along side kNN clustering algorithm has shown very good results when compared to classical machine learning approaches, some of which are quite advanced, beating such algorithms in certain circumstances. 

If you spend some time thinking about it, compression is quite evidently with its link to intelligence is not as strange as it might seem at first. 

Abstraction is itself a method of compressing the meaning of the parts that make up something into more understandable, approximate meanings which hide the complexities under their general description and method of abstraction. In programming when you are using abstraction you are essentially compressing code you use into reusable parts, with names which describe their general function to allow for first, understandability within the system and second, reduction in complexity of having to know all of the underlying choices and instructions that make it up. This is especially noticeable when you think that when two parts are alike, they can be compressed into a single function which is called twice. This natural compressibility of code, is what makes building large code bases very complex, since complexity of code is an essential property of programming (Brooks-NoSilverBullet). The benefit to using abstractions is great which is why abstraction is used effectively everywhere in engineering. Chip design uses abstraction to describe the sub components used to create the whole computer. You begin with transistors, continue with gates, logical units and more complex systems. The underlying abstractions are clear. They provide us with a powerful way of using understandable components using minimal need of understanding everything that was taken to built, and yet having maximum effect of usefulness. When sketching parts for a robot, you have a set of abstractions built into the software. You first build the parts that will make your robot. Then, you build the sub-assemblies which when put together into the final assembly will create your final robot. This way of using abstractions makes the creator of the robot not have to constantly have to think about the individual parts, and instead shift focus into the bigger forming assemblies which make up the bigger assembly. At every level of abstraction, you lose some ability of configuration but gain both the re-usability and conceptual understanding of the parts which make up the final robot. 

Abstraction is useful for humans because we find it easier to deal with abstract concepts than with many simple details. Would you find it easier to think about how each an every ant in a ant colony interacts one with the other or just simple behaviour descriptions of the ant colony as a whole. Even if the abstraction lacks the accuracy of knowing all the details it is necessary as humans are not capable of dealing with them. 

When abstracting you are essentially finding the basic building blocks which make up your program and are compressing the instruction into these basic blocks. This process of abstracting is best done continues since, if done beforehand when you don't know which basic blocks you would really need, you will eventually end up having to refactor basic blocks until you find that the state of your code is at equilibrium. With this in mind, we can see how systems which need a tailor made solution with strict design can be made up. A progression of simple instructions, which eventually find their place inside basic blocks, in a continues process of refinement leads to larger systems that make up the program, and a comprehensible well managed code. If you instead follow the reverse approach of first coming up with the large systems, basic blocks, and general design of a system before you write even a single line of code, you are essentially defining the structure of your code in a way that a slight mistake which you will make can cause large structure redesigns and time taken to refactor or redesign the system. This is of course assuming that the system is new and there is no already good known design which you can use. The web is a typical example of a known problem, with known solution which everybody uses. In such cases it is infact better to use already existing solutions that people have spend much time in finding instead of inventing the wheel all over again using the methodological process of exploration and refinement.

Though this approach is, by some, admired as the epiphany of programming. Since you can test all cases for failure, make sure that the program does what you intended it to do, and have somewhat of an understanding of what happens in the code. There are those who believe that in some cases it might not be our saving grace, but instead a horrible mistake. Abstractions leak. Leaky abstractions are abstractions who's underlying decisions which made it up leak and end-up conflicting with the user of the abstraction. This is a direct result of the approximation the abstraction gives by leaving out the details and with them important decisions the user of the abstraction is unaware of. Attempts at thinking about and trying to avoid these leaks have been made. The proposal of meta-abstractions which are interfaces that allow the user to change the underlying details of the abstraction he uses to fit his needs have been made. An example shown is a virtual memory system made up of objects that manage memory as blocks. The policy of evicting memory pages is left for the user to decide using a meta-interface. You inherit the initial objects properties and then change the functions regarding the policy. Again this is a way which tries to bridge the gap between the details left out by the abstraction and the leak of the decisions made when the abstraction was made. The reason for why we care so much about leaky abstractions is because they are everywhere and because of the unreasonable effectiveness of code-reuse in programmers productivity. Reducing the cost of reusing already existing abstractions by bridging gaps in the abstraction is essential for good libraries and components common place in all software. With good ideas throughout the software industry history, there is a good reason to believe that understanding how to build better abstractions could help tremendously the building of the software systems of the future.

With the proliferation of software, software projects can now grow to immense sizes. With software being responsible for more critical systems than ever, and the cost of a mistake is high, it has become important for systems we design to be robust to failures. For that reason programmers have developed two different approaches for designing robust software. The most popular solution is to create provable or testable software where you can grantee the resilience of the software against a set of tests the programmer expects to be correct. This has the major benefit of changes to code that conform to the tests will make sure behaviour has not changed under these software changes. 

# Complexity

A systems complexity might be characterized in a number of ways. One would be 
the Kolmogorov complexity. The definition is the minimum program which required 
to represent a string. From this we can inquire that a more complex system is
one who's parts are all unique. For if they are not unique they can be 
compressed and represented more compactly. This is the main argument for the 
"essential complexity" of software as Brooks defined is No Silver Bullet.
While it is correct I feel like some aspect is forgotten. Some software while 
seemingly significantly smaller are more complex than their large counterparts.
Why might this be the case? I believe that it is because of how inter-wind the
parts that make up the system are.

Think of the economic market we live in. Each person is making seemingly simple
decisions regarding his daily life buys and sells. And yet the markets is so 
complex there is almost no hope for ever being able to accurately predict it's 
chaotic behaviour. The interactions themselves might be simple but the sheer 
amount of even simple decisions and tendencies make the economic of a country 
seem like it's own entity and require very high level theories of how it works.
It is just impossible for us to understand the economy on a high level without
approximating some of it's behaviour. This approximation is what abstractions 
are all about in software and in theories in general. 

There are popular approaches for thinking about complex systems. The holistic 
approach says that you must look at the whole in-order to understand how it 
works. The reductionistic approach says that you can break the system into 
parts until the system is understandable. These approaches try to approximate 
the general system by ignoring the details. This seems to be a natural human 
behaviour. We are not capable of reasoning about all of the details while 
looking at the bit picture, for this reason we just ignore them. Sometimes the 
details leak through our abstracted view of the system which break our 
understanding. 


Biological systems seem to contain endless amounts of complexity inside their 
inner workings and yet they are capable of changing and adapting better than 
any system we humans can design.

## Software complexity

Today software can be characterized by it's never ending complexity.
Software seems to know no bounds when it comes to features. Features are what 
sell software and they are to be included at any price. Some of this price is 
the endless growth of complexity of software. For some this complexity is 
manifested through bloat. Applications that take gigabytes of storage space to 
install, and hog available resources. While for some applications much of its 
size and resource utilization is high due to its nature, for most I would say, 
this is not the case. Complexity also harms the cost of maintenance. Adobe has 
made maintaining and adding features it's software a technical nightmare. Old 
bugs and crashes haunt Adobe Premiere to this day[^1] with no saving grace in 
sight. 






Since the word complexity can be used to mean many terms, I will define some terms to make it easier to distinguish the different kinds of complexity I will talk about. 

**System complexity** - The amount of inter connections and reliance's the systems parts have one on each other. The more inter connections and reliance's the more states the program has to handle correctly. This implies that a simple system is one with independent parts. If you understand how a single part works, you don't have to worry about it any more, your responsibility of knowing how it works is over[^1]. There is also another aspect to system complexity. It is the randomness of the system. If the system is random and large, the amount of unique information alone is enough to categorize something as more complex than the other.

**Easy/Hard** - How much energy it would take to do something. If something takes more energy to do, it is harder to do. The reverse is true.

**Readability** - How much of the system complexity is forwarded to the programmer to comprehend and manage at the average case. If you only have to handle the large parts of the system that don't contain much of the system's complexity, as they were hidden/not presented you wouldn't need to care about that complexity until you would need to change a part of the system. 


Now that we have defined some terms we can begin thinking about the problem. The problem of complexity is one of dependence and states.


Programmers handle the problem of system complexity by refactoring/rewriting code. The problem of readability is handled using abstraction. The problem of a hard/easy program is handled by correct understanding of a problem. 


NOTE(Ziv): Great example to showcase complexity is the braid problem of bread. When you don't braid the doe it looks simple. When the lines interwind because you braid them, you end up with a more complex looking bread.[^2]
Another term which is useful to think about when talking about complexity is "tangle". Since when two things are tangled you have to think not only about them as a sperate entity but also how each and every one of them responds, and their how their interactions make way for a good/bad emergent behavior.


How do we handle the complexity of the programs that we create? 
    - Refactoring
    - Rewriting
    - Abstraction 

Refactoring is essentially taking the code and modifying it to solve the problem we face better. 
Rewriting the code is completely abandoning the effort made since we recognize that refactoring the existing code will take more effort than rewriting it with the new insights we gained. 
Abstraction is a tool for handling complexity where we hide some of the details of how something works, to not constantly have to worry about these details for when we want to use a basic block of our program. 

Each and every of these ways we handle complexity is of-course used in different times and for different reasons or usecases. 



The aspect oriented programming is a interesting paradigm that I think is worth to search further about.  
https://www.cs.ubc.ca/~gregor/papers/kiczales-ECOOP1997-AOP.pdf

Kruger's Reusable components and Software robustness have in them a similar section in which they advocate for the automatic search of different components that confirm to constraints.


# Code-Reuse

Seemingly good practices like code-reuse are over-used irresponsibly, with people including huge libraries for the use of a single function. Sometimes it is in search of a high quality library that implements something you are not an expert on.

It can be the community around the library along with the increase transfer-ability of skills across teams. Maybe you just don't want to maintain that part. These are the reasons one might turn to reusing code instead of coding it from scratch.

On the other hand, using other people's code means conforming to the constraints that the people who created the library had. Bigger libraries impose more constraints, and more libraries impose more constraints. If the constraints don't meet yours, you might end-up spending more time on adjusting your/other's code to make it work than if you had wrote it from scratch. And if you have invested too much into a bad library... too bad.

> it's fine if a library isn't completely perfect, none is, it's a time saver. you sacrifice a little bit of personal peace for the sake of getting some shit done or not needing to have the mental overhead of dealing with another person's crap. especially not a person or people who you can't directly contact...the maintainers are simply not my friends, they're not my colleagues, if i need LLVM fixed i cry, if i need any huge library fixed i need to go learn their BS along with the original problem space. if i need a feature added, i also cry. 
> — NeGate, on [handmade-network fishbowl](https://handmade.network/fishbowl/libraries/#chatlog__message-container-1003003441322397797)

Worst are the changing constraints. Usually this happens with package managers when updating dependencies. In general, package managers allow for quick download-and-use type of behavior with regard to dependencies, and easily updating dependencies. We can notice problems when a library author has decided to change the behavior or api of his library. The update will cause many programs dependent on that old version's behavior or api to break. For this reason, explicit upgrades to stable versions of software is preferred, rather than the more convenient automatic option.

We can see this in the [https://handmade.network/](https://handmade.network/)">handmade-community where single-header libraries are popular. e.g. [https://github.com/nothings/stb](https://github.com/nothings/stb)`stb_image` and `stb_freetype`. They require explicit upgrades, the ability to modify source code, and they usually do not have dependencies themselves. Along with the fact that single-header libraries are usually smaller, they dodge to some degree the more destructive nature of large managed dependencies while still bringing great value.

Sometimes you just need more than what people fit into a single-header library or you don't want to change the source code and instead just extend what is already there. This will touch on library design a bit. But it is important to know because if a library is [well designed](https://www.youtube.com/watch?v=5l2wMgm7ZOk/), then it will meet a much wider range of people's needs and won't become a large time-sink down the line if maintenance is needed in the code-base.



The important take a way here is when you buy into a big library, you sell some personal responsibility away which can be good... until things go wrong




---
# References
[^1]: Antifragile
[^2]: The book GEB - Godel, Escher, Bach 
