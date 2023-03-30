from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.application import MIMEApplication

import smtplib


import Emailconfig


import os,io
import zipfile
import socket

from git import Repo
def sendMail_Zip(z,projectMsg,projectDir='./'):
    
    print("send starting....")
    # 输入Email地址和口令:
    from_addr = Emailconfig.from_addr
    password = Emailconfig.password
    # 输入收件人地址:
    to_addr = Emailconfig.to_addr
    # 输入SMTP服务器地址:
    smtp_server = Emailconfig.smtp_server

    import time
		
    repo=Repo(projectDir)
    headcommit = repo.head.commit

    mailText='url:{0}  \nbranch:{1}\nhash:{2}\nmessage:{3}'.format(repo.remotes.origin.url,repo.head.reference,repo.head.commit,headcommit.message)
	
    content='proect:\n{0}\n\n{1} \n\n\n\n{2}'.format(projectMsg,mailText,time.asctime())
	
    textApart = MIMEText(content)
	
    zipApart=MIMEApplication(z.getvalue())
    zip_name= str(repo.head.reference)
    zipApart.add_header('Content-Disposition','attachment',filename=zip_name+'.zip')

    m= MIMEMultipart()

    m.attach(textApart)
    m.attach(zipApart)
    m['Subject']='proect :{0} {1}'.format(headcommit.message,zip_name)

    server = smtplib.SMTP(smtp_server, 25) # SMTP协议默认端口是25
    #server.set_debuglevel(1)
    server.login(from_addr, password)
    server.sendmail(from_addr, to_addr.split(','), m.as_string())
    server.quit()
    print("send over")


def sendMail_text(title,textContent):
    # 输入Email地址和口令:
    from_addr = Emailconfig.from_addr
    password = Emailconfig.password
    # 输入收件人地址:
    to_addr = Emailconfig.to_addr
    # 输入SMTP服务器地址:
    smtp_server = Emailconfig.smtp_server

    textApart = MIMEText(textContent)
    m= MIMEMultipart()

    m.attach(textApart)
    m['Subject']='revert '+title
    
    server = smtplib.SMTP(smtp_server, 25) # SMTP协议默认端口是25
    #server.set_debuglevel(1)
    server.login(from_addr, password)
    server.sendmail(from_addr, to_addr.split(','), m.as_string())
    server.quit()


def sendFolder(folderPath,projectDir):
    if os.path.exists(folderPath)==False:
        print(F"{folderPath}  don't exists")
        return
    print(F"{folderPath}  exists")
    ziplist=[]

    for dir,subdirs,files in os.walk(folderPath):
        if "logs" in subdirs:
            subdirs.remove("logs")
        if ".vs" in subdirs:
            subdirs.remove(".vs")
        
        if "config.ini" in files:
            files.remove("config.ini")
            
        for fileItem in files:
            ziplist.append(os.path.join(dir,fileItem))
        for dirItem in subdirs:
            ziplist.append(os.path.join(dir,dirItem))
    
    print(ziplist)
    
    in_memory_zip = io.BytesIO()

    #in_memory_zip="c:\\logs\\aa.zip"
    #zipfile.ZipFile.setpassword(pwd=b"147258369")
    z = zipfile.ZipFile(in_memory_zip,'w',zipfile.ZIP_DEFLATED)
    z.setpassword(b'123456')
    for i in ziplist:
        z.write(i,i.replace(folderPath,''))
    z.close()

    hostName=socket.gethostname()
    projectMsg=f"codesPath:{projectDir} \n codePC:{hostName} {socket.gethostbyname(hostName)}"
    projectMsg+=f'\n zip password : 123456'
    sendMail_Zip(in_memory_zip,projectMsg=projectMsg,projectDir=projectDir)
