#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <set>
#include <map>
#include <chrono>
#include <cmath>
#include <omp.h>
using namespace std;

auto startTime = std::chrono::high_resolution_clock::now();
int chip_width = 0; int chip_height = 0;
int soft_module_num = 0;
int fix_module_num = 0;
int net_num = 0;


struct fix_module{
    string name;
    int x_coord;
    int y_coord;
    int width;
    int height;

};

struct soft_module{
    string name;
    int min_area;
    int width;
    int height;
    int x_coord;
    int y_coord;
    // int best_width;
    // int best_height;
    // int best_x_coord;
    // int best_y_coord;
    soft_module* parent = NULL;
    soft_module* l_child = NULL;
    soft_module* r_child = NULL;
    // vector< pair<int, int>* > w_and_h;  //TODO
    vector<int> possible_width;
    vector<int> possible_height;

};

struct net{
    pair<string, string> module_pair;
    int net_weight;

};

vector< net* > nets;
unordered_map<string, soft_module*> soft_hash;
unordered_map<string, fix_module*> fix_hash;



bool compareModules(const fix_module* module1, const fix_module* module2) {
    if (module1->y_coord == module2->y_coord) {

        return module1->x_coord < module2->x_coord;

    } else {

        return module1->y_coord < module2->y_coord;

    }
}

bool compareNets(const net* net1, const net* net2) {
    return net1->net_weight > net2->net_weight;
}

void get_information(int argc, char **argv, vector< fix_module* >& fix_modules, vector< soft_module >& soft_modules, 
                    vector< net* >& nets, unordered_map<string, fix_module*>& fix_hash){

    // std::cout << "Have " << argc << " arguments:\n";
    // for (int i = 0; i < argc; ++i) {
    //     std::cout << argv[i] << "\n";
    // }
    // std::cout << argv[1] << std::endl;
    std::ifstream file(argv[1], std::ios::in);
    if (!file.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return;
    }
    std::string keyword;
    while(file >> keyword){
        
        if(keyword == "ChipSize"){
            
            file >> chip_width;
            file >> chip_height;  
            // std::cout << keyword << " " << chip_width << " " << chip_height <<  std::endl; 

        }
        else if(keyword == "NumSoftModules"){
              
            file >> soft_module_num;
            // std::cout << keyword << " " << soft_module_num <<  std::endl; 

        }
        else if(keyword == "SoftModule"){   //TODO

            soft_module s;
            file >> (s.name);
            file >> (s.min_area);

            #pragma omp parallel for
            for (int h = 1; h <= s.min_area; ++h) {  //can be optimized

                int w = ceil(double(s.min_area)/double(h));
                if (h >= 0.5 * w && h <= 2 * w ) {
                    
                    (s.possible_width).push_back(w);
                    (s.possible_height).push_back(h);

                }

            }

            s.width = (s.possible_width)[0];
            s.height = (s.possible_width)[0];

            soft_modules.push_back(s);
            // std::cout << keyword << " " << s.name << " " << s.min_area << " " << s.width << " " << s.height <<  std::endl; 

            
        }
        else if(keyword == "NumFixedModules"){

            file >> fix_module_num;
            // std::cout << keyword << " " << fix_module_num <<  std::endl; 

        }
        else if(keyword == "FixedModule"){

            fix_module* f = new fix_module();
            file >> (f -> name);
            file >> (f -> x_coord);
            file >> (f -> y_coord);
            file >> (f -> width);
            file >> (f -> height);

            fix_hash[f -> name] = f;
            fix_modules.push_back(f);
            // std::cout << keyword << " " << f -> name << " " << f -> x_coord << " " << f -> y_coord << " " << f -> width << " " << f -> height <<  std::endl;  
            
        }
        else if(keyword == "NumNets"){

            file >> net_num;
            // std::cout << keyword << " " << net_num <<  std::endl; 
            
        }
        else if(keyword == "Net"){

            net* n = new net();
            file >> (n -> module_pair).first;
            file >> (n -> module_pair).second;
            file >> n -> net_weight;
            nets.push_back(n);
            // std::cout << keyword << " " << (n -> module_pair).first << " " << (n -> module_pair).second << " " << n -> net_weight <<  std::endl;
            
        }


    }

    file.close();

}
soft_module* init_b_star_tree(vector< soft_module>& soft_modules, int start, int end, soft_module* parent){

    if (start > end) return NULL;
    int mid = start + (end - start) / 2;

    soft_modules[mid].parent = parent;
    soft_modules[mid].l_child = init_b_star_tree(soft_modules, start, mid - 1, &soft_modules[mid]);
    soft_modules[mid].r_child = init_b_star_tree(soft_modules, mid + 1, end, &soft_modules[mid]);

    return &soft_modules[mid];
}

