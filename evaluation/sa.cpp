#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <string>
#include <vector>
#include <math.h>
#include <unordered_set>

using namespace std;

#define n_phases 6
#define phase_n_stones 10
#define n_patterns 11
#define n_dense2 2
#define n_add_input 3
#define n_add_dense1 8
#define n_all_input 30
#define max_canput 40
#define max_surround 80
#define max_canput_2 20
#define max_surround_2 40
#define max_evaluate_idx 64000 //59049

#define p31 3
#define p32 9
#define p33 27
#define p34 81
#define p35 243
#define p36 729
#define p37 2187
#define p38 6561
#define p39 19683
#define p310 59049

#define sc_w 6400
#define step 100

const int pattern_sizes[n_patterns] = {8, 8, 8, 5, 6, 7, 8, 10, 10, 10, 10};
const int eval_sizes[n_patterns + 1] = {p38, p38, p38, p35, p36, p37, p38, p310, p310, p310, p310, max_canput * max_surround_2 * max_surround_2};
int eval_arr[n_phases][2][n_patterns + 1][max_evaluate_idx];
vector<vector<vector<vector<int>>>> test_data;
vector<vector<vector<double>>> test_labels;
double scores[n_phases][2];
vector<int> test_memo[n_phases][2][n_patterns + 1][max_evaluate_idx];
vector<double> test_scores[n_phases][2];
//vector<double> test_memo_scores[n_phases][2][n_patterns + 1][max_evaluate_idx];
unordered_set<int> used_idxes[n_phases][2][n_patterns + 1];
vector<int> used_idxes_vector[n_phases][2][n_patterns + 1];


inline unsigned long long tim(){
    return chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
}

int xorx=123456789, xory=362436069, xorz=521288629, xorw=88675123;
inline double myrandom(){
    int t = (xorx^(xorx<<11));
    xorx = xory;
    xory = xorz;
    xorz = xorw;
    xorw = (xorw^(xorw>>19))^(t^(t>>8));
    return (double)(xorw) / 2147483648.0;
}

inline int myrandrange(int s, int e){
    return s + (int)(myrandom() * (e - s));
}

#define time_num_step 10000
#define time_step_width (1.0 / time_num_step)
#define prob_num_step 1000000
#define prob_step_width (100.0 / prob_num_step)
double start_temp = 0.00000001;
double end_temp = 0.00000000001;
double temperature_arr[time_num_step];
double prob_arr[prob_num_step];

double temperature_x(double x){
    return pow(start_temp, 1 - x) * pow(end_temp, x);
    //return start_temp + (end_temp - start_temp) * x;
}

void anneal_init(){
    double x;
    for (int idx = 0; idx < time_num_step; idx++){
        x = time_step_width * idx;
        temperature_arr[idx] = temperature_x(x);
    }
    for (int idx = 0; idx < prob_num_step; idx++){
        x = -prob_step_width * idx;
        prob_arr[idx] = exp(x);
    }
}

double calc_temperature(double strt, double now, double tl){
    return temperature_x((now - strt) / tl);
}

double prob(double p_score, double n_score, double strt, double now, double tl){
    double dis = p_score - n_score;
    if (dis >= 0)
        return 1.0;
    return exp(dis / calc_temperature(strt, now, tl));
}

double prob_fast(double p_score, double n_score, double strt, double now, double tl){
    double dis = p_score - n_score;
    if (dis >= 0)
        return 1.0;
    int temperature_idx = (int)((now - strt) / tl / time_step_width);
    int prob_idx = (int)min((double)(prob_num_step - 1), -dis / temperature_arr[temperature_idx] / prob_step_width);
    return prob_arr[prob_idx];
}

void input_param(){
    ifstream ifs("f_param.txt");
    if (ifs.fail()){
        cerr << "evaluation file not exist" << endl;
        exit(1);
    }
    string line;
    int t =0;
    int phase_idx, player_idx, pattern_idx, pattern_elem, dense_idx, canput, sur0, sur1, i, j, k;
    for (phase_idx = 0; phase_idx < n_phases; ++phase_idx){
        for (player_idx = 0; player_idx < 2; ++player_idx){
            cerr << "=";
            for (pattern_idx = 0; pattern_idx < n_patterns + 1; ++pattern_idx){
                for (pattern_elem = 0; pattern_elem < eval_sizes[pattern_idx]; ++pattern_elem){
                    ++t;
                    getline(ifs, line);
                    eval_arr[phase_idx][player_idx][pattern_idx][pattern_elem] = stoi(line);
                }
            }
        }
    }
    cerr << t << endl;
}

