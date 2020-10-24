#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sstream>
#include <map>
#include <vector>
#include <unistd.h> 
#include <queue>

using namespace std;

int lines = 0;
int lastSecond = 0;
double avgInputTransRate = 0.0;
double outputTransRate = 0.0;
double outputTime = 0.0;

map<char,string> addr;
map<char,string> :: iterator itrAddr;       // Stores addresses
map<string,int> priority;
map<string,int> :: iterator itrPrior; 		// Stores priority
map<string,int> startTime;
map<string,int> :: iterator itrStart;		// Stores start times
map<string,int> dataLength;
map<string,int> :: iterator itrLength;		// Stores length of each data block

ofstream outfile;
const char * outfilename = "priority.txt";  // Temporary output file


// Prints frame output to the terminal/screen
void printOutput (vector<string> startBasedBlocks[]){

	// Priority queue to store and pop out data based on priority
	priority_queue <int> priorityQueue; 
	string tempArr[lines-1];
	
	// Run till last second (9 for given input) is achieved
	for (int i = 0; i < lastSecond; i++){
		string tempArr[lines-1];

		/* For each data block having same start time, find its priority, store its name in an array, 
		   and push priority in queue. */

		for (int j = 0; j< startBasedBlocks[i].size() ; j++){

			itrPrior = priority.find(startBasedBlocks[i].at(j));
			tempArr[(itrPrior -> second) - 1] = startBasedBlocks[i].at(j);
			int priority = -1 * (itrPrior -> second);	
			priorityQueue.push(priority);

		}

		// Pop all data blocks with same start time
		while (!priorityQueue.empty()) {

			// Start of frame
			if(outputTime == 0){
				cout <<"SF\n";
			}

			/* Value at top of queue would be data block with highest priority. 
			   So it is popped first. */

			int topVal = -1 * (priorityQueue.top());
			string block = tempArr[topVal - 1];
			itrAddr = addr.find(block[0]);

			// Print address of data block
			cout << itrAddr -> second<<", ";

			// Calculate time to transmit packets  for a particular data block
			itrLength = dataLength.find(block);

			/* Schedule output frame based on output trans. rate. and length of data block
			   1.0/outputTransRate = 1/2 sec = 0.5 sec is the duration of each sub frame. */

			double timeElapsed = (1.0/outputTransRate) * itrLength -> second;

			// Print duration of data block
			cout<< outputTime<<" "<< outputTime + timeElapsed <<" "<<block<<"\n";

			// Keep adding time as you go along while printing the frames and blocks
			outputTime = outputTime + timeElapsed;
			
			/* Terminate frame after its duration of 1 sec, i,e, each frame consists of 2 subframes
               of duration 0.5 each. */

			int checkInt = int(outputTime);
			if((checkInt == outputTime) && outputTime != lastSecond){
				cout << "EF"<<"\n";
				cout << "SF"<<"\n";
			}

			// End of frame
			if(outputTime == lastSecond){
				cout <<"EF\n";
			}	

			priorityQueue.pop();
		}	
	}
}


// Fills up array of vectors with block data as per their start time
void fillBlockData(){

	ifstream file;
	file.open(outfilename);
    string line;
    int longPcktLength = 0;
    int lineNo = 0;
   
   // Reads priority.txt line by line
    while(getline(file,line)){
    	
    	lineNo++;
    	string token;
    	stringstream ss(line);

    	// Tokenizes each line
    	while(getline(ss,token,',')){

    		int duration[2] = {0};

    		// Removes any stray space after colon (:)
    		if(token[8] == ' '){
    			token.erase(token.begin()+8);
    		}
    		
    		string subToken;
    		stringstream s(token);
			string timeDuration;
    		int k = 0;

    		// Tokenizes sections separated by space
    		while(getline(s,subToken,' ')){
    		
    			const char * ptr = subToken.c_str();
    			if(ptr[0] == 'S'){
    				ptr = &ptr[8];
    				if(ptr[0] == ' '){
    					ptr++;
    				}
    			}	

    			k++;

    			// If token is NOT the data block name or the third token
    			if(k != 3){
    				int time;
    				stringstream z;
    				z << ptr;
    				z >> time;
 	
 					// Calculates last second/highest time value till which input is being generated
    				if(time > lastSecond){
    					lastSecond = time;
    				}

    				// If it is first token, then it is start time
    				if( k == 1){
    					duration[0] = time;
    				}

    				// If it is second token, then it is end time
    				else {
    					duration[1] = time;
    				}
    			}
  			
    			else {
    				// Fill up priority for that data block using priority.txt
		  			priority[subToken] = lineNo;

		  			// Set start time and length of data block 
		  			startTime[subToken] = duration[0];	
    				int diff = duration[1] - duration[0];
    				dataLength[subToken] = diff;

    				if( diff > 1 ){
    					longPcktLength += diff - 1;
    				}
    			}				
    		}		
    	}
    }
 
    file.close();

    /* Fill an array of vectors where each index represents integral start time
       and corresponding vector represents list of data blocks with that start time. */

    vector<string> startBasedBlocks[lastSecond + 1];
    for(itrStart = startTime.begin(); itrStart != startTime.end(); ++itrStart) { 
    	startBasedBlocks[itrStart -> second].push_back(itrStart -> first);
    } 

    /* Calculate avg. transmission rate for input and using same rate for output,
       since output rate can be >= input rate. So, for input.txt, both rates would be 2 packets/sec. */ 

    double rate = double(priority.size() + longPcktLength)/lastSecond;
    outputTransRate = avgInputTransRate = rate;

    printOutput(startBasedBlocks);
}



// Takes input.txt as the input file
int main (int argc, char * argv[]){

	if (argc != 2){
		cout << "Please enter input file name after "<<argv[0]<<"\n";
		exit(1);                           
	}

	string infilename = argv[1];
	ifstream infile;
	const char * fname = infilename.c_str();
	infile.open(fname);

	if(!infile.is_open()){
		cout << "Couldn't open file! : "<<infilename<<"\n";
		exit(1);
	}

	string lineFromFile;

	/* Calculating number of lines in the input file as it is not deterministic
	   Extra credit criteria */

	while(getline(infile,lineFromFile)){	
		lines++;
	}
	
	string line[lines];

	// Resetting pointer to start of file
	infile.clear();                                   
    infile.seekg(0,infile.beg);

    // Storing lines from input.txt into an array
    int i = 0;
    while(getline(infile,line[i])){
		i++;
	}

	infile.close();

	/* Below code section takes the input sources from input.txt, re-arranges them as 
	   per priority given in last line and then writes to a file 'priority.txt'. So,the 
	   source with highest priority is at line 1, all the way down to least priority source. */

	outfile.open(outfilename);
	if(!outfile.is_open()){
		cout << "Couldn't open file! : "<<outfilename<<"\n";
		exit(1);
	}
	
	string tokenLastLine;
	stringstream ss(line[lines-1]);
	const char * a;
	const char * b;

	// Tokenizing last line having source priority
	while(getline(ss,tokenLastLine,' ')){ 

		/* For each tokenized source, searches the input, and outputs the source/line
		   from max priority to min priority */
		for (int j = 0; j < lines-1; j++){
			a = line[j].c_str();
			b = tokenLastLine.c_str();	
			string q;
			stringstream c;
			c << j;
			c >> q;

			addr[a[6]] = q;

			if(a[6] == b[0]){

				// writes to file
				outfile << line[j]<<"\n";
			}
		}
	}  
	   
    outfile.close();  

   	fillBlockData();
          
	return 0;
}