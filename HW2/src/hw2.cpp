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

// using namespace std;


long long die_size;
int dieA_util, dieB_util, numCells, num_tech, tech_cell_count;
int tech_count = 0; //check whether tech1 or tech2
bool first_to_sec;
std::string dieA_tech, dieB_tech;
auto startTime = std::chrono::high_resolution_clock::now();

struct Net{

    // int net_id;
    int cell_nums;
    int left_count;
    int right_count;
    // std::set<int> connectCells;

};
struct Cell{

    int gain;
    int cell_id;
    int first_tech_area;
    int sec_tech_area;
    bool is_first;
    bool is_lock;
    // std::set<int> connectNets;

};

bool compareCells(const Cell* cell1, const Cell* cell2) {
    return cell1 -> sec_tech_area > cell2 -> sec_tech_area;  //改 > public6就會過
}

//meet the balance condition or not
bool is_balance(long long a_area, long long b_area){   

    if(((double)a_area / (double)die_size) <= ((double)dieA_util / 100) && ((double)b_area / (double)die_size) <= ((double)dieB_util / 100)){
    
        return true;
    
    }
    
    return false;

}

void get_information(int argc, char **argv, std::unordered_map<int, Net*>& Nets, std::vector<Cell*>& Cells, 
                     std::unordered_map< int, std::set<int> >& cell_array, std::unordered_map< int, std::set<int> >& net_array){

    // std::cout << "Have " << argc << " arguments:\n";
    // for (int i = 0; i < argc; ++i) {
    //     std::cout << argv[i] << "\n";
    // }

    std::cout << argv[1] << std::endl;
    std::ifstream file(argv[1], std::ios::in);
    if (!file.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return;
    }

    std::string keyword;
    std::unordered_map< std::string, int> first_tech_lib_cell;
    std::unordered_map< std::string, int> sec_tech_lib_cell;
    std::string tech, lib_cell_name, cell_name, net_name;
    int num; 
    long long width, height;
    
    int net_id = 0;
    bool is_lib = true; // Cell | cell name | lib cell name

    while(file >> keyword){
        
        if(keyword == "NumTechs"){
            
            file >> num_tech;
            // std::cout << "NumTechs " << num_tech <<  std::endl;  

        }
        else if(keyword == "Tech"){
            
            tech_count++;
            file >> tech;
            file >> tech_cell_count;
            // std::cout << "Tech " << tech << " " << tech_cell_count <<  std::endl;  

        }
        else if(keyword == "LibCell"){

            file >> cell_name;
            file >> width;
            file >> height;
            // std::cout << "LibCell " << cell_name << " " << width << " " << height <<  std::endl;

            if(tech_count == 1) first_tech_lib_cell[cell_name] = width * height;
            else sec_tech_lib_cell[cell_name] = width * height;
            
        }
        else if(keyword == "DieSize"){

            file >> width;
            file >> height;
            // std::cout << "DieSize " << width << " " << height <<  std::endl; 
            die_size = width * height;

        }
        else if(keyword == "DieA"){

            file >> dieA_tech;
            file >> dieA_util;
            // std::cout << "DieA " << dieA_tech << " " << dieA_util <<  std::endl;
        }
        else if(keyword == "DieB"){

            file >> dieB_tech;
            file >> dieB_util;
            // std::cout << "DieB " << dieB_tech << " " << dieB_util <<  std::endl; 

        }
        else if(keyword == "NumCells"){

            file >> numCells;
            // std::cout << "NumCells " << numCells <<  std::endl; 

        }
        else if(keyword == "Cell" && is_lib){

            file >> cell_name;
            file >> lib_cell_name;
            Cell* c = new Cell();
            c -> cell_id = std::stoi( cell_name.substr( 1, cell_name.length() - 1 ) );
            c -> first_tech_area = first_tech_lib_cell[lib_cell_name];
            // c -> sec_tech_area = first_tech_lib_cell[lib_cell_name];  //!!!!
            c -> sec_tech_area = sec_tech_lib_cell[lib_cell_name];
            Cells.push_back(c);

            // std::cout << "Cell " << cell_name << " " << lib_cell_name <<  std::endl; 

        }
        else if(keyword == "NumNets"){

            file >> num;
            // std::cout << "NumNets " << num <<  std::endl;
            is_lib = false;

        }
        else if(keyword == "Net"){

            file >> net_name;
            file >> num;
            Net* n = new Net();
            net_id = std::stoi(net_name.substr(1, net_name.length() - 1));
            n -> cell_nums = num;
            Nets[ net_id ] = n;
            // std::cout << "Net " << net_name << " " << num <<  std::endl; 
            

        }
        else if(keyword == "Cell" && !is_lib){

            file >> cell_name;
            cell_array[ std::stoi( cell_name.substr(1, cell_name.length() - 1) ) ].insert( net_id );
            net_array[ net_id ].insert( std::stoi( cell_name.substr(1, cell_name.length() - 1) ) );

            // (Nets[Nets.size() - 1] -> connectCells).insert(  );
            // std::cout << "Cell " << cell_name <<  std::endl; 

        }

    }

    file.close();

}
void partition(std::unordered_map<int, Net*>& Nets, std::vector<Cell*>& Cells, std::unordered_map<int, Cell*>& Cells_hash, std::unordered_map< int, std::set<int> >& cell_array,     //TODO
               std::unordered_map< int, std::set<int> >& net_array, std::set< Cell* >& first_tech_set, std::set< Cell* >& sec_tech_set,
               long long& first_total_weight, long long& sec_total_weight){

    std::sort(Cells.begin(), Cells.end(), compareCells);
    for(auto &a : Cells){
        // std::cout << a -> first_tech_area << std::endl;
        sec_total_weight += a -> sec_tech_area;
        // first_total_weight += a -> first_tech_area; 
    }

    // std::cout << "total size of cell in first tech:" << first_total_weight << std::endl;
    // std::cout << "total size of cell in sec tech:" << sec_total_weight << std::endl;
    // std::cout << "die_size:" << die_size << std::endl;

    for(auto &a : Cells){

        if(!is_balance(first_total_weight, sec_total_weight)){

            first_tech_set.insert(a);
            a -> is_first = true;

            first_total_weight += a -> first_tech_area;
            sec_total_weight -= a -> sec_tech_area;

        }
        else{

            sec_tech_set.insert(a);
            a -> is_first = false;          

        }

    }

    if(!is_balance(first_total_weight, sec_total_weight)){

        bool go_to_first = true;
        first_total_weight = 0;
        sec_total_weight = 0;
        first_tech_set.clear();
        sec_tech_set.clear();

        for(auto &a : Cells){

            if(go_to_first){
                if(is_balance(first_total_weight + a -> first_tech_area, sec_total_weight)){

                    first_tech_set.insert(a);
                    a -> is_first = true;
                    first_total_weight += a -> first_tech_area;
                    // for(auto& b : cell_array[a->cell_id]) Nets[b] -> left_count += 1;
                    go_to_first = false;

                }
                else{

                    sec_tech_set.insert(a);
                    a -> is_first = false;
                    sec_total_weight += a -> sec_tech_area;
                    // for(auto& b : cell_array[a->cell_id]) Nets[b] -> right_count += 1;

                }
            }
            else{
                if(is_balance(first_total_weight, sec_total_weight + a -> sec_tech_area)){

                    sec_tech_set.insert(a);
                    a -> is_first = false;
                    sec_total_weight += a -> sec_tech_area;
                    // for(auto& b : cell_array[a->cell_id]) Nets[b] -> right_count += 1;
                    go_to_first = true;

                }
                else{

                    first_tech_set.insert(a);
                    a -> is_first = true;
                    first_total_weight += a -> first_tech_area;
                    // for(auto& b : cell_array[a->cell_id]) Nets[b] -> left_count += 1;

                }

            }

        }
    }
    for(auto &a : Cells){

        for(auto& b : cell_array[a->cell_id]){

            if(a -> is_first) Nets[b] -> left_count += 1;
            else Nets[b] -> right_count += 1;
        }
    }


    // std::cout << "total size of cell in first tech (after partitioning):" << first_total_weight << std::endl;
    // std::cout << "total size of cell in sec tech (after partitioning):" << sec_total_weight << std::endl;
    // std::cout << "die_size:" << die_size << std::endl;

}

