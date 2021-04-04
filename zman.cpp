#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <cstdio>
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

  /* if the json is empty, we simply delete them */
  if (j.size() == 0 && filename.find("table.json") == string::npos) {
    if (remove(filename.c_str()) != 0) throw SRE ("Couldn't open file " + filename);
    return;
  }

  ofs.open(filename, ios::out | ios::trunc);
  if (ofs.fail()) throw SRE ("Couldn't open file " + filename);
  ofs << j;
  ofs.close();
}

inline string to_lowercase(const string &from) {
  size_t i;
  string s = from;
  for (i = 0; i < s.size(); i++) {
    if (s[i] >= 'A' && s[i] <= 'Z') s[i] += 'a' - 'A';
  }
  return s;
}


class Man {
public:
  Man(const char *a, const char *entry_name);
  ~Man() {};

protected:

  void create_default_file();
  void set_entry(json &j, const string &entry_name, const string &filename, bool append);
  void add_entry(const string &entry_name, bool append, bool overwrite = false);
  void get_entry(const string &entry_name);
  void remove_entry(const string &entry_name);
  void list_entries();
  void get_all_entries(vector <string> &entries);
  void similar_entries(const string &entry_name);

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
  if (stat(s.c_str(), &buf) < 0) write_json_to_file(json::object(), s);
  
}

void Man::similar_entries(const string &entry_name) {
  vector <string> keys;
  string s, s2, rs;
  size_t i;
  get_all_entries(keys);

  rs = "";
  s = to_lowercase(entry_name);
  for (i = 0; i < keys.size(); i++) {
    s2 = to_lowercase(keys[i]);
    if (s.find(s2) != string::npos || s2.find(s) != string::npos) rs += "\"" + keys[i] + "\", ";
  }

  if (rs != "") {
    rs.pop_back();
    rs.pop_back();
    printf("Did you mean %s\n", rs.c_str());
  }

}

void Man::remove_entry(const string &entry_name) {
  json table, entries;

  get_json_from_file(table, home + TABLE_PATH);

  if (table.contains(entry_name)) {
    get_json_from_file(entries, table[entry_name]);
    entries.erase(entry_name);
    write_json_to_file(entries, table[entry_name]);

    table.erase(entry_name);
    write_json_to_file(table, home + TABLE_PATH);

    printf("Delete entry %s successfully\n", entry_name.c_str());
  } else {
    similar_entries(entry_name);
    throw SRE("No entry for " + entry_name);
  }
}

void Man::get_entry(const string &entry_name) {
  json table, entries;

  get_json_from_file(table, home + TABLE_PATH);

  if (table.contains(entry_name)) {
    get_json_from_file(entries, table[entry_name]);
    cout << (string) entries[entry_name] << endl;
  } else {
    similar_entries(entry_name);
    throw SRE("No entry for " + entry_name);
  }
}


void Man::list_entries() {

  vector <string> keys;
  size_t i;

  get_all_entries(keys);

  if (keys.size() == 0) {
    cout << "Entries are empty" << endl; 
    return;
  }

  sort(keys.begin(), keys.end());

  for (i = 0; i < keys.size(); i++) cout << keys[i] << endl;
}

void Man::get_all_entries(vector <string> &keys) {
  json table;

  get_json_from_file(table, home + TABLE_PATH);
  for (auto it = table.begin(); it != table.end(); ++it) {
    keys.push_back(it.key());
  }
}



void Man::set_entry(json &j, const string &entry_name, const string &filename, bool append) {
  string s, line;
  ofstream o;

  if (append && j.contains(entry_name)) s = (string) j[entry_name] + "\n";
  else s = "";

  while (getline(cin, line)) s += line + "\n";

  s.pop_back();
  j[entry_name] = s;
  write_json_to_file(j, filename);
}


Man::Man(const char *a, const char *entry_name) {
  string s;

  s = (string) a;

  create_default_file();
  if (s == "add") {
    if (entry_name == NULL) throw SRE("add action must have entry name");
    add_entry(entry_name, false);

  } else if (s == "append") {
    if (entry_name == NULL) throw SRE("append action must have entry name");
    add_entry(entry_name, true);

  } else if (s == "remove" || s == "rm") {
    if (entry_name == NULL) throw SRE("remove action must have entry name");
    remove_entry(entry_name);

  } else if (s == "list" || s == "l") {
    list_entries();

  } else if (s == "get" || s == "g") {
    if (entry_name == NULL) throw SRE("get action must have entry name");
    get_entry(entry_name);

  } else if(s == "overwrite" || s == "o") {
    if (entry_name == NULL) throw SRE("overwrite action must have entry name");
    add_entry(entry_name, false, true);

  } else {
    throw SRE("actions must be one of add_entry|append|remove|list|get|overwrite");
  }
}


void Man::add_entry(const string &entry_name, bool append, bool overwrite) {
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
  
  if (table.contains(entry_name) && !append && !overwrite) {
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

  if (append == false && overwrite == false) {
    table[entry_name] = (string) buf;
    write_json_to_file(table, s);
  }

  set_entry(content, entry_name, buf, append);
  if (append) printf("Append to entry %s successfully\n", entry_name.c_str());
  else if (overwrite) printf("Overwrite entry %s successfully\n", entry_name.c_str());
  else printf("Add new entry %s successfully\n", entry_name.c_str());
}

void usage() {
  cerr << "zman action [entry_name]" << endl;
  cerr << "Actions:" << endl;
  cerr << "  add:         add a new entry" << endl;
  cerr << "  o/overwrite: overwrite a entry with new content" << endl;
  cerr << "  append:      append the new content to a entry" << endl;
  cerr << "  rm/remove:   remove a entry" << endl;
  cerr << "  l/list:      list all enties" << endl;
  cerr << "  g/get:       get the content of an entry" << endl;
  cerr << endl;
  cerr << "zman reads the content from stdin" << endl;
}


int main(int argc, char **argv) {
  
  string a;
  Man *man;

  if (argc == 1) { usage(); return 1; }

  a = argv[1];

  try {
    man = new Man(argv[1], argc >= 3 ? argv[2] : NULL);
  } catch (SRE &e) {
    cerr << e.what() << endl;
    return 1;
  }

  delete man;
  return 0;

}