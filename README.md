# pipelineDP
Dynamic Program for pipeline network routing. Based on a report by Uri Shamir (1969) and later published in 1971 as:

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

```