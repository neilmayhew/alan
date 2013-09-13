#!/usr/bin/env python
from sys import argv, exit
from subprocess import check_call, call, Popen, PIPE

if len(argv) == 1 :
    print argv[0] + " - a program to extract location graph data from an Alan game"
    print
    print "Usage: " + argv[0] + " <alan source file>"
    exit()

script, filename = argv

process = Popen(["alan", "-dump", "ci", filename], stdout=PIPE)

output = process.communicate(None)[0]
lines = ''.join(output).split("\n")

i = iter(lines)
line = i.next()
message = line

l = 0
try :
    while line.find("ADV:") == -1 :
        line = i.next()
        message += "\n" + line

except :
    print message

print 'digraph finite_state_machine {'
print '  rankdir=LR;'
print '  node [shape=octagon;style=filled;]'

# Mapping from possible exit directions to DOT port directions
portname = {"n":":n", "north":":n",
            "s":":s", "south":":s",
            "e":":e", "east":":e",
            "w":":w", "west":":w",
            "se":":se", "southeast":":se",
            "sw":":sw", "southwest":":sw",
            "ne":":ne", "northeast":":ne",
            "nw":":nw", "northwest":":nw"
}

try :
    while 1 :
        while line.find("INS:") == -1 :
            line = i.next()

        while line.find("id:") == -1 :
            line = i.next()
        idline = line
        line = i.next()

        # Found an instance. Is it a direct decendant of "location"?
        # We can't handle indirect decendants yet
        while line.find("parentId:") == -1 :
            line = i.next()
        if line.find("location") == -1 :
            continue;

        # Yes, that was a location. What's it's name?
        # 8th field and then remove the part after colon
        id = idline.split()[7].split(":")[0].strip('"')
        print '  ' + id + '[label="' + id + '"];'

        # Is there an exit?
        line = i.next()
        original = line
        while 1 :
            while line.find("DIRECTION") == -1 and line.find("INS:") == -1 :
                line = i.next()
            if line.find("INS:") != -1 :
                # No, found next instancs, so start over
                break

            directions = []

            # Yes, so find the direction(s)
            # 9th field and then remove the part after colon
            while line.find("DIRECTION") != -1 :
                direction = line.split()[11].split(":")[0].strip('"')
                directions.append(direction)
                line = i.next()

            # So, to where does it lead?
            # line = i.next()
            target = line.strip("\n\r")
            line = i.next()

            # Did that line split?
            if line[0] != '.' :
                # Yes, so paste them together again
                target = target + line.strip()
            target = target.split()[11].strip().split(":")[0].strip('"')
            
            # Print all directions
            for direction in directions :
                try :
                    port = portname[direction]
                except KeyError :
                    port = ""
                print "  " + id + port + " -> " + target + " [label=" + direction + "];"

except StopIteration as x :
    print "}"
    pass

