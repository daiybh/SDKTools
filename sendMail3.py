

from sendMailBase import sendFolder
import sys,os



def main():
    currDir=os.path.dirname(__file__)
    baseFolder=fr'{currDir}\bin'
    args = sys.argv[1:]
    if len(args)>0:
        baseFolder = args[0]
    print(baseFolder)
    sendFolder(baseFolder,currDir)

if __name__=='__main__':
    main()
