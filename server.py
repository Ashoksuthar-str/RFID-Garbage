from flask import Flask, request, jsonify
import sqlite3
from datetime import datetime

app = Flask(__name__)

DATABASE = "data.db"


# --------------------------------------------------
# DATABASE / TABLES
# --------------------------------------------------

def init_db():

    conn = sqlite3.connect(DATABASE)

    cursor = conn.cursor()

    # Store UID -> user name
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS uid_info (
            uid TEXT PRIMARY KEY,
            name TEXT NOT NULL
        )
    """)

    # Store every waste transaction
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS waste_data (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            uid TEXT NOT NULL,
            name TEXT NOT NULL,
            weight1 REAL NOT NULL,
            weight2 REAL NOT NULL,
            weight3 REAL NOT NULL,
            weight4 REAL NOT NULL,
            time TEXT NOT NULL
        )
    """)

    conn.commit()
    conn.close()


# --------------------------------------------------
# GET REQUEST
# Get all waste data for a UID
# --------------------------------------------------

@app.route("/<uid>", methods=["GET"])
def get_data(uid):

    conn = sqlite3.connect(DATABASE)

    conn.row_factory = sqlite3.Row

    cursor = conn.cursor()

    cursor.execute("""
        SELECT *
        FROM waste_data
        WHERE uid = ?
        ORDER BY id ASC
    """, (uid,))

    rows = cursor.fetchall()

    conn.close()

    data = []

    for row in rows:

        data.append({
            "id": row["id"],
            "uid": row["uid"],
            "name": row["name"],
            "weight1": row["weight1"],
            "weight2": row["weight2"],
            "weight3": row["weight3"],
            "weight4": row["weight4"],
            "time": row["time"]
        })

    return jsonify({
        "error":"None",
        "uid": uid,
        "data": data
    })


# --------------------------------------------------
# POST REQUEST
# Store waste transaction
# --------------------------------------------------

@app.route("/", methods=["POST"])
def save_data():

    data = request.get_json()

    if not data:

        return jsonify({
            "error": "JSON data is required"
        }), 400


    uid = data.get("uid")
    weight1 = data.get("weight1")
    weight2 = data.get("weight2")
    weight3 = data.get("weight3")
    weight4 = data.get("weight4")


    # Check UID

    if not uid:

        return jsonify({
            "error": "UID is required"
        }), 400


    # Check weight

    if weight1 is None:

        return jsonify({
            "error": "Weight is required"
        }), 400


    # Convert weight to number

    try:

        weight1 = float(weight1)
        weight2 = float(weight2)
        weight3 = float(weight3)
        weight4 = float(weight4)

    except (ValueError, TypeError):

        return jsonify({
            "error": "Weight must be a number"
        }), 400


    # --------------------------------------------------
    # Find user's name using UID
    # --------------------------------------------------

    conn = sqlite3.connect(DATABASE)

    cursor = conn.cursor()

    cursor.execute("""
        SELECT name
        FROM uid_info
        WHERE uid = ?
    """, (uid,))

    result = cursor.fetchone()


    # UID doesn't exist

    if result is None:

        conn.close()

        return jsonify({
            "error": "UID is not registered"
        }), 404


    # Get actual name

    name = result[0]


    # --------------------------------------------------
    # Get current time
    # --------------------------------------------------

    current_time = datetime.now().isoformat()


    # --------------------------------------------------
    # Store transaction
    # --------------------------------------------------

    cursor.execute("""
        INSERT INTO waste_data
        (uid, name, weight1, weight2, weight3, weight4, time)
        VALUES (?, ?, ?, ?, ?, ?, ?)
    """, (
        uid,
        name,
        weight1,
        weight2,
        weight3,
        weight4,
        current_time
    ))


    new_id = cursor.lastrowid

    conn.commit()
    conn.close()


    # --------------------------------------------------
    # Response
    # --------------------------------------------------

    return jsonify({
        "error":"None",
        "message": "Data stored successfully",

        "data": {
            "id": new_id,
            "uid": uid,
            "name": name,
            "weight1": weight1,
            "weight2": weight2,
            "weight3": weight3,
            "weight4": weight4,
            "time": current_time
        }

    })


# --------------------------------------------------
# REGISTER UID / NAME
# --------------------------------------------------

@app.route("/name", methods=["POST"])
def add_name():

    data = request.get_json()

    if not data:

        return jsonify({
            "error": "JSON data is required"
        }), 400


    uid = data.get("uid")
    name = data.get("name")


    if not uid:

        return jsonify({
            "error": "UID is required"
        }), 400


    if not name:

        return jsonify({
            "error": "Name is required"
        }), 400


    conn = sqlite3.connect(DATABASE)

    cursor = conn.cursor()


    try:

        cursor.execute("""
            INSERT INTO uid_info
            (uid, name)
            VALUES (?, ?)
        """, (
            uid,
            name
        ))

        conn.commit()

    except sqlite3.IntegrityError:

        conn.close()

        return jsonify({
            "error": "UID already registered"
        }), 409


    conn.close()


    return jsonify({

        "message": "User registered successfully",
        "error":"None",
        "data": {
            "uid": uid,
            "name": name
        }

    })


# --------------------------------------------------
# START SERVER
# --------------------------------------------------

if __name__ == "__main__":

    init_db()

    app.run(
        host="0.0.0.0",
        port=5000,
        debug=True
    )
