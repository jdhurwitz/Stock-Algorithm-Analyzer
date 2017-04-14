//
//  dataProcessor.cpp
//  StockTool
//
//  Created by Jonathan Hurwitz on 12/21/15.
//  Copyright © 2015 Jonathan Hurwitz. All rights reserved.
//

#include "dataProcessor.hpp"
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>


using namespace std;

int NUM_DAYS_IN_CHUNK = 1;


dataProcessor::dataProcessor(string dir, vector<string> filelist,
                             int chunk_num, double limit, double step,
                             double sell_perc, int sell_option, double spread_perc){
    
    if(filelist.empty())
        perror("Empty filelist. \n");
    
    m_numdays = 0;
    m_curclose = 0;
    m_curopen = 0;
    m_path = dir;
    m_chunknum = chunk_num;
    m_spread_perc = spread_perc;
    m_sell_perc = sell_perc;
    m_sell_option = sell_option;
    
    //Build vector of all file names
    m_filenames = filelist;
    
    

    
    m_numstocks = (int)m_filenames.size();
    

    /*TODO
     
     Change the directory from /path/Stocktool
     to /path/Divided Data/Chunk n
     */
    
    
    for(int i = 0; i < m_filenames.size(); i++){
        m_filenames[i] += to_string(chunk_num);
    }
    
    
    //Figure out number of days
    //Assume all files have same #
    string first_file = m_filenames[0]+".csv";
    m_numdays = findNumDays(first_file);
    
    m_ndtoprocess = m_numdays;
    cout<<"Num days to process: "<<m_ndtoprocess<<endl;
    
    if(PERFORM_FILE_CHECK){
        cout<<"Verifying that all files have the same # of days..."<<endl;
        if(verifyNumDays())
            cout<<"Num day verification passed."<<endl;
    }
    
    if(setupPtable(limit, step) == 0)
        cout<<"Percentage table setup for day: "+to_string(m_chunknum)+", spread: "+to_string(m_spread_perc)+"."<<endl;

    //Setup vectors
    for(int i = 0; i< m_numstocks; i++){
        vector<double> tempvec;
        tempvec.resize(m_percentages.size());
        fill(tempvec.begin(), tempvec.end(), 1);
        m_ongoing.push_back(tempvec);
        m_day.push_back(tempvec);
    }

   // tickers.close();
}


dataProcessor::~dataProcessor(){
    cleanUp();
}

void dataProcessor::setAllFalse(){
    map<double, bool>::iterator it;
    for(it = m_percentages.begin(); it != m_percentages.end(); it++){
        it->second = false;
    }
    return;
}

bool dataProcessor::verifyNumDays(){
    vector<string>::iterator it;
    it = m_filenames.begin();
    //Obtain first
    string prev_file = *it;
    int n_days_prev = findNumDays(*it);
    int n_days_cur = 0;
    it++;
    while(it != m_filenames.end()){
        n_days_cur = findNumDays(*it);
        if(n_days_prev != n_days_cur){
            cout<<"Files have different number of days"<<endl;
            return false;
        }
        //We matched alright
        else{
            cout<<"OK for files: "<<prev_file<<","<<*it<<endl;
            prev_file = *it;
            n_days_prev = n_days_cur;
        }
        it++;
    }
    return true;
}


int dataProcessor::findNumDays(string filename){
    ifstream file(m_path + filename);
    string s;
    int day_count = 0;
    string prev_day = "";
    
    //skip first line
    //getline(file, s);
    
    while(getline(file, s)){
        string ss = s.substr(3,2);
        if(ss != prev_day){
            day_count++;
            //cout<<ss<<endl;
            prev_day = ss;
        }
    }
    file.close();
    return day_count;
}

void dataProcessor::cleanUp(){
    map<double, multimap<int, dnode*>*>::iterator it;

    for(auto it = m_ptable.begin(); it != m_ptable.end(); it++){
        //free each map
        multimap<int, dnode*>::iterator m_it;
        for(m_it = it->second->begin(); m_it != it->second->end();){
           // multimap<int, dnode*>::iterator erase_it = m_it;
            
            if(m_it->second != NULL){
                delete m_it->second;
                m_it->second = NULL;
                m_it = it->second->erase(m_it);
            }else
                m_it++;
        }
    }
    return;
}

