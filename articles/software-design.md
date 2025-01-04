
Article on system design. This should include the following: 

Essential properties of designing systems which will not change. 
Handling Complexity of systems 
Handling Invisibility of code 
Handling Cost of building and using systems
Handling Changeability and evolution of systems 
Handling Conformity to interfaces, features and more 
Handling Designers bad culture and trade-offs (during bad times)


NOTE(ziv): Rethink about the history part. I quite enjoy writing about it and jet I have not found that it is useful for much yet. It does not serve the articles but hinders it. Both in my time and in effort writing it first. I believe it deserves a separate article which will be more fun to write. That said, there is an important component of economics and history which I will have to explore just not as thoroughly as I initially wanted.
- History
    As a species we got here today in a marvelous way which we should remember is a miracle. Held together by the social structure, and effected by the local, global states of the industrial revolution. Like all biological creatures we know of humans came from the great biological soup of molecules inside our home called earth. While all other creatures had better defenses like shields to protect them or offensive ways of attacking using fangs or venom, humans came on top using a more lethal weapon under the name "Intelligence". We with our squishy minds have came and changed the world. We can move faster using cars, move more weight using machines, and form a hive mind of telecommunication network of sharing of knowledge and experience. The squishy things advancement knows no bounds. We live in a global hive mind where values of people change, times change, and as the saying goes "Hard times create strong men, strong men create good times, good times create weak men, and weak men create hard times". This saying I believe is a major creator of bad design, bad rules which people follow and low quality goals which they try to achieve. In much the same way Goodharth's law which is crucial in understanding how human over-optimization causes bad things, and how randomness helps mitigate it's bad effects. 
- Meaning  (This is something that I need to think about quite a bit more)
    The essence of everything that we build is it's meaning. If we know where meanings comes from we can look for it. We can see it more clearly and allow us to express our design with more intention and vividness than we would have been able to. This is a precursor to complexity and compression both which rely heavily on the meaning the systems carry.
- Complexity
    Complexity is a measure for how intertwined is a system. Complexity is bad for a system when it does not capture it's essence, yet it is good when it helps drive intent of a system forward by capturing the whole space of possibilities we might want. Now what do I mean by that? 
- Compression
    Compression is the art of distilling information and therefore meaning. Compression as we explore it can be seen as a useful mechanism for determining complexity, though you have to be careful in using it on superficial things like code length, which seemingly are correlated to complexity yet outliers exist, think of Conway's game of life where immense complexity is born out of a very simple program. There is a phase change where some things beyond some complexity make immense turns and the states they posses explodes. This type of behavior is normal and should not be ignored and instead used for our benefit. This is a powerful mode of thought which I will explore (think about language and how it uses simple meaningful words and rules, to capture a seemingly unlimited space of meanings we want to convey). The type of explosion of state is something I will have to name and capture it's intent. And maybe expand a bit later on. 
- Abstraction 
    This topic is one which should serve as a basis for most of my design principles since it is tackling one of the biggest problems engineers face worldwide. And I should spend much time thinking and writing about it as possible since I have much to say but I don't currently know how to say it.
- Component-Reuse (Across many disciplines)
    This is the continuation of abstraction and should be treated as such
- Invisibility (Accidents, Unexpected Behavior)
    We must remember that the things we create have real world effects, and safety of systems must be considered when designing them
- Evolution (Changeability, OpenSource)
    Any system we might build will eventually need to adapt to the new circumstances the world puts forth we must tackle this problem (as it should help understand how systems must be build to accommodate change)
- Conformity (interfaces, features)
- Education (Knowledge Fundamentals, Mentorship)
- Institutional Decline (Effects on us)



For years now it seems humans have been the dominant species on this planet. Organized together in weird structures, held by stories and powered by intelligence, humans seem to not only survive great calamities but also advance their knowledge and capabilities through and through. For as much as I would love to jump into the meat of this article I believe, to appreciate it fully, it is important to remember the challenges we as species had to overcome to get to where we are.

# History of our kind

The first homo-sapiens were hunter-gatherers, roaming large areas around without attachment, gathering mostly scarce plants that managed to survive the cold climate. These times were marked by their climate instability and hostile environment for food growth and development. It likely human potential for progress was held back to a degree by such environment. Around 13,000 years ago the world entered the Younger Dryas period, a millennium-long cold snap that temporarily reversed the warming process experienced since the Last Glacial Maximum[^1]. Around 12,000 years ago the Younger Dryas ended and the Holocene, the current era, began. Climate stabilized forming the climate patterns we know today. The effects on plants has been dramatic. Randall Carlson in his essay "Redemption Of The Beast" reviewed carbon-dioxide and its effects on the biosphere. In his essay he looks extensively at the effects carbon-dioxide can have on plants since plants are fundamental to all living organisms. He kindly points out that, with the effects of a hotter planet, more stable climate, and larger carbon-dioxide reserves, in the last 11,000 years plants experienced a period of growth and health. Recent data from NASA's satellite imagery shows the world has become greener[^6] from carbon-dioxide fertilization. These macro changes beginning 11,000 years ago have most likely prompted the world wide domestication of different crop types in different parts of the world, with grains being notable for humans development[^7]. 

> If we consider why the human population has never been as great as the present we are lead directly to the realization that for most of the time that modern homo sapiens sapiens has occupied this planet, the Earth has been in the grip of the brutal cold of an ice age. We know from recent history that times of global cooling have been detrimental to human population growth and times of global warming have been conducive to population expansion and social advancement. [see: Redemption Of The Beast, Randall Carlson pp 80]

