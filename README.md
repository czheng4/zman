# zman
This is a pared-down version of man to help us store useful commands, notes, etc.
As a Linux programmer, I deal with the terminal on a daily basis. Although `man` command is 
powerful to help us to look up some useful command lines, system calls, etc, it contains too much info and we tend
to get lost there. The goal of `zman` is to help you manage your own `man` page. 


# How to use it
Let's clone this repo and compiler it first.
```
UNIX> git clone https://github.com/czheng4/zman.git
...
UNIX> cd zman
UNIX> make
c++ -O3 zman.cpp -o zman -std=c++11
UNIX> cp zman /usr/local/bin/  # I copy this to /usr/local/bin, which is my gobal path
                               # So, I can use this program everywhere

UNIX> zman
zman action [entry_name]
Actions:
  add_entry :  add_entry a new entry
  o/overwrite: overwrite a entry with new content
  append:      append the new content to a entry
  rm/remove:   remove a entry
  l/list:      list all enties
  g/get:       get the content of an entry

zman reads the content from stdin
```
Let's say I have some notes about git commands I would like to use `zman` to store them.
```
UNIX> cat git_command.txt
git branch -a             /* show the all branch */
git checkout branch_name  /* check out a branch */
git branch -d branch_name /* delete a local branch */

git add [file]   /* add the file */
git commit -m "message"
git push
UNIX> zman add "git commands" < git_commands.txt
Add new entry git commands successfully
UNIX> zman get "git"
Did you mean "git commands"
No entry for git
UNIX> zman get "git commands"
git branch -a             /* show the all branch */
git checkout branch_name  /* check out a branch */
git branch -d branch_name /* delete a local branch */

git add [file]   /* add the file */
git commit -m "message"
git push
```

