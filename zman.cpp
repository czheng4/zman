#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include "include/json.hpp"
using namespace nlohmann;
using namespace std;


#define DIR_NAME "/.zman"
#define TABLE_PATH DIR_NAME + "/table.json"
#define MAX_FILE_SIZE 100000000  // 100 MB
typedef std::runtime_error SRE;


inline void get_json_from_file(json &j, const string &filename) {
  ifstream ifs;
  j.clear();

  ifs.open(filename, ios::in);
  if (ifs.fail()) throw SRE ("Couldn't open file " + filename);
  ifs >> j;
  ifs.close();
}

inline void write_json_to_file(const json &j, const string &filename) {
  ofstream ofs;

  ofs.open(filename, ios::out | ios::trunc);
  if (ofs.fail()) throw SRE ("Couldn't open file " + filename);
  ofs << j;
  ofs.close();
}


class Man {
public:
  Man(const char *a, const char *entry_name);
  ~Man() {};

  void add(const string &entry_name);
  void create_default_file();
  void init_json_file(const string &filename);
  void set_entry(json &j, const string &entry_name, const string &filename, bool overwrite);
  void get_entry(const string &entry_name);

  string home;
};


/** Get the home path and 
   create ~/DIR_NAME directory and ~/TABLE_PATH file if they do not exist */
void Man::create_default_file() {
  struct stat buf;
  ofstream o;
  string s;
  

  home = (string) getenv("HOME");
  s = home + DIR_NAME;

  if (stat(s.c_str(), &buf) < 0) {
    if (mkdir(s.c_str(), 0700) == -1) {
      throw SRE("Can't create directory " + s);
    }
  }

  s = home + TABLE_PATH;
  if (stat(s.c_str(), &buf) < 0) init_json_file(s);
  
}


void Man::get_entry(const string &entry_name) {
  json table, entries;
  string s;

  get_json_from_file(table, home + TABLE_PATH);

  if (table.contains(entry_name)) {
    get_json_from_file(entries, table[entry_name]);
    cout << (string) entries[entry_name] << endl;
  } else {
    throw SRE("No entry for " + entry_name);
  }
}


void Man::init_json_file(const string &filename) {
  ofstream o;
  o.open(filename, ios::out);
  if (o.fail()) throw SRE ("Can't create file " + filename);
  o << "{}";
  o.close();
}



void Man::set_entry(json &j, const string &entry_name, const string &filename, bool overwrite) {
  string s, line;
  ofstream o;

  if (overwrite) s = "";
  else s = j[entry_name];

  while (getline(cin, line)) s += line + "\n";

  s.pop_back();
  cout << s << endl;
  j[entry_name] = s;
  write_json_to_file(j, filename);
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
    get_entry(entry_name);
  } else {
    throw SRE("actions must be one of add|append|remove|list|get");
  }
}


void Man::add(const string &entry_name) {
  json table;
  json content;
  string filename;
  fstream f, entry_f;
  string s;
  char c;
  char buf[1024];
  int i;
  struct stat stat_buf;

  s = home + TABLE_PATH;

  get_json_from_file(table, s);
  
  if (table.contains(entry_name)) {
    sprintf(buf, "Entry %s exists. Use overwrite action to overwrite it", entry_name.c_str());
    throw SRE (buf);
  } 

  i = 0;
  while (1) {
    sprintf(buf, "%s%s/zman_%d.json", home.c_str(), DIR_NAME, i);
    if (stat(buf, &stat_buf) < 0) { 
      content = json::object();
      break;
    }
    if (stat_buf.st_size < MAX_FILE_SIZE) {
      get_json_from_file(content, (string) buf);
      break;
    }
    i++;
  }

  table[entry_name] = (string) buf;
  write_json_to_file(table, s);
  set_entry(content, entry_name, buf, true);
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

  try {
    man = new Man(argv[1], argc >= 3 ? argv[2] : NULL);
  } catch (SRE &e) {
    cerr << e.what() << endl;
  }





}