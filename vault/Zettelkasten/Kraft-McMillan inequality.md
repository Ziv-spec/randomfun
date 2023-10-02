202309091714
Status: #idea
Tags:

# Kraft-McMillan inequality

Terminology: 

A B-ary code is a code $\large C : X \to A^* \text{ s.t } |A| = B \ge 1$
$\large \text{e.g. } A = \{0, 1\} \: B = 2, \text{ 2-ary}; A = \{ 1, 2, 3 \} \: B = 3, \text{ 3-ary}$

Theorem:
(a) McMillan: For any uniquely decodable B-ary code C,
$\Huge \sum_{x\in X} \frac{1}{B^{l(x)}} \le 1 \text{ where } l(x) = |C(x)|$

(b) Kraft: If $\large l: X \to \{1, 2, 3, 4, ...\}$ satisfies $\Huge \sum_{x \in X} \frac{1}{B^{l(x)}} \le 1$
There exists a B-ary prefix code C s.t. $\large |C(x)| = l(x)$   $\large \forall x \in X$

Practice: 

| x   | p(x) | C(x) | l(x) |
| --- | ---  | ---  | ---  | 
| a   | 0.5  | 0    | 1    |
| b   | 0.25 | 10   | 2    |
| c   | 0.125| 110  | 3    |
| d   | 0.125| 111  | 3    |


Proof


---
# References

[(IC 2.8) Kraft-McMillan inequality - statement](https://www.youtube.com/watch?v=yHw1ka-4g0s&list=PLE125425EC837021F&index=15)


