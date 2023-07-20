# pipelineDP
Dynamic Program for pipeline network routing. Based on a report by Uri Shamir (1969) and later published in 1971 as:

https://shamir.net.technion.ac.il/files/2012/04/1971-Optimal-Route-for-Pipelines-in-Two-Phase-flow.pdf

# Sample output

```
Optimal route with minimal cost : 0 4 18 35 53 60 
Cumulative cost : 0 44103.3 82758.2 132613 182473 217274 

Optimal route with minimal length : 0 4 18 35 52 60 
Cumulative cost: 0 47131 82758.2 132613 190295 218215 
Cumulative length : 0 6459.24 12184.9 18631.5 25706.3 30566.2 

Optimal route with minimal pressureloss : 0 3 18 34 49 60 
Cumulative cost: 0 48512.4 87167.3 154161 203970 229964 
Cumulative pressureloss : 0 8.65608 15.1931 26.44 33.3541 37.886 
```