inline int calc_add_idx(vector<int> arr){
    //return arr[42] * max_surround * max_surround + arr[43] * max_surround + arr[44];
    //return arr[42] / 2 * max_surround_2 * max_surround_2 + arr[43] / 2 * max_surround_2 + arr[44] / 2;
    return arr[42] * max_surround_2 * max_surround_2 + arr[43] / 2 * max_surround_2 + arr[44] / 2;
}

void input_test_data(){
    int i, j;
    for (i = 0; i < n_phases; ++i){
        vector<vector<vector<int>>> tmp_data;
        test_data.push_back(tmp_data);
        vector<vector<double>> tmp_data3;
        test_labels.push_back(tmp_data3);
        for (j = 0; j < 2; ++j){
            vector<vector<int>> tmp_data2;
            test_data[i].push_back(tmp_data2);
            vector<double> tmp_data4;
            test_labels[i].push_back(tmp_data4);
        }
    }
    ifstream ifs("big_data.txt");
    if (ifs.fail()){
        cerr << "evaluation file not exist" << endl;
        exit(1);
    }
    string line;
    int phase, player, score;
    int idxes[45];
    int t = 0;
    //for (i = 0; i < 220000; ++i)
    //    getline(ifs, line);
    int nums[n_phases][2];
    for (i = 0; i < n_phases; ++i){
        for (j = 0; j < 2; ++j)
            nums[i][j] = 0;
    }
    const int pattern_nums[42] = {
        0, 0, 0, 0,
        1, 1, 1, 1,
        2, 2, 2, 2,
        3, 3, 3, 3,
        4, 4, 4, 4,
        5, 5, 5, 5,
        6, 6,
        7, 7, 7, 7,
        8, 8, 8, 8,
        9, 9, 9, 9,
        10, 10, 10, 10
    };
    while (getline(ifs, line) && t < 200000000){
        ++t;
        istringstream iss(line);
        iss >> phase;
        iss >> player;
        for (i = 0; i < 45; ++i)
            iss >> idxes[i];
        iss >> score;
        if (player == 1)
            score = -score;
        vector<int> tmp;
        //cerr << phase << " " << player << " " << score << endl;
        for (i = 0; i < 45; ++i)
            tmp.push_back(idxes[i]);
        for (i = 0; i < 42; ++i)
            used_idxes[phase][player][pattern_nums[i]].emplace(tmp[i]);
        used_idxes[phase][player][11].emplace(calc_add_idx(tmp));
        test_data[phase][player].push_back(tmp);
        test_labels[phase][player].push_back(score * step);
        for (i = 0; i < 42; ++i)
            test_memo[phase][player][pattern_nums[i]][tmp[i]].push_back(nums[phase][player]);
        test_memo[phase][player][11][calc_add_idx(tmp)].push_back(nums[phase][player]);
        test_scores[phase][player].push_back(0);
        ++nums[phase][player];
    }
    cerr << t << endl;
    cerr << "loaded data" << endl;
    for (phase = 0; phase < n_phases; ++phase){
        for (player = 0; player < 2; ++player){
            for (i = 0; i < n_patterns + 1; ++i){
                for (auto elem: used_idxes[phase][player][i])
                    used_idxes_vector[phase][player][i].push_back(elem);
            }
        }
    }

    for (i = 0; i < n_phases; ++i){
        for (j = 0; j < 2; ++j)
            cerr << test_labels[i][j].size() << " ";
        cerr << endl;
    }

    int u = 0;
    for (i = 0; i < n_patterns + 1; ++i)
        u += eval_sizes[i];
    cerr << u << endl;
    for (phase = 0; phase < n_phases; ++phase){
        for (player = 0; player < 2; ++player){
            u = 0;
            for (i = 0; i < n_patterns + 1; ++i){
                u += (int)used_idxes[phase][player][i].size();
            }
            cerr << u << " ";
        }
        cerr << endl;
    }
}

