import re
def addButt(title, top, msg):
    return {'type': 'button', 'title': title, 'publish_topic': top, 'publish_payload': msg}


def addText(title, top):
    return {'type': 'text', 'title': title, 'subscribe_topic': top}

res=""
stuck=True

while(stuck):
    
    print()
    print("[B]utton")
    print("[T]ext")
    print("[D]one")
    inp=input()

    if inp.lower()=="b":
        tit=input("Title: ")
        top=input("Topic: ")
        msg=input("Payload: ")
        res+=str(addButt(tit,top,msg))
        inp=""
    elif inp.lower()=="t":
            tit=input("Title: ")
            top=input("Topic: ")
            res+=str(addText(tit,top))
            inp=""
    elif inp.lower()=="d":
        stuck=False
        res=re.sub("\'","\"",res)
        res=re.sub("}{","},{",res)
        res="["+res+"]"
        print(res)
    inp=""

inp=input("Print to file? (y/n) ")
if inp.lower()=="y" or inp.lower()=='yes':
    outfile=open("tiles.json","w")
    outfile.write(res)
    outfile.close()

input("ENTER to quit")
