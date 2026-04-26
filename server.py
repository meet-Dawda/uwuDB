from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
import hashlib

import uwudb 

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

# Instantiate the C++ B+ Tree in RAM (order m=5)
db = uwudb.Tree(5)

# We need an integer key counter since our B+ Tree uses int keys
current_id_counter = 1 

# Pydantic models for JSON parsing
class UserAuth(BaseModel):
    username: str
    password: str

def hash_password(password: str) -> str:
    return hashlib.sha256(password.encode()).hexdigest()
@app.post("/register")
def register_user(user: UserAuth):
    global current_id_counter
    
    new_row = uwudb.Row()
    
    new_row.columns = [user.username, hash_password(user.password)]
    
    user_id = current_id_counter
    db.insert(user_id, new_row)
    
    current_id_counter += 1
    
    print(f"--- Tree State after registering {user.username} ---")
    db.display() 
    
    return {"status": "success", "user_id": user_id, "message": "User registered in C++ DB"}
@app.post("/login")
def login_user(user_id: int, user: UserAuth):
    try:
        # Search the C++ tree in O(log n) time
        db_row = db.dataSearch(user_id)
        
        # Columns[0] is username, Columns[1] is hashed password
        stored_username = db_row.columns[0]
        stored_password_hash = db_row.columns[1]
        
        if stored_username == user.username and stored_password_hash == hash_password(user.password):
            return {"status": "success", "message": "Login successful!"}
        else:
            raise HTTPException(status_code=401, detail="Invalid credentials")
            
    except Exception as e:
        raise HTTPException(status_code=404, detail=str(e))