int dataProcessor::setupPtable(double p_limit, double step){
   // if(fmod(num_data_points, p_limit) != 0){
     //   cout<<"Invalid limit or data points"<<endl;
       // return -1;
    //}
    if(step > p_limit){
        cout<<"Step larger than largest data point."<<endl;
        return -1;
    }
    
    int num_data_pts = 0;
    double num_dp_dec = p_limit/step;
    if(floor(num_dp_dec) == num_dp_dec){
        num_data_pts = num_dp_dec;
    }else{
        cout<<"Non-integer # of data points"<<endl;
        return -1;
    }
    
    for(int i = 0; i<= num_data_pts; i++){
        double perc = ((step*i)/100);
        
        //double perc = 0;
        ///if(fmod(perc_temp, 100.00) == 0)
           // double perc = perc_temp/ 100.00;
        //multimap<double, dnode*>* temp = new multimap<double, dnode*>;
        multimap<int, dnode*>* temp = new multimap<int, dnode*>;
        m_ptable[perc] = temp;

        //Add percentage to the vector
    //    m_percentages.push_back(perc);
        m_percentages[perc] = false;
    }

    
    return 0;
}


/*
 Helper functions to extract values from string.
 Some code is duplicated because of return value.
 
 OPEN   = 1
 CLOSE  = 2
 LOW    = 3
 */
double dataProcessor::findPrice(string line, int p_type){
    int comparison_val = 0;
    switch(p_type){
        case OPEN:
            comparison_val = 2;
            break;
        case HIGH:
            comparison_val = 3;
            break;
        case CLOSE:
            comparison_val = 5;
            break;
        case LOW:
            comparison_val = 4;
            break;
        default:
            comparison_val = 0;
    }
    if(comparison_val == 0){
        cout<<"Cannot find specified price. Check p_type parameter."<<endl;
        return -1;
    }
    
    int num_commas = 0;
    string price = "";
    for(int i = 0; i < line.length(); i++){
        if(line[i] == ',')
            num_commas++;
        
        if(num_commas >= comparison_val && num_commas < (comparison_val+1) && line[i] != ','){
            price += line[i];
        }
    }
    string::size_type sz;
    return stod(price, &sz);
}

double dataProcessor::findValInFile(string filepath, double sell_goal){
    //use findPrice
    ifstream cur_file(filepath);
    string line = "";
    double line_sell_price = 0;
    
    while(getline(cur_file, line)){
        line_sell_price = findPrice(line, HIGH);
        if(line_sell_price >= sell_goal){
            return line_sell_price;
        }
    }
    
    //If the sell isn't found
    return VAL_NOT_FOUND;
}

int dataProcessor::findTime(string line){
    int num_commas = 0;
    string time = "";
    for(int i = 0; i < line.length(); i++){
        if(line[i] == ',')
            num_commas++;
        if(num_commas >= 1 && num_commas < 2 && line[i] != ','){
            time += line[i];
        }
    }
    string::size_type sz;
    return stod(time, &sz);
}


string dataProcessor::createTwoDec(double price){
    string s = to_string(price);
    string output = "";
    int dist = 0;
    int num_wanted = 0;
    
    bool pt_found = false;
    for(int i = 0; i < s.length(); i++){
        if(s[i] == '.')
            pt_found = true;
        if(pt_found)
            dist++;
        if(dist > 3)
            break;
        output += s[i];
        num_wanted++;

    }
    return output.substr(0, num_wanted);
}





