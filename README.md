# CompilerPrinciples-Lab
:rocket: ​Labs of Compiler Principles, including 4 parts:

- Lexical Analysis
- LL(1)
- LR(1)
- Semantic Analysis

:rocket: `LL(1) Parser Generator.pdf` and `SLR Parser Generator.pdf` are generated from [Online LL / SLR / LR Generator](http://jsmachines.sourceforge.net/machines/), which is strongly recommended.



## Lab1-Lexical Analysis

- Symbol Table is defined in `LexAnalysis/c_keys.txt`
- Source Code is in `LexAnalysis/LexAnalysis.h` 
- Transition Diagram:

![CompilePrinciplePlot.001](https://tva1.sinaimg.cn/large/007S8ZIlly1geu9w5ito3j31hc0u079c.jpg)

![CompilePrinciplePlot.001](https://tva1.sinaimg.cn/large/007S8ZIlly1geua167s00j31hc0u0tdg.jpg)

![CompilePrinciplePlot.002](https://tva1.sinaimg.cn/large/007S8ZIlly1geua1k1dufj31hc0u0q7m.jpg)

![CompilePrinciplePlot.004](https://tva1.sinaimg.cn/large/007S8ZIlly1geu9y114udj31hc0u0799.jpg)

![CompilePrinciplePlot.005](https://tva1.sinaimg.cn/large/007S8ZIlly1geu9zo6aayj31hc0u0jtf.jpg)

## Lab 2 - LL(1)

The analysis table is computed manually. However, I stronglly recommend using **the online generator** mentioned above.

![image-20200703204500925](https://tva1.sinaimg.cn/large/007S8ZIlly1gge2ktiypzj31hd0u0787.jpg)



## Lab 3 - LR(1)

The analysis table is in `SLR Parser Generator.pdf`.



## Lab 4 - Semantic Analysis

The analysis table is in `LL(1) Parser Generator.pdf`. I did the translation schema during LL(1) procedure.