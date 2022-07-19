import matplotlib.pyplot as plt

s0 = '''59	733 mae 0.00148207 mse 0.00747858 n_data 1165938 n_param 528065 n_used_params 37671
58	670 mae 1.72615 mse 6.06521 n_data 1169517 n_param 528065 n_used_params 88370
57	653 mae 2.46903 mse 11.2146 n_data 1173224 n_param 528065 n_used_params 140766
56	654 mae 2.97058 mse 15.7668 n_data 1176515 n_param 528065 n_used_params 183294
55	636 mae 3.34599 mse 19.6235 n_data 1179408 n_param 528065 n_used_params 213017
54	634 mae 3.59767 mse 22.5228 n_data 1181643 n_param 528065 n_used_params 231032
53	651 mae 3.80313 mse 24.9832 n_data 1183383 n_param 528065 n_used_params 241441
52	650 mae 3.9298 mse 26.5923 n_data 1184759 n_param 528065 n_used_params 247185
51	615 mae 4.02845 mse 27.8628 n_data 1185784 n_param 528065 n_used_params 250782
50	525 mae 4.19917 mse 30.0624 n_data 1286683 n_param 528065 n_used_params 287935
49	524 mae 4.23916 mse 30.6137 n_data 1287980 n_param 528065 n_used_params 289377
48	514 mae 4.26203 mse 30.9882 n_data 1289061 n_param 528065 n_used_params 290352
47	491 mae 4.26701 mse 30.9948 n_data 1290304 n_param 528065 n_used_params 290265
46	508 mae 4.25308 mse 30.8069 n_data 1291275 n_param 528065 n_used_params 290330
45	550 mae 4.15662 mse 29.455 n_data 1227594 n_param 528065 n_used_params 271288
44	562 mae 4.1044 mse 28.7446 n_data 1193611 n_param 528065 n_used_params 246921
43	561 mae 4.07883 mse 28.4155 n_data 1195093 n_param 528065 n_used_params 245026
42	546 mae 4.06026 mse 28.2003 n_data 1196235 n_param 528065 n_used_params 243300
41	539 mae 4.03371 mse 27.919 n_data 1197384 n_param 528065 n_used_params 241383
40	559 mae 4.02612 mse 27.8746 n_data 1198325 n_param 528065 n_used_params 239618
39	552 mae 4.00605 mse 27.7505 n_data 1199109 n_param 528065 n_used_params 237650
38	559 mae 4.02561 mse 28.1186 n_data 1199715 n_param 528065 n_used_params 235696
37	564 mae 4.04156 mse 28.4715 n_data 1200216 n_param 528065 n_used_params 233115
36	571 mae 4.11399 mse 29.5397 n_data 1200593 n_param 528065 n_used_params 230682
35	588 mae 4.30888 mse 32.3274 n_data 1200918 n_param 528065 n_used_params 228331
34	589 mae 4.43721 mse 34.2522 n_data 1201170 n_param 528065 n_used_params 226294
33	586 mae 4.56017 mse 36.1269 n_data 1201349 n_param 528065 n_used_params 223857
32	590 mae 4.66557 mse 37.8001 n_data 1201500 n_param 528065 n_used_params 221612
31	634 mae 4.76428 mse 39.4352 n_data 1201600 n_param 528065 n_used_params 218624
30	645 mae 4.87246 mse 41.2529 n_data 1201640 n_param 528065 n_used_params 216133
29	646 mae 5.00876 mse 43.6035 n_data 1201671 n_param 528065 n_used_params 212875
28	656 mae 5.12748 mse 45.7825 n_data 1201677 n_param 528065 n_used_params 209744
27	657 mae 5.25139 mse 48.1207 n_data 1201679 n_param 528065 n_used_params 205564
26	670 mae 5.35998 mse 50.1345 n_data 1201680 n_param 528065 n_used_params 201941
25	676 mae 5.46805 mse 52.2879 n_data 1201684 n_param 528065 n_used_params 196749
24	684 mae 5.57723 mse 54.5016 n_data 1201689 n_param 528065 n_used_params 192171
23	700 mae 5.68088 mse 56.641 n_data 1201694 n_param 528065 n_used_params 185618
22	712 mae 5.77615 mse 58.7431 n_data 1201700 n_param 528065 n_used_params 179056
21	727 mae 5.889 mse 61.1443 n_data 1201704 n_param 528065 n_used_params 170939
20	751 mae 6.04731 mse 64.8435 n_data 1201759 n_param 528065 n_used_params 163472
19	2393 mae 6.36952 mse 73.6586 n_data 533337 n_param 528065 n_used_params 126414
18	2381 mae 6.89784 mse 89.7818 n_data 533350 n_param 528065 n_used_params 116919
17	2447 mae 7.71937 mse 115.229 n_data 533351 n_param 528065 n_used_params 105656
16	2492 mae 8.4651 mse 140.555 n_data 533371 n_param 528065 n_used_params 94777
15	2546 mae 9.46746 mse 174.525 n_data 533381 n_param 528065 n_used_params 83115
14	2574 mae 10.3309 mse 205.711 n_data 533399 n_param 528065 n_used_params 72059
13	2627 mae 11.4117 mse 243.333 n_data 533414 n_param 528065 n_used_params 60697
12	2673 mae 12.3503 mse 278.916 n_data 533433 n_param 528065 n_used_params 50074
11	2761 mae 13.4044 mse 317.776 n_data 533438 n_param 528065 n_used_params 39574
10	2817 mae 14.3434 mse 354.258 n_data 533457 n_param 528065 n_used_params 29422
9	2761 mae 12.8046 mse 294.112 n_data 586611 n_param 528065 n_used_params 18507'''

x0 = []
y0 = []
for line in s0.splitlines():
    line_split = line.split()
    #if int(line_split[0]) > 20:
    x0.append(int(line_split[0]))
    y0.append(float(line_split[3]))

s1 = '''29	314 600 345.584 1.34994
28	274 600 771.565 3.01393
27	254 600 950.331 3.71223
26	246 600 1041.44 4.06813
25	226 601 1105.64 4.31892
24	205 600 1128.64 4.40876
23	202 600 1123.77 4.38973
22	229 600 1068.41 4.17347
21	234 600 1042.92 4.07392
20	232 601 1025.86 4.00728
19	231 600 1015.66 3.96744
18	233 602 1023.72 3.99889
17	233 601 1086.98 4.24601
16	237 601 1143.84 4.46811
15	238 601 1196.85 4.6752
14	242 602 1269.22 4.95789
13	245 600 1338.92 5.23016
12	251 601 1402.53 5.47864
11	258 601 1465.15 5.72325
10	268 602 1532.73 5.98722
9  1 428 1694.84 6.62046
8  1 454 1786.29 6.97769
7  1 496 1864.17 7.28191
6  1 1092 1940.67 7.58073
5  1 1612 2224.65 8.69004    
4  1 1823 2324.23 9.07903
3  1 2291 2410.24 9.41499
2  1 2535 2468.24 9.64155
1  1 2793 2500.04 9.76579
0  1 385 2514.14 9.82084'''

x1 = []
y1 = []
for line in s1.splitlines():
    line_split = line.split()
    x1.append(int(line_split[0]) * 2)
    y1.append(float(line_split[4]))


plt.plot(x0, y0, label='data_0')
plt.plot(x1, y1, label='data_1')
plt.show()