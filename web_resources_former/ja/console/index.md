# Egaroucid for Console

ダウンロードまたは手元でのコンパイルが必要です。



## ダウンロード

以下から自分の環境に合ったものをダウンロードし、任意の場所に解凍してください。実行ファイル(<code>Egaroucid_for_console.exe</code>や<code>Egaroucid_for_console.out</code>)をコンソールから実行すると起動します。



Egaroucidはx64のAVX2バージョンに最適化して作っていますが、こちらは概ね2013年以降のCPUでないと動作しません。その場合にはGenericバージョンを使用してください。



なお、ARMおよびARM64バージョンは動作確認ができていませんので、問題がありましたら[こちらのフォーム](https://docs.google.com/forms/d/e/1FAIpQLSd6ML1T1fc707luPEefBXuImMnlM9cQP8j-YHKiSyFoS-8rmQ/viewform?usp=sf_link)よりご報告ください。



<table>
    <tr>
        <td>OS</td>
        <td>CPU</td>
        <td>追加要件</td>
        <td>リリース日</td>
        <td>ダウンロード</td>
    </tr>
    <tr>
        <td>Windows</td>
        <td>x64(標準)</td>
        <td>AVX2(標準)</td>
        <td>2022/10/10</td>
        <td>[Egaroucid 6.0.0](https://github.com/Nyanyan/Egaroucid/releases/download/v6.0.0/Egaroucid_6_0_0_setup_Windows.exe)</td>
    </tr>
</table>

過去のバージョンや各バージョンのリリースノートは[GitHubのリリース](https://github.com/Nyanyan/Egaroucid/releases)からご覧ください。



## 手元でのコンパイル

**MacOSでの動作確認はまだできていません。**

<ul>
    <li><code>g++</code>コマンドが必要
        <ul>
            <li>Windowsではバージョン12.2.0で動作確認済</li>
            <li>Ubuntuではバージョン11.3.0で動作確認済</li>
        </ul>
    </li>
    <li>C++17の機能が必要</li>
</ul>

コードを入手します。



<code>$ git clone git@github.com:Nyanyan/Egaroucid.git</code>



ディレクトリを移動します。



<code>$ cd Egaroucid/src</code>



<code>g++</code>コマンドにてコンパイルします。出力ファイルは任意の名前で構いません。



<code>$ g++ -O2 Egaroucid_console.cpp -o Egaroucid_for_console.exe -mtune=native -march=native -mfpmath=both -pthread -std=c++17 -Wall -Wextra
</code>



実行します。



<code>$ Egaroucid_for_console.exe</code>



## 使い方

<code>$ Egaroucid_for_console.exe -help</code>



を実行すると使えるオプションやコマンドが確認できます。



## Go Text Protocol (GTP)対応

GTPは囲碁向けに作られた通信プロトコルですが、これを用いてオセロが使えるソフトウェアがいくつか存在します。Egaroucid for ConsoleはGTPに対応していますので、これらのソフトウェアから呼び出すことが可能です。



GTPコマンドを使う場合には



<code>$ Egaroucid_for_console.exe -gtp</code>



を実行してください。



WindowsにてGoGuiを用いた動作確認を、UbuntuにてQuarryを用いた動作確認を行いました。



### GoGui

GoGuiでの動作画面は以下のようになります。

<div class="centering_box">
    <img class="pic2" src="img/gogui_with_egaroucid.png">
</div>

まず、<code>プログラム>新規プログラム</code>からEgaroucidを登録します。その際、コマンドには<code>-gtp</code>を追加し、ワーキングディレクトリは<code>Egaroucid/src</code>にしてください。

<div class="centering_box">
    <img class="pic2" src="img/gogui_new_program.png">
    <img class="pic2" src="img/gogui_new_program2.png">
</div>

そして、<code>プログラム>プログラムを起動</code>から追加したEgaroucidを選択することでEgaroucidを起動できます。

<div class="centering_box">
    <img class="pic2" src="img/gogui_launch.png">
</div>

GoGuiにおいては、盤面の行が上下反転しているので、<code>表示>Board Orientation>Flip Horizontally</code>を選択すると修正できます。

<div class="centering_box">
    <img class="pic2" src="img/gogui_orientation.png">
</div>



### Quarry

Quarryでの動作画面は以下のようになります。

<div class="centering_box">
    <img class="pic2" src="img/quarry_with_egaroucid.png">
</div>

<code>New Game</code>や<code>Preferences</code>から<code>Manage Engine List</code>を開き、Egaroucidを追加します。このとき、コマンドに<code>-gtp</code>を追加してください。



手番を選択してゲームを開始するとEgaroucidが起動します。

<div class="centering_box">
    <img class="pic2" src="img/quarry_setting1.png">
    <img class="pic2" src="img/quarry_setting2.png">
</div>





## フォルダ構成

Egaroucid for Consoleはいくつかの外部ファイルを必要とします。上記の方法でダウンロードやコンパイルした場合には特にエラーなく動きますが、もし動かなくなった場合にはフォルダ構成を確認してください。

<ul>
    <li>Egaroucid_for_console.exe</li>
    <li>resources
        <ul>
            <li>hash (ハッシュファイル なくても動きます)
                <ul>
                    <li>hash23.eghs</li>
                    <li>hash24.eghs</li>
                    <li>hash25.eghs</li>
                    <li>hash26.eghs</li>
                    <li>hash27.eghs</li>
                </ul>
            </li>
            <li>book.egbk (bookファイル)</li>
            <li>eval.egev (評価ファイル)</li>
        </ul>
    </li>
</ul>

