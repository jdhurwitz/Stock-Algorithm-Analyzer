//
//  dataProcessor.hpp
//  StockTool
//
//  Created by Jonathan Hurwitz on 12/21/15.
//  Copyright © 2015 Jonathan Hurwitz. All rights reserved.
//

#ifndef dataProcessor_hpp
#define dataProcessor_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <list>
#include "constants.h"

/*
 Day node will be stored in the multi map (int->day node).
 
 */
struct dnode{
    std::string ticker;
    double buy;
    double sell;
        
    //Sell price w/ spread perc but not fee
    double sell_adj;
    
    //(sell/buy)
    double result;
    bool hasDuplicate;
};

struct pnode{
    double percentage;
    int day;
};

class dataProcessor{
public:
    dataProcessor() {}
    dataProcessor(std::string dir, std::vector<std::string> filelist,
                  int chunk_num, double limit, double step, double sell_perc,
                  int sell_option, double spread_perc);
    int findNumDays(std::string filename);
    bool verifyNumDays();
    ~dataProcessor();
    
    void processAll();
    void processDay(int day);
    void storeValue(int day, std::string ticker, int buy, int sell);
    
    //Number of data points between two integers 0 -> 1 in 0.25 would be 4 not 3
    int setupPtable(double p_limit, double step);
    
    /*
     Default parameters for usage when not doing other sell point strategies 
     
     */
    void addEntryToPercent(double perc, int time, std::string ticker,
                           double buy, double sell = 0, double result = 0);
    void addCloseToEntry(double sell, std::string ticker);
    double getAdjustResult(double sell);
    void printResults();
    void writeToCsv(int day);
    void cleanUp();
    
    //Helper functions
    std::string createTwoDec(double price);
    double findPrice(std::string line, int p_type);
    
    //Identify a sell point in a file
    double findValInFile(std::string file, double sell_goal);
    int findTime(std::string line);
    
    int getNumDays(){ return m_numdays; }
    void setNumDays(int numDays){ m_numdays = numDays; }
    
private:
    std::string              m_path;
    int                      m_chunknum;
    std::vector<std::string> m_filenames;
    
    /*0th column for a day.
     Sell = close
     Buy = open
     */
    std::vector<double> m_zcol;

    /*
     map percentages->*multimap<int, dnode*>
     
     Percentage mapping for one day
     */
    std::map<double, std::multimap<int, dnode*> *> m_ptable;
    
    
    //Map from percentages->map(time, sell/buy)
   // std::map<double, std::map<int, double>> day_final;
    
    //2D matrices for final output
    std::vector<std::vector<double>> m_ongoing;
    std::vector<std::vector<double>> m_day;

   // std::vector<double> m_percentages;
    
    //Double = percentage, Bool = found or not
    std::map<double, bool> m_percentages;
    void setAllFalse();
    
    double m_curclose;
    double m_curopen;
    
    //Target sell percentage
    double m_sell_perc;
    
    //spread percentage
    double m_spread_perc;
    
    //Which type of sell strategy we use
    int m_sell_option;
    
    int m_numstocks;
    int m_ndtoprocess; //# days to process
    int m_numdays;
};
#endif /* dataProcessor_hpp */
