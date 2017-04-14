//
//  constants.h
//  StockTool
//
//  Created by Jonathan Hurwitz on 5/6/16.
//  Copyright © 2016 Jonathan Hurwitz. All rights reserved.
//

#ifndef constants_h
#define constants_h


//File redundancy check
#define PERFORM_FILE_CHECK   0

//For variable not found
#define VAL_NOT_FOUND        42.42




/*constants for spread
 0.01% -> 0.2%
 */
static const double SPREAD_LOWER = 0.0001;
static const double SPREAD_UPPER = 0.002;
static const double SPREAD_STEP  = 0.0001;

enum SellType{
    OPTION_CLOSE_SELL,
    OPTION_FLOATING_SELL,
    OPTION_CONCRETE_SELL

};

//These are the possible values on a line in the stock file.
enum LineValue{
    OPEN,
    CLOSE,
    LOW,
    TIME,
    HIGH,
    
};

//For writing data
static const std::string WRITE_PATH = "/Users/Jonny/Documents/Algorithmic Stock Trading/Divided Data (by day)/Spread Results/";


#endif /* constants_h */