As time went on nomadic groups moved into agricultural settlements. This shift marked the agricultural revolution of the time. Humans began spending a large portion of their time and effort onto cultivating a small food variety (wheat, corn, rice, potatoes, millet). Although we know that humans shifted from gathering their food to growing it, the choice at least for those times seems illogical. Cultivating wheat is no easy matter. Ancient farmers had to work the land removing rocks and pebbles, watching out for any worm or bug that might infect the plant, or hungry animals waiting to eat their precious food. Watering is important along with fertilizer for land constantly sucked out of minerals. So much time was spent cultivating wheat, settlements formed around farmland, anchoring humans in place. Yuval Harari in his popular book Sapiens shows us the perspective of the wheat at the time. For wheat a fragile plant had become a growing empire taking over the world by frenzy with fields and fields made out entirely out of wheat. Utilizing dumb humans to expand beyond what it would ever be able on it's own. How were we fooled? What did it offer in return? By growing their food, humans had a larger surplus of the food they were so keen on eating; for humans are greedy in demand, growing their population and narrowing their buffers, humans had to keep growing more, as there was never enough food to feed all the new faces and less food meant certain death. Wheat allowed population to increase from a hundred to hundreds and even thousands using this moving wheel, yet keeping the population dirt poor for long [see: Yuval Harari, Sapiens pp. 87-89 (Hebrew version)]. 

> Human history as a whole has been characterized by a seemingly inexorable trend toward higher levels of complexity, specialization, and sociopolitical control, processing of greater quantities of energy and information, formation of ever larger settlements, and development of more complex and capable technologies. [see: Joseph Trainter, The Collapse Of Complex Societies pp. 3]

With the population increasing at a steady pace, societies steadily got more complex. Complexity as I will now define, has a similarity to general complexity, simply flavored in a social context. Much like what Trainter did I will define complexity using two key terms, inequality and heterogeneity. Inequality refers to the differentiation of ranking, or unequal access to resources. Heterogeneity is the number of distinct parts to a society. Simpler societies had steadily decreased inequality as they increased in heterogeneity. 

Complex societies of those times were still relatively simple in modern terms. Leadership in the simplest societies tended to be minimal. It is personal and charismatic, and exists only for special purposes. Hierarchical control is not institutionalized, but is limited to definite spheres of activity at specific times, and rests substantially on persuasion [see: Service 1962; Fried 1967, Political Systems – An Introduction to Anthropology]. This stands in dark contrast to our current institutionalized and legalized reality where states rule. Trainter touches upon this uniqueness after reviewing simple societies throughout history, in his book "The Collapse Of Complex Societies".

> The citizens of modern complex societies usually do not realize that we are an anomaly of history. Throughout the several million years that recognizable humans are known to have lived, the common political unit was the small, autonomous community, acting independently, and largely self-sufficient. Robert Carneiro has estimated that 99.8 percent of human history has been dominated by these autonomous local communities (1978; 219). It has only been within the last 6000 years that something unusual has emerged: the hierarchical, organized, interdependent states that are the major reference for our contemporary political experience. [see: Joseph Trainter, The Collapse Of Complex Societies pp. 24]


rewrite and rethink: - "
From ancient construction of Gobekli Tepe, to the Great Pyramids, and today's large marvels like the Burj Khalifa, seemingly magical constructions were enabled by constant innovation in design and technology.


[^1]: https://www.haaretz.com/archaeology/2021-11-22/ty-article/study-reveals-how-end-of-ice-age-led-prehistoric-humans-to-settle-down/0000017f-efac-df98-a5ff-efadcc2b0000
[^2]: https://randallcarlson.com/wp-content/uploads/2022/10/The-Redemption-of-the-Beast.pdf
[^3]: The Origin of the State: Land Productivity or Appropriability? by Joram Mayshar, Omer Moav, and Luigi Pascali
[^4]: Sapiens - Yuval Harari
[^5]: https://gwern.net/doc/sociology/2022-mayshar.pdf
[^6]: https://www.nasa.gov/technology/carbon-dioxide-fertilization-greening-earth-study-finds/ 
[^7]: https://warwick.ac.uk/fac/soc/economics/staff/omoav/mmp_15_nov_2020.pdf



In the recent times computers have risen they quickly became integral to human life. We rely on carefully manufactured pieces of sand to help us do almost anything imaginable. Forming a cloud of tiny devices surrounding us; tiny superpowers we can use. We communicate long distances with our phones, make online purchases on our computers, entertain ourselves using our TV, research using computer models and so much more. Much in the same way cars assist humans in going further distances faster and unlocking opportunities otherwise deemed impossible computers allow us to make computations faster, augmenting all aspects of modern life.

With time, cultural changes and introduction to new words help designers talk about the dimensions of their design. One such dimension is complexity. "




*The lead:*
Complexity is a measure for how intertwined is a system. Complexity is bad for a system when it does not capture it's essence, yet it is good when it helps drive intent of a system forward by capturing the whole space of possibilities we might want. Now what do I mean by that? 

----
# Beginning - (Complexity)

Let us consider the inherent properties of this irreducible essence of modern software systems: complexity, conformity, changeability, invisibility, and culture adaptability.