void output_param(){
    int phase_idx, player_idx, pattern_idx, pattern_elem, dense_idx, canput, sur0, sur1, i, j;
    for (phase_idx = 0; phase_idx < n_phases; ++phase_idx){
        for (player_idx = 0; player_idx < 2; ++player_idx){
            cerr << "=";
            for (pattern_idx = 0; pattern_idx < n_patterns + 1; ++pattern_idx){
                for (pattern_elem = 0; pattern_elem < eval_sizes[pattern_idx]; ++pattern_elem){
                    cout << eval_arr[phase_idx][player_idx][pattern_idx][pattern_elem] << endl;
                }
            }
        }
    }
    cerr << endl;
}

inline double loss(int x, int siz){
    //double sq_size = sqrt((double)siz);
    //double tmp = (double)x / sq_size;
    return (double)x / (double)siz * (double)x;
}

inline int calc_score(int phase, int player, int i){
    return
        eval_arr[phase][player][0][test_data[phase][player][i][0]] + 
        eval_arr[phase][player][0][test_data[phase][player][i][1]] + 
        eval_arr[phase][player][0][test_data[phase][player][i][2]] + 
        eval_arr[phase][player][0][test_data[phase][player][i][3]] + 
        eval_arr[phase][player][1][test_data[phase][player][i][4]] + 
        eval_arr[phase][player][1][test_data[phase][player][i][5]] + 
        eval_arr[phase][player][1][test_data[phase][player][i][6]] + 
        eval_arr[phase][player][1][test_data[phase][player][i][7]] + 
        eval_arr[phase][player][2][test_data[phase][player][i][8]] + 
        eval_arr[phase][player][2][test_data[phase][player][i][9]] + 
        eval_arr[phase][player][2][test_data[phase][player][i][10]] + 
        eval_arr[phase][player][2][test_data[phase][player][i][11]] + 
        eval_arr[phase][player][3][test_data[phase][player][i][12]] + 
        eval_arr[phase][player][3][test_data[phase][player][i][13]] + 
        eval_arr[phase][player][3][test_data[phase][player][i][14]] + 
        eval_arr[phase][player][3][test_data[phase][player][i][15]] + 
        eval_arr[phase][player][4][test_data[phase][player][i][16]] + 
        eval_arr[phase][player][4][test_data[phase][player][i][17]] + 
        eval_arr[phase][player][4][test_data[phase][player][i][18]] + 
        eval_arr[phase][player][4][test_data[phase][player][i][19]] + 
        eval_arr[phase][player][5][test_data[phase][player][i][20]] + 
        eval_arr[phase][player][5][test_data[phase][player][i][21]] + 
        eval_arr[phase][player][5][test_data[phase][player][i][22]] + 
        eval_arr[phase][player][5][test_data[phase][player][i][23]] + 
        eval_arr[phase][player][6][test_data[phase][player][i][24]] + 
        eval_arr[phase][player][6][test_data[phase][player][i][25]] + 
        eval_arr[phase][player][7][test_data[phase][player][i][26]] + 
        eval_arr[phase][player][7][test_data[phase][player][i][27]] + 
        eval_arr[phase][player][7][test_data[phase][player][i][28]] + 
        eval_arr[phase][player][7][test_data[phase][player][i][29]] + 
        eval_arr[phase][player][8][test_data[phase][player][i][30]] + 
        eval_arr[phase][player][8][test_data[phase][player][i][31]] + 
        eval_arr[phase][player][8][test_data[phase][player][i][32]] + 
        eval_arr[phase][player][8][test_data[phase][player][i][33]] + 
        eval_arr[phase][player][9][test_data[phase][player][i][34]] + 
        eval_arr[phase][player][9][test_data[phase][player][i][35]] + 
        eval_arr[phase][player][9][test_data[phase][player][i][36]] + 
        eval_arr[phase][player][9][test_data[phase][player][i][37]] + 
        eval_arr[phase][player][10][test_data[phase][player][i][38]] + 
        eval_arr[phase][player][10][test_data[phase][player][i][39]] + 
        eval_arr[phase][player][10][test_data[phase][player][i][40]] + 
        eval_arr[phase][player][10][test_data[phase][player][i][41]] + 
        eval_arr[phase][player][11][calc_add_idx(test_data[phase][player][i])];
}

