import sys

toStoreAllNumbers = []
count = 0
for eachLine in sys.stdin:
    presentNumber = int(eachLine.strip())
    toStoreAllNumbers.append(presentNumber)
    if(presentNumber>555555 and presentNumber<777777):
        count = count+1;

print('Count of numbers with in the range: ' +str(count))
print('Total count of the numbers: '+str(len(toStoreAllNumbers)))