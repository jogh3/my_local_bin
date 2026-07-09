#include <curl/curl.h>
#include <stdio.h>
#include <string.h>
#include <stdexcept>
#include <iostream>
#include <vector>

struct entry{
  std::string title;
  std::string size;
  std::string hash;
  std::string seeders;
  std::string leeches;
};

size_t write_callback(void *contents,size_t size,size_t nmemb, void *userp){
  size_t realsize = size * nmemb;
  std::string *mem = (std::string *)userp;
  mem->append((char *)contents,realsize);
  return realsize;
}
std::string getinfo(std::string data, std::string key) {
  int kloc = data.find(key)+key.length();
  std::vector<int> locations;
  int res = -1;
  while ((res = data.find("\"",res+1)) != std::string::npos){
    locations.push_back(res);
  }
  for (int i = 0; i < locations.size(); i++){
    if (locations.at(i) > kloc && (data[locations.at(i)+1] == ',' || data[locations.at(i)+1] == '}')){
      return data.substr(kloc,locations.at(i)-kloc);
    }
  }
  return "unknown"; 
}

std::vector<struct entry> query_site(std::string url, CURL *ptr){ 
  std::string sitedata;
  // struct curl
  curl_easy_setopt(ptr,CURLOPT_COOKIEFILE,"");
  curl_easy_setopt(ptr, CURLOPT_USERAGENT,"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36");
  curl_easy_setopt(ptr,CURLOPT_URL,url.c_str());
  curl_easy_setopt(ptr,CURLOPT_WRITEFUNCTION,write_callback);
  curl_easy_setopt(ptr,CURLOPT_WRITEDATA,&sitedata);
  curl_easy_perform(ptr);
  //std::cout << sitedata[0] << std::endl;
  std::vector<std::string> items;
  int isgoing = 1;
  int start,end;
  while (isgoing == 1){
    start = sitedata.find("{");
    end = sitedata.find("}");
    if (start == std::string::npos || end == std::string::npos) {
      isgoing = 0;
    } else{
      items.push_back(sitedata.substr(start,end-start+1));
      sitedata.erase(start,end-start+1);
    }
  }
  sitedata.clear();
  // std::cout << items.at(0) << std::endl;
  std::vector<struct entry> parsed;
  struct entry singular;
  long long bsize;
  double gbsize;
  for (int i = 0; i < items.size(); i++){
    // std::cout << "entry " << i << " i: " << items.at(i) << "\n" << std::endl;
    singular.title = getinfo(items.at(i),"\"name\":\"");
    std::string size_str = getinfo(items.at(i), "\"size\":\"");
    if(size_str != "unknown" && size_str != "") {
        bsize = std::stoll(size_str);
        size_str.erase();
    } else {
        bsize = 0;
    }
    gbsize = bsize/1000000000.0;
    char buffer[64];
    snprintf(buffer,64,"%.2f",gbsize);
    singular.size = buffer;
    singular.size += "GB";
    singular.hash = getinfo(items.at(i),"\"info_hash\":\"");
    singular.seeders = getinfo(items.at(i),"\"seeders\":\"");
    singular.leecs = getinfo(items.at(i),"\"leechers\":\"");
    parsed.push_back(singular);
  }
  return parsed;
}

int main(int argc, char* argv[]) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  CURL *ptr = curl_easy_init();
  if (ptr != NULL) {
    // std::cout << "found ptr\n";
    std::string mode = argv[1];
    if ( mode == "q" ){
      std::vector<struct entry> fout = query_site(argv[2],ptr);
      for (int i = 0; i < fout.size(); i++){
        struct entry singular = fout.at(i);
        std::cout << singular.title << "|" << singular.size << "|" << singular.hash << "|" << singular.seeders << "|" << singular.leeches << std::endl;
      }     
    } else {throw std::runtime_error("no argument submitted");}

  } else{throw std::runtime_error("curl ptr is null");}
  
  return 0;
}

