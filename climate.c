/**
 * climate.c
 *
 * Performs analysis on climate data provided by the
 * National Oceanic and Atmospheric Administration (NOAA).
 *
 * Input:    Tab-delimited file(s) to analyze.
 * Output:   Summary information about the data.
 *
 * Compile:  run make
 *
 * Example Run:      ./climate data_tn.tdv data_wa.tdv
 *
 *
 * Opening file: data_tn.tdv
 * Opening file: data_wa.tdv
 * States found: TN WA
 * -- State: TN --
 * Number of Records: 17097
 * Average Humidity: 49.4%
 * Average Temperature: 58.3F
 * Max Temperature: 110.4F on Mon Aug  3 11:00:00 2015
 * Min Temperature: -11.1F on Fri Feb 20 04:00:00 2015
 * Lightning Strikes: 781
 * Records with Snow Cover: 107
 * Average Cloud Cover: 53.0%
 * -- State: WA --
 * Number of Records: 48357
 * Average Humidity: 61.3%
 * Average Temperature: 52.9F
 * Max Temperature: 125.7F on Sun Jun 28 17:00:00 2015
 * Min Temperature: -18.7F on Wed Dec 30 04:00:00 2015
 * Lightning Strikes: 1190
 * Records with Snow Cover: 1383
 * Average Cloud Cover: 54.5%
 *
 * TDV format:
 *
 * CA» 1428300000000»  9prcjqk3yc80»   93.0»   0.0»100.0»  0.0»95644.0»277.58716
 * CA» 1430308800000»  9prc9sgwvw80»   4.0»0.0»100.0»  0.0»99226.0»282.63037
 * CA» 1428559200000»  9prrremmdqxb»   61.0»   0.0»0.0»0.0»102112.0»   285.07513
 * CA» 1428192000000»  9prkzkcdypgz»   57.0»   0.0»100.0»  0.0»101765.0» 285.21332
 * CA» 1428170400000»  9prdd41tbzeb»   73.0»   0.0»22.0»   0.0»102074.0» 285.10425
 * CA» 1429768800000»  9pr60tz83r2p»   38.0»   0.0»0.0»0.0»101679.0»   283.9342
 * CA» 1428127200000»  9prj93myxe80»   98.0»   0.0»100.0»  0.0»102343.0» 285.75
 * CA» 1428408000000»  9pr49b49zs7z»   93.0»   0.0»100.0»  0.0»100645.0» 285.82413
 *
 * Each field is separated by a tab character \t and ends with a newline \n.
 *
 * Fields:
 *      state code (e.g., CA, TX, etc),
 *      date (time of observation as a UNIX date),
 *      geolocation (geohash string),
 *      humidity (0 - 100%),
 *      snow (1 = snow present, 0 = no snow),
 *      cloud cover (0 - 100%),
 *      lightning strikes (1 = lightning strike, 0 = no lightning),
 *      pressure (Pa),
 *      surface temperature (Kelvin)
 */

/*
    Author: Meghana Gadiraju - 20459994
    Project 3
    Professor: Vahab
    Date: November 29,2018

*/

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_STATES 50

/* TODO: Add elements to the climate_info struct as necessary. */
struct climate_info 
{
    char code[3]; //state code
    unsigned long num_records;//number of records per state
    long double temp;//sum of temperatures in Kelvin (need to be converted to fahrenheit)

    long double humidity;//humidity (0-100)
    long double avg_humidity;//averge humidity

    int snow;//(0 = no snow, 1 = snow)
    long double clouds;// cloud cover (0-100)
    long double avg_clouds;//average cloud cover

    long double sum_temperatures;//average temperature
    
    long double max_temp;//maximum temperature in Kelvin, need to convert to fahernheit
    long int max_date;//UNIX time that needs to be converted to a readable date/time

    long double min_temp;//minimum temperature in Kelvin, need to convert to fahernheit
    long int min_date;//UNIX time that needs to be converted to a readable date/time

    int lightning;//lightning strikes (0 = no lightning, 1 = lightning)
    int date;//UNIX time that needs to be converted to a readable date/time
};


struct climate_info *newState(char* tok[], int tok_index);//declaring struct
void analyze_file(FILE *file, struct climate_info *states[], int num_states);//declaring method
void print_report(struct climate_info *states[], int num_states);//declaring method

