import requests


SERVER_URL = "http://172.20.10.2:5000"

while True:
    choice = input("1-Get , 2-Post : ")
    if choice == "1":
        uid = input("Enter UID : ")
        req = requests.get(SERVER_URL+"/"+uid)
        data = req.text
        print(data)
    elif choice == "2":
        uid = input("Enter UID : ")
        weight = input("Enter WEIGHT : ")
        data = {
            "uid":uid,
            "weight":weight
        }
        req = requests.post(SERVER_URL,json=data)
        error = req.json()["error"]
        if error == "None":
            print("Done")
        else:
            print(error)
        if error == "UID is not registered":
            uid = input("Enter UID : ")
            name = input("Enter NAME : ")
            uid_info = {
                "uid":uid,
                "name":name,
            }
            req = requests.post(SERVER_URL+"/name",json=uid_info)
            print(req.text)