int getCutSize(std::unordered_map<int, Net*>& Nets){
    
    int cut_size = 0;
    for (const auto& pair : Nets) {
        
        if(pair.second -> left_count && pair.second -> right_count){

            cut_size++;

        }
        
    }

    return cut_size;

}

void initialize(std::set< Cell* >& first_tech_set, std::set< Cell* >& sec_tech_set, std::unordered_map< int, std::set<int> >& cell_array, 
                std::unordered_map<int, Net*>& Nets){

    for(auto& first_cell : first_tech_set){

        first_cell -> gain = 0;
        for(auto& nets : cell_array[first_cell -> cell_id] ){

            if( Nets[nets] -> left_count == 1 ) (first_cell -> gain)++;
            if( Nets[nets] -> right_count == 0 ) (first_cell -> gain)--;

        }

    }
    for(auto& sec_cell : sec_tech_set){

        sec_cell -> gain = 0;
        for(auto& nets : cell_array[sec_cell -> cell_id] ){

            if( Nets[nets] -> right_count == 1 ) (sec_cell -> gain)++;
            if( Nets[nets] -> left_count == 0 ) (sec_cell -> gain)--;

        }

    }
    
}

void build_bucketlist(std::map<int, std::set<int>>& first_cell_gain, std::map<int, std::set<int>>& sec_cell_gain, std::unordered_map<int, Cell*>& Cells_hash){
    // lock住的就不用build
    for(auto& cell : Cells_hash){

        if(cell.second -> is_first && !(cell.second -> is_lock)){

            first_cell_gain[cell.second -> gain].insert(cell.second -> cell_id);

        }
        else if(!(cell.second -> is_first) && !(cell.second -> is_lock)){

            sec_cell_gain[cell.second -> gain].insert(cell.second -> cell_id);

        }

    }
    
}
int get_max_gain(std::map<int, std::set<int>>& first_cell_gain, std::map<int, std::set<int>>& sec_cell_gain,            //TODO
                 long long& first_total_weight, long long& sec_total_weight, std::unordered_map<int, Cell*>& Cells_hash){

    auto first_iter = first_cell_gain.rbegin();
    auto sec_iter = sec_cell_gain.rbegin();

    while(first_iter != first_cell_gain.rend() || sec_iter != sec_cell_gain.rend()){ 
        
        if(sec_iter == sec_cell_gain.rend() || (first_iter != first_cell_gain.rend() && sec_iter != sec_cell_gain.rend() && (first_iter -> first > sec_iter -> first))){
            
            for(auto& element_in_set : first_iter -> second){

                if(is_balance(first_total_weight - (Cells_hash[element_in_set] -> first_tech_area), sec_total_weight + (Cells_hash[element_in_set] -> sec_tech_area))){
                    
                    first_total_weight = first_total_weight - (Cells_hash[element_in_set] -> first_tech_area);
                    sec_total_weight = sec_total_weight + (Cells_hash[element_in_set] -> sec_tech_area);                    
                    (first_iter -> second).erase(element_in_set);
                    if( (*first_iter).second.empty() ) first_cell_gain.erase((*first_iter).first);
                    first_to_sec = true;
                    return element_in_set;

                }

            }
            first_iter++;

        }
        else if(first_iter == first_cell_gain.rend() || (first_iter != first_cell_gain.rend() && sec_iter != sec_cell_gain.rend() && (first_iter -> first <= sec_iter -> first))){

            for(auto& element_in_set : sec_iter -> second){

                if(is_balance(first_total_weight + (Cells_hash[element_in_set] -> first_tech_area), sec_total_weight - (Cells_hash[element_in_set] -> sec_tech_area))){
                    
                    first_total_weight = first_total_weight + (Cells_hash[element_in_set] -> first_tech_area);
                    sec_total_weight = sec_total_weight - (Cells_hash[element_in_set] -> sec_tech_area);                    
                    (sec_iter -> second).erase(element_in_set);
                    if( (*sec_iter).second.empty() ) sec_cell_gain.erase((*sec_iter).first);
                    first_to_sec = false;
                    return element_in_set;

                }

            }
            sec_iter++;

        }

    }
    
    return -1;    

}

