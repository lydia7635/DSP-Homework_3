import sys
import re

inFile = open(sys.argv[1], 'r', encoding = "big5-hkscs")

outDict = {}

for line in inFile.readlines():
    line = line.strip()
    str = re.split('/| ', line)

    for dictKey in str:
        try:
            outDict[dictKey[0]].add(str[0])
        except KeyError:
            outDict[dictKey[0]] = {str[0]}
    
inFile.close()

sortedKey = sorted(outDict)

outFile = open(sys.argv[2], 'w', encoding = "big5-hkscs")

for key in sortedKey:
    outFile.write(key)
    for val in outDict[key]:
        outFile.write(" %s" % val)
    outFile.write("\n")

outFile.close()
