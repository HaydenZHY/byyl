#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <math.h>
#include <queue>
#include <stack>
#include <stdlib.h>

using namespace std;
#define NFA_SIZE 30

typedef struct Transition_Rule{
    char rec;int now;int next;
} Trans_Rule;

bool operator<(const Transition_Rule &a,const Transition_Rule &b){
    return a.now + 1000*int(a.rec) < b.now + 1000*int(b.rec);
}
typedef struct fa{
    vector<char> input_symbols;       
    set<int> states;                  
    map<int, string> state_labels;   
    set<Trans_Rule> Transition_Rule;     
    set<int> start; set<int> final;                   
} FA;

map<string, string> symbols_name = {
    {"int","KW"},{"void","KW"},{"return","KW"},{"const","KW"},{"main","KW"},{"float","KW"},{"if","KW"},{"else","KW"},
    {"+","OP"},{"-","OP"},{"*","OP"},{"/","OP"},{"%","OP"},{"=","OP"},{">","OP"},
    {"<","OP"},{"==","OP"},{"<=","OP"},{">=","OP"},{"!=","OP"},{"&&","OP"},{"||","OP"},
    {"(","SE"},{")","SE"},{"{","SE"},{"}","SE"},{";","SE"},{",","SE"}
};
map<string, string> processed_symbols_name = {};
map<string, int> token_to_number = {
    {"int", 1},{"void", 2},{"return", 3},{"const", 4},{"main", 5},{"float", 6},{"if", 7},{"else", 8},{"+", 6}, 
    {"-", 7},   {"*", 8},   {"/", 9},{"%", 10}, {"=", 11},{">", 12}, {"<", 13}, {"==", 14}, {"<=", 15}, {">=", 16},{"!=", 17}, {"&&", 18},  {"||", 19},
    {"(", 20},{")", 21}, {"{", 22},{"}", 23},{";", 24},{",", 25},
};
 
vector<char> input_symbols = {'n','l','o','s','_','0','=','>','<','!','&','|','-','.'};
map<int,string> state_labels = {{1,"n"},{2,"l"},{3,"o"},{4,"s"},{5,"_"},{7,"="},{8,">"},{9,"<"},
{10,"!"},{11,"&"},{12,"|"},{13,"INT"},{14,"SE"},{15,"I&K"},{16,"OP"},{17,"none"},{18,"OP"},{20,"FLOAT"}}; 
set<int> lex_start = {17};
set<int> lex_final = {13,14,15,16,18,20};
set<Trans_Rule> transmission_map = {{'@',1,13},{'n',13,13},{'@',2,15},{'l',15,15},{'_',15,15},{'n',15,15},{'@',4,14},{'@',3,16},
{'@',5,15},{'@',7,16},{'@',8,16},{'=',7,16},{'=',8,16},{'@',9,16},{'=',9,16},{'=',10,16},{'&',11,16},{'|',12,16},{'n',18,13},{'n',17,1},{'l',17,2},{'o',17,3},{'s',17,4},
{'_',17,5},{'=',17,7},{'>',17,8},{'<',17,9},{'!',17,10},{'&',17,11},{'|',17,12},{'-',17,18},{'.',13,20},{'@',20,20},{'n',20,20}};
 
 
set<int> get_closure(const set<int>& starting_states_set, const FA& input_nfa) {
    set<int> reachable_via_epsilon = starting_states_set;
    set<int> states_to_process = starting_states_set;

    while (!states_to_process.empty()) {
        int current_processing_state = *states_to_process.begin();
        states_to_process.erase(states_to_process.begin());

        for (const auto& rule : input_nfa.Transition_Rule) {
            if (rule.rec == '@' && current_processing_state == rule.now) {
                if (reachable_via_epsilon.insert(rule.next).second) {
                    states_to_process.insert(rule.next);
                }
            }
        }
    }

    return reachable_via_epsilon;
}

set<int> state_process(set<int> current_states_set, char input_char, FA input_nfa){
    set<int> states_after_input;
    for(int current_state_id : current_states_set){
        for(const auto& rule : input_nfa.Transition_Rule){
            if(rule.now == current_state_id && rule.rec == input_char){
                states_after_input.insert(rule.next);
            }
        }
    }
    return get_closure(states_after_input, input_nfa);
}