void dataProcessor::processAll(){
    for(int i = 1; i <= m_ndtoprocess; i++){
        processDay(i);
        
        double zcol_avg = 0;
        
        /*CHECK FOR DROPS THAT HAPPENED AT THE SAME TIME
         -Average the values
         -Store the averaged value as the sell/buy value

         */
        map<double, multimap<int, dnode*> *>::iterator it;
        for(it = m_ptable.begin(); it != m_ptable.end(); it++){
            multimap<int, dnode*> mm_temp = *(it->second);
            multimap<int, dnode*>::iterator mm_it;
            
            //Iterating through multimap with times as keys
            //Go through and mark duplicates
            vector<double> dup_vals;

            for(mm_it = mm_temp.begin(); mm_it != mm_temp.end(); mm_it++){
                multimap<int, dnode*>::iterator dup_it1 = mm_temp.lower_bound(mm_it->first);
                multimap<int, dnode*>::iterator dup_it2 = mm_temp.upper_bound(mm_it->first);
                multimap<int, dnode*>::iterator dup_temp = mm_temp.lower_bound(mm_it->first);
                dup_temp = dup_it1;

                
                //only one element
                if(mm_temp.count(dup_it1->first) == 1)
                    continue;
                else{
                    while(dup_temp != dup_it2){
                        dup_vals.push_back(dup_temp->second->result);
                        dup_temp++;
                    }
                    
                    long double sum = 0;
                    long double average = 0;
                    //Sum all things in the vector in average
                    for(int i = 0; i < dup_vals.size(); i++){
                        sum += dup_vals[i];
                    }
                    //Calculate average
                    average = sum/dup_vals.size();
                    
                    //Go through and set all result values to average
                    dup_temp = dup_it1;
                    while(dup_temp != dup_it2){
                        dup_temp->second->result = average;
                        dup_temp++;
                    }
                    if(dup_it2 != mm_temp.end())
                        mm_it = dup_it2;
                }

            }

        }
        
        /*
         ZERO COLUMN
         -All cells will have the same value since this is averaged
         -Calculate average and insert it into all cells
         */
        if(m_zcol.size() == m_filenames.size()){
            cout<<"Zero column number matches expected."<<endl;
            double sum = 0;
            for(int i = 0; i < m_zcol.size(); i++){
                sum += m_zcol[i];
            }
            zcol_avg = sum/(m_zcol.size());
        }
        //cout<<m_zcol[0]<<endl;
        
        /*
         WRITE EVERYTHING TO DATA STRUCTURE
         -Iterate through each percentage drop
            -Add 0 column value (zcol_avg)
            -Add in all stocks that dropped in order of map
            -For all remaining entries, add a 1

        ***Iterate through columns not rows (memory layout).
         */
        map<double, multimap<int, dnode*> *>::iterator p_it = m_ptable.begin();
        multimap<int, dnode*>::iterator m_it = p_it->second->begin();
        
        int c = 0;
        while(p_it != m_ptable.end()){
            m_it = p_it->second->begin();
            for(int j = 0; j < m_filenames.size(); j++){
                if(c == 0){
                    m_day[j][c] = zcol_avg;
                    continue;
                }
                if(m_it != p_it->second->end()){
                    
                    
                    m_day[j][c] = m_it->second->result;
                    m_it++;
                }else{
                    //write a 1
                    m_day[j][c] = 1;
                }
                
               // m_it++;
            }
            
            p_it++;
            c++;
        }

        
        /*
         MULTIPLY values of m_ongoing to m_day and store result
         in corresponding cell of m_ongoing
         */
        for(int i = 0; i < m_filenames.size(); i++){
            for(int j = 0; j < m_percentages.size(); j++){
                m_ongoing[i][j] = m_ongoing[i][j] * m_day[i][j];
            }
        }
        
        //Clear data structures for the next day
        m_zcol.clear();
       // m_zcol.resize(0);
        cleanUp();
        
        //Write day by day
       // writeToCsv(i);
    }
    
    writeToCsv(m_chunknum);
    return;
}



void dataProcessor::addEntryToPercent(double perc, int time, string ticker,
                                      double buy, double sell, double result){
    
    dnode* temp = new dnode;
    temp->buy = buy;
    temp->ticker = ticker;
    temp->sell = sell;
    temp->result = result;
    temp->hasDuplicate = false;
    
   // multimap<int, dnode*>* mm_temp = new multimap<int, dnode*>;
    //(*mm_temp).insert(pair<int, dnode*>(time, temp));
    m_ptable[perc]->insert(pair<int, dnode*>(time, temp));
    
    
    return;
}


double dataProcessor::getAdjustResult(double sell){
    //Calculate spread difference
    double trade_penalty = 0.015;
    double spread_penalty = m_spread_perc*sell;
    int sp2;
    double tDigit = (spread_penalty*10000);
    int tDigitCheck = (int) tDigit%100;
    if(tDigitCheck != 0){
        spread_penalty += 0.01;
        spread_penalty*=100;
        sp2 = spread_penalty;
        spread_penalty = ((double)sp2)/100;
    }
    
    return (sell - (spread_penalty+trade_penalty));
    
}


