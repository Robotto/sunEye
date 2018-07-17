#!/usr/bin/env python

import time
import os
import socket
import subprocess
#from tcp_info import TcpInfo
#from tqdm import tqdm



# import elementtree.ElementTree as ET
import xml.etree.ElementTree as ET


if __name__ == "__main__":

    print time.ctime(), "startup!"

    #TCP_IP = '192.168.0.170'
    TCP_IP = '192.168.0.5'
    #TCP_IP = '192.168.0.110'
    #TCP_IP = '127.0.0.1'
    TCP_PORT = 1338
    BUFFER_SIZE = 1024  # Normally 1024, but we want fast response

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR,1)  # make socket reuseable, for debugging (enables you to rerun the program before the socket has timed out)
    s.bind((TCP_IP, TCP_PORT))
    s.listen(1)

    print 'Done.. Opening TCP port', TCP_PORT

    while True:
        try:
            conn, addr = s.accept()
            print time.ctime(), 'Connection from:', addr

            #print TcpInfo.from_socket(conn).tcpi_bytes_acked

            #            while True:  # looks like connection timeout is ~60 seconds.
            #os.system('./procedure.sh')
            #time.sleep(10)

            p = subprocess.call("./procedure.sh", shell=True)

            #open bmp file, send it byte for byte
            filesize = os.path.getsize('full.bmp')
            print 'filesize: ' + str(filesize) + 'Bytes'
            with open('full.bmp', 'r') as bitmap:
                byte = bitmap.read(1)
                stream = bytearray()
                #byteCounter=0


                while byte != "":
                    stream.append(byte)
                    byte = bitmap.read(1)

                    #if len(stream)==1024 or byte=="":
                    #    conn.send(stream)
                    #    stream = bytearray()
                    #    currentPosition=bitmap.tell()*1.0
                    #    percentage = currentPosition/filesize*100.0
                    #    print '%.2f%%' %percentage

                bitmap.close()
            print 'sending data...'

            #setup progress bar:

            #t = tqdm(total=filesize, desc="Transmitting", unit="B",unit_scale=True, unit_divisor=1024)

            #lastTxBytes=0
            #set non blocking# .
            #conn.setblocking(0)

            conn.send(stream)

            #txBytes=TcpInfo.from_socket(conn).tcpi_bytes_acked

            #while txBytes < filesize:
                #time.sleep(0.1)

            #    txBytes=TcpInfo.from_socket(conn).tcpi_bytes_acked

            #    t.update(txBytes-lastTxBytes)

            #    lastTxBytes=txBytes

                #print txBytes
            #t.close()


                    # Do stuff with byte.
                    #byte = bitmap.read(1)
                    #conn.send(byte)

            print 'TX done.'

            print 'Closing connection'
            conn.shutdown(socket.SHUT_RDWR)
            conn.close()

            print '--------------------------'
            print 


        except Exception as e:
            # print "hmm.. It looks like there was an error: " + str(e)
            print time.ctime(), 'Client disconnected... :', str(e)
            print '--------------------------'
            conn.close()