void update_gain(int max_gain_id, std::unordered_map< int, std::set<int> >& cell_array, std::unordered_map< int, std::set<int> >& net_array, 
                 std::unordered_map<int, Cell*>& Cells_hash, std::unordered_map<int, Net*>& Nets, 
                 std::map<int, std::set<int>>& first_cell_gain, std::map<int, std::set<int>>& sec_cell_gain, bool first_to_sec){

    //if first_to_sec == true ----> front block = first、 to block = sec

    if(first_to_sec){
    /*first to sec*/
        for(auto& nets : cell_array[max_gain_id]){   //for each net n on the base cell

            if(Nets[nets] -> right_count == 0){
                
                for(auto& cells_in_net : net_array[nets]){
                    
                    /*cell is in the first die and unlock(exist in the first hash table)*/
                    if(Cells_hash[cells_in_net] -> is_first && first_cell_gain.count(Cells_hash[cells_in_net] -> gain)
                       && first_cell_gain[Cells_hash[cells_in_net] -> gain].count(cells_in_net)){

                        first_cell_gain[Cells_hash[cells_in_net] -> gain].erase(cells_in_net);

                        if( first_cell_gain[Cells_hash[cells_in_net] -> gain].empty() ) 
                            first_cell_gain.erase(Cells_hash[cells_in_net] -> gain);  
                        
                        Cells_hash[cells_in_net] -> gain ++;
                        first_cell_gain[Cells_hash[cells_in_net] -> gain].insert(cells_in_net);
                        

                    }/*cell is in the sec die and unlock(exist in the sec hash table)*/
                    else if(!(Cells_hash[cells_in_net] -> is_first) && sec_cell_gain.count(Cells_hash[cells_in_net] -> gain) 
                            && sec_cell_gain[Cells_hash[cells_in_net] -> gain].count(cells_in_net)){

                        sec_cell_gain[Cells_hash[cells_in_net] -> gain].erase(cells_in_net);

                        if( sec_cell_gain[Cells_hash[cells_in_net] -> gain].empty() ) 
                            sec_cell_gain.erase(Cells_hash[cells_in_net] -> gain);

                        Cells_hash[cells_in_net] -> gain ++;
                        sec_cell_gain[Cells_hash[cells_in_net] -> gain].insert(cells_in_net);
                        

                    }
                }

            }
            else if(Nets[nets] -> right_count == 1){

                for(auto& cells_in_net : net_array[nets]){
                
                    if(!(Cells_hash[cells_in_net] -> is_first) && sec_cell_gain.count(Cells_hash[cells_in_net] -> gain)
                       && sec_cell_gain[Cells_hash[cells_in_net] -> gain].count(cells_in_net)){
                            
                        sec_cell_gain[Cells_hash[cells_in_net] -> gain].erase(cells_in_net);

                        if( sec_cell_gain[Cells_hash[cells_in_net] -> gain].empty() ) 
                            sec_cell_gain.erase(Cells_hash[cells_in_net] -> gain);
                        
                        Cells_hash[cells_in_net] -> gain --;
                        sec_cell_gain[Cells_hash[cells_in_net] -> gain].insert(cells_in_net);
                        
    
                    }
                
                }

            }

            Cells_hash[max_gain_id] -> is_first = false;
            Nets[nets] -> left_count --;
            Nets[nets] -> right_count ++;

            if(Nets[nets] -> left_count == 0){
                
                for(auto& cells_in_net : net_array[nets]){
                    
                    if(Cells_hash[cells_in_net] -> is_first && first_cell_gain.count(Cells_hash[cells_in_net] -> gain) 
                       && first_cell_gain[Cells_hash[cells_in_net] -> gain].count(cells_in_net)){

                        first_cell_gain[Cells_hash[cells_in_net] -> gain].erase(cells_in_net);

                        if( first_cell_gain[Cells_hash[cells_in_net] -> gain].empty() ) 
                            first_cell_gain.erase(Cells_hash[cells_in_net] -> gain);  
                        
                        Cells_hash[cells_in_net] -> gain --;
                        first_cell_gain[Cells_hash[cells_in_net] -> gain].insert(cells_in_net);
                        

                    }
                    else if(!(Cells_hash[cells_in_net] -> is_first) && sec_cell_gain.count(Cells_hash[cells_in_net] -> gain)
                            && sec_cell_gain[Cells_hash[cells_in_net] -> gain].count(cells_in_net)){

                        sec_cell_gain[Cells_hash[cells_in_net] -> gain].erase(cells_in_net);

                        if( sec_cell_gain[Cells_hash[cells_in_net] -> gain].empty() ) 
                            sec_cell_gain.erase(Cells_hash[cells_in_net] -> gain);
                        
                        Cells_hash[cells_in_net] -> gain --;
                        sec_cell_gain[Cells_hash[cells_in_net] -> gain].insert(cells_in_net);
                        

                    }
          
                }

            }
            else if(Nets[nets] -> left_count == 1){

                for(auto& cells_in_net : net_array[nets]){
                
                    if(Cells_hash[cells_in_net] -> is_first && first_cell_gain.count(Cells_hash[cells_in_net] -> gain) 
                       && first_cell_gain[Cells_hash[cells_in_net] -> gain].count(cells_in_net)){
                            
                        first_cell_gain[Cells_hash[cells_in_net] -> gain].erase(cells_in_net);

                        if( first_cell_gain[Cells_hash[cells_in_net] -> gain].empty() ) 
                            first_cell_gain.erase(Cells_hash[cells_in_net] -> gain);  

                        Cells_hash[cells_in_net] -> gain ++;
                        first_cell_gain[Cells_hash[cells_in_net] -> gain].insert(cells_in_net);
                        
    
                    }
                }
            }

        }        

    }
    else{
        /*sec to first*/
        for(auto& nets : cell_array[max_gain_id]){   //for each net n on the base cell
            // std::cout << "nets:" << nets << std::endl;
            if(Nets[nets] -> left_count == 0){
                
                for(auto& cells_in_net : net_array[nets]){
                    
                    /*cell is in the first die and unlock(exist in the first hash table)*/
                    if(Cells_hash[cells_in_net] -> is_first && first_cell_gain.count(Cells_hash[cells_in_net] -> gain)
                       && first_cell_gain[Cells_hash[cells_in_net] -> gain].count(cells_in_net)){

                        first_cell_gain[Cells_hash[cells_in_net] -> gain].erase(cells_in_net);

                        if( first_cell_gain[Cells_hash[cells_in_net] -> gain].empty() ) 
                            first_cell_gain.erase(Cells_hash[cells_in_net] -> gain);      

                        Cells_hash[cells_in_net] -> gain ++;
                        first_cell_gain[Cells_hash[cells_in_net] -> gain].insert(cells_in_net);
                        

                    }/*cell is in the sec die and unlock(exist in the sec hash table)*/
                    else if(!(Cells_hash[cells_in_net] -> is_first) && sec_cell_gain.count(Cells_hash[cells_in_net] -> gain)
                            && sec_cell_gain[Cells_hash[cells_in_net] -> gain].count(cells_in_net)){
                        
                        // std::cout << "count " <<sec_cell_gain[Cells_hash[cells_in_net] -> gain].count(cells_in_net) << std::endl;
                        // std::map<int, std::set<int>>& sec_cell_gain
                        sec_cell_gain[Cells_hash[cells_in_net] -> gain].erase(cells_in_net);

                        if( sec_cell_gain[Cells_hash[cells_in_net] -> gain].empty() ) 
                            sec_cell_gain.erase(Cells_hash[cells_in_net] -> gain);

                        Cells_hash[cells_in_net] -> gain ++;
                        sec_cell_gain[Cells_hash[cells_in_net] -> gain].insert(cells_in_net);
                        

                    }
              
                }

            }
            else if(Nets[nets] -> left_count == 1){

                for(auto& cells_in_net : net_array[nets]){
                
                    if((Cells_hash[cells_in_net] -> is_first) && first_cell_gain.count(Cells_hash[cells_in_net] -> gain)
                        &&  first_cell_gain[Cells_hash[cells_in_net] -> gain].count(cells_in_net)){
                            
                        first_cell_gain[Cells_hash[cells_in_net] -> gain].erase(cells_in_net);

                        if( first_cell_gain[Cells_hash[cells_in_net] -> gain].empty() ) 
                            first_cell_gain.erase(Cells_hash[cells_in_net] -> gain);   

                        Cells_hash[cells_in_net] -> gain --;
                        first_cell_gain[Cells_hash[cells_in_net] -> gain].insert(cells_in_net);
                        
                    }
                
                }

            }

            Cells_hash[max_gain_id] -> is_first = true;
            Nets[nets] -> left_count ++;
            Nets[nets] -> right_count --;

            if(Nets[nets] -> right_count == 0){
                
                for(auto& cells_in_net : net_array[nets]){
                    
                    if(Cells_hash[cells_in_net] -> is_first && first_cell_gain.count(Cells_hash[cells_in_net] -> gain)
                       && first_cell_gain[Cells_hash[cells_in_net] -> gain].count(cells_in_net)){

                        first_cell_gain[Cells_hash[cells_in_net] -> gain].erase(cells_in_net);

                        if( first_cell_gain[Cells_hash[cells_in_net] -> gain].empty() ) 
                            first_cell_gain.erase(Cells_hash[cells_in_net] -> gain);   

                        Cells_hash[cells_in_net] -> gain --;
                        first_cell_gain[Cells_hash[cells_in_net] -> gain].insert(cells_in_net);
                        

                    }
                    else if(!(Cells_hash[cells_in_net] -> is_first) && sec_cell_gain.count(Cells_hash[cells_in_net] -> gain) 
                            && sec_cell_gain[Cells_hash[cells_in_net] -> gain].count(cells_in_net)){

                        sec_cell_gain[Cells_hash[cells_in_net] -> gain].erase(cells_in_net);

                        if( sec_cell_gain[Cells_hash[cells_in_net] -> gain].empty() ) 
                            sec_cell_gain.erase(Cells_hash[cells_in_net] -> gain);

                        Cells_hash[cells_in_net] -> gain --;
                        sec_cell_gain[Cells_hash[cells_in_net] -> gain].insert(cells_in_net);
                        
                    }

                }

            }
            else if(Nets[nets] -> right_count == 1){

                for(auto& cells_in_net : net_array[nets]){
                
                    if(!(Cells_hash[cells_in_net] -> is_first) && sec_cell_gain.count(Cells_hash[cells_in_net] -> gain) 
                       && sec_cell_gain[Cells_hash[cells_in_net] -> gain].count(cells_in_net)){
                        
                        sec_cell_gain[Cells_hash[cells_in_net] -> gain].erase(cells_in_net);

                        if( sec_cell_gain[Cells_hash[cells_in_net] -> gain].empty() ) 
                            sec_cell_gain.erase(Cells_hash[cells_in_net] -> gain);

                        Cells_hash[cells_in_net] -> gain ++;
                        sec_cell_gain[Cells_hash[cells_in_net] -> gain].insert(cells_in_net);
                        
    
                    }
                }
            }

        }

    }

}