inline double first_scoring(){
    int phase, player, i, j, score;
    double avg_score, res = 0.0, err;
    for (phase = 0; phase < n_phases; ++phase){
        for (player = 0; player < 2; ++player){
            avg_score = 0.0;
            for (i = 0; i < test_data[phase][player].size(); ++i){
                score = calc_score(phase, player, i);
                //cerr << eval_arr[phase][player][11][calc_add_idx(test_data[phase][player][i])] << endl;
                //cerr << score << endl;
                //exit(0);
                err = loss(test_labels[phase][player][i] - score, test_data[phase][player].size());
                avg_score += err;
                test_scores[phase][player][i] = err;
            }
            scores[phase][player] = avg_score;
            res += avg_score / n_phases / 2;
        }
    }
    return res;
}

inline double scoring(int phase, int player, int pattern, int idx){
    int i, j, score;
    double avg_score, res = 0.0, err;
    int data_size = test_data[phase][player].size();
    avg_score = scores[phase][player];
    for (i = 0; i < test_memo[phase][player][pattern][idx].size(); ++i){
        avg_score -= test_scores[phase][player][test_memo[phase][player][pattern][idx][i]];
        score = calc_score(phase, player, test_memo[phase][player][pattern][idx][i]);
        err = loss(test_labels[phase][player][test_memo[phase][player][pattern][idx][i]] - score, data_size);
        test_scores[phase][player][test_memo[phase][player][pattern][idx][i]] = err;
        avg_score += err;
    }
    scores[phase][player] = avg_score;
    for (i = 0; i < n_phases; ++i){
        for (j = 0; j < 2; ++j)
            res += scores[i][j] / n_phases / 2;
    }
    return res;
}

inline void scoring_mae(){
    int phase, player, i, j, score;
    double avg_score, res = 0.0;
    for (phase = 0; phase < n_phases; ++phase){
        for (player = 0; player < 2; ++player){
            avg_score = 0;
            for (i = 0; i < test_data[phase][player].size(); ++i){
                score = calc_score(phase, player, i);
                avg_score += fabs(test_labels[phase][player][i] - (double)score) / test_data[phase][player].size();
            }
            cerr << avg_score << " ";
            res += avg_score / n_phases / 2;
        }
    }
    cerr << " " << res << "                                     ";
}

void sa(unsigned long long tl){
    unsigned long long strt = tim(), now = tim();
    double score = first_scoring(), n_score;
    int phase, player, pattern, idx, f_val;
    int t = 0, u = 0;
    cerr << score << " ";
    scoring_mae();
    cerr << endl;
    for (;;){
        ++t;
        phase = myrandrange(0, n_phases);
        player = myrandrange(0, 2);
        pattern = myrandrange(0, n_patterns + 1);
        idx = used_idxes_vector[phase][player][pattern][myrandrange(0, (int)used_idxes[phase][player][pattern].size())];
        f_val = eval_arr[phase][player][pattern][idx];
        eval_arr[phase][player][pattern][idx] += myrandrange(-50, 51);
        n_score = scoring(phase, player, pattern, idx);
        if (n_score < score){
            score = n_score;
            ++u;
        } else{
            eval_arr[phase][player][pattern][idx] = f_val;
            scoring(phase, player, pattern, idx);
        }
        if ((t & 0b11111111111) == 0){
            now = tim();
            if (now - strt > tl)
                break;
            cerr << '\r' << (int)((double)(now - strt) / tl * 1000) << " " << t << " " << u << " ";
            cerr << score << " ";
            scoring_mae();
        }
    }
    cerr << '\r';
    cerr << score << " ";
    scoring_mae();
    cerr << endl;
    cerr << first_scoring() << endl;
    cerr << t << " " << u << endl;
}

int main(){
    input_param();
    input_test_data();

    int hour = 9;
    int minute = 0;
    minute += hour * 60;

    sa(minute * 60 * 1000);
    output_param();

    return 0;
}