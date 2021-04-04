#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include "include/json.hpp"
using namespace nlohmann;
using namespace std;


#define STORE_DIR_PATH "/.zman/"
#define MAX_FILE_SIZE 100000000  // 100 MB
typedef std::runtime_error SRE;


class Man {
public:
  Man(const char *a, const char *entry_name);
  ~Man() {};

  void add(const char *entry_name);
  void create_default_file();

  string user_path;
};



void Man::create_default_file() {
  FILE *fp;
  struct stat buf;
  int exists;
  char table[200], user[200];
  ofstream o;
  int i;

  fp = popen("echo ~", "r");
  i = fread(user, sizeof(char), 200, fp);
  user[i-1] = '\0';
  pclose(fp);

  strcpy(table, user);
  strcat(table, STORE_DIR_PATH);
  user_path = (string) table;

  if (stat(table, &buf) < 0) {
    cout << mkdir(table, 0744) << endl;
  }

  strcat(table, "table.json");
  if (stat(table, &buf) < 0) {
    o.open(table, ios::out);
    o << "{}";
    o.close();
  }


  
}
Man::Man(const char *a, const char *entry_name) {
  string s;

  s = (string) a;

  create_default_file();
  if (s == "add") {
    if (entry_name == NULL) throw SRE("add action must have entry name");
    add(entry_name);

  } else if (s == "append") {

  } else if (s == "remove") {

  } else if (s == "list") {

  } else if (s == "get") {

  } else {
    throw SRE("actions must be one of add|append|remove|list|get");
  }
}


void Man::add(const char *entry_name) {
  json table;
  string filename;
  ifstream f;
  string s;

  s = user_path + "table.json";

  f.open(s, ios::in);

  f >> table;
  cout << table << endl;




  f.close();


}

void usage() {
  cerr << "zman action [entry_name]" << endl;
  cerr << "Actions:" << endl;
  cerr << "  add :   add a new entry. It overwrites the content if the entry exists" << endl;
  cerr << "  append: append the new content to a entry" << endl;
  cerr << "  remove: remove a entry" << endl;
  cerr << "  list:   list all enties" << endl;
  cerr << "  get:    get the content of an entry" << endl;
}


int main (int argc, char **argv) {
  
  string a;
  Man *man;

  if (argc == 1) { usage(); return 0; }

  a = argv[1];

  man = new Man(argv[1], argc >= 3 ? argv[2] : NULL);





}