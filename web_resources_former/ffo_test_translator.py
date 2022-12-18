import pyperclip

head = '''<table>
<tr>
<td>番号</td>
<td>深さ</td>
<td>最善手</td>
<td>手番の評価値</td>
<td>探索時間(秒)</td>
<td>訪問ノード数</td>
<td>NPS</td>
</tr>
'''

'''
#5.4.0
idxes = [0, 3, 13, 15, 6, 17, 19]
need_coord_translate = 13
'''
'''
#5.5.0
idxes = [0, 2, 6, 4, 11, 8, 16]
need_coord_translate = -1
'''

#5.7.0以降
idxes = [0, 2, 6, 4, 10, 8, 12]
need_coord_translate = -1

def coord_translator(cell):
    cell = 63 - int(cell)
    y = cell // 8
    x = cell % 8
    return chr(ord('a') + x) + str(y + 1)

whole_time = 0
whole_nodes = 0
res = head
while True:
    data = input().split()
    try:
        use_data = []
        for idx in idxes:
            if idx == need_coord_translate:
                use_data.append(coord_translator(data[idx]))
            else:
                if idx == idxes[4]:
                    use_data.append(str(round(int(data[idx]) / 1000, 3)))
                else:
                    use_data.append(data[idx])
            if idx == idxes[4]:
                whole_time += int(data[idx]) / 1000
            elif idx == idxes[5]:
                whole_nodes += int(data[idx])
        res += '<tr>\n'
        for use_datum in use_data:
            res += '<td>'
            res += use_datum
            res += '</td>\n'
        res += '</tr>\n'
    except:
        break
res += '''<tr>
<td>全体</td>
<td>-</td>
<td>-</td>
<td>-</td>
<td>''' + str(round(whole_time, 3)) + '''</td>
<td>''' + str(whole_nodes) + '''</td>
<td>''' + str(round(whole_nodes / whole_time)) + '''</td>
</tr>
'''
res += '</table>'

#print(res)

pyperclip.copy(res)