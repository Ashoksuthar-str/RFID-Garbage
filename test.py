import requests
import time

SERVER_URL = "http://127.0.0.1:5000"


while True:

    choice = input(
        "\n"
        "==============================\n"
        "        USER MANAGEMENT\n"
        "==============================\n"
        "  1) See User Data\n"
        "  2) Add Weight Manually\n"
        "  3) Add New User\n"
        "  4) Change Server\n"
        "  5) Exit\n"
        "------------------------------\n"
        "  Enter your option: "
    )

    # --------------------------------
    # SEE USER DATA
    # --------------------------------
    if choice == "1":

        print("\n------------------------------")
        print("         USER DATA")
        print("------------------------------")

        
        uid = input("  Enter UID: ")
        try:
            req = requests.get(SERVER_URL + "/" + uid)
            data = req.json()

            print("\n------------------------------")
            print(f"         USER: {uid}")
            print("------------------------------")

            total_weight = 0

            for user in data["data"]:
                print(f"  ID      : {user['id']}")
                print(f"  Name    : {user['name']}")
                print(f"  Weight1  : {user['weight1']} g")
                print(f"  Weight2  : {user['weight2']} g")
                print(f"  Weight3  : {user['weight3']} g")
                print(f"  Weight4  : {user['weight4']} g")
                print(f"  Time    : {user['time']}")
                print("  ----------------------------")
                
                for i in range(4):
                    total_weight += user[f"weight{i+1}"]

            print(f"  TOTAL WEIGHT : {total_weight} Kg")
            print("------------------------------")
            time.sleep(1)
            print("\n  ✓ Data printed successfully.")
            time.sleep(1)
        except:
            time.sleep(1)
            print("Error Getting user data")
            time.sleep(1)



    # --------------------------------
    # ADD  WEIGHT
    # --------------------------------
    elif choice == "2":

        print("\n------------------------------")
        print("          ADD WEIGHT")
        print("------------------------------")

        uid = input("  Enter UID: ")
        weight1 = input("  Enter Weight (Kg): ")
        weight2 = input("  Enter Weight (Kg): ")
        weight3 = input("  Enter Weight (Kg): ")
        weight4 = input("  Enter Weight (Kg): ")

        data = {
            "uid": uid,
            "weight1": weight1,
            "weight2": weight2,
            "weight3": weight3,
            "weight4": weight4
        }
        try:
            req = requests.post(SERVER_URL, json=data)
            error = req.json()["error"]

            if error == "None":
                time.sleep(1)
                print("\n  ✓ Weight added successfully.")
                time.sleep(1)

            else:
                time.sleep(1)
                print(f"\n  ✗ Error: {error}")
                time.sleep(1)
        except:
            time.sleep(1)
            print("Error Connecting to Server")
            time.sleep(1)



    # --------------------------------
    # ADD NEW USER
    # --------------------------------
    elif choice == "3":
        
        print("\n------------------------------")
        print("       MANUAL WEIGHT")
        print("------------------------------")

        uid = input("  Enter UID: ")
        name = input("  Enter Name: ")
        
        uid_info = {
            "uid": uid,
            "name": name,
        }
        try:
            req = requests.post(
                SERVER_URL + "/name",
                json=uid_info
            )
            try:
                error = req.json()["error"]

                if error == "None":
                    time.sleep(1)
                    print("\n  ✓ User added successfully.")
                    time.sleep(1)
                else:
                    time.sleep(1)
                    print(f"\n  ✗ Error: {error}")
                    time.sleep(1)

            except:
                time.sleep(1)
                print(f"\n  Response: {req.text}")
                time.sleep(1)

            print("------------------------------")
        except:
            time.sleep(1)
            print("Error Connecting to Server")
            time.sleep(1)

    # --------------------------------
    # CHANGE SERVER
    # --------------------------------
    elif choice == "4":

        print("\n------------------------------")
        print("        CHANGE SERVER")
        print("------------------------------")

        time.sleep(1)
        print(f"  Current Server: {SERVER_URL}")
        time.sleep(1)

        new_server = input("\n  Enter New Server URL: ").strip()

        time.sleep(1)
        if new_server:
            SERVER_URL = new_server.rstrip("/")

            print("\n  ✓ Server URL changed successfully.")
            print(f"  New Server: {SERVER_URL}")

        else:
            print("\n  ✗ Server URL cannot be empty.")

        time.sleep(1)
        print("------------------------------")
        time.sleep(1)


    # --------------------------------
    # EXIT
    # --------------------------------
    elif choice == "5":

        time.sleep(1)
        print("\n==============================")
        print("     Exiting User Management")
        print("==============================\n")
        time.sleep(1)

        break


    # --------------------------------
    # INVALID OPTION
    # --------------------------------
    else:

        time.sleep(1)
        print("\n  ✗ Invalid option.")
        print("  Please select 1, 2, 3, or 4.")
        time.sleep(1)