int fm(std::map<int, std::set<int>>& first_cell_gain, std::map<int, std::set<int>>& sec_cell_gain, 
       long long& first_total_weight, long long& sec_total_weight, 
       std::unordered_map< int, std::set<int> >& cell_array, std::unordered_map< int, std::set<int> >& net_array, 
       std::unordered_map<int, Cell*>& Cells_hash, std::unordered_map<int, Net*>& Nets,
       std::set< Cell* >& first_tech_set, std::set< Cell* >& sec_tech_set){

    std::vector<Cell*> max_gain_list;
    int max_gain_id;
    std::chrono::seconds maxRunTime(250);

    while( !first_cell_gain.empty() || !sec_cell_gain.empty() ){   

        max_gain_id = get_max_gain(first_cell_gain, sec_cell_gain, first_total_weight, sec_total_weight, Cells_hash);
        if(max_gain_id == -1) break;
        max_gain_list.push_back(Cells_hash[max_gain_id]);
        update_gain(max_gain_id, cell_array, net_array, Cells_hash, Nets, first_cell_gain, sec_cell_gain, first_to_sec);
        
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
        if (elapsedTime >= maxRunTime) {
            std::cout << "time out" << std::endl; 
            break;
        }

    }

    // std::cout << "vector: " ;
    // for(auto& a : max_gain_list){
    //     std::cout << a -> gain << " " ;
    // }
    // std::cout << std::endl;

    /*max partial sum*/
    int max_sum = 0;
    int current_sum = 0;
    int end_index = 0;

    if(!max_gain_list.empty()){

        max_sum = max_gain_list[0] -> gain;
        current_sum = max_gain_list[0] -> gain;
        
        for (int i = 1; i < max_gain_list.size(); ++i) {
            
            current_sum += max_gain_list[i] -> gain;
            if(current_sum > max_sum){

                max_sum = current_sum;
                end_index = i;

            }

        }

    }

    /**/
    // std::cout << "max sum " << max_sum << std::endl;
    if(max_sum <= 0) return -1;

    for(int i = 0; i <= end_index; i++){     //move cells between two sets

        max_gain_list[i] -> is_lock = true;
        if(!(max_gain_list[i] -> is_first)){
               
            // std::cout << "first to second" << std::endl;
            first_tech_set.erase(max_gain_list[i]);
            sec_tech_set.insert(max_gain_list[i]);

        }
        else{
            
            // std::cout << "second to first" << std::endl;
            sec_tech_set.erase(max_gain_list[i]);
            first_tech_set.insert(max_gain_list[i]);

        }

    }
    

    return max_sum; 

}