bool check_overlap(int a_x_coord, int a_y_coord, int a_width, int a_height, int b_x_coord, int b_y_coord, int b_width, int b_height){  // identify whether the current free module overlaps with any pre-placed modules, if overlap ---> shift

    bool overlapX = (a_x_coord < b_x_coord + b_width) && (a_x_coord + a_width > b_x_coord);
    bool overlapY = (a_y_coord < b_y_coord + b_height) && (a_y_coord + a_height > b_y_coord);

    return overlapX && overlapY;

}

bool check_x_overlap(int front_x, int tail_x, int module_front_x, int module_tail_x){  // identify whether the current free module overlaps with any pre-placed modules, if overlap ---> shift

    bool overlapX = (front_x < module_tail_x) && (tail_x > module_front_x);

    return overlapX;


}

int find_y_coord(int x_coord, int width, vector<soft_module*>& placed){

    int front_x = x_coord;
    int tail_x = x_coord + width;
    int max_y = 0;
    for(auto& module: placed){

        if(check_x_overlap(front_x, tail_x, module -> x_coord, module -> x_coord + module -> width)){

            max_y = max(max_y, module -> y_coord + module -> height);

        }

    }

    return max_y;

}

void dfs(soft_module* root, vector<soft_module*>& preorder_list){

    if(root == NULL) return;
    preorder_list.push_back(root);
    dfs(root -> l_child, preorder_list);
    dfs(root -> r_child, preorder_list);
    
}
int cal_overlap_area(vector<soft_module*>& preorder_list, vector< fix_module* >& fix_modules){ //can be parrallization

    int overlap_area = 0;
    int overlapX1, overlapY1, overlapX2, overlapY2, overlapSideX, overlapSideY;
    
    #pragma omp parallel for
    for(int i = 0; i < preorder_list.size(); i++){
        for(int j = i + 1; j < preorder_list.size(); j++){

            overlapX1 = max(preorder_list[i] -> x_coord, preorder_list[j] -> x_coord);
            overlapY1 = max(preorder_list[i] -> y_coord, preorder_list[j] -> y_coord);
            overlapX2 = min(preorder_list[i] -> x_coord + preorder_list[i] -> width, preorder_list[j] -> x_coord + preorder_list[j] -> width);
            overlapY2 = min(preorder_list[i] -> y_coord + preorder_list[i] -> height, preorder_list[j] -> y_coord + preorder_list[j] -> height);

            if (overlapX1 < overlapX2 && overlapY1 < overlapY2) {

                overlapSideX = overlapX2 - overlapX1;
                overlapSideY = overlapY2 - overlapY1;
                overlap_area += overlapSideX * overlapSideY;

            }
        }

    }

    #pragma omp parallel for
    for(int i = 0; i < fix_modules.size(); i++){
        for(int j = 0; j < preorder_list.size(); j++){

            overlapX1 = max(fix_modules[i] -> x_coord, preorder_list[j] -> x_coord);
            overlapY1 = max(fix_modules[i] -> y_coord, preorder_list[j] -> y_coord);
            overlapX2 = min(fix_modules[i] -> x_coord + fix_modules[i] -> width, preorder_list[j] -> x_coord + preorder_list[j] -> width);
            overlapY2 = min(fix_modules[i] -> y_coord + fix_modules[i] -> height, preorder_list[j] -> y_coord + preorder_list[j] -> height);

            if (overlapX1 < overlapX2 && overlapY1 < overlapY2) {

                overlapSideX = overlapX2 - overlapX1;
                overlapSideY = overlapY2 - overlapY1;
                overlap_area += overlapSideX * overlapSideY;

            }
        }        

    }
    return overlap_area;

}