void dataProcessor::addCloseToEntry(double sell, string ticker){
    map<double, std::multimap<int, dnode*> *>::iterator it;
    multimap<int, dnode*>::iterator mm_it;

    double adjusted_sell = getAdjustResult(sell);
    
    
    for(it = m_ptable.begin(); it != m_ptable.end(); it++){
        for(mm_it = (it->second)->begin(); mm_it != (it->second)->end(); mm_it++){
            if(mm_it->second->ticker == ticker){
                mm_it->second->sell = adjusted_sell;
                mm_it->second->result = adjusted_sell/(mm_it->second->buy);
            }
        }
    }
    return;
}

/*
 Scan through a number of days including current day to find a target percentage drop. 
 Sell on that drop.
 
Values to store in node:
 -File name
 -Whole line of data file
 
 */




void dataProcessor::printResults(){
    map<double, std::multimap<int, dnode*> *>::iterator it;
    multimap<int, dnode*>::iterator mm_it;
    
    for(it = m_ptable.begin(); it != m_ptable.end(); it++){
        for(mm_it = (it->second)->begin(); mm_it != (it->second)->end(); mm_it++){
           // mm_it->second->sell = close;
            //mm_it->second->result = close/(mm_it->second->buy);
            cout<<"Ticker: "<<mm_it->second->ticker<<", buy: "<<mm_it->second->buy<<", sell: "<<mm_it->second->sell<<", result: "<<mm_it->second->result;
            cout<<endl;
            
        }
    }
    
    return;
}