void reset( long long& first_total_weight, long long& sec_total_weight, 
            std::unordered_map< int, std::set<int> >& cell_array, std::unordered_map< int, std::set<int> >& net_array, 
            std::unordered_map<int, Cell*>& Cells_hash, std::unordered_map<int, Net*>& Nets,
            std::set< Cell* >& first_tech_set, std::set< Cell* >& sec_tech_set){  //TODO

    /*first die*/
    long long first_weight_sum = 0;
    for(auto& cell_in_first : first_tech_set){
        
        cell_in_first -> is_first = true;
        first_weight_sum += cell_in_first -> first_tech_area;

    }
    first_total_weight = first_weight_sum;

    /*sec die*/
    long long sec_weight_sum = 0;
    for(auto& cell_in_sec : sec_tech_set){
        
        cell_in_sec -> is_first = false;
        sec_weight_sum += cell_in_sec -> sec_tech_area;

    }
    sec_total_weight = sec_weight_sum;

    int left, right;
    for(auto& cell_in_net : net_array){
        
        left = 0; right = 0;
        for(auto& cell : cell_in_net.second){

            if(Cells_hash[cell] -> is_first) left++;
            else right ++;

        }
        Nets[cell_in_net.first] -> left_count = left;
        Nets[cell_in_net.first] -> right_count = right;
        

    }

}


