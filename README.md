# pipelineDP
Dynamic Program for pipeline network routing.

The pipeline problem being solved here is a diamond shaped network with the known inlet and outlet
but an indeterminate route. The allowed routes all flow from inlet to outlet through connecting
layers (or 'strips'). Some layers are completely connected to the next layer, some are not.

The edges between the points in adjacent layers have their own terrain clearing costs and elevation
gains, the latter of which is important when considering headloss (pressureloss) in the type of
pipe that is being modelled here. Specifically, this DP considers a combined headloss for the
gas and the oil components in the pipe. 

This implementation is able to compute the global minimization of cost, length or pressureloss
as well as minimize cost for a specific pressureloss. The approach taken suffers from some
quantization errors as a result of trying to reduce memory load. This raises the possibility
that the minimum-cost pressureloss solutions are not actually the global optimum. 

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

Solving for pressureloss of 38 while minimizing cost
Route (nodes): 0 3 10 18 26 34 41 51 56 60 
Route (strip,index): (0,0) (1,2) (2,3) (3,5) (4,5) (5,5) (6,3) (7,4) (8,2) (9,0) 
Cumulative cost: 0 36762.8 48512.4 64013 90627 119394 154161 180341 200783 228026 
Cumulative length: 0 5406.3 7301.39 9410.31 13031.3 16945.1 22057.9 25137.9 27919.2 31925.5 
Cumulative hills: 0 0 0 0 0 0 5 5 5 5 
Cumulative pressureloss: 0 6.40938 8.65608 11.1563 15.4491 20.0891 26.44 30.0915 33.3888 38.1384 

Solving for pressureloss of 39 while minimizing cost
Route (nodes): 0 3 10 18 26 34 41 51 57 60 
Route (strip,index): (0,0) (1,2) (2,3) (3,5) (4,5) (5,5) (6,3) (7,4) (8,3) (9,0) 
Cumulative cost: 0 36762.8 48512.4 64013 90627 119394 154161 180341 198726 227317 
Cumulative length: 0 5406.3 7301.39 9410.31 13031.3 16945.1 22057.9 25137.9 27639.3 31843.8 
Cumulative hills: 0 0 0 0 0 0 5 5 25 25 
Cumulative pressureloss: 0 6.40938 8.65608 11.1563 15.4491 20.0891 26.44 30.0915 34.2152 39.1998 

Solving for pressureloss of 40 while minimizing cost
Route (nodes): 0 3 8 14 21 31 41 51 55 60 
Route (strip,index): (0,0) (1,2) (2,1) (3,1) (4,0) (5,2) (6,3) (7,4) (8,1) (9,0) 
Cumulative cost: 0 36762.8 54985.2 89299.7 113893 138637 158095 184275 208301 234295 
Cumulative length: 0 5406.3 8345.38 13014 16630.6 19541.7 22403.3 25483.2 28752.1 32574.7 
Cumulative hills: 0 0 0 0 3 3 28 28 28 28 
Cumulative pressureloss: 0 6.40938 9.89378 15.4286 19.89 23.3411 28.1813 31.8328 35.7081 40.24 

Solving for pressureloss of 41 while minimizing cost
Route (nodes): 0 3 10 18 26 34 41 52 58 60 
Route (strip,index): (0,0) (1,2) (2,3) (3,5) (4,5) (5,5) (6,3) (7,5) (8,4) (9,0) 
Cumulative cost: 0 36762.8 48512.4 64013 90627 119394 154161 182150 200619 231337 
Cumulative length: 0 5406.3 7301.39 9410.31 13031.3 16945.1 22057.9 25150.6 27866.6 32384 
Cumulative hills: 0 0 0 0 0 0 5 5 50 50 
Cumulative pressureloss: 0 6.40938 8.65608 11.1563 15.4491 20.0891 26.44 30.1066 35.9323 41.2879 

Solving for pressureloss of 42 while minimizing cost
Route (nodes): 0 3 10 18 26 33 40 52 58 60 
Route (strip,index): (0,0) (1,2) (2,3) (3,5) (4,5) (5,4) (6,2) (7,5) (8,4) (9,0) 
Cumulative cost: 0 36762.8 48512.4 64013 90627 126921 161930 191217 209685 240404 
Cumulative length: 0 5406.3 7301.39 9410.31 13031.3 17969.2 22732.4 25968.4 28684.5 33201.8 
Cumulative hills: 0 0 0 0 0 0 0 0 45 45 
Cumulative pressureloss: 0 6.40938 8.65608 11.1563 15.4491 21.3032 26.9502 30.7866 36.6124 41.9679 

Solving for pressureloss of 43 while minimizing cost
Route (nodes): 0 3 10 18 26 34 41 51 59 60 
Route (strip,index): (0,0) (1,2) (2,3) (3,5) (4,5) (5,5) (6,3) (7,4) (8,5) (9,0) 
Cumulative cost: 0 36762.8 48512.4 64013 90627 119394 154161 180341 195024 229825 
Cumulative length: 0 5406.3 7301.39 9410.31 13031.3 16945.1 22057.9 25137.9 27297.2 32415 
Cumulative hills: 0 0 0 0 0 0 5 5 85 85 
Cumulative pressureloss: 0 6.40938 8.65608 11.1563 15.4491 20.0891 26.44 30.0915 37.284 43.3514 

```