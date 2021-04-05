#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <map>
using namespace std;


#define DIR_NAME "/.zman"
#define TABLE_PATH DIR_NAME + "/table.txt"
#define MAX_FILE_SIZE 100000000  // 100 MB
typedef std::runtime_error SRE;


inline void get_map_from_file(map <string, string> &m, const string &filename) {
  FILE *f;
  int size;
  char *key, *val;
  string s;
  int i;


  f = fopen(filename.c_str(), "r");
  if (f == NULL) throw SRE ("Couldn't open file " + filename);
  
  while (fread(&size, sizeof(int), 1, f) == 1) {
    key = new char[size + 1];
    key[size] = '\0';
    if (fread(key, sizeof(char), size, f) != size) throw SRE("Internal error: file is being tampered");
    if (fread(&size, sizeof(int), 1, f) != 1) throw SRE("Internal error: file is being tampered");

    val = new char[size + 1];
    val[size] = '\0';

    if (fread(val, sizeof(char), size, f) != size) throw SRE("Internal error: file is being tampered");

    s = "";
    for (i = 0; i < size; i++) s += val[i];
    m.insert(make_pair((string)key, s));

    delete key;
    delete val;
  }

  fclose(f);
}

inline void write_map_to_file(const map <string, string> &m, const string &filename) {
  FILE *f;
  int size;
  map <string, string>::const_iterator mit;

  /* if the json is empty, we simply delete them */
  if (m.size() == 0 && filename.find("table.txt") == string::npos) {
    if (remove(filename.c_str()) != 0) throw SRE ("Couldn't open file " + filename);
    return;
  }

  f = fopen(filename.c_str(), "w");
  if (f == NULL) throw SRE ("Couldn't open file " + filename);
  
  for (mit = m.begin(); mit != m.end(); ++mit) {
    size = (mit->first).size();
    fwrite(&size, sizeof(int), 1, f);
    fwrite((mit->first).c_str(), sizeof(char), size, f);

    size = (mit->second).size();
    fwrite(&size, sizeof(int), 1, f);
    fwrite((mit->second).c_str(), sizeof(char), size, f);
  }

  fclose(f);
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
  void set_entry(const string &entry_name, const string &filename, bool append);
  void add_entry(const string &entry_name, bool append, bool overwrite = false);
  void get_entry(const string &entry_name);
  void remove_entry(const string &entry_name);
  void list_entries();
  void similar_entries(const string &entry_name);

  string home;
  map <string, string> entries; /* key is the entry_name and val is filename where it stores the content */
  map <string, string> content; /* key is the entry_name and val is its content */
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
  if (stat(s.c_str(), &buf) < 0) write_map_to_file(entries, s);
}

void Man::similar_entries(const string &entry_name) {

  string s, s2, rs;
  size_t i;

  rs = "";
  s = to_lowercase(entry_name);
  for (auto mit = entries.begin(); mit != entries.end(); ++mit) {
    s2 = to_lowercase(mit->first);
    if (s.find(s2) != string::npos || s2.find(s) != string::npos) rs += "\"" + mit->first + "\", ";
  }

  if (rs != "") {
    rs.pop_back();
    rs.pop_back();
    fprintf(stderr, "Did you mean %s\n", rs.c_str());
  }

}

void Man::remove_entry(const string &entry_name) {


  if (entries.find(entry_name) != entries.end()) {
    get_map_from_file(content, entries[entry_name]);
    content.erase(entry_name);
    write_map_to_file(content, entries[entry_name]);

    entries.erase(entry_name);
    write_map_to_file(entries, home + TABLE_PATH);

    printf("Delete entry %s successfully\n", entry_name.c_str());
  } else {
    similar_entries(entry_name);
    throw SRE("No entry for " + entry_name);
  }
}

void Man::get_entry(const string &entry_name) {
 

  if (entries.find(entry_name) != entries.end()) {
    get_map_from_file(content, entries[entry_name]);
    cout << content[entry_name];
  } else {
    similar_entries(entry_name);
    throw SRE("No entry for " + entry_name);
  }
}


void Man::list_entries() {

  
  if (entries.size() == 0) {
    cout << "Entries are empty" << endl; 
    return;
  }

  for (auto mit = entries.begin(); mit != entries.end(); ++mit) cout << mit->first << endl;
}




void Man::set_entry(const string &entry_name, const string &filename, bool append) {
  string s, line;
  ofstream o;


  if (append && content.find(entry_name) != content.end()) s = content[entry_name];
  else s = "";

  while (getline(cin, line)) s += line + "\n";

  s.pop_back();

  content[entry_name] = s;
  write_map_to_file(content, filename);
}

void Man::add_entry(const string &entry_name, bool append, bool overwrite) {
  string filename;
  string s;
  char c;
  char buf[1024];
  int i;
  struct stat stat_buf;

  s = home + TABLE_PATH;
  
  if (entries.find(entry_name) != entries.end() && !append && !overwrite) {
    sprintf(buf, "Entry %s exists. Use overwrite action to overwrite it", entry_name.c_str());
    throw SRE (buf);
  } 

  i = 0;
  while (1) {
    sprintf(buf, "%s%s/zman_%d.txt", home.c_str(), DIR_NAME, i);
    if (stat(buf, &stat_buf) < 0) break;
    
    if (stat_buf.st_size < MAX_FILE_SIZE) {
      get_map_from_file(content, (string) buf);
      break;
    }
    i++;
  }


  set_entry(entry_name, buf, append);

  if (append == false && overwrite == false) {
    entries[entry_name] = strdup(buf);
    write_map_to_file(entries, s);
  }
  if (append) printf("Append to entry %s successfully\n", entry_name.c_str());
  else if (overwrite) printf("Overwrite entry %s successfully\n", entry_name.c_str());
  else printf("Add new entry %s successfully\n", entry_name.c_str());
}


Man::Man(const char *a, const char *entry_name) {
  string s;

  s = (string) a;

  create_default_file();
  get_map_from_file(entries, home + TABLE_PATH);

  // get_all_entries();

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




void usage() {
  cerr << "zman action [entry_name]" << endl;
  cerr << "Actions:" << endl;
  cerr << "  add:         add a new entry" << endl;
  cerr << "  o/overwrite: overwrite a entry with new content" << endl;
  cerr << "  append:      append the new content to a entry" << endl;
  cerr << "  rm/remove:   remove a entry" << endl;
  cerr << "  l/list:      list all entries" << endl;
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