int main(int argc, char **argv) {

    startTime = std::chrono::high_resolution_clock::now();

    std::unordered_map<int, Net*> Nets; 
    std::vector<Cell*> Cells;
    std::unordered_map< int, std::set<int> > cell_array;
    std::unordered_map< int, std::set<int> > net_array;
    std::set< Cell* > first_tech_set;
    std::set< Cell* > sec_tech_set;
    std::unordered_map<int, Cell*> Cells_hash; 
    
    get_information(argc, argv, Nets, Cells, cell_array, net_array); //get cell/net information, build

    
    long long first_total_weight = 0;
    long long sec_total_weight = 0;
    
    if(tech_count == 1) for(auto &a : Cells) a -> sec_tech_area = a -> first_tech_area; //1 tech
    
    partition(Nets, Cells, Cells_hash, cell_array, net_array, first_tech_set, sec_tech_set, first_total_weight, sec_total_weight); //partition into two set

    for(auto &a : Cells) Cells_hash[a -> cell_id] = a;

    std::map<int, std::set<int>> first_cell_gain, sec_cell_gain;
    int mps, cut_size;
    
    std::chrono::seconds maxRunTime(250);
    do{
        
        initialize(first_tech_set, sec_tech_set, cell_array, Nets); //compute cell gain
        build_bucketlist(first_cell_gain, sec_cell_gain, Cells_hash); // build first_cell_gain, sec_cell_gain
        mps = fm(first_cell_gain, sec_cell_gain, first_total_weight, sec_total_weight, cell_array, net_array, 
                 Cells_hash, Nets, first_tech_set, sec_tech_set);
        first_cell_gain.clear(); sec_cell_gain.clear();
        reset(first_total_weight, sec_total_weight, cell_array, net_array, 
                    Cells_hash, Nets, first_tech_set, sec_tech_set);


        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
        if (elapsedTime >= maxRunTime) {
            // std::cout << "time out" << std::endl; 
            break;
        }   

    }while(mps != -1); //max partial sum <= 0 就結束

    cut_size = getCutSize(Nets);

    std::ofstream output_file(argv[2], std::ios::out);
    output_file << "CutSize " <<  cut_size << std::endl;
    output_file << "DieA " <<  first_tech_set.size() << std::endl;
    for(auto& die_num : first_tech_set){

        output_file << "C" <<  die_num -> cell_id << std::endl;

    }
    output_file << "DieB " <<  sec_tech_set.size() << std::endl;
    for(auto& die_num : sec_tech_set){

        output_file << "C" <<  die_num -> cell_id << std::endl;

    }
    output_file.close();

    return 0;
}