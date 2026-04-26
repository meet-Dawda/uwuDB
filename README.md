# UwU DB
\>A simplified database engine. \
\>I had my DBMS exam and was supposed to learn B+ trees \
\>So i thought of implementing one myself
B-tree to B+tree  
\>then simple data storage ops 
glued to the python pybind11 to use with FastAPI \
\>viola custom database 
lol


### How to get the ./uwudb.(that uglyahh extention) 
```
c++ -O3 -Wall -shared -std=c++17 -fPIC $(python3 -m pybind11 --includes) bindings.cpp -o uwudb$(python3-config --extension-suffix)
```

### How to use 
1. initiate the databse with m being the order
```
    db = uwudb.Tree(m:int)
``` 
2. Start with hardcoded key value (gotta tell python that this is an integer)
```
    current_id_counter = 1 
```
3. get the row by
```
    new_row = uwudb.Row()
```
4. insertion 
```
    db.insert(id, row) #bear for now
```
5. searching for specific key
```
        db_row = db.dataSearch(user_id)
```
6. nuff said


\> Thinkng of adding SQL parsing for this 🤔 \
\> LMK twinn  
\> VENV AND OTHER FILES GOT UPLOADED (ACCIDENT) 😭

Byeee! 
UwUDB
