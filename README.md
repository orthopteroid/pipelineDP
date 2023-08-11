# pipelineDP
Dynamic Program for pipeline network routing.

The pipeline problem being solved here is a diamond shaped network with the known inlet and outlet at the tips but an indeterminate route.

The allowed routes all flow from inlet to outlet through connecting layers (or 'strips') of a finite number of points in the form a Directed Acyclic Graph.

The edges between the points in adjacent layers have their own terrain clearing costs and elevation gains, the latter of which is important when considering headloss (pressureloss) in the type of pipe that is being modelled here. Specifically, this DP considers a combined headloss for the gas and the oil components in the pipe. 

This implementation is able to compare the cost, length and pressureloss minimizations as well as solve for routes that provide specific amount of pressure loss (using a memory-intensive quantized pressureloss table). Ideally, the requested route would be cost-minimized but that would be additional work for later.

Based on a report by Uri Shamir (1969) and later published in 1971 as:

https://shamir.net.technion.ac.il/files/2012/04/1971-Optimal-Route-for-Pipelines-in-Two-Phase-flow.pdf

# Sample output

```
Minimized cost 
Optimal route (nodes): 0 6 12 20 27 37 46 53 59 60 
Optimal route (strip,index): (0,0) (1,5) (2,5) (3,7) (4,6) (5,8) (6,8) (7,6) (8,5) (9,0) 
Cumulative cost: 0 24215 45802.4 57698.6 82758.2 101638 132613 163290 182473 217274 
Cumulative length: 0 3561.03 7012.21 7897.01 12184.9 14753.7 18631.5 23096.6 25917.6 31035.4 
Cumulative hills: 0 53 53 53 53 53 273 273 348 348 
Cumulative pressureloss: 0 7.29082 11.2086 10.4045 15.4881 18.5334 40.2134 38.558 46.2455 52.3128 

Minimized length 
Optimal route (nodes): 0 6 11 20 27 37 46 53 59 60 
Optimal route (strip,index): (0,0) (1,5) (2,4) (3,7) (4,6) (5,8) (6,8) (7,6) (8,5) (9,0) 
Cumulative cost: 0 24215 47131 57698.6 82758.2 101638 132613 164351 190295 218215 
Cumulative length: 0 3561.03 6459.24 7897.01 12184.9 14753.7 18631.5 22914.5 25706.3 30566.2 
Cumulative hills: 0 53 53 53 53 53 273 273 348 348 
Cumulative pressureloss: 0 7.29082 8.70001 10.4045 15.4881 18.5334 40.2134 42.6851 49.1799 46.2553 

Minimized pressureloss 
Optimal route (nodes): 0 4 11 20 27 34 41 51 55 60 
Optimal route (strip,index): (0,0) (1,3) (2,4) (3,7) (4,6) (5,5) (6,3) (7,4) (8,1) (9,0) 
Cumulative cost: 0 31090.3 47131 57698.6 87167.3 119394 154161 180341 203970 229964 
Cumulative length: 0 4572.11 6459.24 7897.01 12815.4 16945.1 22057.9 25137.9 27987.6 31810.2 
Cumulative hills: 0 18 18 18 18 18 23 23 23 23 
Cumulative pressureloss: 0 6.46274 8.70001 10.4045 15.1931 20.0891 26.44 30.0915 33.3541 37.886 

Solving for pressureloss of 38
Route (nodes): 0 3 10 18 26 33 40 49 55 60 
Route (strip,index): (0,0) (1,2) (2,3) (3,5) (4,5) (5,4) (6,2) (7,2) (8,1) (9,0) 
Cumulative cost: 0 36762.8 48512.4 64013 90627 126921 161930 188823 206002 231995 
Cumulative length: 0 5406.3 7301.39 9410.31 13031.3 17969.2 22732.4 25896.3 28233.5 32056.1 
Cumulative hills: 0 0 0 0 0 0 0 0 0 0 
Cumulative pressureloss: 0 6.40938 8.65608 11.1563 15.4491 21.3032 26.9502 30.7011 33.4719 38.0037 

Solving for pressureloss of 39
Route (nodes): 0 3 9 15 23 31 39 47 55 60 
Route (strip,index): (0,0) (1,2) (2,2) (3,2) (4,2) (5,2) (6,1) (7,0) (8,1) (9,0) 
Cumulative cost: 0 36762.8 51759.9 81612.3 103705 136323 164365 186133 202053 228047 
Cumulative length: 0 5406.3 7611.75 11673.3 14922.2 18759.6 22574.9 25776 27942 31764.6 
Cumulative hills: 0 0 0 0 25 25 25 25 25 25 
Cumulative pressureloss: 0 6.40938 9.02402 13.8392 19.1385 23.6879 28.2111 32.0061 34.574 39.1059 

Solving for pressureloss of 40
Route (nodes): 0 3 8 15 23 30 40 51 57 60 
Route (strip,index): (0,0) (1,2) (2,1) (3,2) (4,2) (5,1) (6,2) (7,4) (8,3) (9,0) 
Cumulative cost: 0 36762.8 54985.2 78816.9 100909 138239 159459 184320 202705 231296 
Cumulative length: 0 5406.3 8345.38 11587.8 14836.7 19228.5 22115.5 25040.3 27541.7 31746.2 
Cumulative hills: 0 0 0 0 25 25 25 25 45 45 
Cumulative pressureloss: 0 6.40938 9.89378 13.7378 19.0371 24.2438 27.6665 31.1339 35.2576 40.2422 

Solving for pressureloss of 41
Route (nodes): 0 3 10 15 23 30 40 51 57 60 
Route (strip,index): (0,0) (1,2) (2,3) (3,2) (4,2) (5,1) (6,2) (7,4) (8,3) (9,0) 
Cumulative cost: 0 36762.8 48512.4 85710.2 107803 145133 166352 191213 209599 238189 
Cumulative length: 0 5406.3 7301.39 12362.3 15611.2 20003 22890 25814.8 28316.2 32520.7 
Cumulative hills: 0 0 0 0 25 25 25 25 45 45 
Cumulative pressureloss: 0 6.40938 8.65608 14.656 19.9554 25.162 28.5847 32.0521 36.1758 41.1604 

Solving for pressureloss of 42
Route (nodes): 0 3 9 17 27 34 42 51 57 60 
Route (strip,index): (0,0) (1,2) (2,2) (3,4) (4,6) (5,5) (6,4) (7,4) (8,3) (9,0) 
Cumulative cost: 0 36762.8 51759.9 67216.6 87911.7 121582 150364 181049 199434 228025 
Cumulative length: 0 5406.3 7611.75 9884.8 12928.2 17509.1 21741.8 25351.8 27853.3 32057.7 
Cumulative hills: 0 0 0 0 0 0 50 50 70 70 
Cumulative pressureloss: 0 6.40938 9.02402 11.7188 15.3269 20.7578 28.6711 32.9509 37.0746 42.0592 

Solving for pressureloss of 43
Route (nodes): 0 3 8 17 27 34 42 51 57 60 
Route (strip,index): (0,0) (1,2) (2,1) (3,4) (4,6) (5,5) (6,4) (7,4) (8,3) (9,0) 
Cumulative cost: 0 36762.8 54985.2 71992 92687.1 126357 155139 185824 204210 232800 
Cumulative length: 0 5406.3 8345.38 10846.4 13889.8 18470.7 22703.4 26313.4 28814.9 33019.3 
Cumulative hills: 0 0 0 0 0 0 50 50 70 70 
Cumulative pressureloss: 0 6.40938 9.89378 12.8588 16.4669 21.8978 29.8111 34.0909 38.2146 43.1992 


```