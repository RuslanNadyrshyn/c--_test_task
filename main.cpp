// g++ main.cpp -o main //-pthread
// ./main 1.txt

#include <future>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <set>

using namespace std;

int getFileSize(std::string filename) // path to file
{
    FILE *p_file = NULL;
    p_file = fopen(filename.c_str(),"rb");
    fseek(p_file,0,SEEK_END);
    int size = ftell(p_file);
    fclose(p_file);
    return size;
}

int getChunks(string filename, size_t numberThreads, int *starts, int *ends){
  std::ifstream  file;
  file.open(filename);
  
  if (file.is_open()) {
    const int fileSize = getFileSize(filename);
    const size_t blockSize = fileSize/numberThreads;
    ends[numberThreads-1] = fileSize;

    for(int i=1; i < numberThreads; i++) {
      char mychar;
      file.seekg(i*blockSize);

      while(file.good()) {
        file.get(mychar);
        if( !mychar || mychar == ' ') {
          ends[i-1] = file.tellg();
          starts[i] = file.tellg();
          break;
        }
      }
    }
  } else {
    std::cout << "File '" << filename << "' doesn't exists!" << endl;
    return -1;
  }
  file.close();
  return 0;
}

set<string> readWordsFromFile(streampos start, streampos end, string filename)
{
  set<string> dictionary;
  std::ifstream file;
  
  file.open(filename);
  
  if (file.is_open()) {
    string temp = "";
    char mychar;

    file.seekg(start);
    
    while (file) {
  		mychar = file.get();
  		if (mychar == ' ') {
  				dictionary.insert(temp);
  				temp = "";
  		} else if (mychar >= 'a' && mychar <= 'z') {
  				temp.push_back(mychar);
  		} 
      if (file.tellg() == end) {
        dictionary.insert(temp);
        return dictionary;
      }
	  }
  }
  file.close();
  
  return dictionary;
}


int main(int argc, char *argv[]) {
  if (argc < 2){
    cout << "Use './main filename.txt'" << endl;
    return 1;
  }
  
  const string   filename = argv[1];
  const size_t   numberThreads = std::thread::hardware_concurrency();
  int*           starts = new int[numberThreads];
  int*           ends = new int[numberThreads];
  set<string>    resultDictionary;
  std::ifstream  file;
  starts[0] = 0;
  
  int a = getChunks(filename, numberThreads, starts, ends);
  if (a == -1) {
    return -1;
  }
  
  for(int i = 0; i < numberThreads; i++) {
      auto start = starts[i];
      auto end = ends[i];
      auto result = std::async(std::launch::async, [start, end, filename]() {
          return readWordsFromFile(start, end, filename);
        });
    result.wait();
    
    auto res = result.get();
    resultDictionary.insert(res.begin(), res.end());
  }

  for (auto it = resultDictionary.begin(); it != resultDictionary.end(); ++it)
			std::cout << *it << " " << endl;

  std::cout << "Unique words: " << resultDictionary.size() << endl;
  delete [] starts;
  delete [] ends;

  return 1;
}