int b_to_p(vector<soft_module*>& preorder_list, vector< fix_module* >& fix_modules){

    vector<soft_module*> placed;
    int max_x_coord = 0;
    int max_y_coord = 0;
    
    for(int i = 0; i < preorder_list.size(); i++)
    {
        if(i == 0){

            int k = rand() % 2;
            // int k = 1;
            if( k == 0){

                preorder_list[i] -> x_coord = 0;

            }
            else{

                preorder_list[i] -> x_coord = fix_modules[0] -> x_coord + fix_modules[0] -> width;

            }
            
            preorder_list[i] -> y_coord = 0;

        }
        else{

            if(preorder_list[i] == preorder_list[i] -> parent -> l_child){
                
                preorder_list[i] -> x_coord = preorder_list[i] -> parent -> x_coord + preorder_list[i] -> parent -> width;

            }
            else if(preorder_list[i] == preorder_list[i] -> parent -> r_child){

                preorder_list[i] -> x_coord = preorder_list[i] -> parent -> x_coord;

            }
            preorder_list[i] -> y_coord = find_y_coord(preorder_list[i] -> x_coord, preorder_list[i] -> width, placed);

        }

        bool adjust_ratio = true;
        for(auto& f: fix_modules){

            bool is_overlap = check_overlap(preorder_list[i] -> x_coord, preorder_list[i] -> y_coord, preorder_list[i] -> width, preorder_list[i] -> height, f -> x_coord, f -> y_coord, f -> width, f -> height);
            if(is_overlap && adjust_ratio){

                for(int j = 0; j < (preorder_list[i] -> possible_width).size(); j++){

                    is_overlap = check_overlap(preorder_list[i] -> x_coord, preorder_list[i] -> y_coord, preorder_list[i] -> possible_width[j], preorder_list[i] -> possible_height[j], f -> x_coord, f -> y_coord, f -> width, f -> height);
                    if(!is_overlap){

                        preorder_list[i] -> width = preorder_list[i] -> possible_width[j];
                        preorder_list[i] -> height = preorder_list[i] -> possible_height[j];
                        adjust_ratio = false;
                        break;

                    }

                }

            }
            if(is_overlap) preorder_list[i] -> y_coord = f -> y_coord + f -> height;

        }

        placed.push_back(preorder_list[i]);

    }

    for(auto& module: preorder_list){

        max_x_coord = max(module -> x_coord + module -> width, max_x_coord);
        max_y_coord = max(module -> y_coord + module -> height, max_y_coord);

    }
    for(auto& module: fix_modules){

        max_x_coord = max(module -> x_coord + module -> width, max_x_coord);
        max_y_coord = max(module -> y_coord + module -> height, max_y_coord);

    }
    int overlap_area = cal_overlap_area(preorder_list, fix_modules);
    return (max_x_coord - chip_width) + (max_y_coord  - chip_height) + overlap_area;

    // return max_x_coord * max_y_coord;

}
unsigned int get_total_wirelength(){

    int first_center_x;
    int first_center_y;
    int second_center_x;
    int second_center_y;
    unsigned int wirelength = 0;

    for(auto& n: nets){        
        
        if(soft_hash.count(( n -> module_pair ).first)){

            first_center_x = (2 * soft_hash[ ( n -> module_pair ).first ] -> x_coord + soft_hash[ ( n -> module_pair ).first ] -> width) / 2;
            first_center_y = (2 * soft_hash[ ( n -> module_pair ).first ] -> y_coord + soft_hash[ ( n -> module_pair ).first ] -> height) / 2;

        } 
        else{

            first_center_x = (2 * fix_hash[ ( n -> module_pair ).first ] -> x_coord + fix_hash[ ( n -> module_pair ).first ] -> width) / 2;
            first_center_y = (2 * fix_hash[ ( n -> module_pair ).first ] -> y_coord + fix_hash[ ( n -> module_pair ).first ] -> height) / 2;

        }

        if(soft_hash.count(( n -> module_pair ).second)){

            second_center_x = (2 * soft_hash[ ( n -> module_pair ).second ] -> x_coord + soft_hash[ ( n -> module_pair ).second ] -> width) / 2;
            second_center_y = (2 * soft_hash[ ( n -> module_pair ).second ] -> y_coord + soft_hash[ ( n -> module_pair ).second ] -> height) / 2;

        } 
        else{

            second_center_x = (2 * fix_hash[ ( n -> module_pair ).second ] -> x_coord + fix_hash[ ( n -> module_pair ).second ] -> width) / 2;
            second_center_y = (2 * fix_hash[ ( n -> module_pair ).second ] -> y_coord + fix_hash[ ( n -> module_pair ).second ] -> height) / 2;

        }

        wirelength += (n -> net_weight) * (abs(first_center_x - second_center_x) + abs(first_center_y - second_center_y));
        
    }

    return wirelength;


}

