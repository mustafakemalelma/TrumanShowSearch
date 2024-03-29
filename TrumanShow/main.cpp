#include <iostream>
#include <fstream>

using namespace std;

void computeLPSArray(char* pat, int M, int* lps);
void KMPSearch(char* pat, char* txt, char* otherPat, bool isFirstWordMissing);
size_t hopWord(size_t initial, bool isForward, char* txt, char* pat, bool isCornerWordMissing);
bool checkOther(size_t foundIndex, char* pat, char* lastPat, char* txt);
void getFirstPart(string statement, size_t emptyBoxIndex, char** firstStatementPart);
void getLastPart(string statement, size_t emptyBoxIndex, char** lastStatementPart);

int main(int argc, const char * argv[]) {
    
    auto start = chrono::high_resolution_clock::now();
    int count = 0;
    
    //Get the file and store in a char array
    ifstream trumanShowFile("the_truman_show_script.txt");
    string contents((istreambuf_iterator<char>(trumanShowFile)),istreambuf_iterator<char>());
    char trumanShow[contents.length()];
    strcpy(trumanShow, contents.c_str());
    
    ifstream statementFile("statements.txt");
    if(statementFile.is_open()) {
        string statement;
        
        while(getline(statementFile, statement)) {
            count++;
            size_t emptyBoxIndex = string::npos;
            for(int i = 0; i < statement.length() - 2; i++) {
                if(statement[i] == '_' && statement[i+1] == '_' && statement[i+2] == '_')
                    emptyBoxIndex = i;
            }
            
            char* firstStatementPart = nullptr;
            char* lastStatementPart = nullptr;
            
            if(emptyBoxIndex == 0) { //First word is missing
                getLastPart(statement, emptyBoxIndex, &lastStatementPart);
                KMPSearch(lastStatementPart, trumanShow, nullptr, true);
            }
            else if(emptyBoxIndex == statement.length() - 3) { // Last word is missing
                getFirstPart(statement, emptyBoxIndex, &firstStatementPart);
                KMPSearch(firstStatementPart, trumanShow, nullptr, false);
            }
            else { // One of the middle words is missing
                getFirstPart(statement, emptyBoxIndex, &firstStatementPart);
                getLastPart(statement, emptyBoxIndex, &lastStatementPart);
                KMPSearch(firstStatementPart, trumanShow, lastStatementPart, false);
            }
            
            if(firstStatementPart != nullptr) delete[] firstStatementPart;
            if(lastStatementPart != nullptr) delete[] lastStatementPart;
        }
        
        statementFile.close();
    }
    
    trumanShowFile.close();
    
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
    
    cout << "Statement Count: " << count << endl;
    cout << "Duration: " << duration.count() << "ms" << endl;
    
    return 0;
}

void getFirstPart(string statement, size_t emptyBoxIndex, char** firstStatementPart) {
    string firstPart = statement.substr(0, emptyBoxIndex);
    *firstStatementPart = new char[firstPart.length()];
    strcpy(*firstStatementPart, firstPart.c_str());
}

void getLastPart(string statement, size_t emptyBoxIndex, char** lastStatementPart) {
    unsigned long lastPartSize = statement.length() - (emptyBoxIndex + 3);
    string lastPart = statement.substr(emptyBoxIndex + 3, lastPartSize);
    *lastStatementPart = new char [lastPart.length()];
    strcpy(*lastStatementPart, lastPart.c_str());
}

size_t hopWord(size_t initial, bool isForward, char* txt, char* pat, bool isCornerWordMissing) {
    size_t nextSpace = initial;
    size_t patLength = strlen(pat);
    string finishers = " !?,.";
    
    while(true) {
        if(nextSpace >= patLength + initial) return string::npos;
        char nextChar = txt[nextSpace];
        
        size_t foundIndex = string::npos;
        if(isCornerWordMissing) {
            if(nextChar == ' ') foundIndex = 1;
        }
        else {
            for(int i = 0; i < finishers.length(); i++) {
                if(finishers[i] == nextChar)
                    foundIndex = i;
            }
        }
        
        if(foundIndex != string::npos) {
            if(isCornerWordMissing)break;
            
            if(isForward && pat[0] == nextChar) break;
            else if(!isForward && pat[patLength - 1] == nextChar) break;
        }
        
        if(isForward)
            nextSpace++;
        else
            nextSpace--;
        
        if(nextSpace == 0) break;
    }
    
    return nextSpace;
}

bool checkOther(size_t foundIndex, char* pat, char* lastPat, char* txt) {
    size_t patLength = strlen(pat);
    size_t missingStartingPoint = foundIndex + patLength;
    
    //Hop 1 word
    size_t nextSpace = hopWord(missingStartingPoint, true, txt, lastPat, false);
    if(nextSpace == string::npos) return false;
    
    //Last part check
    size_t lastStatementLength = strlen(lastPat);
    long foundLast = nextSpace;
    for (size_t i = 0; i < lastStatementLength - 1; i++){
        if(txt[nextSpace + i] != lastPat[i]){
            foundLast = -1;
            break;
        }
    }
    
    if(foundLast != -1){
        size_t lengtOfMissingWord = foundLast - missingStartingPoint;
        string parsedTxt = txt;
        cout << "Original: " << pat << "___" << lastPat << endl;
        cout << "Found: " << pat << parsedTxt.substr(missingStartingPoint, lengtOfMissingWord) << lastPat << endl;
        cout << "---------" << endl;
        
        return true;
    }
    
    return false;
}

void KMPSearch(char* pat, char* txt, char* otherPat, bool isFirstWordMissing) {
    int M = (int) strlen(pat);
    int N = (int) strlen(txt);
    
    int lps[M];
    
    computeLPSArray(pat, M, lps);
    
    int i = 0;
    int j = 0;
    while (i < N) {
        if (pat[j] == txt[i]) {
            j++;
            i++;
        }
        
        if (j == M) {
            if(otherPat != nullptr) {
                if(checkOther(i-j, pat, otherPat, txt)) return;
            }
            else {
                string parsedTxt = txt;
                if(isFirstWordMissing) {
                    //Hop 1 word backward
                    size_t nextSpace = hopWord(i-j - 1, false, txt, pat, true);
                    
                    cout << "Original: " << "___" << pat << endl;
                    cout << "Found: " << parsedTxt.substr(nextSpace, i-j-nextSpace) << pat << endl;
                    cout << "---------" << endl;
                }
                else {
                    //Hop 1 word forward
                    size_t nextSpace = hopWord(i-j + M, true, txt, pat, true);
                    
                    cout << "Original: " << pat << "___" << endl;
                    cout << "Found: " << pat << parsedTxt.substr(i-j + M, nextSpace - (i-j+M)) << endl;
                    cout << "---------" << endl;
                }
                
                return;
            }
            
            j = lps[j - 1];
        }
        else if (i < N && pat[j] != txt[i]) {
            if (j != 0)
                j = lps[j - 1];
            else
                i = i + 1;
        }
    }
    
    cout << "---------" << endl;
    cout << "Statement not found!" << endl;
    cout << "---------" << endl;
        
}

void computeLPSArray(char* pat, int M, int* lps) {
    int len = 0;
    
    lps[0] = 0;
    
    int i = 1;
    while (i < M) {
        if (pat[i] == pat[len]) {
            len++;
            lps[i] = len;
            i++;
        }
        else {
            if (len != 0) {
                len = lps[len - 1];
            }
            else {
                lps[i] = 0;
                i++;
            }
        }
    }
}