Complexity. When we speak of [complexity](https://en.wikipedia.org/wiki/Complexity) in the systems we build, the dimension we care about is from the simple to the complex and it's reverse. This dimension captures how interwind the systems we care about are, as their parts interact in multiple ways and cause non-linear, emergent behavior. Simple system therefore have parts which can be isolated from the rest, changes to one part does not effect others, no unexpected ramifications or unwanted side effects.

A simple analogy can be made when we look no further than the making of bread. In the process of making bread, the doe is usually split into smaller parts shaped into strips and braided before it is finally baked. Splitting the bread and braiding it is not required as bread can be baked braided just as well as a simple lump of doe. Notice how I said that doe has a simple shape while a braided doe does not. If you think about it for a moment, after braiding bread the doe strips interwind and after finishing, reversing this process becomes difficult. If you had put the two strips one next to the other pulling one causes no effect on the others, meanwhile, after braiding pulling a single strip without effects on all others is impossible, you would have to carefully consider how each line moves about it's surroundings as it slips around moving up and down, hopefully not tearing the rest of the doe apart.

The ideal system we build should have complex behavior with a simple intelligible implementation, allowing us to benefit from the behavior we want avoiding the downsides of a complex system. To create such program you would have to compress all of the behavior you want and translate it into the shortest program which is capable of satisfying it. 

This is hard to achieve since any parts similar will be made into one and marks complexity to be an essential property of software systems.

> Software entities are more complex for their size than perhaps any other human construct, because no two parts are alike (at least above the statement level). If they are, we make the two similar parts into one, a subroutine, open or closed. In this respect software systems differ profoundly from computers, buildings, or automobiles, where repeated elements abound. [see: Brooks, No Silver Bullet pp]



For this reason when designing and building a system we should look to simplify the complex behavior into a implementation. For doing this there are three approaches we will explore:

- simplifying requirements 
- splitting requirements into smaller parts
- increase requirements to capture more behavior than you need


For many engineering diciplines this complexity manifests itself the repetition of similar parts. Designing todays electronics is a prime example of such repetition where many compoentns have been standardized. 


Invisibility. Just like finding a single pasta strand in a bowl of tangled pasta mess complex systems tend to be harder to modify then their simpler equivalents. Software is unique in this space as the increase in its complexity is invisible. When building a car you can see clearly when an engine has more parts and is more complex then it's predecessor. When building software you can't notice when the program has become significantly more complex. Most programmers use line count as a measure for complexity but it is only a partial measure. A 10,000 line program which does nothing but print 1 is less complex than a 10,000 line game. The simple fact that software complexity can not be meaningfully measured using a high quality property means that programmers can't know what to look for. What more is even if we have a metric like loc (lins of code) it's effectiveness deminishes over time because of goodhart's law. Tinygrad is a great example for how goodarths law effects metric based programming. Even if we count the lines in a vertical fashion in the tinygrad project horizontal space has become extremely important to utilize and dense. To a large degree this is a way that people optimize for line count while not doing the actual job of simplifying. It's a easy escape. This is what we want to avoid, and can't seem to be able to unless, of course, someone can see through this trickery and strive for the goal of simplicity. Chasing metrics is not the solution, what we need is a new culture.





[^1]: https://kristoff.it/blog/simple-not-just-easy/
[^2]: https://www.youtube.com/watch?v=LKtk3HCgTa8

https://en.wikipedia.org/wiki/Kolmogorov_complexity


# Meaning

The word meaning is often misundrtood. The concept is quite general and I believe it is also quite elusive. If you take something like words where when you say them,  and english person can interpret it one way, while a afro-american can interpret it in a completely different way. In korea the word niger is the same as hello. For this reason the content itself or in this case the word itself doesn't contain all the meaning the word has. The fact that it is said to Americans or Koreans matter. So the language you speak makes you understand things difffernetly than other people.   So the sounds of the words on partially contain the meaning.  We do know that the sounds contain some meaning since if you were to say different words to the same people they will understand different things. So words do contain meaning but the one interpreting them also contains part of the meaning. It is also the context within which the words are interpreted that matters. For meaning to be whole you need both content and interpertation. In our case the content is the words and the interpertation is the people who interpret the content to their hearts content. 

In the book  GEB An Eternal Golden Braid. Douglus points out that meaning is stuck in a strange dance between content and interpretation. His observation is even more powerful than our simple observation that meaning lives inside both the content or it's interpertation . 

Imagine 

Life is built around signaling, this text is itself a signal with some meaning I try to convey, reading only half of this text will not allow you to comprehend it's full meaning, just like when encountering many words one doesn't know will result in confusion. So where does meaning lay?

In his seminal text Godel Escher Bach, Hofstadter states that meaning itself is a strange dance between content and interpretation. For meaning to be found, content, which by itself meaningless must be interpreted. When standing on their own, content and interpretation are interchangeable. GEB illustrates this by imagining a world where DNA is similar to all organisms and the decoding mechanism is what makes them unique. For example a cat and a fish would contain the same DNA, but the decoding mechanisms which made them grow is what differentiates between them. The genetic information would then not be contained in the DNA but in the decoding mechanisms themselves. The fact that the decoding mechanisms for life are similar while DNA varies is what makes DNA the content and decoding mechanisms the interpreter. 

Because of this strange dance between content and interpretation, when looking at the meaning something has, you must take into account both what you want to.

, compression is an art of distilling information into it's essence therefore condensing meaning. 
Language itself is a compact description for the set of all valid sentences. You have letters which are meaningless, words which contain meaning and yet have limited meaning, rules for how to form sentences and you have sentences built out of both words and rules which are meaninful on their own. Infact, language has many interesting properties because of the huge set they create verses the small description required. The explosion of the set of meaningful valid sentences which you can get from a relatively small set of words and rules, is what makes language so powerful. 

Compression will forever stay an art as it is uncomputable. The problem can be restated as finding the shortest description of a program that can produce a file which, in our case, is equivalent to arbitrary data we want to compress. When restated as such, the problem is equivalent to finding the Kolomogorov complexity of said program, which hits the same wall of the Halting Problem.

One more interesting thought on compression. If we know that taking some initial state and some rules of inference are infact the shortest program for a problem, and it is the compressed form of all the states possible by that program, we can by state that it is isomorphic to a formal system. Formal systems are known to have truths which are not inside the formal system. 

If we know that the most compressed form of anything is the shortest content and a interpreter which generates the wanted result, we can say that anything that is compressed that way is a formal system. 

On a philosophical note, Stephen Wolfarm published A new kind of science in which he directly states that his new science is looking at the universe as a formal system. In much the same way that a formal system has truths which it can not state, the universe does not seem to ever reach a state which can be true but will never happen. This might be like looking at a green sun. The chemicals which will enable the sun to produce green radiation are likely never to occur in large quantities in the sun to produce the result. But the rules which the universe seems to be based on, suggest that a green sun can happen in the universe it is a "truth" and yet we will never see it. This, along with my first line of thought, suggests to me that the reason why Stephen has found the concept of irreducability when studying cellular automuta is because he when trying to compress the results from his experiments, has found himself stuck at a seemingly impossible problem. The same same level of difficulty as finding the answer to the Halting Problem. 

Another concept introduced is isomorphism. The term refers to the equivalence of meaning in two formal systems given by their consistancy when compared. e.g. Imagine you want to cook a couple eggs for breakfast. You first go to your refrigerator, take 2 eggs and put them on the counter. Since 2 eggs are no enough you return to your refrigerator and take 3 more. You then count how many there are on the counter and you find 5 eggs on the counter. Now you have probably already known that taking 2 eggs and adding 3 more eggs would result in 5 eggs. How did you know that? The answer is that you knew that the formal expression 2+3=5 is equivalent in meaning to the counting of eggs. Since you have learned this from a young age you are capable of computing the result easily. This equivalence or should I say isomorphism is a powerful concept which allows us to take the meaning of one and apply it onto the other. The real world is the only thing that is meaninful, since we live in it we give some "inherit" meaning to things in the world. In our example, the counting of eggs being a real physical action has meaning, while some expression we made up has to "gain" meaning by isomorphism as we say that the action of counting 2 eggs and 3 eggs is equivalent to addition of 2 and 3.

Another example of this split would be a image compressor. A lossless compressor can have many ways in which it can encode an image to compress it's "meaning". One way would be to give them a smaller representation based on statistical frequencies. So a pixel whos color is more like to show up will take up less bits to represent. This scheme would allow any one who wants to decode the message simply decode the compressed message by replacing compressed form of pixels with their colors. Another scheme for compressing the message could be to do a simple counting of similar pixels and grouping of them together. This would in-fact make the algorithm on the decoding end, contain information it did not contain before. It now knows that pictures contain many similar colored pixels one after the other. This type of information we can not deduce from the message itself. It is unique to the encoder/decoder scheme which we propose here. This unique information is exactly why the information of a message might be contained in the message passed as well as the decoder which decodes it. 

Compression is strangely connected to intelligence. With the advance of modern machine learning techniques, research seems to suggest that there is a correlation between how well an algorithm can compress information and how well it can make intelligent predictions. In a recent research paper, researchers demonstrated a new technique for word similarity test. They built a metric based on the compressed length of strings A and B along with their concatenation A + B. Doing so is roughly equivalent of doing a cross-entropy comparison of the two strings. This along side kNN clustering algorithm has shown very good results when compared to classical machine learning approaches, some of which are quite advanced, beating such algorithms in certain circumstances. 

Some are great proponents of this idea and have laied out a prize. The Hutter prize is led by...




A systems complexity might be characterized in a number of ways. One would be the Kolmogorov complexity. The definition is the minimum program which required to represent a string. From this we can inquire that a more complex system is one who's parts are all unique. For if they are not unique they can be compressed and represented more compactly. This is the main argument for the "essential complexity" of software as Brooks defined is No Silver Bullet.  While it is correct I feel like some aspect is forgotten. Some software while seemingly significantly smaller are more complex than their large counterparts.  Why might this be the case? I believe that it is because of how inter-wind the
parts that make up the system are.

Think of the economic market we live in. Each person is making seemingly simple decisions regarding his daily life buys and sells. And yet the markets is so complex there is almost no hope for ever being able to accurately predict it's chaotic behaviour. The interactions themselves might be simple but the sheer amount of even simple decisions and tendencies make the economic of a country seem like it's own entity and require very high level theories of how it works.  It is just impossible for us to understand the economy on a high level without approximating some of it's behaviour. This approximation is what abstractions are all about in software and in theories in general. 

There are popular approaches for thinking about complex systems. The holistic approach says that you must look at the whole in-order to understand how it works. The reductionistic approach says that you can break the system into parts until the system is understandable. These approaches try to approximate the general system by ignoring the details. This seems to be a natural human behaviour. We are not capable of reasoning about all of the details while looking at the bit picture, for this reason we just ignore them. Sometimes the details leak through our abstracted view of the system which break our understanding. 

Biological systems seem to contain endless amounts of complexity inside their inner workings and yet they are capable of changing and adapting better than any system we humans can design. This is going to be a very large premise into the idea that we can learn from biological systems and try to achieve their positive characteristics.


Another aspect which GEB book has touched upon is how the brain compresses the information which it holds. It seems like symbols that we use to convey meaning though are connected in many ways. Think of the word home which is connected to a building and a place to live in and your home and so on. There are infinite ways to connect between symbols and our brains seem to give weight to the connection importance. The weight assigned to the connection between two symbols will determine the likelihood of the brain to make a connection between the two symbols. You can think of it like a path of least resistance is formed for strong connections of symbols which is what forms opinion and knowledge in our brains. That said, weak connections with enough conviction can become stronger and strong connections weaker. The space of possible connections is never lost and instead is morphing to accommodate changes knowledge and update it's belief system. This is part of what I believe makes the brain so strong and anti-fragile with respect to knowledge acquisition and retention. 

This makes me think that if we can make some program where the space of possible solutions to the problem is large. We can have a dynamic weight system for solving it. For example if a plain needs a auto-pilot control system which has full control over an air-craft. We would like the output space of possible outcomes to not be too constraint. Think of a unexpected plain loss of engines and or extreme situation which the plain has not expected before. Humans have the ability to adapt to this situation with their ability of understanding the situation they are at and can think of a plan to get out of the situation. Machines on the other hand do not have such general capacity. To correct for this reality we would have to create a system which is capable of solving more paths than we plan the plain on taking. The space of possible maneuvers can be seen as the entire space, and we won't subtract the space and instead apply a weighting to the paths we would want to take. With of course the common paths having strong weights which lead to their solutions; uncommon paths with weak links should be saved for a time where unexpected things are in need of resolution. Constant updating should occur to strengthen and weaken links according to the result which we see from the data we get back. 

This system should create a highly dynamic path resolution algorithm which is also very very large in it's memory footprint. But for what it's worth, there might be some merit to this exploration of the theoretical in practice. This is something which I would have to look for myself, and report the results for in the future. Do common machine learning tactics work in a good enough manner to solve this type of problem or do we want a completely different approach. This is something which concerns safety of many passengers and if we decide to take it how to avoid accidents while developing? There are so many of these questions of operation which I can't think of fast enough while type on and on. Though this is the case I still firmly believe that there is some merit to this exploration if only to think of how such systems which don't compromise the space of solutions and instead assign weights to it and follow the path of least resistance to reach the resolution it would like to take.

--- (rethink the following it is kind of shit): 
One more approach would be to accept more inputs than are needed for a single problem. If you are familiar with programming, you might have heard of operator overloading. This is a simple concept to follow. Think of operations that you can do to mathematical primitives like addition of two natural numbers; subtraction of rational numbers and so on. The language supports overloading the same operator which you would use to primitive types onto different types. The programmer can choose to add two Pizza structures if he so wishes and make an addition operator which would "add" two Pizzas. This feature allows for the acceptance of different input into the same known code paths. For example if you would like to have a function which does some permutation on some data that you feed it and returns some results in integers, you may also want the same functionality be done on floating point numbers instead. You can use a generic type and feed it whichever type you want and expect a correctness of behavior be retained.



## Software complexity

===== checkpoint
Today software can be characterized by it's never ending complexity; projects know no bounds with ever growing requirements. Constantly changing feature additions, changing interfaces, and different people with skill sets that differ 

When searching of whom to blame, the current economy stands tall shining over all others. In much the same way as computers today have become complex entities, software seems to take large strides towards ever growing complexity in their design. The psychology of this phenomena I believe to be quite simplistic. When given the option, people rather have a fast complex chip than a slow simple one; simplicity and good design don't sell. Since people would rather pay for features than bug fixes, this all leads the culture we have to day which promotes the faster building of software entities, ever large requirement list, and lowering the cost of initial building of said entities. In a workplace which doesn't promote and pay for high quality people to build design good software you will not find quality; in a culture which doesn't promote the creation of simple stable software, these qualities will stay absent.

Project Oberon by Nikolaus Writh, has been an attempt at fixing years and years of neglect.

the endless growth of complexity of software. For some this complexity is manifested through bloat. Applications that take gigabytes of storage space to install, and hog available resources. While for some applications much of its 
size and resource utilization is high due to its nature, for most I would say, this is not the case. Complexity also harms the cost of maintenance. Adobe has made maintaining and adding features it's software a technical nightmare. Old 
bugs and crashes haunt Adobe Premiere to this day[^1] with no saving grace in sight. 


[^1]: adobe link to people talknig about how much it suck but it also can't change


How do we handle the complexity of the programs that we create? 
    - Refactoring
    - Rewriting
    - Abstraction 


# Abstraction

Abstraction is itself a method of compressing the meaning of the parts that make up something into more understandable, approximate meanings which hide the complexities under their general description and method of abstraction. In programming when you are using abstraction you are essentially compressing code you use into reusable parts, with names which describe their general function to allow for first, understandability within the system and second, reduction in complexity of having to know all of the underlying choices and instructions that make it up. The benefit to using abstractions is great which is why abstraction is used effectively everywhere in engineering. Chip design uses abstraction to describe the sub components used to create the whole computer. You begin with transistors, continue with gates, logical units and more complex systems. The underlying abstractions are clear. They provide us with a powerful way of using understandable components using minimal need of understanding everything that was taken to built, and yet having maximum effect of usefulness. When sketching parts for a robot, you have a set of abstractions built into the software. You first build the parts that will make your robot. Then, you build the sub-assemblies which when put together into the final assembly will create your final robot. This way of using abstractions makes the creator of the robot not have to constantly have to think about the individual parts, and instead shift focus into the bigger forming assemblies which make up the bigger assembly. At every level of abstraction, you lose some ability of configuration but gain both the re-usability and conceptual understanding of the parts which make up the final robot. 

Abstraction is useful for humans because we find it easier to deal with abstract concepts than with many simple details. Would you find it easier to think about how each an every ant in a ant colony interacts one with the other or just simple behaviour descriptions of the ant colony as a whole. Even if the abstraction lacks the accuracy of knowing all the details it is necessary as humans are not capable of dealing with them.  - rethink the sentence 

When abstracting you are essentially finding the basic building blocks which make up your program and are compressing the instruction into these basic blocks. This process of abstracting is best done continues since, if done beforehand when you don't know which basic blocks you would really need, you will eventually end up having to refactor basic blocks until you find that the state of your code is at equilibrium. With this in mind, we can see how systems which need a tailor made solution with strict design can be made up. A progression of simple instructions, which eventually find their place inside basic blocks, in a continues process of refinement leads to larger systems that make up the program, and a comprehensible well managed code. If you instead follow the reverse approach of first coming up with the large systems, basic blocks, and general design of a system before you write even a single line of code, you are essentially defining the structure of your code in a way that a slight mistake which you will make can cause large structure redesigns and time taken to refactor or redesign the system. This is of course assuming that the system is new and there is no already good known design which you can use. The web is a typical example of a known problem, with known solution which everybody uses. In such cases it is infact better to use already existing solutions that people have spend much time in finding instead of inventing the wheel all over again using the methodological process of exploration and refinement.

Though this approach is, by some, admired as the epiphany of programming. Since you can test all cases for failure, make sure that the program does what you intended it to do, and have somewhat of an understanding of what happens in the code. There are those who believe that in some cases it might not be our saving grace, but instead a horrible mistake. Abstractions leak. Leaky abstractions are abstractions who's underlying decisions which made it up leak and end-up conflicting with the user of the abstraction. This is a direct result of the approximation the abstraction gives by leaving out the details and with them important decisions the user of the abstraction is unaware of. Attempts at thinking about and trying to avoid these leaks have been made. The proposal of meta-abstractions which are interfaces that allow the user to change the underlying details of the abstraction he uses to fit his needs have been made. An example shown is a virtual memory system made up of objects that manage memory as blocks. The policy of evicting memory pages is left for the user to decide using a meta-interface. You inherit the initial objects properties and then change the functions regarding the policy. Again this is a way which tries to bridge the gap between the details left out by the abstraction and the leak of the decisions made when the abstraction was made. The reason for why we care so much about leaky abstractions is because they are everywhere and because of the unreasonable effectiveness of code-reuse in programmers productivity. Reducing the cost of reusing already existing abstractions by bridging gaps in the abstraction is essential for good libraries and components common place in all software. With good ideas throughout the software industry history, there is a good reason to believe that understanding how to build better abstractions could help tremendously the building of the software systems of the future.

With the proliferation of software, software projects can now grow to immense sizes. With software being responsible for more critical systems than ever, and the cost of a mistake is high, it has become important for systems we design to be robust to failures. For that reason programmers have developed two different approaches for designing robust software. The most popular solution is to create provable or testable software where you can grantee the resilience of the software against a set of tests the programmer expects to be correct. This has the major benefit of changes to code that conform to the tests will make sure behaviour has not changed under these software changes. 

The compression-oriented programming Casey Muratori showes and Brooks top-down design techniuqe. I strongly believe after using these techniques myself for quite some time now, that I can explain both why they are so powerful and how come it feels so natural to use them over other techniques (like design first and then implement and so on).

The aspect oriented programming is a interesting paradigm that I think is worth to search further about. 
https://www.cs.ubc.ca/~gregor/papers/kiczales-ECOOP1997-AOP.pdf

Kruger's Reusable components and Software robustness have in them a similar section in which they advocate for the automatic search of different components that confirm to constraints.


# Code-Reuse

Reuse is of essence to all known design. In it's nature, reuse helps us reduce costs associated with the design and material production. When designing and eventually building modern constructions, we rely on known techniques, building materials, construction techniques, and more. Apartments are cloned across levels and sometimes to reduce costs even further whole buildings are copied across a neighbourhood. Here reuse is clearly visible in real life. Designs manifest themselves in a visible way which allow us to see when reuse happens. 


Seemingly good practices like code-reuse are over-used irresponsibly, with people including huge libraries for the use of a single function. Sometimes it is in search of a high quality library that implements something you are not an expert on.

It can be the community around the library along with the increase transfer-ability of skills across teams. Maybe you just don't want to maintain that part. These are the reasons one might turn to reusing code instead of coding it from scratch.

On the other hand, using other people's code means conforming to the constraints that the people who created the library had. Bigger libraries impose more constraints, and more libraries impose more constraints. If the constraints don't meet yours, you might end-up spending more time on adjusting your/other's code to make it work than if you had wrote it from scratch. And if you have invested too much into a bad library... too bad.

> it's fine if a library isn't completely perfect, none is, it's a time saver. you sacrifice a little bit of personal peace for the sake of getting some shit done or not needing to have the mental overhead of dealing with another person's crap. especially not a person or people who you can't directly contact...the maintainers are simply not my friends, they're not my colleagues, if i need LLVM fixed i cry, if i need any huge library fixed i need to go learn their BS along with the original problem space. if i need a feature added, i also cry. [NeGate, on [handmade-network fishbowl](https://handmade.network/fishbowl/libraries/#chatlog__message-container-1003003441322397797)]

Worst are the changing constraints. Usually this happens with package managers when updating dependencies. In general, package managers allow for quick download-and-use type of behavior with regard to dependencies, and easily updating dependencies. We can notice problems when a library author has decided to change the behavior or api of his library. The update will cause many programs dependent on that old version's behavior or api to break. For this reason, explicit upgrades to stable versions of software is preferred, rather than the more convenient automatic option.

We can see this in the [https://handmade.network/](https://handmade.network/)">handmade-community where single-header libraries are popular. e.g. [https://github.com/nothings/stb](https://github.com/nothings/stb)`stb_image` and `stb_freetype`. They require explicit upgrades, the ability to modify source code, and they usually do not have dependencies themselves. Along with the fact that single-header libraries are usually smaller, they dodge to some degree the more destructive nature of large managed dependencies while still bringing great value.

Sometimes you just need more than what people fit into a single-header library or you don't want to change the source code and instead just extend what is already there. This will touch on library design a bit. But it is important to know because if a library is [well designed](https://www.youtube.com/watch?v=5l2wMgm7ZOk/), then it will meet a much wider range of people's needs and won't become a large time-sink down the line if maintenance is needed in the code-base.



The important take a way here is when you buy into a big library, you sell some personal responsibility away which can be good... until things go wrong




When I began programming I had only the concern of making things work. Later at high school I was taught that I needed to "design" my programs before programming takes place, using UML like drawings I began programming in such way, designing the architecture of whole programs before I place a single line of code. The basic principle behind the idea was that design was much harder than the act of writing the text needed to make the design work. Of course later I found that this is nonsense, completely abolishing this type of programming style.




# Safety 

One thing that I was never taught or thought about was the safety of program. Until I read great article The Coming Software Apocalypse showcasing how the proliferation of software and how it quickly seeped through the cracks of everyday safety systems. From the elevator to managing water, cars, medical equipment and almost all critical systems today, software is at the bedrock of society. This placed large safety concerns since bugs in software are invisible. 

NOTES: We shall explore the problem of software safety using examples of real world cases: 

# Antifragile

This quality has many uses when you understand how it is isomorphic to so many things in this world. Taleb coined this terms and in his book through the various examples and many other things this quality and how to think about it in context to all other things. This is what makes it so impactful, it is the fact that it speaks volumes about general systems, and makes isomorphic statements about them. Through analysing biological systems, the good properties which the exhibit are ones which some of our systems would love to have. Understanding anti-fragility is crucial to having this view on the world on general systems. 

Size. Smaller means better for if a smaller unit fails, the failure will have a significantly smaller total impact. This along side the reduction of risk since you will be diversifying your "investment" in a single large system into smaller ones, which is known to be mathematically better[^].

Randomness. Antifragile systems like randomness. Randomness is the stimulus that makes Antifragile systems thrive while staleness causes the large failures leading to death. The greatest benefit of randomness is that you can not food randomness. If you have a known schedule of things that you have to accomplish, and you plan beforehand everything in your day, you can cheat things that you don't want to do. If instead in a random day of your week something chooses for you to do whatever it is you don't want to do or remember to do, it will cause you to avoid the amateur mistake of optimizing it out of your life. 

A classical example of such over optimization can be seen in today's classrooms. To track the students progress teachers had to create a proxy measure for increase in knowledge. The test was invented to allow teachers to measure this progress. It is unlikely you yourself don't know that periodic tests done at a known date with students having enough time to prepare in advance, usually allow themselves to optimize for the test instead of learning. Humans like to over-optimize, when they have a metric to optimize for in this case their grades, they might learn for the exam a day before or learning at home without listening at classes at all. This is sub-optimal for the initial requirement of knowing the knowledge imparted by their teachers, but optimal to get good grades with the least amount of effort. This is the problem which is most commonly known as Goodhart's law. To mitigate this problem teachers can randomly test their students in surprise. This will not allow students to have the capacity of cheating or cramming studying. It will effectively become a significantly stronger measure which is the original intent. Here randomness plays a crucial role in making the measure become more robust to cramming and cheating which are common problems in tests.

Therac 25 accident had a similar case where the warning system constantly spit out non-understandable messages, eventually the operators optimized their for these messages mostly ignoring them. This seemingly innocent optimization caused a horrific accident exposing patients to orders of magnitude more radiation then allowed. 

Compression. Here I mean that antifragile systems usually are designed such that they have a explosion of states they can handle compared to their size. Their representation is compressed into a seemingly small footprint while being able to handle a very large state space. Think about language. English is composed of 22 letters, 170,000+ words, and rules to follow. The rules are sometimes hidden in the way we speak, not all are known and written on a piece of paper. Which explains the failure of common approaches to capture language in comparison to the relative success machine learning algorithms had in recent years. For the amount of words and rules, this is a relatively compact representation since you can represent using almost anything you might want in English. The small amount of building blocks and rules of inference create a explosion of states it can fall into, evidently this makes English extremely robust since you have many ways of saying the same thing using any number of words ordered in any way that follows the language rules. In this large space of correct sentences, we find properties which make language as robust as it is for conversation. Redundancy of words and degeneracy in sentences. 

Taleb believes the main reason for much of systems failure is caused by rare events he calls Black Swans. Black Swans are tail events which rarely happen and have disastrous repercussions when they do occur. These events cause such damage to systems that are underprepared that failure follows.

---
# References
[^1]: Antifragile
[^2]: The book GEB - Godel, Escher, Bach 
[^3]: Brooks-NoSilverBullet





The next topic is unrelated to the rest of the text. I just wanted a place to write my thoughts that I had for some time now. For this reason you should ignore this section until I remove it completely.

# Economy 

Learning about economy and money in general has been, and still is, quite the journey. Learning about money, what are markets, government intervention, political strife, patterns caused by the forces moving the markets and the history from which these patters are formed allows me to understand the world around me better. There is no new information here just accumulation of my own financial understanding, and some thoughts that I have which are not interesting by themselves.

To make transactions happen in the past, people would trade commodities with other commodities. This made gauging prices hard, and trading in general not as accessible as could be. When money was invented, people could set a price for commodities such as food, utilities, and services using a universally accepted medium of exchange. In the case of the ancients it might have been wheat, gold or other precious metals, eventually making coins, paper money and digital money. The important thing here is that there is a universally agreed upon medium of exchange, even if when standing on it's own might be worthless. In the financial world there were 2 big inventions which changed the landscape forever. The invention of the banking system and investment institutions like Wall Street. The [origins of banking](https://firstutahbank.com/the-history-of-banking-from-ancient-times-to-now/) can be traced back to ancient Mesopotamia, around 2000 BC though, banking as we know it began around the 17th and the 19th centuries. With the invention of the banking system, the concept of money has extended. People could use something new called *credit* to pay for things they could not have been able to afford otherwise. The invention of credit allowed people and businesses to expand the scope of capital available to them greatly, allowing for greater economical expansions as money could be borrowed to facilitate the growing businesses needs. While the banking system allowed successful businesses to greatly expand their operations, it did not allow for great risk to be taken since it increased their risk for people defaulting on their loans (not paying back). For entrepreneurs who need capital to take big risks while expecting big gains the banking system could not provide the answer. For individuals the risk is also too high. Think of someone in the 19th century wanting to sail into the sea and find gold in another place. The risk taken here is immense as you could lose all of your large investment, although if successful, the reward would compensate you greatly for your risk. To minimize risk, the invention of shares with the rise of financial markets was invented.Now multiple people place their savings in entrepreneurs and risk takers to go and bring them profits. Shares of companies just represent parts of the whole, allowing many people to pour investment and reduce risk taken by any single individual.

Today transactions happen by the use of money or credit to buy goods, services, and financial assets. The price of goods, services and financial assets is calculated by the total spending of money and credit divided by the total quantity of these goods. Because the credit is used like money and is usually calculated as money people make the mistake in thinking that it is money. Credit is in-fact a contract between two parties where one has debt which is a liability that eventually needs to be repaid, and the another has an asset (credit). When the debt+interest is repaid, the transaction finishes and both credit and debt disappear. Since debt has to be repaid, markets follow a cycle like pattern. First, when credit is made a economic boom with it's great times ensues. When it is time to repay the debt as interest go up, the economy slows, causing a economic bust. When borrowing you are borrowing from your future self as evident from this cycle like behavior. 

There are 2 forces which influence the creation and removal of money in a major way. Interest rates and money printing. The central bank uses these forces to manipulate the creation and removal of money from the system.


During hard time where money printing is in order it is interesting to note the fact that the ones who gain the most are the rich people since they accumulate wealth that goes up in value. The assets which go up in value make the rich richer and the poor poorer since they can not allow themselves to buy assets and thus enjoy the same riches. The problem is exacerbating with time and a trend which will continue until a great political strife will happen which will seek to redistribute wealth. This is a trend which takes decades if not normally centuries to materialize yet, it is something which as Ray Dalio notes is part of a longer cycle which we are a part of.


# Productivity

I want to share a small bit of my experience with productivity. For quite the longest amount of time now I have been programming as a novice programmer building small projects that take at most a couple months to finish. Working on large projects has never been something I am good at. The compiler project I had built was done in many more months than would be required had I actually sat down and wrote it in a more productive way. What I mean here is that the way I was managing my time was quite wasteful. I could sit down all day and play or read or do anything but work. I had a strong believe or some might call it a sense that I was string progressing in a reasonable rate, or at least somewhat reasonable. Today if you are to ask me the same question the answer would be: "My time management was pitiful and the results showed me this fact". I was not that I had a bad project or anything like that. I was more that the things that I wanted to accomplish had taken such of a small scope in comparison to what I knew I could make. It was more of a self insufficiency more than anything else. In that project I had plenty of time, and I could reasonable plan my work to make something which I could be proud of. I believe that part of the problem for not accomplishing much was the fact that I had plenty of time. Your work is going to fill out your time, if you have 8 hours to accomplish something it will take 8 hours. If on the other hand, you had 2 hours for the same amount of work, you would most likely finish more work in these 2 hours then in the first 4 of the first example. The urgency I believe that is created is something no one should underestimate. 

After around a year which I took a break from programming, I returned and found out that I was very very slow in my thinking. Almost too slow even I would say as I spent an entire day doing almost nothing. The big revelation was of course setting a short time limit. I set a 15 minute time limit and saw my productivity shoot. The next day I set a 1 hour time limit still good results. The urgency for pushing through the times where you are unsure as to how to continue with doing something which will let you push into the correct direction has been the major contributing factor. 

Not only creating urgency of work is important, I have also found that reflecting our my work has done wonders in finding what are the things that I may be lacking and having some sort of metric for evaluating my work is what I should be concentrating my time now. 