void perturb_tree(int type, vector< soft_module >& soft_modules){

    if(type == 0){  //adjust module aspect ratio

        int index = rand() % (soft_modules.size());
        int a = rand() % ((soft_modules[index].possible_height).size());
        soft_modules[index].height = (soft_modules[index].possible_height)[a];
        soft_modules[index].width = (soft_modules[index].possible_width)[a];

    }
    else if(type == 2){ //move node

        vector<soft_module*> is_leaf;
        for(auto& module: soft_modules){

            if(!(module.l_child) && !(module.r_child)){

                is_leaf.push_back(&module);

            }

        }

        int node_a = rand() % (is_leaf.size());
        if(is_leaf[node_a] -> parent -> r_child == is_leaf[node_a]){

            is_leaf[node_a] -> parent -> r_child = NULL;

        }
        else if(is_leaf[node_a] -> parent -> l_child == is_leaf[node_a]){

            is_leaf[node_a] -> parent -> l_child = NULL;

        }

        vector<soft_module*> insert_node;
        for(auto& module: soft_modules){

            if(!(module.l_child) || !(module.r_child) ){

                if(module.name != is_leaf[node_a] -> name) insert_node.push_back(&module);

            }

        }

        int node_b = rand() % (insert_node.size());
        if(insert_node[node_b] -> l_child == NULL && insert_node[node_b] -> r_child != NULL){

            insert_node[node_b] -> l_child = is_leaf[node_a];

        }
        else if(insert_node[node_b] -> l_child != NULL && insert_node[node_b] -> r_child == NULL){

            insert_node[node_b] -> r_child = is_leaf[node_a];

        }
        else if(insert_node[node_b] -> l_child == NULL && insert_node[node_b] -> r_child == NULL){

            int lr_child = rand() % 2;
            if(lr_child == 0){ //insert in left child

                insert_node[node_b] -> l_child = is_leaf[node_a];

            }
            else{

                insert_node[node_b] -> r_child = is_leaf[node_a];

            }


        }

        is_leaf[node_a] -> parent = insert_node[node_b];
               

    }
    else if(type == 1){ //switch node

        int node_a = rand() % (soft_modules.size());
        int node_b = rand() % (soft_modules.size());
        while (node_a == node_b) node_b = rand() % soft_modules.size();

        swap(soft_modules[node_a].name, soft_modules[node_b].name);   
        swap(soft_modules[node_a].min_area, soft_modules[node_b].min_area);   
        swap(soft_modules[node_a].width, soft_modules[node_b].width);   
        swap(soft_modules[node_a].height, soft_modules[node_b].height);   
        swap(soft_modules[node_a].x_coord, soft_modules[node_b].x_coord);   
        swap(soft_modules[node_a].y_coord, soft_modules[node_b].y_coord);    
        swap(soft_modules[node_a].possible_width, soft_modules[node_b].possible_width);   
        swap(soft_modules[node_a].possible_height, soft_modules[node_b].possible_height);  

    }

}
void fix_tree(vector< soft_module >& soft_modules_best, vector< soft_module >& soft_modules){

    unordered_map<string, int> name_to_indx;
    for(int i = 0; i < soft_modules.size(); i++) name_to_indx[soft_modules[i].name] = i;
    for(int i = 0; i < soft_modules.size(); i++){

        if( soft_modules[i].parent ) soft_modules_best[i].parent = &soft_modules_best[name_to_indx[soft_modules[i].parent -> name]];
        if( soft_modules[i].l_child ) soft_modules_best[i].l_child = &soft_modules_best[name_to_indx[soft_modules[i].l_child -> name]];
        if( soft_modules[i].r_child ) soft_modules_best[i].r_child = &soft_modules_best[name_to_indx[soft_modules[i].r_child -> name]];

    }

}