FA deter_nfa(FA source_nfa) {
    FA result_dfa;
    queue<set<int>> unprocessed_state_sets;
    map<set<int>, int> state_set_to_id_map;
    int next_available_id = 1;

    result_dfa.input_symbols = source_nfa.input_symbols;
    result_dfa.start = {1};
    result_dfa.states.insert(1);

    set<int> initial_closure = get_closure(source_nfa.start, source_nfa);
    state_set_to_id_map[initial_closure] = 1;
    unprocessed_state_sets.push(initial_closure);

    while (!unprocessed_state_sets.empty()) {
        set<int> current_set_of_nfa_states = unprocessed_state_sets.front();
        unprocessed_state_sets.pop();
        int current_dfa_state_id = state_set_to_id_map[current_set_of_nfa_states];

        for (char input_sym : source_nfa.input_symbols) {
            set<int> next_set_of_nfa_states = state_process(current_set_of_nfa_states, input_sym, source_nfa);
            if (next_set_of_nfa_states.empty()) continue;

            if (state_set_to_id_map.find(next_set_of_nfa_states) == state_set_to_id_map.end()) {
                next_available_id++;
                state_set_to_id_map[next_set_of_nfa_states] = next_available_id;
                result_dfa.states.insert(next_available_id);
                unprocessed_state_sets.push(next_set_of_nfa_states);

                for (int nfa_state_in_set : next_set_of_nfa_states) {
                    if (source_nfa.final.count(nfa_state_in_set)) {
                        result_dfa.final.insert(next_available_id);
                        result_dfa.state_labels[next_available_id] = source_nfa.state_labels[nfa_state_in_set];
                        break;
                    }
                }
            }
            result_dfa.Transition_Rule.insert({input_sym, current_dfa_state_id, state_set_to_id_map[next_set_of_nfa_states]});
        }
    }

    return result_dfa;
}
FA minimize_fa(FA source_dfa) {
    FA minimized_dfa;

    minimized_dfa.input_symbols = source_dfa.input_symbols;

    map<int, int> old_to_new_state_mapping;
    set<int> non_accepting_states, accepting_states;

    for (int dfa_state : source_dfa.states) {
        if (source_dfa.final.count(dfa_state)) {
            accepting_states.insert(dfa_state);
        } else {
            non_accepting_states.insert(dfa_state);
        }
    }

    int current_new_id = 0;
    for (int state_id : non_accepting_states) {
        old_to_new_state_mapping[state_id] = current_new_id++;
    }
    for (int state_id : accepting_states) {
        old_to_new_state_mapping[state_id] = current_new_id++;
    }

    for (auto const& mapping_pair : old_to_new_state_mapping) {
        int old_id = mapping_pair.first;  
        int new_id = mapping_pair.second; 
        minimized_dfa.states.insert(new_id);
        if (source_dfa.start.count(old_id)) {
            minimized_dfa.start.insert(new_id);
        }
        if (source_dfa.final.count(old_id)) {
            minimized_dfa.final.insert(new_id);
            minimized_dfa.state_labels[new_id] = source_dfa.state_labels[old_id];
        }
    }

    for (const auto& transition_rule : source_dfa.Transition_Rule) {
        int new_from_state = old_to_new_state_mapping[transition_rule.now];
        int new_to_state = old_to_new_state_mapping[transition_rule.next];
        char input_char = transition_rule.rec;
        minimized_dfa.Transition_Rule.insert({input_char, new_from_state, new_to_state});
    }

    return minimized_dfa;
}

int isletter(char ch){return (ch == ' ' || ch == '\t') ? 0 : 1;}

char GetCharType(char current_char , char next_char_peek){
    if(isdigit(current_char)) return 'n';
    if(current_char == '.') return '.';
    if(isalpha(current_char)) return 'l';
    if(current_char == '+' || current_char == '-' || current_char == '*' || current_char == '/' || current_char == '%') return 'o';
    if(current_char == '(' || current_char == ')' || current_char == '{' || current_char == '}' || current_char == ';' || current_char == ',') return 's';
   
    return current_char; 
}

void build_table(int current_state_id, string token_string, FA deterministic_finite_automaton){
    int state_id = current_state_id;
    string token_value = token_string;
    const FA& dfa_instance = deterministic_finite_automaton;
    string state_label_category = dfa_instance.state_labels.at(state_id);

    if((state_label_category == "INT" || state_label_category == "SE" || state_label_category == "OP") && !processed_symbols_name.count(token_value)){
        processed_symbols_name.insert({token_value, state_label_category});
    }
    else if(state_label_category == "I&K" && symbols_name[token_value] == "KW" && !processed_symbols_name.count("KW")){
        processed_symbols_name.insert({token_value, state_label_category});
    }
    else if(!processed_symbols_name.count(token_value)) {
        processed_symbols_name.insert({token_value, "IDN"});
    }
}