int main(int argc, char *argv[]) //argc is argument count (# of arguments), argv is the vector of arguments ()
{

    FILE *fp;
    /* TODO: fix this conditional. You should be able to read multiple files. */
    if (argc < 2) //if # of arguments does not equal 2
    {
        printf("Usage: %s tdv_file1 tdv_file2 ... tdv_fileN \n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Let's create an array to store our state data in. As we know, there are
     * 50 US states. */
    struct climate_info *states[NUM_STATES] = { NULL };

    int i;
    for (i = 1; i < argc; ++i) 
    {
         fp = fopen(argv[i], "r");//file pointer 
         /* TODO: Open the file for reading */
       if (fp  == NULL )
        {
         /* TODO: If the file doesn't exist, print an error message and move on * to the next file. */
            fprintf( stderr, "%s: Failed to open file: %s!\n", argv[0], argv[i] );
            return EXIT_FAILURE;
        }
        /* TODO: Analyze the file */
        analyze_file(fp, states, NUM_STATES);       
    }

    /* Now that we have recorded data for each file, we'll summarize them: */
    print_report(states, NUM_STATES);
    fclose( fp );//closes file
    return 0;//success
}

struct climate_info *findState(char* line)//looks for/tokenizes each line of the files.
{ 
    char* token = strtok(line, "\t");
    char* tok[9];

    int tok_index = 0;

    while (token != NULL && tok_index < 9)//tokenizing
    {
        tok[tok_index++] = token;
        token = strtok(NULL, "\t");
    }
    return newState(tok, tok_index);
}

struct climate_info *newState(char* tok[], int tok_index)//allocates space for an instance of the struct for each state
{ 
    struct climate_info* state = malloc(sizeof(struct climate_info));//allocating space
    strncpy(state->code, tok[0], 2);
    char time[20];//creating a char instead of dividing UNIX time by 1000. If this isn't done, there would be integer overload
    strncpy(time, tok[1], 10);//limits characters to 10, which basically divides time by 1000
    state -> date = atoi(time);//atoi because string representation of an int
    state->clouds = atof(tok[5]);//atof to return a double
    state->lightning = atof(tok[6]);
    state->humidity = atof(tok[3]);
    state->snow = atof(tok[4]);
    state->temp = ((atof(tok[8]))*9/5 - 459.67);

    return state;
}


void add_State(struct climate_info *states[], struct climate_info *state, int index)//adds new state to states array
{
        if (states[index] == NULL)
        {
            states[index] = state;
            states[index]->num_records = 1;
        }
}

void newInfo(struct climate_info *states[], struct climate_info *state, int index)//adds info to each state if the state already has instance of a struct
{
    long double max_temp = state->temp;
    long double min_temp = state->temp;

    states[index]->num_records++;
    states[index]->temp += state->temp;//to convert to fahernheit from  *9/5 and - 459.67

    states[index]->humidity += state->humidity;
    states[index]->clouds += state->clouds; 
    
    states[index]->lightning += state->lightning;//checks for lightning strikes
    states[index]->snow += state->snow;//checks for snow cover  

    states[index]->avg_humidity = ((states[index]-> humidity)/states[index]->num_records);//finds the average humidity

    states[index]->sum_temperatures = (states[index]-> temp);//finds the average temperature


    if (min_temp < states[index]->min_temp)//Finds the min temperature
    {
        states[index]->min_temp = state->temp;
        states[index]->min_date = state->date;
    }
   
    if (max_temp > states[index]->max_temp)//Finds the max temperature and date of temperature
    {
        states[index]->max_temp = state->temp;
        states[index]->max_date = state->date;
    }



    states[index]->avg_clouds = ((states[index]->clouds)/states[index]->num_records);//finds the average cloud cover

}

/* TODO function documentation */
void analyze_file(FILE *file, struct climate_info **states, int num_states) //adds info to state that already exists, adds a new state and updates info for that state if it doesn't exist yet. 
{
    /* TODO: We need to do a few things here:
         *
         *       * Tokenize the line.
         *       * Determine what state the line is for. This will be the state
         *         code, stored as our first token.
         *       * If our states array doesn't have a climate_info entry for
         *         this state, then we need to allocate memory for it and put it
         *         in the next open place in the array. Otherwise, we reuse the
         *         existing entry.
         *       * Update the climate_info structure as necessary.
         */
    const int line_sz = 100;
    char line[line_sz];

    while (fgets(line, line_sz, file) != NULL)
    {
        struct climate_info *state = findState(line);

        if (state == NULL)
        {
            return;
        }
        char *code = state->code;
        for(int i = 0; i < num_states;i++)                  
        {                                                   
            if (states[i] == NULL) //adds a new state
            {                                         
                add_State(states, state,i);                 
                break;                                      
            } 
            else //updates info
            {                                        
                int clim_val = strcmp(states[i]->code, code);    
                if (clim_val == 0)                               
                {                                           
                    newInfo(states, state, i);             
                    break;                                  

                }                                           
            }                                               
        }           
    }
}


void print_report(struct climate_info *states[], int num_states)//printing info
{
    printf("States found: ");
    int i;
    for (i = 0; i < num_states; ++i) 
    {
        if (states[i] != NULL) 
        {
            struct climate_info *info = states[i];
            printf("%s ", info->code);
        }
    }
    printf("\n");

    for (i = 0; i < num_states; ++i) 
    {
        if (states[i] != NULL) 
        {
            printf("State: %s\nNumber of Records: %lu\n", states[i]->code,states[i]->num_records); 
            printf("Average Humidity: %0.1Lf%s\nAverage Temperature: %0.1Lf%s\n",states[i]->avg_humidity,"%",(states[i]->sum_temperatures/states[i]->num_records),"F");

            printf("Max Temperature: %0.1Lf%s", states[i]->max_temp,"F on ");
            printf("%s", ctime(&states[i]->max_date));
            printf("Min Temperature: %0.1Lf%s", states[i]->min_temp, "F on ");
            printf("%s", ctime(&states[i]->min_date));

            printf("Lightning Strikes: %d\nRecords with Snow Cover: %d\nAverage Cloud Cover: %0.1Lf%s\n", states[i]->lightning,states[i]->snow, states[i]->avg_clouds, "%\n__________________________________________\n");
        }
    }
}