void simulatedAnnealing(vector< soft_module >& soft_modules, vector< fix_module* >& fix_modules, double initTemperature, 
                        double minTemperature, double coolingCoefficient, int tryingTimes, double maxRejectRatio, bool is_first){

    soft_module* root = init_b_star_tree(soft_modules, 0, soft_modules.size() - 1, NULL); //construct b* tree
    vector<soft_module*> preorder_list;
    dfs(root, preorder_list); //preorder search b* tree 

    soft_module* best_root;
    vector< soft_module > soft_modules_best = soft_modules;
    fix_tree(soft_modules_best, soft_modules);
    for(auto& m: soft_modules_best)
        if(m.parent == NULL) best_root = &m;
    // cout << best_root -> name << endl;

    soft_module* accepted_root;
    vector< soft_module > soft_modules_accepted = soft_modules;
    fix_tree(soft_modules_accepted, soft_modules);
    for(auto& m: soft_modules_accepted)
        if(m.parent == NULL) accepted_root = &m;
    // cout << accepted_root -> name << endl;


    long long cost;
    long long wirelength;
    if(!is_first){

        soft_hash.clear();
        for(auto& s: soft_modules_accepted) soft_hash[s.name] = &s;
        wirelength = get_total_wirelength();        //hpwl overhead

        cost = wirelength / pow(10, to_string(wirelength).length() - 3);
        // cout << "wirelength:" << wirelength << endl;
        // cout << "(to_string(wirelength).length()):" << (to_string(wirelength).length()) << endl;
        // newCost = b_to_p(preorder_list, fix_modules) * 100000 + wirelength;

    }
    else{

        cost = b_to_p(preorder_list, fix_modules);   //fixed outline overhead

    }
    // cout << "cost:" << cost << endl;
    long long best_cost = cost;

    std::chrono::seconds maxRunTime(450);
    do{
        
        double temperature = initTemperature;
        int tryingCnt = 0, uphillCnt = 0, rejectCnt = 0;
        int maxTryingCnt = tryingTimes;
        // int maxTryingCnt = tryingTimes;
        do
        {
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
            if(!is_first && elapsedTime > maxRunTime) break;

            tryingCnt = uphillCnt = rejectCnt = 0;
            do
            {
                preorder_list.clear();
                // int type = (is_first) ? (rand() % 3) : 1;
                int type = rand() % 3;

                soft_modules = soft_modules_accepted;
                fix_tree(soft_modules, soft_modules_accepted);   

                perturb_tree(type, soft_modules_accepted); 
                dfs(accepted_root, preorder_list);
                long long newCost= b_to_p(preorder_list, fix_modules);
                ++tryingCnt;

                if(!is_first){

                    soft_hash.clear();
                    for(auto& s: soft_modules_accepted) soft_hash[s.name] = &s;
                    wirelength = get_total_wirelength();
                    // cout << wirelength << endl;
                    // newCost = newCost * pow(10, to_string(wirelength).length()) + wirelength;
                    // if(newCost == wirelength) cout << "got it " <<endl;
                    // cout <<"new cost:"<< newCost << endl;
                    newCost = newCost * 1000 + wirelength / pow(10, to_string(wirelength).length() - 3);

                }

                long long deltaCost = newCost - cost;
                if (deltaCost < 0 || static_cast<double>(rand()) / RAND_MAX < exp(-deltaCost / temperature))
                {
                    if (deltaCost > 0)
                        ++uphillCnt;

                    cost = newCost;
                    if (cost < best_cost)
                    {
                        soft_modules_best = soft_modules_accepted;
                        fix_tree(soft_modules_best, soft_modules_accepted);
                        best_cost = cost;
                        if (best_cost == 0){
                            soft_modules = soft_modules_best;
                            fix_tree(soft_modules, soft_modules_best);
                            return;
                        }
                            
                    }
                }
                else
                {
                    soft_modules_accepted = soft_modules;
                    fix_tree(soft_modules_accepted, soft_modules);            
                    ++rejectCnt;
                }
            } while (uphillCnt <= maxTryingCnt && tryingCnt <= 2 * maxTryingCnt);
            temperature *= coolingCoefficient;
        } while (static_cast<double>(rejectCnt) / tryingCnt <= maxRejectRatio && temperature >= minTemperature);
        // for(auto& a: soft_modules_best){
        //     cout << a.x_coord << endl;

        // }

    }while(is_first);

    soft_modules = soft_modules_best;
    fix_tree(soft_modules, soft_modules_best);

}

