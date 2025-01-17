# Egaroucid 6.2.0 ベンチマーク

## The FFO endgame test suite

<a href="http://radagast.se/othello/ffotest.html" target="_blank" el=”noopener noreferrer”>The FFO endgame test suite</a>はオセロAIの終盤探索力の指標として広く使われるベンチマークです。各テストケースを完全読みし、最善手を計算します。探索時間と訪問ノード数を指標に性能を評価します。NPSはNodes Per Secondの略で、1秒あたりの訪問ノード数を表します。

使用CPUはCore i9-11900Kです。

### Egaroucid for Console 6.2.0 Windows x64 SIMD

<table>
<tr>
<th>番号</th>
<th>深さ</th>
<th>最善手</th>
<th>手番の評価値</th>
<th>探索時間(秒)</th>
<th>訪問ノード数</th>
<th>NPS</th>
</tr>
<tr>
<td>#40</td>
<td>20@100%</td>
<td>a2</td>
<td>+38</td>
<td>0.079</td>
<td>20675942</td>
<td>261720784</td>
</tr>
<tr>
<td>#41</td>
<td>22@100%</td>
<td>h4</td>
<td>+0</td>
<td>0.156</td>
<td>28392068</td>
<td>182000435</td>
</tr>
<tr>
<td>#42</td>
<td>22@100%</td>
<td>g2</td>
<td>+6</td>
<td>0.208</td>
<td>49139741</td>
<td>236248754</td>
</tr>
<tr>
<td>#43</td>
<td>23@100%</td>
<td>g3</td>
<td>-12</td>
<td>0.401</td>
<td>86360041</td>
<td>215361698</td>
</tr>
<tr>
<td>#44</td>
<td>23@100%</td>
<td>b8</td>
<td>-14</td>
<td>0.176</td>
<td>28565965</td>
<td>162306619</td>
</tr>
<tr>
<td>#45</td>
<td>24@100%</td>
<td>b2</td>
<td>+6</td>
<td>1.352</td>
<td>476921635</td>
<td>352752688</td>
</tr>
<tr>
<td>#46</td>
<td>24@100%</td>
<td>b3</td>
<td>-8</td>
<td>0.383</td>
<td>83495062</td>
<td>218002772</td>
</tr>
<tr>
<td>#47</td>
<td>25@100%</td>
<td>g2</td>
<td>+4</td>
<td>0.162</td>
<td>22332907</td>
<td>137857450</td>
</tr>
<tr>
<td>#48</td>
<td>25@100%</td>
<td>f6</td>
<td>+28</td>
<td>0.853</td>
<td>150805811</td>
<td>176794620</td>
</tr>
<tr>
<td>#49</td>
<td>26@100%</td>
<td>e1</td>
<td>+16</td>
<td>1.266</td>
<td>314363012</td>
<td>248312015</td>
</tr>
<tr>
<td>#50</td>
<td>26@100%</td>
<td>d8</td>
<td>+10</td>
<td>5.923</td>
<td>1485495809</td>
<td>250801250</td>
</tr>
<tr>
<td>#51</td>
<td>27@100%</td>
<td>e2</td>
<td>+6</td>
<td>2.485</td>
<td>697258715</td>
<td>280587008</td>
</tr>
<tr>
<td>#52</td>
<td>27@100%</td>
<td>a3</td>
<td>+0</td>
<td>2.061</td>
<td>505277066</td>
<td>245161118</td>
</tr>
<tr>
<td>#53</td>
<td>28@100%</td>
<td>d8</td>
<td>-2</td>
<td>10.488</td>
<td>3319261482</td>
<td>316481834</td>
</tr>
<tr>
<td>#54</td>
<td>28@100%</td>
<td>c7</td>
<td>-2</td>
<td>15.945</td>
<td>4960272938</td>
<td>311086418</td>
</tr>
<tr>
<td>#55</td>
<td>29@100%</td>
<td>g6</td>
<td>+0</td>
<td>42.92</td>
<td>11362599782</td>
<td>264739044</td>
</tr>
<tr>
<td>#56</td>
<td>29@100%</td>
<td>h5</td>
<td>+2</td>
<td>5.791</td>
<td>960708045</td>
<td>165896744</td>
</tr>
<tr>
<td>#57</td>
<td>30@100%</td>
<td>a6</td>
<td>-10</td>
<td>11.061</td>
<td>2827877585</td>
<td>255662018</td>
</tr>
<tr>
<td>#58</td>
<td>30@100%</td>
<td>g1</td>
<td>+4</td>
<td>6.609</td>
<td>1364499647</td>
<td>206460833</td>
</tr>
<tr>
<td>#59</td>
<td>34@100%</td>
<td>e8</td>
<td>+64</td>
<td>0.221</td>
<td>3362984</td>
<td>15217122</td>
</tr>
<tr>
<td>全体</td>
<td>-</td>
<td>-</td>
<td>-</td>
<td>108.54</td>
<td>28747666237</td>
<td>264857806</td>
</tr>
</table>







## Edax4.4との対戦

現状世界最強とも言われるオセロAI、<a href="https://github.com/abulmo/edax-reversi" target="_blank" el=”noopener noreferrer”>Edax 4.4</a>との対戦結果です。

初手からの対戦では同じ進行ばかりになって評価関数の強さは計測できないので、初期局面から8手進めた互角に近いと言われる状態から打たせて勝敗を数えました。このとき、同じ進行に対して両者が必ず先手と後手の双方を1回ずつ持つようにしました。こうすることで、両者の強さが全く同じであれば勝率は50%となるはずです。

テストには<a href="https://berg.earthlingz.de/xot/index.php" target="_blank" el=”noopener noreferrer”>XOT</a>に収録されている局面を使用しました。

bookは双方未使用です。

Egaroucid勝率が0.5を上回っていればEgaroucidがEdaxに勝ち越しています。

### Egaroucidが黒番

<table>
<tr>
<th>レベル</th>
<th>Egaroucid勝ち</th>
<th>引分</th>
<th>Edax勝ち</th>
<th>Egaroucid勝率</th>
</tr>
<tr>
<td>1</td>
<td>615</td>
<td>23</td>
<td>362</td>
<td>0.63</td>
</tr>
<tr>
<td>5</td>
<td>566</td>
<td>48</td>
<td>386</td>
<td>0.59</td>
</tr>
<tr>
<td>10</td>
<td>589</td>
<td>115</td>
<td>296</td>
<td>0.67</td>
</tr>
<tr>
<td>15</td>
<td>114</td>
<td>31</td>
<td>55</td>
<td>0.67</td>
</tr>
</table>







### Egaroucidが白番

<table>
<tr>
<th>レベル</th>
<th>Egaroucid勝ち</th>
<th>引分</th>
<th>Edax勝ち</th>
<th>Egaroucid勝率</th>
</tr>
<tr>
<td>1</td>
<td>619</td>
<td>26</td>
<td>355</td>
<td>0.64</td>
</tr>
<tr>
<td>5</td>
<td>535</td>
<td>44</td>
<td>421</td>
<td>0.56</td>
</tr>
<tr>
<td>10</td>
<td>451</td>
<td>119</td>
<td>430</td>
<td>0.51</td>
</tr>
<tr>
<td>15</td>
<td>108</td>
<td>36</td>
<td>56</td>
<td>0.66</td>
</tr>
</table>



