import os
os.system("clear")
os.system("./endwin")
cont = True
start = True
while(cont):
    print("Continue? (Y/n)")
    if start is False:
        contintest = input()
    else:
        contintest = "y"
        start = False
    if contintest in ("Y", "y", ""):
        os.system("clear\nrm Solitaire\nclang++ -std=gnu++11 -lncurses main.cpp -o Solitaire\n")
        os.system("rm endwin\nclang++ -std=gnu++11 -lncurses endwin.cpp -o endwin")
        if os.system("./Solitaire") != 0:
            os.system("./endwin")
            print("An error has occured. Most likely a segfault or compilation error.")
            exit()
        input("\nProgram finished. Press enter to exit . . . ")
        os.system("clear")
    elif contintest in ("n", "N"):
        cont = False