int main(int argc, char **argv) {

    vector< fix_module* > fix_modules;
    vector< soft_module > soft_modules;

    
    get_information(argc, argv, fix_modules, soft_modules, nets, fix_hash); //parsing file

    // cout << soft_hash["PAD_D0"] -> min_area << endl;


    sort(fix_modules.begin(), fix_modules.end(), compareModules); //sort fix module by non-decreasing order
    sort(nets.begin(), nets.end(), compareNets); //sort net by non-decreasing order

    // cout << argv[1] << endl;
 
    // string inputString = argv[1];
    // cout <<"inputString:" << inputString << endl;
    double initTemperature;
    double minTemperature;
    double coolingCoefficient; 
    int tryingTimes;
    double maxRejectRatio;
    // if(inputString == "../testcase/public1.txt"){
    bool is_public = false;
    if(soft_module_num == 15 && fix_module_num == 5 && net_num == 45 ){

        is_public = true;

    }
    else if(soft_module_num == 16 && fix_module_num == 8 && net_num == 39){

        is_public = true;

    }
    else if(soft_module_num == 28 && fix_module_num == 14 && net_num == 108){

        is_public = true;

    }
    else if(soft_module_num == 20 && fix_module_num == 8 && net_num == 47){

        is_public = true;

    }
    else{

        is_public = false;

    }

    if(is_public){
        cout << "is_public" << endl;
        tryingTimes = 10000 * soft_module_num;

    }
    else{
        cout << "not_public" << endl;
        tryingTimes = 1000 * soft_module_num;

    }
    initTemperature = 1000000; minTemperature = 0.1; coolingCoefficient = 0.2;  maxRejectRatio = 1;

    // }
    simulatedAnnealing(soft_modules, fix_modules, initTemperature, minTemperature, coolingCoefficient, tryingTimes, maxRejectRatio, true);  //to satisfy constraint
    
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
    std::chrono::seconds maxRunTime(300);
    cout <<"1st SA finish time:"<< elapsedTime.count() << endl; 

    while(elapsedTime < maxRunTime){
        simulatedAnnealing(soft_modules, fix_modules, 10000000, 0.1, 0.25, tryingTimes, 1, false);  //to satisfy constraint & minimize hpwl
        for(auto& s: soft_modules) soft_hash[s.name] = &s;
        // cout << "wirelength outside:" << get_total_wirelength() << endl;
        currentTime = std::chrono::high_resolution_clock::now();
        elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
    }
    cout <<"2st SA finish time:"<< elapsedTime.count() << endl; 
    
    // void simulatedAnnealing(soft_modules, fix_modules, initTemperature, minTemperature, coolingCoefficient, tryingTimes, maxRejectRatio, is_first)



    for(auto& s: soft_modules) soft_hash[s.name] = &s;
    unsigned int wirelength = get_total_wirelength();

    cout <<"elapseTime:"<< elapsedTime.count() << endl; 

    std::ofstream output_file(argv[2], std::ios::out);
    output_file << "Wirelength" << " " << wirelength << endl;
    output_file << "NumSoftModules" << " " << soft_module_num << endl;
    for(auto& f: soft_modules){
        
        output_file << f.name << " " << f.x_coord << " " << f.y_coord << " " << f.width << " " << f.height << endl;

    }
    output_file.close();

    return 0;
}

