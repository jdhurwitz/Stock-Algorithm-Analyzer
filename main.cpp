//
//  main.cpp
//  StockTool
//  Phase 3 of the stock analyzer.
//  Created by Jonathan Hurwitz on 12/15/15.
//  Copyright © 2015 Jonathan Hurwitz. All rights reserved.
//

/*
-In
 */

#include <cassert>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include "dataProcessor.hpp"
#include <cmath>
#include <thread>
#include "constants.h"

#define UPPER_LIM          5
#define STEP               0.5
#define UPPER_CHUNK_LIM    3247
#define LOWER_CHUNK_LIM    2996
#define SELL_PERCENTAGE    0.5



using namespace std;

#define MAX_PATH           1024
string ABS_PATH  = "/Users/Jonny/Documents/Algorithmic Stock Trading/Divided Data (by day)/Chunk ";

//Used for DP input
/*
 For chunks at day level granularity, use 3247 as an upper limit since one stock is 
 missing from the last 5 day trading period.
 */





//static const int num_threads = (UPPER_CHUNK_LIM - LOWER_CHUNK_LIM)+1;
static const int num_threads = (int)(SPREAD_UPPER/SPREAD_LOWER);
static const int MAXSIZE = 20;


struct stock{
    string ticker;
    double buy;
    double sell;
};



/*Multithreaded processing*/
void process(int t_id, vector<dataProcessor*> dpvec){
    cout<<t_id<<endl;
    
    

    
    //Perform processing
   // for(int i = LOWER_CHUNK_LIM; i <= UPPER_CHUNK_LIM; i++){
   for(int i=0; i < dpvec.size(); i++){
        dpvec[i]->processAll();
    }
    
    
    //    }
    
/*  DP parameter list
    dir, filelist, chunk num, upper lim, step
    
 
    dataProcessor *dparr[(UPPER_CHUNK_LIM-LOWER_CHUNK_LIM)+1];
    for(int i = LOWER_CHUNK_LIM; i <= UPPER_CHUNK_LIM; i++){
        string dir = ABS_PATH + to_string(i) + "/";
        //cout<<"Dir of target file: "<<dir<<endl;
        dparr[i-1] = new dataProcessor(dir, filenames, i, UPPER_LIM, STEP);
        
    }
    
    //Perform processing
    for(int i = LOWER_CHUNK_LIM; i <= UPPER_CHUNK_LIM; i++){
        dparr[i-1]->processAll();
    }
    */
}



int main(int argc, const char * argv[]) {

    char buf[MAX_PATH];
    cout<<getcwd(buf, MAX_PATH)<<endl;
    
    ifstream file("filelist.txt");
    string str;
    vector<string> filenames;
    while(getline(file, str)){
        filenames.push_back(str);
    }
    
    int num_sp_percs = SPREAD_UPPER/SPREAD_LOWER;
    vector<double> spread_percs;
    
    double cur_perc = SPREAD_LOWER;
    for(int i = 0; i<num_sp_percs; i++){
//        spread_percs[i] = cur_perc;
        spread_percs.push_back(cur_perc);
        cur_perc += SPREAD_STEP;
    }
    

    assert(("Vector size and spread perc num differ.", spread_percs.size() == num_sp_percs));
    
    vector<vector<dataProcessor*>> king_vec;
    vector<dataProcessor*> dpvec;
   
    for(int i = 0; i < num_sp_percs; i++){
        dpvec.clear();
        for(int j = LOWER_CHUNK_LIM; j <= UPPER_CHUNK_LIM; j++){
            string dir = ABS_PATH + to_string(j) + "/";
            dataProcessor* dpptr = new dataProcessor(dir, filenames, j, UPPER_LIM, STEP, SELL_PERCENTAGE, OPTION_CLOSE_SELL, spread_percs[i]);
            dpvec.push_back(dpptr);
            
        }

        king_vec.push_back(dpvec);
        
    }
   
    

    //Create threads
    if(num_threads != king_vec.size()){
        perror("Analyzer wants to create # threads != vector size.");
        exit(1);
    }
    
    thread t[num_threads];
    for(int i = 0; i < num_threads; ++i){
        t[i] = thread(process, i, king_vec[i]);
    }
    
    //Join all threads
    for(int i = 0; i< num_threads; ++i){
        t[i].join();
    }
    
    vector<vector<dataProcessor*>>::iterator i;
    vector<dataProcessor*>::iterator j;
    for(i = king_vec.begin(); i != king_vec.end(); ++i){
        
        for(j = i->begin(); j != i->end(); ++j){
            delete (*j);
        }
        i->clear();
    }
    //dataProcessor DP("filelist.txt", 12, 5, 0.5);
    //DP.processAll();
   
    

    /*
     DP parameter list
     dir, filelist, chunk num, upper lim, step
     
     Get path
   
    
    dataProcessor *dparr[(UPPER_CHUNK_LIM-LOWER_CHUNK_LIM)+1];
    for(int i = LOWER_CHUNK_LIM; i <= UPPER_CHUNK_LIM; i++){
        string dir = ABS_PATH + to_string(i) + "/";
        //cout<<"Dir of target file: "<<dir<<endl;
        dparr[i-1] = new dataProcessor(dir, filenames, i, UPPER_LIM, STEP);

    }
    
    //Perform processing
    for(int i = LOWER_CHUNK_LIM; i <= UPPER_CHUNK_LIM; i++){
        dparr[i-1]->processAll();
    }
      */


    return 0;
    
}
