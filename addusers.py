import requests

SERVER_URL = "http://172.20.10.4:5000"

uid = input("Enter UID : ")
name = input("Enter NAME : ")
uid_info = {
    "uid":uid,
    "name":name,
}
req = requests.post(SERVER_URL+"/name",json=uid_info)
print(req.text)
print("Completed!")