std::string tokenize(int current_token_state, std::string recognized_token_lexeme, FA processing_automaton){
    std::string result_output_string = "<";
    std::string token_category_tag;
    std::string token_value_content;

    std::string current_state_descriptive_label = processing_automaton.state_labels[current_token_state];

    auto number_mapping_iterator = token_to_number.find(recognized_token_lexeme);

    if (number_mapping_iterator != token_to_number.end()) {
        int mapped_token_number = number_mapping_iterator->second;
        token_value_content = std::to_string(mapped_token_number);

        if (current_state_descriptive_label == "I&K") {
             token_category_tag = "KW";
        } else if (current_state_descriptive_label == "OP") {
             token_category_tag = "OP";
        } else if (current_state_descriptive_label == "SE") {
             token_category_tag = "SE";
        } else {
            token_category_tag = "ERROR";
        }

    } else {
        if (current_state_descriptive_label == "INT") {
            token_category_tag = "INT";
            token_value_content = recognized_token_lexeme;
        } else if (current_state_descriptive_label == "FLOAT") {
            token_category_tag = "FLOAT";
            token_value_content = recognized_token_lexeme;
        } else if (current_state_descriptive_label == "I&K") {
            token_category_tag = "IDN";
            token_value_content = recognized_token_lexeme;
        } else {
             token_category_tag = "UNKNOWN";
             token_value_content = recognized_token_lexeme;
             std::cerr << "Error: Token '" << recognized_token_lexeme << "' not found in map and state label '" << current_state_descriptive_label << "' not INT, FLOAT, or I&K for state " << current_token_state << std::endl;
        }
    }

    result_output_string += token_category_tag;
    result_output_string += ",";
    result_output_string += token_value_content;
    result_output_string += ">";

    return result_output_string;
}
void perform_lexical_analysis(const char* input_file_path, FA resulting_dfa, std::string output_file_suffix){
    std::fstream input_source_file(input_file_path);
    std::ofstream tokens_output_file;
    std::string base_output_path ="../syntax/lex/lex";
    std::string file_extension =".txt";
    std::string full_output_file_path = base_output_path + output_file_suffix + file_extension;
    tokens_output_file.open(full_output_file_path, std::ios::out | std::ios::trunc);
    if(input_source_file.is_open() && tokens_output_file.is_open()){
        std::string line_buffer;
        while (getline(input_source_file, line_buffer)){
            int current_automaton_state = *resulting_dfa.start.begin();
            std::string accumulated_token_string = "";
            for( size_t character_index = 0; character_index < line_buffer.length() ; character_index++ ){
                char current_input_char = line_buffer[character_index];
                while (character_index < line_buffer.length() && !isletter(line_buffer[character_index])){
                    character_index++;
                    if (character_index < line_buffer.length()) {
                        current_input_char = line_buffer[character_index];
                    } else {
                        break;
                    }
                }
                if (character_index >= line_buffer.length()) break;
                char char_category = GetCharType(current_input_char, (character_index + 1 < line_buffer.length()) ? line_buffer[character_index + 1] : '\0');
                bool transition_found = false;
                for(const auto& rule_entry : resulting_dfa.Transition_Rule){
                    if((current_automaton_state == rule_entry.now) && (char_category == rule_entry.rec)){
                        current_automaton_state = rule_entry.next;
                        accumulated_token_string.append(1, current_input_char);
                        if(resulting_dfa.final.count(current_automaton_state)){
                            int lookahead_state = -1;
                            char next_char_category = '\0';
                            if (character_index + 1 < line_buffer.length()) {
                                char peek_next = line_buffer[character_index + 1];
                                char peek_next_next = (character_index + 2 < line_buffer.length()) ? line_buffer[character_index + 2] : '\0';
                                next_char_category = GetCharType(peek_next, peek_next_next);
                            }
                            bool lookahead_transition_exists = false;
                            for(const auto& lookahead_rule : resulting_dfa.Transition_Rule){
                                if((current_automaton_state == lookahead_rule.now) && (next_char_category == lookahead_rule.rec)) {
                                    lookahead_state = lookahead_rule.next;
                                    lookahead_transition_exists = true;
                                    break;
                                }
                            }
                            if (!lookahead_transition_exists || !resulting_dfa.final.count(lookahead_state)){
                                std::cout << accumulated_token_string << "\t" << tokenize(current_automaton_state, accumulated_token_string, resulting_dfa) << std::endl;
                                tokens_output_file << accumulated_token_string << "\t" << tokenize(current_automaton_state, accumulated_token_string, resulting_dfa) << std::endl;
                                build_table(current_automaton_state, accumulated_token_string, resulting_dfa);
                                current_automaton_state = *resulting_dfa.start.begin();
                                accumulated_token_string = "";
                            }
                        }
                        transition_found = true;
                        break;
                    }
                }   
            }      
        }
       }
    input_source_file.close();
    tokens_output_file.close();
}


int main(){
    FA NFA, DFA, minimize_dfa;
    NFA.input_symbols = input_symbols;   
    NFA.start = lex_start;
    NFA.final = lex_final;
    NFA.Transition_Rule = transmission_map;
    NFA.state_labels = state_labels;
    DFA = deter_nfa(NFA);
    minimize_dfa = minimize_fa(DFA);
    perform_lexical_analysis("01_var_defn.sy", minimize_dfa,"1");
    perform_lexical_analysis("02_var_defn.sy", minimize_dfa,"2");
    perform_lexical_analysis("03_var_defn.sy", minimize_dfa,"3");
    perform_lexical_analysis("04_var_defn.sy", minimize_dfa,"4");
    perform_lexical_analysis("05_var_defn.sy", minimize_dfa,"5");
}