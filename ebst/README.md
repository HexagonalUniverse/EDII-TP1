EBST
====

Introduction
------------

Considering that the registries disposal on the data-file are ordered, forming a sequence, $\langle r_1, r_2, \dots, r_n \rangle$ such that $i > j \iff r_i > r_j$. Here the registry represents indistinguishably its key. Taking advantage of that property, a direct construction of a balanced binary search tree can be done. Since the sequence is ordered, its middle-most registry divded the file into two other ordered files with approximately the same size. And by that means, it must be balanced. We can keep recursively dividing it to assemble the final balanced bst.


For convention, let's start the index of the sequence in zero: $\langle r_0, r_1, \dots, r_{n - 1}\rangle$. The middle-point, it is, the middlemost registry can said to be the one with index $\lfloor\frac{n}{2}\rfloor$. For example, if $n = 9$, it would be $r_4$ (the fifth registry), and if $n = 10$ or 11 it would be $r_5$. Representing the construction by $\mathcal{B}$, it can be recursively represented as


$$
\mathcal{B}
\left[\begin{array}{c}
    r_0 \\
    r_1 \\
    \dots \\
    r_{\lfloor n / 2 \rfloor}\\
    r_{n - 2} \\
    r_{n - 1} \\
\end{array}\right]
\longrightarrow
\left(
\mathcal{B}
\left[\begin{array}{c}
    r_0 \\
    r_1 \\
    \dots \\
    r_{\lfloor n / 2 \rfloor - 1}
\end{array}\right], 
r_{\lfloor n / 2 \rfloor}, 
\mathcal{B}
\left[\begin{array}{c}
    r_{\lfloor n / 2 \rfloor + 1} \\
    r_{\lfloor n / 2 \rfloor + 2} \\
    \dots \\
    r_{n - 1}
\end{array}\right] \right),
$$

where a root node is constructed from the middle registry, and the two subtrees of it are the construction of the parts; the stop condition being the sequence do not having at least one element.


Considering $\langle a, b, c, d, e, f \rangle$, the traversal over the sequence would be $c \to (a \to b) \to (e \to d \to f)$. If the object that the traversal is delt was a binary tree, then we could interpret it as being a _pre-order traversal_. Since it is over a ordered sequence, denote it _Midpoint Root Traversal_ - MRT for short.


Transcription
-------------

Our objective is to generate a file whose data is structured in a binary search tree model for later be used into search. So to build it we will use our construction $\mathcal{B}$ to iterate along the input, data file stream, and write the data-structured file along the way.

The data-structure is as follows. It will be an array with each node being $h = (r_k, l_{\text{child}}, r_{\text{child}})$, where the children information is actually a internal pointer.

Interpret the input stream as an array $\langle r_0, r_1, \dots, r_{n - 1} \rangle$. Since we have the granularity precision for seeking one register in external memory as well, while obvious, we'll use the same throughout the process. And it can be achieved considering that a subfile (representing a subtree) is that of all sequential elements, from the positions $l$ to $r$, $l \leq r$ - inclusive in both ends. Being so, define $midpoint(l, r) = l + \lfloor \frac{r - l}{2} \rfloor$. At each iteration we'll keep seeking for the middlemost registry of a subfile with it.

If a node $h$ in reality kept pointers to registries in the original file, $l_{\text{child}} = midpoint(l, m - 1)$ and $r_{\text{child}} = midpoint(m + 1, r)$, if them existed, given that $m$ is the current midpoint of $h$. Since it is not the case, a further observation of structure of the ebst is needed. In that sense, realize that, for whatever subfile, we first write down the root, and then the children - in a predictable order. So it is the case that root is always on top of it. Hence, the next $a$ itens after the root $h$ is (either) the left (or the right) subtree; top of which is its root and so, if root is on the position $p$, that child will be on $p + 1$. Similarly, the other child will be on $p + 1 + a$. $a$ relates with $l$ and $r$ in the following way: after a division of a subfile with $n$ itens, the left child will have $\lfloor \frac{n - 1}{2} \rfloor$ itens and the right will have $(n - 1 - \lfloor \frac{n - 1}{2} \rfloor)$. Since $r - l + 1 = n$, $a$ is either $\lfloor \frac{r - l}{2} \rfloor$ or $r - l -\lfloor \frac{r - l}{2}\rfloor$.


Implementation
--------------

The implementation can be done recursively, as does the definition. But the premisse over what we're working is that of $n$ can be so big that would happen problems with memory due to recusing so deep. The iterative one will also have to use quite space, but it will be less, considering issues like function-overhead. By these means we would use a stack to keep track of $l$ and $r$ at each iteration. We initialize it with $0$ and $n - 1$ respectively to process all the file.

At each iteration, we first determine $m$: the middle point. We then stack right, calling $\mathcal{B}(m + 1, r)$, and left, $\mathcal{B}(l, m - 1)$, in whatever order, if there they yet form an valid subtree. So the condition for each of them is to verify if $l \leq r$ in the new means. We then recover the registry at index $m$ and write the node on the output file. In case the left-stacking is done earlier, then the left pointer will be $p + 1 + |\text{right-half after division}|$, and the right pointer will be $p + 1$, if there was a stack for each of them.


Quick Analysis
--------------

The main concern of the use of these techniques is related with memory - since in terms of speed efficiency, ebst may not be the most appropriated. So let's try to check what is the space-complexity the stack yield us.


Obs: The stack is a dynamic-structure, and each item holds only two **int** pointers, so in principle we should be good with it.


Denote by $S(t)$ the size of the stack at the discrete instant $t$. We start adding a item to the stack: 
$$S(0) = 1.$$
At a generic moment, the stack size is $S(t)$. Then we pull the top of the stack:
$$S(t) \to S(t) - 1.$$
And then we add two more itens conditionally. Saying that the resulting tree will be perfectly balanced. So at each step we either add none or both children information to the stack.

In our current convention, the right children is always processed before the left one. By effect, we exhaust all the right subtree before the left one. Note that if is there no left-subtree, the stack size continues the same, because we push but pop in sequence.


If we denote the stack-space used by a perfecly balanced bst of size $T$ by $\mathcal{S}(T)$, then

$$
\mathcal{S}(0) = C, \mathcal{S}(T) = 1 + \mathcal{S}(\lfloor T / 2 \rfloor) \implies$$
$$

$$
\mathcal{S}(T) = 1 + 1   + S(\lfloor T / 2^2 \rfloor) = \dots = n + S(\lfloor T/2^n \rfloor).
$$

$$
\lfloor \frac{T}{2^n} \rfloor = 0 \implies2^n > T \implies n > \log_2{T};
$$

taking $n = \lceil \log_2{T} \rceil$,

$$
\mathcal{S}(T) = \lceil \log_2{T} \rceil,
$$

so after all the space used it have logarithmic base-2 complexity.