void dataProcessor::writeToCsv(int day){
    string d = to_string(day);
    
    /*Check to see if directory exists
     R/W/S permissions for owner, group, and R/S for others.
     */
    int status;
    status = mkdir(WRITE_PATH.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    
    
    ofstream ofs(WRITE_PATH+"results (day= "+d+", spperc= "+to_string(m_spread_perc)+".txt", ofstream::out);
    
    /*Indicates when a cell is positive or not
    "positive matrix file stream*/
    ofstream p_matrixfs(WRITE_PATH+"positive_matrix"+d+".txt", ofstream::out);
    
    string first = "";
    string pos_matrix_line = "";
    map<double, bool>::iterator it = m_percentages.begin();
    
    //Build first line
    for(int i = 0; i < m_percentages.size(); i++){
        
        first += to_string(it->first);

        if(i == m_percentages.size()-1)
            first += "\n";
        else
            first += ", ";
        
        it++;
    }
    ofs << first;
    p_matrixfs << first;
    

    for(int j = 0; j < m_ongoing.size(); j++){
        string line = "";
        pos_matrix_line = "";
        for(int k = 0; k < m_ongoing[j].size(); k++){
            if(m_ongoing[j][k] > 1)
                pos_matrix_line += "1";
            else
                pos_matrix_line += "0";
            
            line += to_string(m_ongoing[j][k]);

            if( k == m_ongoing[j].size()-1){
                line += "\n";
                pos_matrix_line += "\n";
            }
            else{
                line += ", ";
                pos_matrix_line += ", ";
            }
        }
        ofs << line;
        p_matrixfs << pos_matrix_line;
        
        
    }
    
    ofs.close();
    p_matrixfs.close();
    return;
}



void dataProcessor::processDay(int day){
    vector<string>::iterator it;
    
    for(it = m_filenames.begin(); it < m_filenames.end(); it++){
        //File operations
        string stock_file = *it+".csv";
        ifstream cur_file(m_path + stock_file);
        string temp_str;
            
        string prev_day = "";
        int day_count = 0;
            
        //skip first line
        //getline(cur_file, temp_str);
        
        double day_open = 0;
        double prev_sell = 0;
        double close_price = 0;
        
        //Selected a point other than the close
        double custom_sell_pr = 0;
        

        //Parse file line by line
        while(getline(cur_file, temp_str)){
            string ss = temp_str.substr(3,2);
            double open_price = findPrice(temp_str, OPEN);
            m_curopen = open_price;
            
            double low_price = findPrice(temp_str, LOW);
        

            close_price = findPrice(temp_str, CLOSE);
            
            
            
            m_curclose = close_price;
            
            int time = findTime(temp_str);

            //new day
            if(ss != prev_day){
                day_count++;
                if(day_count > day)
                   // cout<<"Recording a new zcol val: "<<to_string(prev_sell/day_open)<<endl;
                    m_zcol.push_back(prev_sell/day_open);
                
                day_open = open_price;
                setAllFalse();
                
               // if(day_count > 1)
                 //   addCloseToEntry(prev_sell);
          
            }
            
            if(day_count == day){
                //parallelize this
                map<double, bool>::iterator i;
                for(i = m_percentages.begin(); i != m_percentages.end(); i++){
                    double drop_goal_full = day_open*(1-(i->first));
                    
                    double drop_goal = floor(drop_goal_full*100)/100;
                    double sell_goal = 0;
                    
                    /*Calculate new sell point
                    switch(m_sell_option){
                        case OPTION_CONCRETE_SELL:
                            
                        case OPTION_FLOATING_SELL:
                            
                        case OPTION_CLOSE_SELL:
                        default:
                            
                            //change this shit
                            break;
                    }*/
                    
                    //Handle 0 column case
                    if(i->first == 0){
                        
                        continue;
                    }

                    /*
                     If the current price of the stock == drop goal
                     WE FOUND A DROP for that percent!
                    */
                    string low = createTwoDec(low_price);
                    string goal = createTwoDec(drop_goal_full);
                    
                    if(low_price <= drop_goal_full && !(i->second)){
                        double time = findTime(temp_str);
                        cout<<"Ticker: "<<*it<<endl;
                        cout<<temp_str<<ss<<endl;
                        cout<<(i->first)<<" drop at: "<<time<<endl;
                        cout<<"Low was: "<<low<<" and goal was: "<<goal<<endl;
                        cout<<"Open was: "<<day_open<<endl;
                        cout<<endl;
                        
                        i->second = true;
                        
                        
                        /* DESIGN CHANGE NOTE
                         low_price is the actual purchase price, so drop_goal_full should be
                         removed and replaced by low_price to increase data accuracy.
                         */
                        double sell_res = 0;
                        switch(m_sell_option){
                            case OPTION_CONCRETE_SELL:
                                sell_goal = day_open*(1+m_sell_perc);
                                sell_res = findValInFile(m_path+stock_file, sell_goal);
                                
                                if(sell_res != VAL_NOT_FOUND)
                                    addEntryToPercent(i->first, time, *it, drop_goal, sell_res, (sell_res/drop_goal));
                                else
                                    //sell at close
                                
                                break;

                            case OPTION_FLOATING_SELL:
                                sell_goal = drop_goal_full*(1+m_sell_perc);
                                sell_res = findValInFile(m_path + stock_file, sell_goal);
                                
                                if(sell_res != VAL_NOT_FOUND)
                                    addEntryToPercent(i->first, time, *it, drop_goal, sell_res, (sell_res/drop_goal));
                                else
                                    //sell at close
                                break;
                            case OPTION_CLOSE_SELL:
                            default:
                                addEntryToPercent(i->first, time, *it, drop_goal);
                                break;
                        }
                        continue;
                    }
                
                }
                prev_day = ss;
                switch(m_sell_option){
                    case OPTION_CONCRETE_SELL:
                        //do nothing
                    case OPTION_FLOATING_SELL:
                        //do nothing
                    case OPTION_CLOSE_SELL:
                    default:
                        prev_sell = close_price;
                }
            }
            
            /*
             This statement will do nothing when chunk granularity 
             is on order of days.
             */
            else if(day_count > day && day != NUM_DAYS_IN_CHUNK){
               // double sell = open_price;
             //   m_zcol.push_back(sell/day_open);
                addCloseToEntry(prev_sell, *it);
                break;
            }
           /* This code section is not needed for chunks at day level granularity.
                If larger chunks are used, it needs to be brought back and support 
                added for the different sell options.
            
            else if(day_count < day){
                prev_day = ss;
                prev_sell = close_price;
                continue;
            }*/

        }
    
        if(day == NUM_DAYS_IN_CHUNK){
            
            //This variable exists because a switch was being used to handle other options
            double zcol_sell = 0;
            
            addCloseToEntry(close_price, *it);
            double adjusted_sell = getAdjustResult(prev_sell);

            m_zcol.push_back(adjusted_sell/day_open);
        }
        
        cur_file.close();
    }

    return;
}