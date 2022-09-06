count=0
f = open("/home/zpw/HME-Quartz-broadwell-master/dataset/soc-LiveJournal1.txt","r")
for line in f.readlines():
    count=count+1
    if count==0:
        print("one")
        print(line)
    elif count==1:
        print("two")
        print(line)
    elif count==2:
        print("three")
        print(line)
    elif count in [3,4,5,6]:
        print("four")
        print(line)

print(count)


