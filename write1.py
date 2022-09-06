with open('/home/zpw/HME-Quartz-broadwell-master/data/soc-LiveJournal1.txt', 'r+') as f:
    content = f.read()        
    f.seek(0, 0)
    f.write('4308451 68993773\n'+content)
with open('/home/zpw/HME-Quartz-broadwell-master/data/soc-LiveJournal1.txt', 'r+') as f:
    content = f.read()        
    f.seek(0, 0)
    f.write('AdjacencyGraph\n'+content)